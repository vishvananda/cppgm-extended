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
// Only branches change; no value, flag or frame fact does.
namespace native {

using lowir::kNoId;

namespace {

const uint32_t kUnresolved = 0xffffffffu;
const uint32_t kWalking = 0xfffffffeu;
const uint32_t kNoChainEnd = 0xfffffffdu;

bool branch_to(const Inst& inst, Op op, uint32_t block)
{
	return inst.op == op && inst.ops[0].kind == OperandKind::Label && inst.ops[0].symbol == block;
}

// O2+, host model, laid-out MIR: a call the function returns from at once
// (a `ret` after it in its block, or first in the block it falls into)
// jumps to its callee past the epilogue (Op::TailJump) when the callee
// cannot reach the frame (the function forms no frame address and passes
// no stack argument), no landing catches it and no strlen prefix precedes
// it. The returned registers are the callee's either way.
uint32_t form_tail_jumps(Function& f)
{
	if (f.variadic || f.eh_records_pushed)
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

uint32_t FunctionSelector::layout_branches()
{
	const uint32_t rotated = rotate_loops(*out_);
	stats_.rotated_loops += rotated;
	thread_decided_tests(*out_, program_);
	place_blocks(*out_);
	// A spill moved into a prologue block may meet its reload there, and a
	// loop's test left only on its entry path (its copy closes the loop)
	// may read a home the entry's block still holds in a register.
	if (shrink_wrap(*out_) || rotated != 0)
		forward_home_reloads(*out_);
	Function& f = *out_;
	const uint32_t blocks = static_cast<uint32_t>(f.blocks.size());
	auto jump_only = [&](uint32_t b) {
		const Block& block = f.blocks[b];
		return b != 0 && block.count == 1 && f.insts[block.first].op == Op::Jmp;
	};
	// A jump to a block that only returns (through jump-only blocks)
	// returns itself when the return is all its epilogue (a frameless
	// return, or no saves and no frame): the return is copied over the jump.
	const bool bare_returns = f.preserve_mask == 0 && f.stack_size == 0;
	for (uint32_t b = 0; b < blocks; ++b)
	{
		Block& block = f.blocks[b];
		if (block.count == 0)
			continue;
		Inst& last = f.insts[block.first + block.count - 1];
		if (last.op != Op::Jmp || last.ops[0].kind != OperandKind::Label)
			continue;
		uint32_t t = last.ops[0].symbol;
		for (unsigned step = 0; step < 4 && t < blocks && jump_only(t); ++step)
			t = f.insts[f.blocks[t].first].ops[0].symbol;
		if (t < blocks && t != b && f.blocks[t].count == 1 && f.insts[f.blocks[t].first].op == Op::Ret &&
			(bare_returns || (f.insts[f.blocks[t].first].flags & MF_FRAMELESS) != 0))
			last = f.insts[f.blocks[t].first];
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
	// The kept blocks' new indexes; a removed block's references take its
	// chain end's.
	uint32_t kept = 0;
	for (uint32_t b = 0; b < blocks; ++b)
	{
		if (end[b] == kNoChainEnd)
			end[b] = b;
		if (end[b] == b)
			end[b] = kept++ | 0x80000000u;
	}
	auto target = [&](uint32_t b) {
		const uint32_t e = end[b];
		return (e & 0x80000000u) ? e & 0x7fffffffu : end[e] & 0x7fffffffu;
	};
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
	// likelier path takes only the conditional branch.
	std::vector<uint64_t>& frequency = layout_frequency_;
	block_frequencies(f, frequency);
	uint32_t index = 0;
	uint32_t laid_out = 0;
	for (uint32_t b = 0; b < blocks; ++b)
	{
		if (!(end[b] & 0x80000000u))
			continue;
		Block block = f.blocks[b];
		bool colder_jcc = false;
		if (block.count >= 2 && f.insts[block.first + block.count - 1].op == Op::Jmp &&
			f.insts[block.first + block.count - 2].op == Op::Jcc && !frequency.empty())
		{
			const Operand& a = f.insts[block.first + block.count - 2].ops[0];
			const Operand& j = f.insts[block.first + block.count - 1].ops[0];
			colder_jcc = a.kind == OperandKind::Label && j.kind == OperandKind::Label && a.symbol < blocks &&
				j.symbol < blocks && frequency[a.symbol] < frequency[j.symbol];
		}
		for (uint32_t i = block.first; i < block.first + block.count; ++i)
		{
			Inst& inst = f.insts[i];
			if (inst.op == Op::JumpTable)
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
		if (next != kNoId && count != 0)
		{
			Inst& last = f.insts[block.first + count - 1];
			if (branch_to(last, Op::Jmp, next))
				--count;
			else if (last.op == Op::Jmp && count >= 2 && branch_to(f.insts[block.first + count - 2], Op::Jcc, next))
			{
				Inst& jcc = f.insts[block.first + count - 2];
				jcc.sub = invert(static_cast<Cond>(jcc.sub));
				jcc.ops[0] = last.ops[0];
				--count;
			}
			else if (colder_jcc && last.op == Op::Jmp && count >= 2 && f.insts[block.first + count - 2].op == Op::Jcc)
			{
				Inst& jcc = f.insts[block.first + count - 2];
				jcc.sub = invert(static_cast<Cond>(jcc.sub));
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
	if (level_ >= 2 && eh_model_ == EhModel::Host)
		laid_out -= form_tail_jumps(f);
	return laid_out;
}

} // namespace native
