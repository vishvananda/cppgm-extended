#include <algorithm>
#include <vector>

#include "native/mir/type_kinds.h"
#include "native/select/block_placement.h"
#include "native/select/bulk_policy.h"
#include "native/select/shrink_wrap.h"

// Shrink-wrapping (O1+, frame-pointer-free functions, before branch
// layout): the blocks the entry reaches without passing a block that needs
// the frame -- one with a call, a frame operand, rsp, or anything but
// plain integer instructions, branches and returns -- and that no such
// block reaches back run frameless when one of them returns. Their uses of
// callee-saved registers take caller-saved registers the region never
// names (nor an incoming argument); each edge out of the region goes
// through a new block that sets the frame up (Op::Prologue: the
// reservation and the saves' pushes) and copies those registers back. The
// region's returns are frameless (MF_FRAMELESS). The region comes first,
// then the prologue blocks, then the rest, so the unwind facts read the
// code before the first prologue as the entry's state
// (object/host_unwind.cpp). No call runs in the region: nothing unwinds
// through it, and landing pads (entered from framed calls only) stay
// framed. Linear in the function's MIR.
namespace native {

namespace {

const Reg kCalleeSaved[6] = { RBX, RBP, R12, R13, R14, R15 };
const Reg kCallerSaved[7] = { RAX, RCX, RDX, RSI, RDI, R8, R9 };
// The most instructions copied into the region from blocks frame-needing
// code also reaches (joins on the way to a return); short of registers,
// only return blocks of at most kReturnCopyCap instructions.
const uint32_t kCopyBudget = 64;
const uint32_t kReturnCopyCap = 4;
// The minimal plan's copies of other frameless blocks (an early exit's
// tests before its return), at most this many instructions in all: the
// largest of these that leaves registers for the renames.
const uint32_t kMinimalCopyBudgets[3] = { 16, 8, 4 };

uint32_t named(const Operand& op);

// Plain integer instructions, direct bulk copies, branches and returns of
// a GPR: what runs without the frame (no frame operand or rsp).
bool frameless_inst(const Inst& inst)
{
	if (inst.op == Op::Nop)
		return true;
	switch (inst.op)
	{
	case Op::Mov:
	case Op::Load:
	case Op::Lea:
	case Op::Add:
	case Op::Sub:
	case Op::And:
	case Op::Or:
	case Op::Xor:
	case Op::Imul:
	case Op::Neg:
	case Op::Not:
	case Op::Shl:
	case Op::Shr:
	case Op::Sar:
	case Op::Setcc:
	case Op::Cmov:
	case Op::Movzx:
	case Op::Sext:
	case Op::Zext:
	case Op::Cmp:
	case Op::Test:
	case Op::Bt:
	case Op::Store:
	case Op::Nop:
	case Op::Jcc:
	case Op::Jmp:
	case Op::Bsf:
	case Op::Bsr:
	case Op::Bswap:
		break;
	// Direct bulk forms (xmm15 and r10/r11 chunks; bulk_policy.h), save a
	// copy whose addresses name both scratch registers (it borrows another
	// on the stack).
	case Op::CopyBytes:
		if ((inst.flags & MF_COPY_DYNAMIC) != 0 || !direct_copy(inst.aux, inst.aux2, true) ||
			((named(inst.ops[0]) | named(inst.ops[1])) & ((1u << R10) | (1u << R11))) == ((1u << R10) | (1u << R11)))
			return false;
		break;
	case Op::ZeroBytes:
		if (!always_direct_zero(inst.aux, true))
			return false;
		break;
	case Op::Ret:
		if (inst.nops != 0 && !(inst.ops[0].kind == OperandKind::Reg && inst.ops[0].reg < 16))
			return false;
		break;
	default:
		return false;
	}
	for (unsigned k = 0; k < inst.nops; ++k)
	{
		const Operand& op = inst.ops[k];
		if (op.kind == OperandKind::Reg && (op.reg >= 16 || op.reg == RSP))
			return false;
		if (op.kind == OperandKind::Mem &&
			(op.reg == kFrame || op.reg == RSP || op.index == RSP || (op.reg != kNoReg && op.reg >= 16) ||
				(op.index != kNoReg && op.index >= 16)))
			return false;
	}
	return true;
}

// The registers an operand names (bits).
uint32_t named(const Operand& op)
{
	uint32_t bits = 0;
	if (op.kind == OperandKind::Reg && op.reg < 16)
		bits |= 1u << op.reg;
	if (op.kind == OperandKind::Mem)
	{
		if (op.reg != kNoReg && op.reg < 16)
			bits |= 1u << op.reg;
		if (op.index != kNoReg && op.index < 16)
			bits |= 1u << op.index;
	}
	return bits;
}

void successors(const Function& f, uint32_t b, std::vector<uint32_t>& out)
{
	out.clear();
	const Block& block = f.blocks[b];
	for (uint32_t i = block.first; i < block.first + block.count; ++i)
	{
		const Inst& inst = f.insts[i];
		if ((inst.op == Op::Jcc || inst.op == Op::Jmp) && inst.ops[0].kind == OperandKind::Label)
			out.push_back(inst.ops[0].symbol);
		else if (inst.op == Op::JumpTable)
			for (uint32_t k = inst.aux; k < inst.aux + inst.aux2; ++k)
				out.push_back(f.jump_targets[k]);
		else if (inst.op == Op::WideCmpBranch)
		{
			out.push_back(inst.ops[2].symbol);
			out.push_back(inst.aux);
		}
	}
}

bool terminated(const Function& f, const Block& block)
{
	if (block.count == 0)
		return false;
	const Op op = f.insts[block.first + block.count - 1].op;
	return op == Op::Jmp || op == Op::Ret || op == Op::FRet || op == Op::Trap || op == Op::JumpTable ||
		op == Op::Throw || op == Op::Resume || op == Op::Exit || op == Op::WideCmpBranch ||
		(op == Op::Call && (f.insts[block.first + block.count - 1].flags & MF_NORETURN) != 0);
}

} // namespace

bool shrink_wrap(Function& f)
{
	const uint32_t n = static_cast<uint32_t>(f.blocks.size());
	if (!f.omit_frame_pointer || f.variadic || f.eh_records_pushed || n < 2 ||
		(f.preserve_mask == 0 && f.stack_size == 0))
		return false;
	// The entry's stores of registers to frame slots it does not otherwise
	// touch (spills at the entry): they move to the prologue blocks, the
	// value waiting in a spare register when the region writes the stored
	// one.
	std::vector<uint32_t> sunk;
	{
		const Block& entry = f.blocks[0];
		for (uint32_t i = entry.first; i < entry.first + entry.count; ++i)
		{
			const Inst& inst = f.insts[i];
			if (inst.op != Op::Store || inst.ops[0].kind != OperandKind::Mem || inst.ops[0].reg != kFrame ||
				inst.ops[0].index != kNoReg || inst.ops[1].kind != OperandKind::Reg || inst.ops[1].reg >= 16 ||
				inst.ops[1].reg == RSP)
				continue;
			// (A GPR load or store touches its type's bytes; take anything
			// else as 16.)
			const int32_t bytes = static_cast<int32_t>(register_bytes(inst.type));
			bool alone = true;
			for (uint32_t j = entry.first; j < entry.first + entry.count && alone; ++j)
			{
				if (j == i)
					continue;
				const Inst& other = f.insts[j];
				const bool gpr = (other.op == Op::Store || other.op == Op::Load) && other.nops == 2 &&
					other.ops[other.op == Op::Store ? 1 : 0].kind == OperandKind::Reg &&
					other.ops[other.op == Op::Store ? 1 : 0].reg < 16;
				const int32_t width = gpr ? static_cast<int32_t>(register_bytes(other.type)) : 16;
				for (unsigned k = 0; k < other.nops; ++k)
					if (other.ops[k].kind == OperandKind::Mem && other.ops[k].reg == kFrame)
						alone = alone && other.ops[k].index == kNoReg &&
							(other.ops[k].disp + width <= inst.ops[0].disp ||
								inst.ops[0].disp + bytes <= other.ops[k].disp);
			}
			if (alone)
				sunk.push_back(i);
		}
	}
	// A load of a moved spill's slot at its store's width: it reads the
	// value from a register in the region (the index in `sunk`, or
	// sunk.size()). The stores are rewritten below: their slots first.
	std::vector<std::pair<int32_t, unsigned>> slots;
	for (uint32_t i : sunk)
		slots.push_back(std::make_pair(f.insts[i].ops[0].disp, register_bytes(f.insts[i].type)));
	auto reload_of = [&](uint32_t i) -> size_t {
		const Inst& inst = f.insts[i];
		if (inst.op != Op::Load || inst.ops[1].kind != OperandKind::Mem || inst.ops[1].reg != kFrame ||
			inst.ops[1].index != kNoReg || inst.ops[0].kind != OperandKind::Reg || inst.ops[0].reg >= 16 ||
			inst.type == lowir::TypeKind::I1)
			return sunk.size();
		for (size_t k = 0; k < slots.size(); ++k)
			if (slots[k].first == inst.ops[1].disp && slots[k].second == register_bytes(inst.type))
				return k;
		return sunk.size();
	};
	std::vector<uint8_t> needs(n, 0);
	std::vector<std::vector<uint32_t>> succs(n);
	for (uint32_t b = 0; b < n; ++b)
	{
		const Block& block = f.blocks[b];
		if (!terminated(f, block))
			return false;
		for (uint32_t i = block.first; i < block.first + block.count && !needs[b]; ++i)
			needs[b] = !frameless_inst(f.insts[i]) && std::find(sunk.begin(), sunk.end(), i) == sunk.end() &&
				reload_of(i) == sunk.size();
		successors(f, b, succs[b]);
	}
	// Blocks a frame-needing block reaches (it never returns to the region).
	std::vector<uint8_t> framed(needs);
	std::vector<uint32_t> work;
	for (uint32_t b = 0; b < n; ++b)
		if (framed[b])
			work.push_back(b);
	while (!work.empty())
	{
		const uint32_t b = work.back();
		work.pop_back();
		for (uint32_t s : succs[b])
			if (s < n && !framed[s])
			{
				framed[s] = 1;
				work.push_back(s);
			}
	}
	if (framed[0])
		return false;
	// A block frame-needing code also reaches that needs no frame itself (a
	// join on the way to a return) gets a frameless copy for the region's
	// edges, within kCopyBudget instructions in all; the copies' own
	// successors are taken alike. Short of registers for that, only short
	// return blocks (a shared `ret`) are copied.
	std::vector<uint8_t> region(n), copied(n), kept(sunk.size());
	std::vector<uint32_t> exits;             // distinct framed targets
	std::vector<std::pair<Reg, Reg>> copies; // (callee-saved, its stand-in)
	std::vector<Reg> spare(sunk.size());
	Reg rename[16];
	// Plans: wide (joins copied within kCopyBudget), narrow (short returns
	// copied), minimal (the entry alone with copies of the short returns it
	// enters; any other successor, framed or not, is an exit).
	enum Plan
	{
		kWide,
		kNarrow,
		kMinimal
	};
	auto plan = [&](Plan mode, uint32_t minimal_budget) -> bool {
		const bool wide = mode == kWide;
		uint32_t budget = kCopyBudget;
		auto short_return = [&](uint32_t b) {
			const Block& block = f.blocks[b];
			return !needs[b] && block.count <= kReturnCopyCap && f.insts[block.first + block.count - 1].op == Op::Ret;
		};
		auto copyable = [&](uint32_t b) { return wide ? !needs[b] && f.blocks[b].count <= budget : short_return(b); };
		// The region: what the entry reaches through blocks not framed.
		region.assign(n, 0);
		copied.assign(n, 0);
		region[0] = 1;
		work.assign(1, 0);
		bool returns = false;
		uint32_t names = 0;
		exits.clear();
		auto scan = [&](uint32_t b) {
			const Block& block = f.blocks[b];
			for (uint32_t i = block.first; i < block.first + block.count; ++i)
			{
				const Inst& inst = f.insts[i];
				returns = returns || inst.op == Op::Ret;
				for (unsigned k = 0; k < inst.nops; ++k)
					names |= named(inst.ops[k]);
			}
		};
		while (!work.empty())
		{
			const uint32_t b = work.back();
			work.pop_back();
			scan(b);
			for (uint32_t s : succs[b])
			{
				if (s >= n || (mode == kMinimal && s == 0))
					return false;
				if (mode == kMinimal)
				{
					// A short return gets a copy (the original may be entered
					// from an exit's framed code), so does another frameless
					// block within kMinimalCopyBudget; anything else is an exit.
					if (copied[s])
						continue;
					if (short_return(s) || (!needs[s] && f.blocks[s].count <= minimal_budget))
					{
						if (!short_return(s))
							minimal_budget -= f.blocks[s].count;
						copied[s] = 1;
						work.push_back(s);
					}
					else if (std::find(exits.begin(), exits.end(), s) == exits.end())
						exits.push_back(s);
				}
				else if (framed[s] && (copied[s] || copyable(s)))
				{
					if (!copied[s])
					{
						copied[s] = 1;
						budget -= f.blocks[s].count;
						work.push_back(s);
					}
				}
				else if (framed[s])
				{
					if (std::find(exits.begin(), exits.end(), s) == exits.end())
						exits.push_back(s);
				}
				else if (!region[s])
				{
					region[s] = 1;
					work.push_back(s);
				}
			}
		}
		if (!returns || exits.empty())
			return false;
		// The region's blocks an exit is still reachable from: a register
		// written only past them (on the way to a return) never reaches a
		// prologue.
		std::vector<uint8_t> to_exit(n, 0);
		for (bool grew = true; grew;)
		{
			grew = false;
			for (uint32_t b = 0; b < n; ++b)
			{
				if ((!region[b] && !copied[b]) || to_exit[b])
					continue;
				for (uint32_t s : succs[b])
					if (s >= n || (!region[s] && !copied[s]) || to_exit[s])
					{
						to_exit[b] = 1;
						grew = true;
						break;
					}
			}
		}
		// A moved store's register stays as it was through the region. The
		// writes in blocks an exit is reachable from count apart from the
		// leaf blocks' (on the way to a return only), whose names are kept.
		uint32_t written = 0, leaf_names = 0;
		uint32_t writes[16] = {}, exit_writes[16] = {};
		for (uint32_t b = 0; b < n; ++b)
		{
			if (!region[b] && !copied[b])
				continue;
			const Block& block = f.blocks[b];
			for (uint32_t i = block.first; i < block.first + block.count; ++i)
			{
				const Inst& inst = f.insts[i];
				if (!to_exit[b])
					for (unsigned k = 0; k < inst.nops; ++k)
						leaf_names |= named(inst.ops[k]);
				if (inst.nops > 0 && inst.ops[0].kind == OperandKind::Reg && inst.ops[0].reg < 16 &&
					inst.op != Op::Store && inst.op != Op::Cmp && inst.op != Op::Test && inst.op != Op::Bt &&
					inst.op != Op::Jcc && inst.op != Op::Jmp && inst.op != Op::Ret)
				{
					if (to_exit[b])
					{
						written |= 1u << inst.ops[0].reg;
						++exit_writes[inst.ops[0].reg];
					}
					++writes[inst.ops[0].reg];
				}
			}
		}
		// One whose register the region writes keeps the value in a spare
		// register from the store's place (allocated with the renames below).
		for (size_t k = 0; k < sunk.size(); ++k)
			kept[k] = (written & (1u << f.insts[sunk[k]].ops[1].reg)) != 0;
		for (uint32_t b = 0; b < n; ++b)
		{
			if (!region[b] && !copied[b])
				continue;
			const Block& block = f.blocks[b];
			for (uint32_t i = block.first; i < block.first + block.count; ++i)
				if (reload_of(i) != sunk.size())
					kept[reload_of(i)] = 1;
		}
		// Callee-saved registers the region names take free caller-saved ones.
		uint32_t incoming = 0;
		for (const AbiPart& part : f.parts)
			if (part.reg != kNoReg && part.reg < 16)
				incoming |= 1u << part.reg;
		for (Reg r = 0; r < 16; ++r)
			rename[r] = r;
		copies.clear();
		unsigned next = 0;
		const Block& entry = f.blocks[0];
		for (Reg c : kCalleeSaved)
		{
			if (!(names & (1u << c)))
				continue;
			// One the region sets just once on the way to an exit, first thing
			// it does with it, from an incoming register it never writes: that
			// register stands in (a leaf block may write it again when no leaf
			// block names the incoming register itself).
			uint32_t at = entry.first;
			while (at < entry.first + entry.count && !(named(f.insts[at].ops[0]) & (1u << c)) &&
				!(named(f.insts[at].ops[1]) & (1u << c)) && !(named(f.insts[at].ops[2]) & (1u << c)))
				++at;
			if (at < entry.first + entry.count && exit_writes[c] == 1)
			{
				const Inst& inst = f.insts[at];
				if (inst.op == Op::Mov && inst.ops[0].kind == OperandKind::Reg && inst.ops[0].reg == c &&
					inst.ops[1].kind == OperandKind::Reg && inst.ops[1].reg < 16 &&
					(incoming & (1u << inst.ops[1].reg)) && writes[inst.ops[1].reg] == 0 &&
					(writes[c] == 1 || !(leaf_names & (1u << inst.ops[1].reg))))
				{
					rename[c] = inst.ops[1].reg;
					copies.push_back(std::make_pair(c, rename[c]));
					continue;
				}
			}
			while (next < 7 && ((names | incoming) & (1u << kCallerSaved[next])))
				++next;
			if (next == 7)
				return false;
			rename[c] = kCallerSaved[next++];
			copies.push_back(std::make_pair(c, rename[c]));
		}
		spare.assign(sunk.size(), kNoReg);
		for (size_t k = 0; k < sunk.size(); ++k)
		{
			if (!kept[k])
				continue;
			while (next < 7 && ((names | incoming) & (1u << kCallerSaved[next])))
				++next;
			if (next == 7)
				return false;
			spare[k] = kCallerSaved[next++];
		}
		return true;
	};
	if (!plan(kWide, 0) && !plan(kNarrow, 0) && !plan(kMinimal, kMinimalCopyBudgets[0]) &&
		!plan(kMinimal, kMinimalCopyBudgets[1]) && !plan(kMinimal, kMinimalCopyBudgets[2]))
		return false;
	// The copies of shared returns, then a prologue block per framed target:
	// the frame, the registers back, the jump on.
	std::vector<uint32_t> target(n, lowir::kNoId);
	for (uint32_t b = 0; b < n; ++b)
	{
		if (!copied[b])
			continue;
		const Block original = f.blocks[b];
		Block block = original;
		block.first = static_cast<uint32_t>(f.insts.size());
		for (uint32_t i = original.first; i < original.first + original.count; ++i)
			f.insts.push_back(f.insts[i]);
		target[b] = static_cast<uint32_t>(f.blocks.size());
		f.blocks.push_back(block);
	}
	const uint32_t frameless_end = static_cast<uint32_t>(f.blocks.size());
	for (uint32_t s : exits)
	{
		Block block = Block();
		block.first = static_cast<uint32_t>(f.insts.size());
		Inst prologue = Inst();
		prologue.op = Op::Prologue;
		f.insts.push_back(prologue);
		for (const std::pair<Reg, Reg>& c : copies)
		{
			Inst copy = Inst();
			copy.op = Op::Mov;
			copy.nops = 2;
			copy.ops[0] = Operand::make_reg(c.first);
			copy.ops[1] = Operand::make_reg(c.second);
			f.insts.push_back(copy);
		}
		for (size_t k = 0; k < sunk.size(); ++k)
		{
			Inst store = f.insts[sunk[k]];
			if (spare[k] != kNoReg)
				store.ops[1] = Operand::make_reg(spare[k]);
			f.insts.push_back(store);
		}
		Inst jump = Inst();
		jump.op = Op::Jmp;
		jump.nops = 1;
		jump.ops[0] = Operand::make_label(s);
		f.insts.push_back(jump);
		block.count = static_cast<uint32_t>(f.insts.size()) - block.first;
		target[s] = static_cast<uint32_t>(f.blocks.size());
		f.blocks.push_back(block);
	}
	for (size_t k = 0; k < sunk.size(); ++k)
	{
		Inst& inst = f.insts[sunk[k]];
		if (spare[k] != kNoReg)
		{
			// The value waits in its spare register (renamed with the rest of
			// the region: the source may be a callee-saved register).
			inst.op = Op::Mov;
			inst.type = lowir::TypeKind::Void;
			inst.ops[0] = Operand::make_reg(spare[k]);
			continue;
		}
		inst.op = Op::Nop;
		inst.nops = 0;
		inst.ops[0] = inst.ops[1] = Operand();
	}
	// The region's code (with the copies): branches out through the
	// copies or prologue blocks, renamed registers, frameless returns.
	for (uint32_t b = 0; b < frameless_end; ++b)
	{
		if (b < n && !region[b])
			continue;
		const Block& block = f.blocks[b];
		for (uint32_t i = block.first; i < block.first + block.count; ++i)
		{
			Inst& inst = f.insts[i];
			const size_t reload = inst.op == Op::Load ? reload_of(i) : sunk.size();
			if (reload != sunk.size())
			{
				// Read from the spare register (an extension at a narrow width).
				const bool wide = register_bytes(inst.type) == 8;
				inst.op = wide ? Op::Mov : (signed_narrow(inst.type) ? Op::Sext : Op::Zext);
				if (wide)
					inst.type = lowir::TypeKind::Void;
				inst.ops[1] = Operand::make_reg(spare[reload]);
			}
			if ((inst.op == Op::Jcc || inst.op == Op::Jmp) && inst.ops[0].kind == OperandKind::Label &&
				inst.ops[0].symbol < n && target[inst.ops[0].symbol] != lowir::kNoId)
				inst.ops[0].symbol = target[inst.ops[0].symbol];
			else if (inst.op == Op::JumpTable)
				for (uint32_t k = inst.aux; k < inst.aux + inst.aux2; ++k)
					if (f.jump_targets[k] < n && target[f.jump_targets[k]] != lowir::kNoId)
						f.jump_targets[k] = target[f.jump_targets[k]];
			for (unsigned k = 0; k < inst.nops; ++k)
			{
				Operand& op = inst.ops[k];
				if (op.kind == OperandKind::Reg && op.reg < 16)
					op.reg = rename[op.reg];
				else if (op.kind == OperandKind::Mem)
				{
					if (op.reg != kNoReg && op.reg < 16)
						op.reg = rename[op.reg];
					if (op.index != kNoReg && op.index < 16)
						op.index = rename[op.index];
				}
			}
			if (inst.op == Op::Mov && inst.ops[0].kind == OperandKind::Reg && inst.ops[1].kind == OperandKind::Reg &&
				inst.ops[0].reg == inst.ops[1].reg)
			{
				inst.op = Op::Nop;
				inst.nops = 0;
				inst.ops[0] = inst.ops[1] = Operand();
			}
			if (inst.op == Op::Ret)
				inst.flags |= MF_FRAMELESS;
		}
	}
	// Placed again, then the frameless blocks (what the entry reaches short
	// of a prologue block) first, then the prologue blocks, then the rest,
	// each in placed order; the prologue block of the earliest-placed exit
	// is last and falls into its target.
	place_blocks(f);
	const uint32_t total = static_cast<uint32_t>(f.blocks.size());
	std::vector<uint8_t> part(total, 2); // 0 frameless, 1 prologue, 2 framed
	for (uint32_t b = 0; b < total; ++b)
		if (f.blocks[b].count != 0 && f.insts[f.blocks[b].first].op == Op::Prologue)
			part[b] = 1;
	std::vector<uint32_t> next_blocks;
	part[0] = 0;
	work.assign(1, 0);
	while (!work.empty())
	{
		const uint32_t b = work.back();
		work.pop_back();
		successors(f, b, next_blocks);
		for (uint32_t s : next_blocks)
			if (s < total && part[s] == 2)
			{
				part[s] = 0;
				work.push_back(s);
			}
	}
	uint32_t hot = lowir::kNoId, hot_target = lowir::kNoId;
	for (uint32_t b = 0; b < total; ++b)
	{
		if (part[b] != 1)
			continue;
		const Inst& jump = f.insts[f.blocks[b].first + f.blocks[b].count - 1];
		if (jump.ops[0].symbol < hot_target)
		{
			hot = b;
			hot_target = jump.ops[0].symbol;
		}
	}
	std::vector<uint32_t> order;
	for (uint8_t k = 0; k < 3; ++k)
	{
		if (k == 2)
		{
			order.push_back(hot);
			order.push_back(hot_target);
		}
		for (uint32_t b = 0; b < total; ++b)
			if (part[b] == k && b != hot && b != hot_target)
				order.push_back(b);
	}
	reorder_blocks(f, order);
	f.shrink_wrapped = true;
	return true;
}

} // namespace native
