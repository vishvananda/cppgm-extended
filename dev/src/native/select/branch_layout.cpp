#include "native/select/block_placement.h"
#include "native/select/function_selector.h"
#include "native/select/mir_cleanup.h"
#include "native/select/shrink_wrap.h"

// Branch layout (O1 and above): the MIR states the branches its encoding
// executes, so the dump's jumps and blocks are the program's. A block
// holding only a jump is threaded away: every reference to it (branches,
// handler records, host landings) names the end of its jump chain, and
// the block is removed (a chain entering a cycle is kept; the entry block
// always is). In the final order a jump to the next block is dropped, a
// conditional branch to the next block followed by a jump becomes one
// inverted conditional branch, and a block's final conditional branch to
// the next block is dropped. A block may then fall through to the next,
// even into a kept jump-only block of a cycle, so the emitter encodes
// laid-out MIR without its own threading (x86/emitter.cpp); its
// fall-through rules find nothing left to do.
//
// One walk per block for the chains (each block is resolved once), one
// pass over the kept blocks' instructions retargeting the branches and
// handler records, the only instructions with labels (and the jump
// tables' entries): linear in the
// function's MIR. Instructions stay in place; a kept block's range shrinks
// past its dropped branches, and removed blocks' jumps lie outside every
// range (moving them cost 3% of -O1 time on a 4M-instruction function).
// Return tails also shed consecutive register copies after duplication;
// their last independent assignment can produce the ABI return register.
namespace native {

using lowir::kNoId;

namespace {

const uint32_t kUnresolved = 0xffffffffu;
const uint32_t kWalking = 0xfffffffeu;
const uint32_t kNoChainEnd = 0xfffffffdu;
// A tail of at most kShortTail instructions that the block before it runs
// into stays a copy of its own where it runs at least as often as the
// function's entry (share_small_tails).
const uint32_t kShortTail = 5;

bool branch_to(const Inst& inst, Op op, uint32_t block)
{
	return inst.op == op && inst.ops[0].kind == OperandKind::Label && inst.ops[0].symbol == block;
}

// Bounded physical-register tails can share code without SSA repair: every
// entering path already supplies exactly the registers that code reads.
// Calls retain their exact target, ABI facts and physical operands. Assembly,
// EH and implicit fallthrough stay outside the candidate set.
bool shared_tail(const Function& f, uint32_t b)
{
	const Block& block = f.blocks[b];
	if (b == 0 || block.landing || block.count < 2)
		return false;
	unsigned count = 0;
	Op last = Op::Nop;
	for (uint32_t i = block.first; i < block.first + block.count; ++i)
	{
		if (f.insts[i].op == Op::Nop)
			continue;
		if (++count > 64)
			return false;
		last = f.insts[i].op;
		switch (f.insts[i].op)
		{
		case Op::Mov:
		case Op::Load:
		case Op::Store:
		case Op::Lea:
		case Op::Add:
		case Op::Sub:
		case Op::And:
		case Op::Or:
		case Op::Xor:
		case Op::Shl:
		case Op::Shr:
		case Op::Sar:
		case Op::Cmp:
		case Op::Test:
		case Op::Setcc:
		case Op::Movzx:
		case Op::Sext:
		case Op::Zext:
		case Op::Jcc:
		case Op::Jmp:
		case Op::Ret:
		case Op::Call:
		case Op::Imul:
		case Op::Neg:
		case Op::Not:
		case Op::Cqo:
		case Op::Idiv:
		case Op::Div:
		case Op::Bsf:
		case Op::Bsr:
		case Op::Popcnt:
		case Op::Rol:
		case Op::Ror:
		case Op::Cmov:
			break;
		default:
			return false;
		}
	}
	return count >= 2 && (last == Op::Jmp || last == Op::Ret);
}

Operand shared_operand(Operand op, const std::vector<uint32_t>& target)
{
	if (op.kind == OperandKind::Label)
		op.symbol = target[op.symbol];
	return op;
}

uint64_t tail_hash(const Function& f, const Block& block, const std::vector<uint32_t>& target)
{
	uint64_t hash = 0;
	auto add = [&](uint64_t value) { hash = (hash ^ value) * 1099511628211ull; };
	for (uint32_t i = block.first; i < block.first + block.count; ++i)
	{
		const Inst& inst = f.insts[i];
		if (inst.op == Op::Nop)
			continue;
		add(static_cast<uint64_t>(inst.op));
		add(static_cast<uint64_t>(inst.type));
		add(static_cast<uint64_t>(inst.type2));
		add(inst.sub);
		add(inst.flags);
		add(inst.aux);
		add(inst.aux2);
		add(inst.eh);
		add(inst.nops);
		for (unsigned k = 0; k < inst.nops; ++k)
		{
			const Operand op = shared_operand(inst.ops[k], target);
			add(static_cast<uint64_t>(op.kind));
			add(static_cast<uint64_t>(op.access));
			switch (op.kind)
			{
			case OperandKind::None:
				break;
			case OperandKind::Reg:
				add(op.reg);
				break;
			case OperandKind::Imm:
			case OperandKind::FImm:
				add(static_cast<uint64_t>(op.type));
				add(static_cast<uint64_t>(op.imm));
				add(static_cast<uint64_t>(op.imm_high));
				break;
			case OperandKind::Mem:
				add(op.reg);
				add(op.index);
				add(op.scale);
				[[fallthrough]];
			case OperandKind::Symbol:
				add(static_cast<uint32_t>(op.disp));
				[[fallthrough]];
			case OperandKind::Label:
				add(op.symbol);
				break;
			}
		}
	}
	return hash;
}

bool same_tail(const Function& f, const Block& a, const Block& b, const std::vector<uint32_t>& target)
{
	uint32_t i = a.first, j = b.first;
	for (;;)
	{
		while (i < a.first + a.count && f.insts[i].op == Op::Nop)
			++i;
		while (j < b.first + b.count && f.insts[j].op == Op::Nop)
			++j;
		if (i == a.first + a.count || j == b.first + b.count)
			return i == a.first + a.count && j == b.first + b.count;
		const Inst& x = f.insts[i++];
		const Inst& y = f.insts[j++];
		if (x.op != y.op || x.type != y.type || x.type2 != y.type2 || x.sub != y.sub || x.flags != y.flags ||
			x.aux != y.aux || x.aux2 != y.aux2 || x.eh != y.eh || x.nops != y.nops)
			return false;
		for (unsigned n = 0; n < x.nops; ++n)
			if (x.ops[n].access != y.ops[n].access ||
				shared_operand(x.ops[n], target) != shared_operand(y.ops[n], target))
				return false;
	}
}

void share_small_tails(
	Function& f, std::vector<uint32_t>& end, const std::vector<uint8_t>& live, const std::vector<uint64_t>& frequency)
{
	if (f.blocks.size() < 8 || !f.eh.landings.empty() || f.eh_records_pushed || !f.code_labels.empty() || f.assembly)
		return;
	const uint32_t n = static_cast<uint32_t>(f.blocks.size());
	std::vector<uint32_t> target(n);
	for (uint32_t b = 0; b < n; ++b)
		target[b] = end[b] == kNoChainEnd ? b : end[b];
	FlatMap<uint64_t, uint32_t, WordKeyHash> tails;
	tails.clear(n);
	bool changed = false;
	// Whether `b` is short, runs at least as often as the entry (placement's
	// estimate), and the block laid out before it runs into it: as a jump to
	// its twin, that path would take a branch to save a few bytes. (A block
	// only jumps reach is retargeted at no cost.)
	auto short_and_entered = [&](uint32_t b) {
		const Block& block = f.blocks[b];
		if (frequency.empty() || frequency[b] < frequency[0])
			return false;
		uint32_t count = 0;
		for (uint32_t i = block.first; i < block.first + block.count; ++i)
			count += f.insts[i].op != Op::Nop ? 1 : 0;
		if (count > kShortTail)
			return false;
		uint32_t p = b;
		while (p-- > 0 && (!live[p] || target[p] != p))
		{
		}
		if (p >= b || f.blocks[p].count == 0)
			return false;
		const Block& before = f.blocks[p];
		for (uint32_t i = before.first + before.count; i-- > before.first && i + 2 >= before.first + before.count;)
		{
			const Inst& inst = f.insts[i];
			if ((inst.op == Op::Jmp || inst.op == Op::Jcc) && inst.ops[0].kind == OperandKind::Label &&
				target[inst.ops[0].symbol] == b)
				return true;
			if (inst.op != Op::Jmp)
				break;
		}
		return false;
	};
	// Forward tails are visited first. A collision is fully compared; a
	// different tail with that hash simply stays separate.
	for (uint32_t b = n; b-- > 1;)
	{
		if (!live[b] || target[b] != b || !shared_tail(f, b))
			continue;
		const uint64_t hash = tail_hash(f, f.blocks[b], target);
		const uint32_t* previous = tails.find(hash);
		if (previous != nullptr && same_tail(f, f.blocks[b], f.blocks[*previous], target))
		{
			if (short_and_entered(b))
				continue;
			target[b] = *previous;
			changed = true;
		}
		else if (previous == nullptr)
			tails.insert(hash, b);
	}
	if (!changed)
		return;
	// A jump chain may end at a tail just shared with an already visited
	// canonical tail. Every such canonical tail maps directly to itself.
	for (uint32_t b = 0; b < n; ++b)
		target[b] = target[target[b]];
	for (uint32_t b = 0; b < n; ++b)
	{
		Block& block = f.blocks[b];
		if (target[b] != b)
		{
			Inst jump;
			jump.op = Op::Jmp;
			jump.nops = 1;
			jump.ops[0] = Operand::make_label(target[b]);
			f.insts[block.first] = jump;
			block.count = 1;
		}
		else
			for (uint32_t i = block.first; i < block.first + block.count; ++i)
			{
				Inst& inst = f.insts[i];
				for (unsigned k = 0; k < inst.nops; ++k)
					inst.ops[k] = shared_operand(inst.ops[k], target);
				if (inst.op == Op::WideCmpBranch)
					inst.aux = target[inst.aux];
			}
	}
	for (uint32_t& b : f.jump_targets)
		b = target[b];
	for (uint32_t b = 0; b < n; ++b)
		if (end[b] != kNoChainEnd)
			end[b] = target[b];
}

// O2+, host model, laid-out MIR: a call the function returns from at once
// (a `ret` after it in its block, or first in the block it falls into)
// jumps to its callee past the epilogue (Op::TailJump) when the callee
// cannot reach the frame (the function forms no frame address and passes
// no stack argument), no landing catches it and no strlen prefix precedes
// it. The returned registers are the callee's either way.
uint32_t form_tail_jumps(Function& f)
{
	// A wrapper with a stronger preservation convention cannot tail-call
	// an ordinary System V function after restoring its saved registers.
	if (f.variadic || f.eh_records_pushed || (f.preserve_mask & kCallerSavedGprs) != 0)
		return 0;
	for (const Inst& inst : f.insts)
		for (unsigned k = 0; k < inst.nops && inst.op != Op::Nop; ++k)
		{
			const Operand& op = inst.ops[k];
			if ((inst.op == Op::Lea && op.kind == OperandKind::Mem &&
					(op.reg == kFrame || op.index == kFrame || op.reg == RSP || op.index == RSP)) ||
				(inst.op != Op::Ret && op.kind == OperandKind::Reg && op.reg == RSP))
				return 0;
		}
	uint32_t formed = 0;
	for (uint32_t b = 0; b < f.blocks.size(); ++b)
	{
		Block& block = f.blocks[b];
		if (block.count == 0)
			continue;
		const uint32_t last = block.first + block.count - 1;
		uint32_t call = lowir::kNoId, ret = lowir::kNoId;
		if (block.count >= 2 && f.insts[last].op == Op::Ret && f.insts[last - 1].op == Op::Call)
		{
			call = last - 1;
			ret = last;
		}
		else if (f.insts[last].op == Op::Call && b + 1 < f.blocks.size() && f.blocks[b + 1].count != 0 &&
			f.insts[f.blocks[b + 1].first].op == Op::Ret)
		{
			call = last;
			ret = f.blocks[b + 1].first;
		}
		if (call == lowir::kNoId)
			continue;
		const Inst& r = f.insts[ret];
		Inst& c = f.insts[call];
		if ((r.flags & MF_FRAMELESS) ||
			(r.nops != 0 &&
				!(r.nops == 1 && r.ops[0].kind == OperandKind::Reg && (r.ops[0].reg == RAX || r.ops[0].reg == XMM0))) ||
			c.eh != 0 || c.aux2 != 0 || (c.flags & (MF_STRLEN_PREFIX | MF_NORETURN)))
			continue;
		c.op = Op::TailJump;
		if (ret == last)
			--block.count;
		++formed;
	}
	return formed;
}

} // namespace

namespace {

// At most eight tail instructions; no call, two-address operation or
// frame/callee-saved destination is crossed or retargeted. The scalar return
// observes every surviving register read before its epilogue.
void coalesce_return(Function& f, Block& block, bool object_in_rax)
{
	uint32_t at = block.first + block.count - 1;
	Inst result = f.insts[at];
	if (object_in_rax && result.op == Op::Ret && result.nops == 0)
	{
		result.nops = 1;
		result.type = lowir::TypeKind::I64;
		result.ops[0] = Operand::make_reg(RAX);
	}

	auto gpr = [](const Operand& op) { return op.kind == OperandKind::Reg && op.reg < 16; };
	auto wide = [](lowir::TypeKind k) {
		return k == lowir::TypeKind::Void || k == lowir::TypeKind::Ptr || k == lowir::TypeKind::I64;
	};
	if (result.op != Op::Ret || result.nops != 1 || !gpr(result.ops[0]) || !wide(result.type))
		return;
	for (uint32_t work = 0; at > block.first && work < 8; ++work)
	{
		Inst& prior = f.insts[--at];
		if (prior.op == Op::Nop)
			continue;
		const Reg dst = result.ops[0].reg;
		if (prior.nops != 2 || !gpr(prior.ops[0]) || prior.ops[0].reg != dst || !(kCallerSavedGprs & (1u << dst)))
			break;
		if (prior.op == Op::Mov && wide(prior.type) && gpr(prior.ops[1]))
		{
			result.ops[0] = prior.ops[1];
			prior.op = Op::Nop;
			prior.nops = 0;
			prior.ops[0] = prior.ops[1] = Operand();
			continue;
		}
		const bool independent = prior.op == Op::Lea || prior.op == Op::Load || prior.op == Op::Sext ||
			prior.op == Op::Zext || (prior.op == Op::Mov && wide(prior.type) && prior.ops[1].kind == OperandKind::Imm);
		if (dst != RAX && independent)
			prior.ops[0] = result.ops[0] = Operand::make_reg(RAX);
		break;
	}
	uint32_t end = block.first + block.count - 1;
	while (end > block.first && f.insts[end - 1].op == Op::Nop)
		--end;
	f.insts[end] = result;
	block.count = end + 1 - block.first;
}

} // namespace

namespace {

// Whether a block ending `jcc A; jmp B` jumps back to a block at or before
// itself while A lies outside the blocks from B to it: the jump closes a
// loop the conditional branch leaves, so the two change places (the branch
// then repeats the loop and the jump runs once, on the way out). `index` is
// the block's place among the `kept` blocks, whose numbers the labels hold.
bool closes_range(const Function& f, const Block& block, uint32_t index, uint32_t kept, bool addressed)
{
	if (addressed || block.count < 2)
		return false;
	const Inst& jcc = f.insts[block.first + block.count - 2];
	const Inst& jump = f.insts[block.first + block.count - 1];
	if (jcc.op != Op::Jcc || jump.op != Op::Jmp || jcc.ops[0].kind != OperandKind::Label ||
		jump.ops[0].kind != OperandKind::Label)
		return false;
	const uint32_t a = jcc.ops[0].symbol, b = jump.ops[0].symbol;
	return a < kept && b <= index && (a < b || a > index);
}

} // namespace

uint32_t FunctionSelector::layout_branches()
{
	const bool addressed = !out_->code_labels.empty();
	const TailCopies copies = addressed || out_->cold ? TailCopies{} : duplicate_short_tails(*out_);
	stats_.rotated_loops += copies.rotations;
	stats_.dispatch_tails += copies.dispatches;
	const bool copied = copies.rotations != 0 || copies.dispatches != 0;
	if (!addressed)
	{
		thread_decided_tests(*out_, program_);
		place_blocks(*out_);
	}
	else
		place_cold_blocks(*out_);
	// A spill moved into a prologue block may meet its reload there, and a
	// loop's test left only on its entry path (its copy closes the loop)
	// may read a home the entry's block still holds in a register. A test
	// copied to a back edge may follow the instruction that set its flags.
	if ((!addressed && shrink_wrap(*out_)) || copied)
		forward_home_reloads(*out_);
	// Keep compare-shaped loop tests through rotation and placement. Once
	// those choices are settled, signed comparisons against zero can read
	// arithmetic's sign bit directly. Do this before compare propagation,
	// which must not export a removed TEST's cleared-overflow promise.
	drop_redundant_tests(*out_, true);
	if (!addressed)
	{
		drop_guarded_integer_extensions(*out_);
		drop_repeated_compares(*out_);
	}
	Function& f = *out_;
	const uint32_t blocks = static_cast<uint32_t>(f.blocks.size());
	std::vector<uint8_t> entries(addressed ? blocks : 0, 0);
	for (const lowir::CodeLabel& label : f.code_labels)
		entries[label.block] = 1;
	auto jump_only = [&](uint32_t b) {
		const Block& block = f.blocks[b];
		return b != 0 && (!addressed || !entries[b]) && block.count == 1 && f.insts[block.first].op == Op::Jmp;
	};
	// A jump to a block that only returns (through jump-only blocks)
	// returns itself when the return is all its epilogue (a frameless
	// return, or no saves and no frame): the return is copied over the jump.
	const bool object_in_rax = signature_->result.is_object() && entry_layout_.result == ArgClass::Mixed &&
		entry_layout_.result_parts == 1 && entry_layout_.result_part[0].reg == RAX;
	const bool bare_returns = f.preserve_mask == 0 && f.stack_size == 0;
	for (uint32_t b = 0; b < blocks; ++b)
	{
		Block& block = f.blocks[b];
		if (block.count == 0)
			continue;
		Inst& last = f.insts[block.first + block.count - 1];
		if (last.op == Op::Ret)
		{
			coalesce_return(f, block, object_in_rax);
			continue;
		}
		if (last.op != Op::Jmp || last.ops[0].kind != OperandKind::Label)
			continue;
		uint32_t t = last.ops[0].symbol;
		for (unsigned step = 0; step < 4 && t < blocks && jump_only(t); ++step)
			t = f.insts[f.blocks[t].first].ops[0].symbol;
		if (t < blocks && t != b && f.blocks[t].count == 1 && f.insts[f.blocks[t].first].op == Op::Ret &&
			(bare_returns || (f.insts[f.blocks[t].first].flags & MF_FRAMELESS) != 0))
		{
			last = f.insts[f.blocks[t].first];
			coalesce_return(f, block, object_in_rax);
		}
	}
	// Host objects: the blocks the entry and the landing pads reach (a
	// return copied over the jump to a returning block, or a test the
	// threading decided, leaves blocks nothing reaches); the others are not
	// laid out. A block whose last instruction may fall through reaches the
	// next.
	std::vector<uint8_t>& live = layout_live_;
	live.assign(blocks, eh_model_ == EhModel::Host ? 0 : 1);
	if (eh_model_ == EhModel::Host)
	{
		std::vector<uint32_t>& work = layout_chain_;
		work.clear();
		auto reach = [&](uint32_t b) {
			if (b < blocks && !live[b])
			{
				live[b] = 1;
				work.push_back(b);
			}
		};
		reach(0);
		for (const lowir::CodeLabel& label : f.code_labels)
			reach(label.block);
		for (uint32_t b = 0; b < blocks; ++b)
			if (f.blocks[b].landing)
				reach(b);
		for (const EhLanding& landing : f.eh.landings)
		{
			if (landing.pad != kNoId)
				reach(landing.pad);
			if (landing.select != kNoId)
				reach(landing.select);
		}
		while (!work.empty())
		{
			const uint32_t b = work.back();
			work.pop_back();
			const Block& block = f.blocks[b];
			Op last = Op::Nop;
			for (uint32_t i = block.first; i < block.first + block.count; ++i)
			{
				const Inst& inst = f.insts[i];
				switch (inst.op)
				{
				case Op::Nop:
					continue;
				case Op::JumpTable:
				case Op::IndirectJump:
					for (uint32_t k = inst.aux; k < inst.aux + inst.aux2; ++k)
						reach(f.jump_targets[k]);
					break;
				case Op::WideCmpBranch:
					reach(inst.aux);
					[[fallthrough]];
				case Op::Jcc:
				case Op::Jmp:
				case Op::EhPush:
					for (unsigned k = 0; k < inst.nops; ++k)
						if (inst.ops[k].kind == OperandKind::Label)
							reach(inst.ops[k].symbol);
					break;
				default:
					break;
				}
				last = inst.op;
			}
			if (!(last == Op::Jmp || last == Op::Ret || last == Op::TailJump || last == Op::JumpTable ||
					last == Op::IndirectJump || last == Op::Trap || last == Op::Resume || last == Op::Throw))
				reach(b + 1);
		}
	}
	// layout_map_[b]: the block ending b's jump chain (b itself unless b is
	// jump-only), or kNoChainEnd for a chain entering a cycle (kept).
	std::vector<uint32_t>& end = layout_map_;
	end.assign(blocks, kUnresolved);
	for (uint32_t b = 0; b < blocks; ++b)
	{
		uint32_t t = b;
		layout_chain_.clear();
		while (end[t] == kUnresolved && jump_only(t))
		{
			end[t] = kWalking;
			layout_chain_.push_back(t);
			t = f.insts[f.blocks[t].first].ops[0].symbol;
		}
		const uint32_t e = end[t] == kUnresolved ? t : (end[t] == kWalking ? kNoChainEnd : end[t]);
		if (end[t] == kUnresolved)
			end[t] = t;
		for (uint32_t c : layout_chain_)
			end[c] = e;
	}
	// (Placement's estimate, read by the old indexes.)
	std::vector<uint64_t>& frequency = layout_frequency_;
	frequency.clear();
	if (!addressed)
		block_frequencies(f, frequency);
	share_small_tails(f, end, live, frequency);
	// The kept blocks' new indexes; a removed block's references take its
	// chain end's.
	uint32_t kept = 0;
	for (uint32_t b = 0; b < blocks; ++b)
	{
		if (end[b] == kNoChainEnd)
			end[b] = b;
		if (end[b] == b && live[b])
			end[b] = kept++ | 0x80000000u;
	}
	auto target = [&](uint32_t b) {
		const uint32_t e = end[b];
		return (e & 0x80000000u) ? e & 0x7fffffffu : end[e] & 0x7fffffffu;
	};
	for (lowir::CodeLabel& label : f.code_labels)
		label.block = target(label.block);
	for (EhLanding& landing : f.eh.landings)
	{
		if (landing.pad != kNoId)
			landing.pad = target(landing.pad);
		if (landing.select != kNoId)
			landing.select = target(landing.select);
	}
	// The kept blocks in order: their branches and handler records name
	// the new indexes, then their fall-through branches are dropped. A
	// block ending `jcc A; jmp B` with neither next jumps unconditionally
	// to the less frequent of the two (placement's estimate), so the
	// likelier path takes only the conditional branch; a jump that closes
	// a loop the branch leaves (closes_range) is the likelier whatever the
	// estimate of the two targets.
	uint32_t index = 0;
	uint32_t laid_out = 0;
	for (uint32_t b = 0; b < blocks; ++b)
	{
		if (!(end[b] & 0x80000000u))
			continue;
		Block block = f.blocks[b];
		bool colder_jcc = false;
		if (block.count >= 2 && f.insts[block.first + block.count - 1].op == Op::Jmp &&
			f.insts[block.first + block.count - 2].op == Op::Jcc && (!frequency.empty() || addressed))
		{
			const Operand& a = f.insts[block.first + block.count - 2].ops[0];
			const Operand& j = f.insts[block.first + block.count - 1].ops[0];
			if (a.kind == OperandKind::Label && j.kind == OperandKind::Label && a.symbol < blocks && j.symbol < blocks)
			{
				const uint32_t probability = f.insts[block.first + block.count - 2].aux;
				colder_jcc = addressed ? probability > 0 && probability < 500
									   : frequency[a.symbol] < frequency[j.symbol];
			}
		}
		for (uint32_t i = block.first; i < block.first + block.count; ++i)
		{
			Inst& inst = f.insts[i];
			if (inst.op == Op::JumpTable || inst.op == Op::IndirectJump)
			{
				for (uint32_t k = inst.aux; k < inst.aux + inst.aux2; ++k)
					f.jump_targets[k] = target(f.jump_targets[k]);
				continue;
			}
			if (inst.op != Op::Jmp && inst.op != Op::Jcc && inst.op != Op::WideCmpBranch && inst.op != Op::EhPush)
				continue;
			for (unsigned k = 0; k < inst.nops; ++k)
			{
				if (inst.ops[k].kind == OperandKind::Label)
					inst.ops[k].symbol = target(inst.ops[k].symbol);
			}
			if (inst.op == Op::WideCmpBranch)
				inst.aux = target(inst.aux);
		}
		const uint32_t next = index + 1 < kept ? index + 1 : kNoId;
		uint32_t count = block.count;
		// The function's last block has no next one: only the exchange applies.
		if (count != 0)
		{
			Inst& last = f.insts[block.first + count - 1];
			if (branch_to(last, Op::Jmp, next))
				--count;
			else if (last.op == Op::Jmp && count >= 2 && branch_to(f.insts[block.first + count - 2], Op::Jcc, next))
			{
				Inst& jcc = f.insts[block.first + count - 2];
				jcc.sub = invert(static_cast<Cond>(jcc.sub));
				if (jcc.aux > 0 && jcc.aux < 1000)
					jcc.aux = 1000 - jcc.aux;
				jcc.ops[0] = last.ops[0];
				--count;
			}
			else if ((colder_jcc || closes_range(f, block, index, kept, addressed)) && last.op == Op::Jmp &&
				count >= 2 && f.insts[block.first + count - 2].op == Op::Jcc)
			{
				Inst& jcc = f.insts[block.first + count - 2];
				jcc.sub = invert(static_cast<Cond>(jcc.sub));
				if (jcc.aux > 0 && jcc.aux < 1000)
					jcc.aux = 1000 - jcc.aux;
				std::swap(jcc.ops[0], last.ops[0]);
			}
			while (count != 0 && branch_to(f.insts[block.first + count - 1], Op::Jcc, next))
				--count;
			stats_.layout_branches += block.count - count;
		}
		block.count = count;
		laid_out += count;
		f.blocks[index++] = block;
	}
	stats_.threaded_blocks += blocks - kept;
	f.blocks.resize(kept);
	f.branches_laid_out = true;
	if (level_ >= 1 && eh_model_ == EhModel::Host && !addressed)
		laid_out -= form_tail_jumps(f);
	return laid_out;
}

} // namespace native
