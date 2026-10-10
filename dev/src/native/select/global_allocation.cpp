#include "native/select/function_selector.h"
#include "native/select/for_each_inst.h"

#include <algorithm>

#include "native/select/operand_roles.h"

// Function-level registers for values used outside their defining block,
// at O0 (O1 and above: register_allocation.cpp).
// Blocks are numbered into one linear order (each block spans its
// instructions plus a start and an end position); a value's hull runs from
// its definition through every block where liveness (a backward walk from
// its uses over predecessors) finds it live, and through the ends of the
// predecessors whose edge transfers write a phi. One linear scan over the
// hulls, sorted by start, gives each a register that no instruction inside
// the hull clobbers (range ORs from per-chunk prefix and suffix ORs and a
// sparse table over the chunks: linear space, O(1) per query), preferring a parameter's incoming register and taking
// callee-saved registers for hulls that cross calls. Values without one
// keep frame homes, as do the values left when the walks exhaust their
// work budget (linear in the function). A block reserves the registers of every hull that
// overlaps it, so block-local allocation never uses them. Functions with
// EH regions keep the conservative frame homes: the dispatcher restores
// callee-saved registers as they were when a handler was pushed.
namespace native {

using lowir::Instruction;
using lowir::kNoId;
using lowir::Opcode;
using lowir::Value;
using lowir::ValueKind;

namespace {

const Reg kCalleeOrder[] = { RBX, R12, R13, R14, R15 };
const Reg kCallerOrder[] = { R8, R9, RDI, RSI, RDX, RCX, RAX };
// At most this many GPR hulls hold registers at once, leaving the rest of
// the allocatable set to block-local values.
const unsigned kMaxGlobalGprs = 7;
const unsigned kMaxDisplacedParameters = 4;
// The liveness walks' work budget (uses and predecessor edges visited)
// per instruction and block of the function, plus a fixed allowance. The
// PA10-PA23 fixtures and the executable benchmarks need at most 2.4 per
// item; values live across most of a long chain of blocks need one walk
// step per value and block.
const uint64_t kLivenessStepsPerItem = 64;
const uint64_t kLivenessSlack = 4096;
// Linear positions per chunk of the clobber range structure.
const uint32_t kClobberChunk = 32;

struct Interval
{
	uint32_t temp;
	int32_t start;
	int32_t end;
	bool operator<(const Interval& other) const { return start != other.start ? start < other.start : end < other.end; }
};

} // namespace

// The registers clobbered at linear positions first..last: within one
// chunk directly, otherwise the first chunk's suffix, the last chunk's
// prefix and the sparse table over the whole chunks between them.
uint32_t FunctionSelector::range_clobbers(int32_t first, int32_t last) const
{
	if (first > last || clobber_at_.empty())
		return 0;
	const uint32_t cf = static_cast<uint32_t>(first) / kClobberChunk;
	const uint32_t cl = static_cast<uint32_t>(last) / kClobberChunk;
	if (cf == cl)
	{
		uint32_t mask = 0;
		for (int32_t i = first; i <= last; ++i)
			mask |= clobber_at_[i];
		return mask;
	}
	uint32_t mask = clobber_suffix_[first] | clobber_prefix_[last];
	if (cl - cf > 1)
	{
		const uint32_t length = cl - cf - 1;
		const unsigned level = 31 - __builtin_clz(length);
		const size_t row = static_cast<size_t>(level) * chunk_count_;
		mask |= chunk_or_[row + cf + 1] | chunk_or_[row + cl - (1u << level)];
	}
	return mask;
}

void FunctionSelector::global_hulls(std::vector<uint32_t>& candidates)
{
	const lowir::FunctionBody& body = *body_;
	std::vector<int32_t> index(temps_.size(), -1);
	for (size_t c = 0; c < candidates.size(); ++c)
		index[candidates[c]] = static_cast<int32_t>(c);
	// Uses of each candidate as (block, linear position), in CSR form.
	std::vector<uint32_t> count(candidates.size() + 1, 0);
	std::vector<std::pair<uint32_t, int32_t>> uses;
	std::vector<uint32_t> owner;
	auto record = [&](Value v, uint32_t b, int32_t lin) {
		v = resolve(v);
		if (v.kind != ValueKind::Temp)
			return;
		const TempInfo& t = temps_[v.index];
		uint32_t parts[3] = { v.index, kNoId, kNoId };
		if ((t.flags & TF_LAZY) && t.addr != kNoId)
		{
			const AddrExpr& expr = addrs_[t.addr];
			parts[0] = kNoId;
			parts[1] = expr.kind == AddrExpr::Temp ? expr.base : kNoId;
			parts[2] = expr.index;
		}
		for (unsigned k = 0; k < 3; ++k)
		{
			if (parts[k] == kNoId || index[parts[k]] < 0)
				continue;
			uses.push_back(std::make_pair(b, lin));
			owner.push_back(static_cast<uint32_t>(index[parts[k]]));
		}
	};
	for_each_inst([&](uint32_t id, uint32_t b) {
		const Instruction& inst = body.instructions[id];
		if ((marks_[id] & IM_DEAD) || (inst.result != kNoId && (temps_[inst.result].flags & TF_ALIAS)))
			return;
		if (inst.op == Opcode::Phi)
		{
			for (uint32_t k = 0; k + 1 < inst.extra_count; k += 2)
			{
				const uint32_t pred = layout_of_[body.operands[inst.extra_first + k].index];
				if (pred != kNoId && blocks_[pred].reachable)
					record(body.operands[inst.extra_first + k + 1], pred,
						block_base_[pred] + static_cast<int32_t>(blocks_[pred].count) + 1);
			}
			return;
		}
		// A folded load reads its address at its consumer, a fused compare
		// its operands at its branch.
		const int32_t lin = block_base_[b] + 1 + read_pos(id);
		for_each_operand(body, inst, [&](Value v, OperandRole) { record(v, b, lin); });
	});
	for (size_t u = 0; u < owner.size(); ++u)
		++count[owner[u] + 1];
	for (size_t c = 0; c < candidates.size(); ++c)
		count[c + 1] += count[c];
	std::vector<std::pair<uint32_t, int32_t>> sorted(uses.size());
	std::vector<uint32_t> fill(count.begin(), count.end() - 1);
	for (size_t u = 0; u < owner.size(); ++u)
		sorted[fill[owner[u]]++] = uses[u];
	// Liveness walk per candidate, within the work budget: the candidates
	// from the one that exhausts it on keep their frame homes.
	std::vector<uint32_t> mark(blocks_.size(), 0);
	std::vector<uint32_t> work;
	uint64_t items = 0;
	for (uint32_t b = 0; b < blocks_.size(); ++b)
		if (blocks_[b].reachable)
			items += blocks_[b].count + 1;
	const uint64_t budget = kLivenessStepsPerItem * items + kLivenessSlack;
	uint64_t steps = 0;
	for (size_t c = 0; c < candidates.size(); ++c)
	{
		const uint32_t temp = candidates[c];
		const TempInfo& t = temps_[temp];
		const uint32_t def_block = t.block;
		const uint32_t epoch = static_cast<uint32_t>(c + 1);
		int32_t start = block_base_[def_block] + 1 + t.def;
		int32_t end = start;
		auto live_in = [&](uint32_t b) {
			if (mark[b] == epoch)
				return;
			mark[b] = epoch;
			start = std::min(start, block_base_[b]);
			end = std::max(end, block_base_[b] + static_cast<int32_t>(blocks_[b].count) + 1);
			work.push_back(b);
		};
		steps += count[c + 1] - count[c];
		for (uint32_t u = count[c]; u < count[c + 1]; ++u)
		{
			const uint32_t b = sorted[u].first;
			const int32_t lin = sorted[u].second;
			end = std::max(end, lin);
			start = std::min(start, lin);
			if (b != def_block || lin < block_base_[def_block] + 1 + t.def)
				live_in(b);
		}
		const Instruction* def = t.def < 0 && temp >= body.parameter_count ? &body.instructions[body.temps[temp].def]
																		   : nullptr;
		if (def != nullptr && def->op == Opcode::Phi)
		{
			// The edge transfers into a phi write its register at the end of
			// each predecessor.
			for (uint32_t k = preds_first_[def_block]; k < preds_first_[def_block + 1]; ++k)
			{
				const uint32_t pred = preds_[k];
				const int32_t pred_end = block_base_[pred] + static_cast<int32_t>(blocks_[pred].count) + 1;
				start = std::min(start, pred_end);
				end = std::max(end, pred_end);
			}
		}
		while (!work.empty() && steps <= budget)
		{
			const uint32_t x = work.back();
			work.pop_back();
			steps += preds_first_[x + 1] - preds_first_[x];
			for (uint32_t k = preds_first_[x]; k < preds_first_[x + 1]; ++k)
			{
				const uint32_t q = preds_[k];
				end = std::max(end, block_base_[q] + static_cast<int32_t>(blocks_[q].count) + 1);
				if (q != def_block)
					live_in(q);
			}
		}
		if (steps > budget)
		{
			work.clear();
			stats_.hull_fallbacks += candidates.size() - c;
			candidates.resize(c);
			break;
		}
		hull_start_[temp] = start;
		hull_end_[temp] = end;
	}
	stats_.liveness_steps += steps;
}

void FunctionSelector::allocate_globals()
{
	const lowir::FunctionBody& body = *body_;
	reserved_.assign(blocks_.size(), 0);
	hull_start_.assign(temps_.size(), -1);
	hull_end_.assign(temps_.size(), -1);
	global_temps_.clear();
	block_globals_first_.assign(blocks_.size() + 1, 0);
	block_globals_.clear();
	split_moves_.clear();
	split_homes_.temps.clear();
	precise_globals_ = false;
	if (level_ >= 1)
	{
		allocate_globals_precise();
		return;
	}
	if (has_eh_)
		return;
	std::vector<uint32_t> candidates;
	for (uint32_t t = 0; t < temps_.size(); ++t)
	{
		const TempInfo& info = temps_[t];
		// Vector parameters keep their parameter slots.
		if ((info.flags & TF_GLOBAL) && (info.flags & TF_DEFINED) && !(info.flags & (TF_ALIAS | TF_DEAD | TF_LAZY)) &&
			(info.cls == VClass::Gpr || info.cls == VClass::Xmm) && info.block != kNoId && info.uses != 0 &&
			!(t < body.parameter_count && info.cls == VClass::Xmm))
			candidates.push_back(t);
	}
	if (candidates.empty())
		return;
	// Linear positions and the clobber range structure.
	block_base_.assign(blocks_.size(), 0);
	int32_t total = 0;
	for (uint32_t b = 0; b < blocks_.size(); ++b)
	{
		block_base_[b] = total;
		if (blocks_[b].reachable)
			total += static_cast<int32_t>(blocks_[b].count) + 2;
	}
	const size_t positions = static_cast<size_t>(total) + 1;
	std::vector<uint32_t> early_at(positions, 0);
	clobber_at_.assign(positions, 0);
	for_each_inst([&](uint32_t id, uint32_t b) {
		if (!live(id))
			return;
		const int32_t lin = block_base_[b] + 1 + pos_[id];
		clobber_at_[lin] = clobber_[id] | early_[id];
		early_at[lin] = early_[id];
	});
	// Range ORs in linear space: per position the OR from its chunk's start
	// and to its chunk's end, and a sparse table over the chunks' ORs.
	clobber_prefix_.resize(positions);
	clobber_suffix_.resize(positions);
	chunk_count_ = static_cast<uint32_t>((positions + kClobberChunk - 1) / kClobberChunk);
	unsigned levels = 1;
	while ((1u << levels) <= chunk_count_)
		++levels;
	chunk_or_.assign(static_cast<size_t>(levels) * chunk_count_, 0);
	for (uint32_t c = 0; c < chunk_count_; ++c)
	{
		const size_t begin = static_cast<size_t>(c) * kClobberChunk;
		const size_t end = begin + kClobberChunk < positions ? begin + kClobberChunk : positions;
		uint32_t mask = 0;
		for (size_t i = begin; i < end; ++i)
			clobber_prefix_[i] = mask |= clobber_at_[i];
		chunk_or_[c] = mask;
		mask = 0;
		for (size_t i = end; i-- > begin;)
			clobber_suffix_[i] = mask |= clobber_at_[i];
	}
	for (unsigned level = 1; level < levels; ++level)
	{
		const size_t row = static_cast<size_t>(level) * chunk_count_;
		const size_t below = row - chunk_count_;
		const uint32_t half = 1u << (level - 1);
		for (uint32_t c = 0; c + 2 * half <= chunk_count_; ++c)
			chunk_or_[row + c] = chunk_or_[below + c] | chunk_or_[below + c + half];
	}
	global_hulls(candidates);
	std::vector<Interval> intervals;
	for (size_t c = 0; c < candidates.size(); ++c)
	{
		Interval interval;
		interval.temp = candidates[c];
		interval.start = hull_start_[candidates[c]];
		interval.end = hull_end_[candidates[c]];
		intervals.push_back(interval);
	}
	std::sort(intervals.begin(), intervals.end());
	// Parameters that cannot keep their incoming register through their
	// hull take callee-saved registers only if all of them fit; otherwise
	// each keeps a frame home and reuses its incoming register while intact.
	unsigned displaced = 0;
	for (size_t i = 0; i < intervals.size(); ++i)
	{
		if (intervals[i].temp >= body.parameter_count)
			continue;
		const Reg incoming = incoming_reg(intervals[i].temp);
		const uint32_t forbid = range_clobbers(intervals[i].start + 1, intervals[i].end - 1) |
			early_at[intervals[i].end];
		if (incoming == kNoReg || (forbid & (1u << incoming)))
			++displaced;
	}
	const bool displaced_fit = displaced <= kMaxDisplacedParameters;
	std::vector<Interval> active;
	std::vector<std::pair<uint32_t, uint32_t>> bindings;
	uint32_t active_mask = 0;
	unsigned active_gprs = 0;
	for (size_t i = 0; i < intervals.size(); ++i)
	{
		const Interval& interval = intervals[i];
		for (size_t a = 0; a < active.size();)
		{
			if (active[a].end < interval.start)
			{
				const Reg r = temps_[active[a].temp].greg;
				active_mask &= ~(1u << r);
				if (!is_xmm(r))
					--active_gprs;
				active[a] = active.back();
				active.pop_back();
			}
			else
				++a;
		}
		TempInfo& t = temps_[interval.temp];
		uint32_t forbid = range_clobbers(interval.start + 1, interval.end - 1) | early_at[interval.end];
		// A variable shift writes its result before reading the count it
		// loaded into rcx: its result may not take that early clobber.
		if (interval.temp >= body.parameter_count && t.def >= 0)
		{
			const lowir::InstId def = body.temps[interval.temp].def;
			const Instruction& inst = body.instructions[def];
			const lowir::BinaryOp op = static_cast<lowir::BinaryOp>(inst.sub);
			if (inst.op == Opcode::Binary &&
				(op == lowir::BinaryOp::Shl || op == lowir::BinaryOp::Shr || op == lowir::BinaryOp::UShr))
				forbid |= early_[def];
		}
		const bool gpr = t.cls == VClass::Gpr;
		if (gpr && active_gprs >= kMaxGlobalGprs)
			continue;
		const uint32_t allowed = (gpr ? kAllocatableGprs & ~reserved_gprs(*out_) : kAllocatableXmms) & ~forbid &
			~active_mask;
		if (allowed == 0)
			continue;
		Reg chosen = kNoReg;
		if (interval.temp < body.parameter_count)
		{
			// A parameter keeps its incoming register for the whole hull, or
			// a frame home with the incoming register reused wherever it is
			// still intact (emit_control.cpp).
			const Reg incoming = incoming_reg(interval.temp);
			if (incoming != kNoReg && (allowed & (1u << incoming)))
				chosen = incoming;
			else if (!displaced_fit)
				continue;
		}
		if (chosen == kNoReg && !gpr)
			chosen = static_cast<Reg>(__builtin_ctz(allowed));
		const bool crosses_call = (forbid & kCallerSavedGprs & kAllocatableGprs) != 0;
		for (unsigned k = 0; k < 5 && chosen == kNoReg && crosses_call; ++k)
			if (allowed & (1u << kCalleeOrder[k]))
				chosen = kCalleeOrder[k];
		for (unsigned k = 0; k < 7 && chosen == kNoReg; ++k)
			if (allowed & (1u << kCallerOrder[k]))
				chosen = kCallerOrder[k];
		for (unsigned k = 0; k < 5 && chosen == kNoReg; ++k)
			if (allowed & (1u << kCalleeOrder[k]))
				chosen = kCalleeOrder[k];
		if (chosen == kNoReg)
			continue;
		t.greg = chosen;
		global_temps_.push_back(interval.temp);
		active.push_back(interval);
		active_mask |= 1u << chosen;
		if (gpr)
			++active_gprs;
		if (kCalleeSavedGprs & (1u << chosen))
			used_callee_saved_ |= 1u << chosen;
		++stats_.global_registers;
		// The blocks the hull overlaps: at most the active hulls cover a
		// block, so the marking is linear in the blocks overall.
		uint32_t b = static_cast<uint32_t>(
			std::upper_bound(block_base_.begin(), block_base_.end(), interval.start) - block_base_.begin());
		b = b == 0 ? 0 : b - 1;
		for (; b < blocks_.size() && block_base_[b] <= interval.end; ++b)
		{
			if (!blocks_[b].reachable)
				continue;
			const int32_t hi = block_base_[b] + static_cast<int32_t>(blocks_[b].count) + 1;
			if (hi < interval.start)
				continue;
			reserved_[b] |= 1u << chosen;
			bindings.push_back(std::make_pair(b, interval.temp));
		}
	}
	// Per block, the hull values to bind on entry (CSR by block).
	block_globals_first_.assign(blocks_.size() + 1, 0);
	for (size_t k = 0; k < bindings.size(); ++k)
		++block_globals_first_[bindings[k].first + 1];
	for (size_t k = 0; k < blocks_.size(); ++k)
		block_globals_first_[k + 1] += block_globals_first_[k];
	block_globals_.assign(bindings.size(), 0);
	std::vector<uint32_t> fill(block_globals_first_.begin(), block_globals_first_.end() - 1);
	for (size_t k = 0; k < bindings.size(); ++k)
		block_globals_[fill[bindings[k].first]++] = bindings[k].second;
}

} // namespace native
