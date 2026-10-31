#include "native/select/function_selector.h"
#include "native/select/for_each_inst.h"

#include <algorithm>
#include <iterator>
#include <map>
#include <stdexcept>

#include "native/select/operand_roles.h"

// Function-level registers at O1 and above (O0 keeps the hull allocator of
// global_allocation.cpp, whose shapes PA24's exact fixtures pin). Each
// value used outside its defining block gets one register for its whole
// life or keeps its frame home; block-local values are still allocated
// while emitting (allocation.cpp).
//
// Slots: block b spans 2 * count + 4 slots from ra_base_[b]: its entry,
// per instruction p a read slot (base + 2 + 2p) and a write slot (+1),
// then the edge transfers' read and write slots, the block's end. A
// value's live range in a block runs from the block's entry (live-in) or
// its definition's write slot (an edge transfer's for a phi, in each
// predecessor) to the block's end (live-out) or its last use. Liveness is
// a backward walk from each value's uses over predecessors -- a host
// landing's include the blocks of the calls and resumes that enter it,
// whose register state the unwinder restores to the call's -- within the
// work budget of the hull walks; values past it keep frame homes.
//
// A use is read at the read slot, except that operands other than an
// integer Binary/Unary's first are read "late", at the write slot: the
// result, written from its first operand, may share that operand's
// register (`mov d, a` vanishes) but no other operand's. Clobbers fall on
// write slots and conflict with ranges strictly across them; early
// clobbers on read slots conflict with the values read there too.
//
// Values are assigned in order of their weighted use counts (each use and
// the definition at its block's estimated executions, block_weights): each
// takes a register no instruction inside
// its range clobbers and no assigned range overlaps (per-register interval
// maps), where adding it keeps every slot's live values within the class's
// registers given the block-local values live there (a pressure tree),
// preferring its phi web's register, its tied operand's, a parameter's
// incoming one, then caller-saved registers unless it crosses a call.
// Emission binds the values live into a block on entry, binds a value at
// its definition, and frees its register after its last use in a block it
// does not leave live; block-local values avoid the registers of values
// defined inside their intervals. Work: O((instructions + live blocks +
// ranges) log) per function; no fixed point.
namespace native {

// PressureTree (allocation_structures.h): the counts and the registers
// assigned over ranges.
void PressureTree::build(const std::vector<int32_t>& base)
{
	size_ = 1;
	while (size_ < base.size())
		size_ <<= 1;
	max_.assign(2 * size_, 0);
	add_.assign(2 * size_, 0);
	cover_.assign(2 * size_, 0);
	below_.assign(2 * size_, 0);
	for (size_t i = 0; i < base.size(); ++i)
		max_[size_ + i] = base[i];
	for (size_t i = size_; i-- > 1;)
		max_[i] = max_[2 * i] > max_[2 * i + 1] ? max_[2 * i] : max_[2 * i + 1];
}

int32_t PressureTree::max(uint32_t first, uint32_t last) const
{
	if (first > last || last >= size_)
		return query(1, 0, size_ - 1, first, last);
	const int32_t none = INT32_MIN / 2;
	int32_t left = none, right = none;
	size_t l = first + size_, r = last + size_;
	size_t la = first + size_ - 1, ra = last + size_ + 1; // (valid past the ends: no adds there)
	const bool left_edge = first != 0, right_edge = last + 1 != size_;
	while (l <= r)
	{
		if (l & 1)
		{
			left = max_[l] > left ? max_[l] : left;
			++l;
		}
		if (!(r & 1))
		{
			right = max_[r] > right ? max_[r] : right;
			--r;
		}
		l >>= 1;
		r >>= 1;
		la >>= 1;
		ra >>= 1;
		if (left_edge && la != 0 && left != none)
			left += add_[la];
		if (right_edge && ra != 0 && right != none)
			right += add_[ra];
	}
	// Above the last level both sides share their ancestors.
	for (; la > 1 && left_edge;)
	{
		la >>= 1;
		if (left != none)
			left += add_[la];
	}
	for (; ra > 1 && right_edge;)
	{
		ra >>= 1;
		if (right != none)
			right += add_[ra];
	}
	return left > right ? left : right;
}

void PressureTree::add(uint32_t first, uint32_t last, int32_t delta, uint32_t regs)
{
	if (first > last || last >= size_)
	{
		update(1, 0, size_ - 1, first, last, delta, regs);
		return;
	}
	size_t l = first + size_, r = last + size_;
	const size_t l0 = l, r0 = r;
	auto cover = [&](size_t node) {
		max_[node] += delta;
		add_[node] += delta;
		cover_[node] |= regs;
		below_[node] |= regs;
	};
	while (l <= r)
	{
		if (l & 1)
			cover(l++);
		if (!(r & 1))
			cover(r--);
		l >>= 1;
		r >>= 1;
	}
	for (size_t i = l0 >> 1; i != 0; i >>= 1)
	{
		max_[i] = (max_[2 * i] > max_[2 * i + 1] ? max_[2 * i] : max_[2 * i + 1]) + add_[i];
		below_[i] = below_[2 * i] | below_[2 * i + 1] | cover_[i];
	}
	for (size_t i = r0 >> 1; i != 0; i >>= 1)
	{
		max_[i] = (max_[2 * i] > max_[2 * i + 1] ? max_[2 * i] : max_[2 * i + 1]) + add_[i];
		below_[i] = below_[2 * i] | below_[2 * i + 1] | cover_[i];
	}
}

uint32_t PressureTree::regs(uint32_t first, uint32_t last) const
{
	if (first > last || last >= size_)
		return regs_of(1, 0, size_ - 1, first, last);
	uint32_t mask = 0;
	bool left = false, right = false;
	size_t l = first + size_, r = last + size_;
	size_t la = first + size_ - 1, ra = last + size_ + 1;
	const bool left_edge = first != 0, right_edge = last + 1 != size_;
	while (l <= r)
	{
		if (l & 1)
		{
			mask |= below_[l++];
			left = true;
		}
		if (!(r & 1))
		{
			mask |= below_[r--];
			right = true;
		}
		l >>= 1;
		r >>= 1;
		la >>= 1;
		ra >>= 1;
		if (left && left_edge && la != 0)
			mask |= cover_[la];
		if (right && right_edge && ra != 0)
			mask |= cover_[ra];
	}
	for (; la > 1 && left && left_edge;)
		mask |= cover_[la >>= 1];
	for (; ra > 1 && right && right_edge;)
		mask |= cover_[ra >>= 1];
	return mask;
}

int32_t PressureTree::query(size_t node, size_t lo, size_t hi, size_t first, size_t last) const
{
	if (last < lo || hi < first)
		return INT32_MIN / 2;
	if (first <= lo && hi <= last)
		return max_[node];
	const size_t mid = (lo + hi) / 2;
	const int32_t a = query(2 * node, lo, mid, first, last);
	const int32_t b = query(2 * node + 1, mid + 1, hi, first, last);
	return (a > b ? a : b) + add_[node];
}

void PressureTree::update(size_t node, size_t lo, size_t hi, size_t first, size_t last, int32_t delta, uint32_t regs)
{
	if (last < lo || hi < first)
		return;
	if (first <= lo && hi <= last)
	{
		max_[node] += delta;
		add_[node] += delta;
		cover_[node] |= regs;
		below_[node] |= regs;
		return;
	}
	const size_t mid = (lo + hi) / 2;
	update(2 * node, lo, mid, first, last, delta, regs);
	update(2 * node + 1, mid + 1, hi, first, last, delta, regs);
	max_[node] = (max_[2 * node] > max_[2 * node + 1] ? max_[2 * node] : max_[2 * node + 1]) + add_[node];
	below_[node] = below_[2 * node] | below_[2 * node + 1] | cover_[node];
}

uint32_t PressureTree::regs_of(size_t node, size_t lo, size_t hi, size_t first, size_t last) const
{
	if (last < lo || hi < first)
		return 0;
	if (first <= lo && hi <= last)
		return below_[node];
	const size_t mid = (lo + hi) / 2;
	return regs_of(2 * node, lo, mid, first, last) | regs_of(2 * node + 1, mid + 1, hi, first, last) | cover_[node];
}

using lowir::Instruction;
using lowir::kNoId;
using lowir::Opcode;
using lowir::TypeKind;
using lowir::Value;
using lowir::ValueKind;

namespace {

const int32_t kGprCapacity = 12;
// The weighted use count (its definition and uses) at which a value opens
// a callee-saved register, whose push and pop run once per call: two
// executions in a loop, one outside loops.
const uint64_t kOpenWeight = 2 * kWeightScale;
const uint64_t kOpenWeightStraight = 2 * kWeightScale;
const int32_t kXmmCapacity = 14;
// The loop discovery's work budget per block, plus a fixed allowance.
const uint64_t kLoopStepsPerBlock = 16;
const uint64_t kLoopSlack = 4096;
// The liveness walks' budget: global_allocation.cpp's (uses and edges per
// instruction and block, plus a fixed allowance).
const uint64_t kLivenessStepsPerItem = 64;
const uint64_t kLivenessSlack = 4096;
const Reg kGlobalCallerOrder[] = { R8, R9, RDI, RSI, RDX, RCX, RAX };
const Reg kGlobalCalleeOrder[] = { RBX, R12, R13, R14, R15, RBP }; // RBP: rbp_free_ only

inline int32_t read_slot(int32_t base, int32_t pos)
{
	return base + 2 + 2 * pos;
}
inline int32_t write_slot(int32_t base, int32_t pos)
{
	return base + 3 + 2 * pos;
}

// An integer, f32 or f64 Binary or Unary computes its result from its first
// operand.
bool ties_first_operand(const Instruction& inst)
{
	return (inst.op == Opcode::Binary || inst.op == Opcode::Unary) && inst.type.kind() != TypeKind::F80 &&
		inst.type.kind() != TypeKind::I128;
}

// Block-local values live per slot, by class (the registers the
// emission-time allocator needs besides the function-level values), in
// the pressure trees.
void local_pressure(SelectorState& state)
{
	const size_t slots = static_cast<size_t>(state.ra_base_.back()) + 1;
	std::vector<int32_t> gpr(slots + 1, 0), xmm(slots + 1, 0);
	for (uint32_t temp = 0; temp < state.temps_.size(); ++temp)
	{
		const TempInfo& t = state.temps_[temp];
		if ((t.flags &
				(TF_GLOBAL | TF_ALIAS | TF_DEAD | TF_LAZY | TF_FOLDED | TF_FUSED | TF_TESTED | TF_ROTATED | TF_RMW)) ||
			t.block == kNoId || t.uses == 0 || (t.cls != VClass::Gpr && t.cls != VClass::Xmm) ||
			!state.blocks_[t.block].reachable)
			continue;
		const int32_t base = state.ra_base_[t.block];
		const int32_t first = t.def < 0 ? base : write_slot(base, t.def);
		const int32_t past = write_slot(base, t.last < t.def ? t.def : t.last) + 1;
		std::vector<int32_t>& diff = t.cls == VClass::Gpr ? gpr : xmm;
		++diff[static_cast<size_t>(first)];
		--diff[static_cast<size_t>(past)];
	}
	for (size_t s = 1; s <= slots; ++s)
	{
		gpr[s] += gpr[s - 1];
		xmm[s] += xmm[s - 1];
	}
	gpr.resize(slots);
	xmm.resize(slots);
	state.ra_gpr_pressure_.build(gpr);
	state.ra_xmm_pressure_.build(xmm);
}

// Spilling around loops (loop_split.h): for each register candidate c
// could take, the blocks of its ranges where the register is taken,
// clobbered or over pressure; feasible when all lie in the splitter's
// candidate loops. The register whose split around just the loops holding
// those blocks costs least: its conflict blocks in `best` (false: none).
// With `homes`, the split homes the value in those blocks themselves.
bool split_choice(
	SelectorState& s, uint32_t c, uint32_t allowed, int32_t capacity, bool homes, std::vector<uint32_t>& best)
{
	uint64_t best_cost = ~0ull;
	std::vector<uint32_t> conflicts;
	// Per range: the registers clobbered in it, those assigned over it,
	// and whether its pressure is full (the same for every register: found
	// on first need).
	const uint32_t first = s.ra_range_first_[c], past = s.ra_range_first_[c + 1];
	std::vector<uint32_t> clobbers(past - first, 0), taken_regs(past - first, 0);
	std::vector<uint8_t> known(past - first, 0); // 1: not full, 2: full
	for (uint32_t bits = allowed; bits != 0; bits &= bits - 1)
	{
		const unsigned reg = static_cast<unsigned>(__builtin_ctz(bits));
		conflicts.clear();
		bool feasible = true;
		for (uint32_t r = first; r < past && feasible; ++r)
		{
			const int32_t lo = s.ra_ranges_[r].first, hi = s.ra_ranges_[r].second;
			if (known[r - first] == 0)
			{
				clobbers[r - first] = s.ra_clobber_.query(lo + 1, hi - 1) | s.ra_early_.query(lo + 1, hi);
				taken_regs[r - first] = s.ra_gpr_pressure_.regs(static_cast<uint32_t>(lo), static_cast<uint32_t>(hi));
				known[r - first] =
					s.ra_gpr_pressure_.max(static_cast<uint32_t>(lo), static_cast<uint32_t>(hi)) + 1 > capacity ? 2 : 1;
			}
			const bool taken = (taken_regs[r - first] >> reg) & 1u;
			const bool clobbered = (clobbers[r - first] >> reg) & 1u;
			const bool full = known[r - first] == 2;
			if (!taken && !clobbered && !full)
				continue;
			const uint32_t b = s.ra_range_block_[r];
			feasible = homes || s.loop_splitter_.inside(b);
			conflicts.push_back(b);
		}
		if (!feasible || conflicts.empty() ||
			!(homes ? s.loop_splitter_.plan_homes(s, c, conflicts, best_cost)
					: s.loop_splitter_.plan(s, c, &conflicts)))
			continue;
		if (s.loop_splitter_.cost < best_cost)
		{
			best_cost = s.loop_splitter_.cost;
			best = conflicts;
		}
	}
	return best_cost != ~0ull;
}

// Whether the ranges [first, past) of ra_ranges_ are pairwise disjoint
// (sorting them in place).
bool disjoint_ranges(std::vector<std::pair<int32_t, int32_t>>& ranges, uint32_t first, uint32_t past)
{
	std::sort(ranges.begin() + first, ranges.begin() + past);
	for (uint32_t r = first + 1; r < past; ++r)
		if (ranges[r].first <= ranges[r - 1].second)
			return false;
	return true;
}

} // namespace

// The blocks whose calls or resumes enter each host landing, in CSR form
// (none for the course model, whose functions with regions keep homes).
// False when such a landing has phis, which no edge transfer can feed.
bool FunctionSelector::exceptional_edges()
{
	const lowir::FunctionBody& body = *body_;
	const uint32_t n = static_cast<uint32_t>(blocks_.size());
	ra_exc_first_.assign(n + 1, 0);
	ra_exc_preds_.clear();
	if (eh_model_ != EhModel::Host || !has_eh_)
		return true;
	std::vector<uint32_t> landing_block;
	for (uint32_t b = 0; b < n; ++b)
	{
		const uint32_t index = eh_landing_index_.empty() ? kNoId : eh_landing_index_[b];
		if (index == kNoId)
			continue;
		if (landing_block.size() <= index)
			landing_block.resize(index + 1, kNoId);
		landing_block[index] = b;
	}
	std::vector<std::pair<uint32_t, uint32_t>> edges;
	for (uint32_t b = 0; b < n; ++b)
	{
		if (!blocks_[b].reachable)
			continue;
		for (lowir::InstId i = body.blocks[blocks_[b].id].first; i != kNoId; i = body.instructions[i].next)
		{
			const uint32_t site = eh_sites_[i];
			if (!live(i) || site == 0)
				continue;
			const Opcode op = body.instructions[i].op;
			uint32_t to = kNoId;
			if (op == Opcode::Call && site - 1 < landing_block.size())
				to = landing_block[site - 1];
			else if ((op == Opcode::Resume || op == Opcode::Jump) && site != kResumeTerminal)
				to = site - 1;
			if (to == kNoId)
				continue;
			if (blocks_[to].has_phi)
				return false;
			edges.push_back(std::make_pair(to, b));
		}
	}
	std::sort(edges.begin(), edges.end());
	edges.erase(std::unique(edges.begin(), edges.end()), edges.end());
	for (size_t k = 0; k < edges.size(); ++k)
		++ra_exc_first_[edges[k].first + 1];
	for (uint32_t b = 0; b < n; ++b)
		ra_exc_first_[b + 1] += ra_exc_first_[b];
	ra_exc_preds_.resize(edges.size());
	for (size_t k = 0; k < edges.size(); ++k)
		ra_exc_preds_[k] = edges[k].second;
	return true;
}

// Loop nesting depth per block: natural loops of the back edges a depth-
// first walk from the entry finds (edges to a block on the walk's stack),
// each header's body once; work-capped (later loops then count less).
void FunctionSelector::loop_depths()
{
	const uint32_t n = static_cast<uint32_t>(blocks_.size());
	ra_depth_.assign(n, 0);
	if (n == 0)
		return;
	std::vector<uint8_t> state(n, 0);
	std::vector<uint32_t> next(n, 0);
	std::vector<std::pair<uint32_t, uint32_t>> back; // (header, latch)
	std::vector<uint32_t> stack(1, 0);
	state[0] = 1;
	next[0] = succ_first_[0];
	while (!stack.empty())
	{
		const uint32_t b = stack.back();
		if (next[b] == succ_first_[b + 1])
		{
			state[b] = 2;
			stack.pop_back();
			continue;
		}
		const uint32_t s = succs_[next[b]++];
		if (state[s] == 1)
			back.push_back(std::make_pair(s, b));
		else if (state[s] == 0 && blocks_[s].reachable)
		{
			state[s] = 1;
			next[s] = succ_first_[s];
			stack.push_back(s);
		}
	}
	std::sort(back.begin(), back.end());
	std::vector<uint32_t> mark(n, 0);
	std::vector<uint32_t> work;
	std::vector<uint32_t>& loop_first = loop_splitter_.loop_first;
	std::vector<uint32_t>& loop_blocks = loop_splitter_.loop_blocks;
	loop_first.assign(1, 0);
	loop_blocks.clear();
	const uint64_t budget = kLoopStepsPerBlock * n + kLoopSlack;
	uint64_t steps = 0;
	uint32_t epoch = 0;
	for (size_t k = 0; k < back.size() && steps <= budget;)
	{
		const uint32_t header = back[k].first;
		++epoch;
		mark[header] = epoch;
		++ra_depth_[header];
		loop_blocks.push_back(header);
		for (; k < back.size() && back[k].first == header; ++k)
		{
			const uint32_t latch = back[k].second;
			if (mark[latch] != epoch)
			{
				mark[latch] = epoch;
				++ra_depth_[latch];
				work.push_back(latch);
				loop_blocks.push_back(latch);
			}
		}
		while (!work.empty() && steps <= budget)
		{
			const uint32_t x = work.back();
			work.pop_back();
			for (uint32_t p = preds_first_[x]; p < preds_first_[x + 1]; ++p)
			{
				++steps;
				const uint32_t q = preds_[p];
				if (mark[q] == epoch)
					continue;
				mark[q] = epoch;
				++ra_depth_[q];
				work.push_back(q);
				loop_blocks.push_back(q);
			}
		}
		work.clear();
		// A walk the budget cut short records no loop.
		if (steps > budget)
			loop_blocks.resize(loop_first.back());
		else
			loop_first.push_back(static_cast<uint32_t>(loop_blocks.size()));
	}
	// Cold blocks: ending in a throw, resume or unreachable, holding a
	// noreturn call, or with only cold successors (their uses weigh
	// nothing in the registers' choice).
	const lowir::FunctionBody& body = *body_;
	ra_cold_.assign(n, 0);
	for (uint32_t b = 0; b < n; ++b)
		for (lowir::InstId i = body.blocks[blocks_[b].id].first; i != kNoId && !ra_cold_[b];
			i = body.instructions[i].next)
		{
			const Instruction& inst = body.instructions[i];
			ra_cold_[b] = inst.op == Opcode::Throw || inst.op == Opcode::Resume || inst.op == Opcode::Unreachable ||
				(inst.op == Opcode::Call && program_.may_end_block(inst));
		}
	for (bool changed = true; changed;)
	{
		changed = false;
		for (uint32_t b = n; b-- > 0;)
		{
			if (ra_cold_[b] || succ_first_[b] == succ_first_[b + 1])
				continue;
			bool all = true;
			for (uint32_t k = succ_first_[b]; k < succ_first_[b + 1] && all; ++k)
				all = ra_cold_[succs_[k]] != 0;
			if (all)
			{
				ra_cold_[b] = 1;
				changed = true;
			}
		}
	}
	block_weights();
}

// Each block's estimated executions per call (Wu and Larus, in fixed
// point): a block's flow splits evenly among its successors that are not
// cold, except that edges leaving its innermost loop share 1/10 of it;
// each loop, innermost first, finds the flow back to its header for one
// entering (at most 0.9), and a header's flow is what enters it over the
// rest (one exit test: ten passes). A block that is not cold weighs at
// least 1. Work: O(blocks + edges) per loop level.
void FunctionSelector::block_weights()
{
	const uint64_t kOne = static_cast<uint64_t>(1) << 16; // probability and frequency unit
	const uint64_t kMaxBack = kOne * 9 / 10;
	const uint64_t kMaxFrequency = kOne << 20;
	const uint32_t n = static_cast<uint32_t>(blocks_.size());
	loop_splitter_.weights.assign(n, 0);
	if (n == 0)
		return;
	// Reverse postorder, and back edges (to a block on the walk's stack).
	// (The scratch persists for its capacity: each vector is assigned before use.)
	static std::vector<uint8_t> state, back, in_back;
	static std::vector<uint32_t> next, post, stack, order, by_size, inner, parent, header_of, stamp, blocks;
	static std::vector<uint64_t> prob, in_prob, again, flow;
	state.assign(n, 0);
	back.assign(succs_.size(), 0);
	next.assign(n, 0);
	post.clear();
	stack.assign(1, 0);
	state[0] = 1;
	next[0] = succ_first_[0];
	while (!stack.empty())
	{
		const uint32_t b = stack.back();
		if (next[b] == succ_first_[b + 1])
		{
			state[b] = 2;
			post.push_back(b);
			stack.pop_back();
			continue;
		}
		const uint32_t k = next[b]++;
		const uint32_t s = succs_[k];
		if (state[s] == 1)
			back[k] = 1;
		else if (state[s] == 0 && blocks_[s].reachable)
		{
			state[s] = 1;
			next[s] = succ_first_[s];
			stack.push_back(s);
		}
	}
	order.assign(n, kNoId); // reverse postorder index
	for (size_t k = 0; k < post.size(); ++k)
		order[post[post.size() - 1 - k]] = static_cast<uint32_t>(k);
	// loop_depths' natural loops, smallest first; each block's innermost
	// and each loop's parent (the smallest holding its header).
	const std::vector<uint32_t>& first = loop_splitter_.loop_first;
	const std::vector<uint32_t>& members = loop_splitter_.loop_blocks;
	const uint32_t loops = first.empty() ? 0 : static_cast<uint32_t>(first.size() - 1);
	by_size.resize(loops);
	for (uint32_t l = 0; l < loops; ++l)
		by_size[l] = l;
	std::sort(by_size.begin(), by_size.end(), [&](uint32_t a, uint32_t b) {
		const uint32_t sa = first[a + 1] - first[a], sb = first[b + 1] - first[b];
		return sa != sb ? sa < sb : a < b;
	});
	inner.assign(n, kNoId);
	parent.assign(loops, kNoId);
	header_of.assign(n, kNoId);
	for (uint32_t k = loops; k-- > 0;)
	{
		const uint32_t l = by_size[k];
		const uint32_t h = members[first[l]];
		if (inner[h] != kNoId)
			parent[l] = inner[h];
		for (uint32_t i = first[l]; i < first[l + 1]; ++i)
			inner[members[i]] = l;
		header_of[h] = l;
	}
	auto contains = [&](uint32_t l, uint32_t b) {
		for (uint32_t x = inner[b]; x != kNoId; x = parent[x])
			if (x == l)
				return true;
		return false;
	};
	// Edge probabilities, by successor index.
	prob.assign(succs_.size(), 0);
	for (uint32_t b = 0; b < n; ++b)
	{
		uint32_t warm = 0, exits = 0;
		for (uint32_t k = succ_first_[b]; k < succ_first_[b + 1]; ++k)
			if (!ra_cold_[succs_[k]])
			{
				++warm;
				exits += inner[b] != kNoId && !contains(inner[b], succs_[k]) ? 1 : 0;
			}
		for (uint32_t k = succ_first_[b]; k < succ_first_[b + 1]; ++k)
		{
			if (ra_cold_[succs_[k]])
				continue;
			const bool exit = inner[b] != kNoId && !contains(inner[b], succs_[k]);
			if (exits == 0 || exits == warm)
				prob[k] = kOne / warm;
			else
				prob[k] = exit ? kOne / 10 / exits : (kOne - kOne / 10) / (warm - exits);
		}
	}
	// The same by predecessor entry (preds_), with each edge's back mark.
	in_prob.assign(preds_.size(), 0);
	in_back.assign(preds_.size(), 0);
	for (uint32_t b = 0; b < n; ++b)
		for (uint32_t p = preds_first_[b]; p < preds_first_[b + 1]; ++p)
			for (uint32_t k = succ_first_[preds_[p]]; k < succ_first_[preds_[p] + 1]; ++k)
				if (succs_[k] == b)
				{
					in_prob[p] = prob[k];
					in_back[p] = back[k];
					break;
				}
	// The flow into b from its predecessors (those stamped `within` unless
	// that is 0) over forward edges, or over back edges.
	stamp.assign(n, 0);
	again.assign(n, 0); // a header's flow back per entering
	flow.assign(n, 0);
	auto inflow = [&](uint32_t b, uint32_t within, bool backward) {
		uint64_t f = 0;
		for (uint32_t p = preds_first_[b]; p < preds_first_[b + 1]; ++p)
			if ((within == 0 || stamp[preds_[p]] == within) && (in_back[p] != 0) == backward)
				f += flow[preds_[p]] * in_prob[p] / kOne;
		return f;
	};
	auto settle = [&](uint32_t b, uint64_t f) {
		f = std::min(f, kMaxFrequency);
		if (header_of[b] != kNoId)
			f = std::min(f * kOne / (kOne - again[b]), kMaxFrequency);
		return f;
	};
	for (uint32_t l : by_size)
	{
		const uint32_t h = members[first[l]];
		blocks.assign(members.begin() + first[l], members.begin() + first[l + 1]);
		std::sort(blocks.begin(), blocks.end(), [&](uint32_t a, uint32_t b) { return order[a] < order[b]; });
		for (uint32_t b : blocks)
		{
			flow[b] = 0;
			stamp[b] = l + 1;
		}
		flow[h] = kOne;
		for (uint32_t b : blocks)
			if (b != h && order[b] != kNoId)
				flow[b] = settle(b, inflow(b, l + 1, false));
		again[h] = std::min(inflow(h, l + 1, true), kMaxBack);
	}
	for (size_t k = post.size(); k-- > 0;)
	{
		const uint32_t b = post[k];
		flow[b] = settle(b, (b == 0 ? kOne : 0) + inflow(b, 0, false));
	}
	for (uint32_t b = 0; b < n; ++b)
		if (!ra_cold_[b] && blocks_[b].reachable)
			loop_splitter_.weights[b] = std::max<uint64_t>(1, (flow[b] * kWeightScale + kOne / 2) / kOne);
}

// Each candidate's uses as (block, slot) in CSR form: a lazy address's
// parts where it folds, a phi operand at its predecessor's transfer read.
void FunctionSelector::collect_uses(const std::vector<int32_t>& index)
{
	const lowir::FunctionBody& body = *body_;
	std::vector<std::pair<uint32_t, int32_t>> uses;
	std::vector<uint32_t> owner;
	auto record = [&](Value v, uint32_t b, int32_t early, int32_t late, bool tied) {
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
			tied = false;
		}
		for (unsigned k = 0; k < 3; ++k)
		{
			if (parts[k] == kNoId || index[parts[k]] < 0)
				continue;
			uses.push_back(std::make_pair(b, tied && k == 0 ? early : late));
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
				if (pred == kNoId || !blocks_[pred].reachable)
					continue;
				const int32_t at = read_slot(ra_base_[pred], static_cast<int32_t>(blocks_[pred].count));
				record(body.operands[inst.extra_first + k + 1], pred, at, at, false);
			}
			return;
		}
		const int32_t pos = read_pos(id);
		const int32_t early = read_slot(ra_base_[b], pos);
		const int32_t late = write_slot(ra_base_[b], pos);
		if (ties_first_operand(inst))
		{
			record(inst.ops[0], b, early, late, true);
			if (inst.op == Opcode::Binary)
				record(inst.ops[1], b, early, late, false);
			return;
		}
		for_each_operand(body, inst, [&](Value v, OperandRole) { record(v, b, early, late, false); });
	});
	ra_use_first_.assign(ra_candidates_.size() + 1, 0);
	for (size_t u = 0; u < owner.size(); ++u)
		++ra_use_first_[owner[u] + 1];
	for (size_t c = 0; c < ra_candidates_.size(); ++c)
		ra_use_first_[c + 1] += ra_use_first_[c];
	ra_uses_.resize(uses.size());
	std::vector<uint32_t> fill(ra_use_first_.begin(), ra_use_first_.end() - 1);
	for (size_t u = 0; u < owner.size(); ++u)
		ra_uses_[fill[owner[u]]++] = uses[u];
}

// Each candidate's live ranges (one per block, by slot) and use weight;
// candidates from the one whose walk exhausts the budget keep homes.
void FunctionSelector::live_ranges()
{
	const lowir::FunctionBody& body = *body_;
	const uint32_t n = static_cast<uint32_t>(blocks_.size());
	std::vector<uint32_t> seen(n, 0), in(n, 0), out(n, 0), transfer(n, 0);
	std::vector<int32_t> last(n, -1);
	std::vector<uint32_t> present, work;
	uint64_t items = 0;
	for (uint32_t b = 0; b < n; ++b)
		if (blocks_[b].reachable)
			items += blocks_[b].count + 1;
	const uint64_t budget = kLivenessStepsPerItem * items + kLivenessSlack;
	uint64_t steps = 0;
	ra_range_first_.assign(1, 0);
	ra_ranges_.clear();
	ra_range_block_.clear();
	ra_freq_.assign(ra_candidates_.size(), 0);
	ra_looped_.assign(ra_candidates_.size(), 0);
	for (size_t c = 0; c < ra_candidates_.size(); ++c)
	{
		const uint32_t temp = ra_candidates_[c];
		const TempInfo& t = temps_[temp];
		const bool param = temp < body.parameter_count;
		const bool phi = !param && body.instructions[body.temps[temp].def].op == Opcode::Phi;
		const uint32_t def_block = param ? kNoId : t.block;
		const uint32_t epoch = static_cast<uint32_t>(c + 1);
		present.clear();
		auto touch = [&](uint32_t b) {
			if (seen[b] != epoch)
			{
				seen[b] = epoch;
				last[b] = -1;
				present.push_back(b);
			}
		};
		auto live_in = [&](uint32_t b) {
			touch(b);
			if (in[b] != epoch)
			{
				in[b] = epoch;
				work.push_back(b);
			}
		};
		uint64_t freq = def_block == kNoId ? kWeightScale : loop_splitter_.weights[def_block];
		bool looped = def_block != kNoId && ra_depth_[def_block] != 0;
		steps += ra_use_first_[c + 1] - ra_use_first_[c];
		for (uint32_t u = ra_use_first_[c]; u < ra_use_first_[c + 1]; ++u)
		{
			const uint32_t b = ra_uses_[u].first;
			touch(b);
			last[b] = std::max(last[b], ra_uses_[u].second);
			freq += loop_splitter_.weights[b];
			looped = looped || ra_depth_[b] != 0;
			if (b != def_block)
				live_in(b);
		}
		if (phi)
		{
			// Live into its block (not through it to the predecessors), and
			// written by each predecessor's transfer.
			touch(def_block);
			in[def_block] = epoch;
			for (uint32_t k = preds_first_[def_block]; k < preds_first_[def_block + 1]; ++k)
			{
				touch(preds_[k]);
				transfer[preds_[k]] = epoch;
			}
		}
		if (def_block != kNoId)
			touch(def_block);
		while (!work.empty() && steps <= budget)
		{
			const uint32_t x = work.back();
			work.pop_back();
			const uint32_t lists[2][2] = { { preds_first_[x], preds_first_[x + 1] },
				{ ra_exc_first_[x], ra_exc_first_[x + 1] } };
			for (unsigned l = 0; l < 2; ++l)
			{
				steps += lists[l][1] - lists[l][0];
				for (uint32_t k = lists[l][0]; k < lists[l][1]; ++k)
				{
					const uint32_t q = l == 0 ? preds_[k] : ra_exc_preds_[k];
					touch(q);
					out[q] = epoch;
					if (q != def_block)
						live_in(q);
				}
			}
		}
		if (steps > budget)
		{
			work.clear();
			stats_.hull_fallbacks += ra_candidates_.size() - c;
			ra_candidates_.resize(c);
			break;
		}
		std::sort(present.begin(), present.end());
		for (size_t k = 0; k < present.size(); ++k)
		{
			const uint32_t b = present[k];
			const int32_t base = ra_base_[b];
			const int32_t count = static_cast<int32_t>(blocks_[b].count);
			const bool live_out = out[b] == epoch;
			const int32_t written = write_slot(base, count);
			// A predecessor ending in a conditional terminator writes the
			// phi in its edge block, where only the values live into the
			// phi's block are (they meet the phi at that block's entry):
			// no range at its transfer.
			const bool edge_write = phi && transfer[b] == epoch && succ_first_[b + 1] - succ_first_[b] > 1;
			if (edge_write && !live_out)
			{
				if (in[b] == epoch)
				{
					ra_ranges_.push_back(std::make_pair(base, std::max(base, last[b])));
					ra_range_block_.push_back(b);
				}
				continue;
			}
			// A phi live into a predecessor that writes it by its transfer
			// and leaves it otherwise dead: dead between its last use and
			// the transfer (the next value can take its register).
			if (transfer[b] == epoch && !live_out && in[b] == epoch)
			{
				ra_ranges_.push_back(std::make_pair(base, std::max(base, last[b])));
				ra_range_block_.push_back(b);
				ra_ranges_.push_back(std::make_pair(written, written));
				ra_range_block_.push_back(b);
				continue;
			}
			int32_t start = base;
			if (in[b] != epoch)
				start = b == def_block && !phi ? write_slot(base, t.def) : written;
			int32_t end = live_out || transfer[b] == epoch ? written : last[b];
			if (end < start)
				end = start;
			ra_ranges_.push_back(std::make_pair(start, end));
			ra_range_block_.push_back(b);
		}
		ra_range_first_.push_back(static_cast<uint32_t>(ra_ranges_.size()));
		ra_freq_[c] = freq;
		ra_looped_[c] = looped ? 1 : 0;
	}
	stats_.liveness_steps += steps;
}

// The registers assignment: candidates by weight; see the file comment.
void FunctionSelector::assign_registers()
{
	const lowir::FunctionBody& body = *body_;
	const size_t m = ra_candidates_.size();
	std::vector<int32_t> cand(temps_.size(), -1);
	for (size_t c = 0; c < m; ++c)
		cand[ra_candidates_[c]] = static_cast<int32_t>(c);
	// Phi webs (union-find) and each result's tied first operand.
	std::vector<uint32_t> group(m);
	for (size_t c = 0; c < m; ++c)
		group[c] = static_cast<uint32_t>(c);
	auto find = [&](uint32_t c) {
		while (group[c] != c)
			c = group[c] = group[group[c]];
		return c;
	};
	std::vector<int32_t> tie(m, -1);
	for (size_t c = 0; c < m; ++c)
	{
		const uint32_t temp = ra_candidates_[c];
		if (temp < body.parameter_count)
			continue;
		const Instruction& def = body.instructions[body.temps[temp].def];
		if (def.op == Opcode::Phi)
		{
			for (uint32_t k = 1; k < def.extra_count; k += 2)
			{
				const Value v = resolve(body.operands[def.extra_first + k]);
				if (v.kind == ValueKind::Temp && cand[v.index] >= 0)
					group[find(static_cast<uint32_t>(cand[v.index]))] = find(static_cast<uint32_t>(c));
			}
		}
		else if (ties_first_operand(def))
		{
			const Value v = resolve(def.ops[0]);
			if (v.kind == ValueKind::Temp && cand[v.index] >= 0)
				tie[c] = cand[v.index];
		}
	}
	// Use hints: a returned value prefers the return register, a call
	// argument in one register that register.
	ra_use_hint_.assign(m, kNoReg);
	for_each_inst([&](uint32_t id, uint32_t) {
		const Instruction& inst = body.instructions[id];
		if (marks_[id] & IM_DEAD)
			return;
		auto hint = [&](Value v, Reg reg) {
			v = resolve(v);
			if (v.kind == ValueKind::Temp && cand[v.index] >= 0 &&
				ra_use_hint_[static_cast<size_t>(cand[v.index])] == kNoReg)
				ra_use_hint_[static_cast<size_t>(cand[v.index])] = reg;
		};
		if (inst.op == Opcode::Return && inst.ops[0].kind != ValueKind::None && !inst.type.is_object())
			hint(inst.ops[0], inst.type.is_float() ? XMM0 : RAX);
		else if (inst.op == Opcode::Call)
		{
			const lowir::Signature* sig = program_.call_boundary(inst);
			if (sig == nullptr)
				return;
			const CallLayout& layout = emission_layout(id, sig, false);
			for (uint32_t a = 0; a < inst.extra_count; ++a)
			{
				const ArgLayout& arg = layout.args[a];
				if (arg.part_count == 1 && layout.parts[arg.first_part].reg != kNoReg &&
					(arg.cls == ArgClass::Gpr || arg.cls == ArgClass::Xmm))
					hint(body.operands[inst.extra_first + a], layout.parts[arg.first_part].reg);
			}
		}
	});
	std::vector<uint32_t> order(m);
	std::vector<int32_t> span(m, 0);
	for (size_t c = 0; c < m; ++c)
	{
		order[c] = static_cast<uint32_t>(c);
		for (uint32_t r = ra_range_first_[c]; r < ra_range_first_[c + 1]; ++r)
			span[c] += ra_ranges_[r].second - ra_ranges_[r].first + 1;
	}
	std::sort(order.begin(), order.end(), [&](uint32_t a, uint32_t b) {
		if (ra_freq_[a] != ra_freq_[b])
			return ra_freq_[a] > ra_freq_[b];
		if (span[a] != span[b])
			return span[a] < span[b];
		return ra_candidates_[a] < ra_candidates_[b];
	});
	local_pressure(*this);
	std::vector<Reg> group_reg(m, kNoReg);
	// Spilled-around-loop candidates' ranges, appended (loop_split.h).
	std::vector<std::pair<uint32_t, uint32_t>> split(m, std::make_pair(0u, 0u));
	std::vector<std::pair<uint32_t, uint32_t>> homed; // (block, temp)
	loop_splitter_.reset(*this);
	// Phi webs' members (CSR by root), for one register for a whole web.
	std::vector<uint32_t> web_first(m + 1, 0), web_members(m);
	for (size_t c = 0; c < m; ++c)
		++web_first[find(static_cast<uint32_t>(c)) + 1];
	for (size_t c = 0; c < m; ++c)
		web_first[c + 1] += web_first[c];
	{
		std::vector<uint32_t> fill(web_first.begin(), web_first.end() - 1);
		for (size_t c = 0; c < m; ++c)
			web_members[fill[find(static_cast<uint32_t>(c))]++] = static_cast<uint32_t>(c);
	}
	// The registers a split candidate may take (choose_register's, before
	// interference): the class's, callee-saved ones past those saved only
	// for values worth their saves.
	auto split_registers = [&](uint32_t c) {
		uint32_t allowed = (kAllocatableGprs | (rbp_free_ ? 1u << RBP : 0u)) & ~reserved_gprs(*out_);
		if (ra_freq_[c] < (ra_looped_[c] ? kOpenWeight : kOpenWeightStraight) &&
			(used_callee_saved_ & (kCalleeSavedGprs | (1u << RBP))) != 0)
			allowed &= ~((kCalleeSavedGprs | (1u << RBP)) & ~used_callee_saved_);
		return allowed;
	};
	auto commit = [&](uint32_t c, Reg reg, uint32_t first, uint32_t past) {
		const uint32_t temp = ra_candidates_[c];
		const bool gpr = temps_[temp].cls == VClass::Gpr;
		for (uint32_t r = first; r < past; ++r)
			(gpr ? ra_gpr_pressure_ : ra_xmm_pressure_)
				.add(static_cast<uint32_t>(ra_ranges_[r].first), static_cast<uint32_t>(ra_ranges_[r].second), 1,
					1u << reg);
		temps_[temp].greg = reg;
		global_temps_.push_back(temp);
		if (group_reg[find(c)] == kNoReg)
			group_reg[find(c)] = reg;
		if ((kCalleeSavedGprs | (1u << RBP)) & (1u << reg))
			used_callee_saved_ |= 1u << reg;
		++stats_.global_registers;
	};
	// The first member of a web to be assigned takes a register free over
	// every member's ranges when they are disjoint and one is: the web's
	// edge transfers then move nothing.
	auto assign_web = [&](uint32_t c, Reg tied) {
		const uint32_t root = find(c);
		if (web_first[root + 1] - web_first[root] < 2 || group_reg[root] != kNoReg)
			return false;
		const uint32_t first = static_cast<uint32_t>(ra_ranges_.size());
		uint64_t weight = 0;
		uint8_t looped = 0;
		for (uint32_t k = web_first[root]; k < web_first[root + 1]; ++k)
		{
			const uint32_t w = web_members[k];
			if (temps_[ra_candidates_[w]].cls != temps_[ra_candidates_[c]].cls)
				return false;
			weight += ra_freq_[w];
			looped |= ra_looped_[w];
			for (uint32_t r = ra_range_first_[w]; r < ra_range_first_[w + 1]; ++r)
				ra_ranges_.push_back(ra_ranges_[r]);
		}
		const uint32_t past = static_cast<uint32_t>(ra_ranges_.size());
		ra_range_block_.resize(past, 0);
		Reg reg = kNoReg;
		if (disjoint_ranges(ra_ranges_, first, past))
		{
			const uint64_t own = ra_freq_[c];
			const uint8_t own_looped = ra_looped_[c];
			ra_freq_[c] = weight;
			ra_looped_[c] = looped;
			reg = choose_register(c, first, past, kNoReg, tied);
			ra_freq_[c] = own;
			ra_looped_[c] = own_looped;
		}
		// choose_register keeps only c's own variable shift off rcx: a web
		// whose other member is one takes registers one by one instead.
		for (uint32_t k = web_first[root]; k < web_first[root + 1] && reg != kNoReg; ++k)
		{
			const uint32_t temp = ra_candidates_[web_members[k]];
			const lowir::InstId def = temp >= body_->parameter_count ? body_->temps[temp].def : lowir::kNoId;
			if (def == lowir::kNoId || body_->instructions[def].op != Opcode::Binary)
				continue;
			const lowir::BinaryOp op = static_cast<lowir::BinaryOp>(body_->instructions[def].sub);
			if ((op == lowir::BinaryOp::Shl || op == lowir::BinaryOp::Shr || op == lowir::BinaryOp::UShr) &&
				(early_[def] & (1u << reg)))
				reg = kNoReg;
		}
		ra_ranges_.resize(first);
		ra_range_block_.resize(first);
		if (reg == kNoReg)
			return false;
		for (uint32_t k = web_first[root]; k < web_first[root + 1]; ++k)
			commit(web_members[k], reg, ra_range_first_[web_members[k]], ra_range_first_[web_members[k] + 1]);
		return true;
	};
	for (size_t k = 0; k < m; ++k)
	{
		const uint32_t c = order[k];
		if (temps_[ra_candidates_[c]].greg != kNoReg)
			continue;
		const Reg tied = tie[c] >= 0 ? temps_[ra_candidates_[static_cast<size_t>(tie[c])]].greg : kNoReg;
		if (assign_web(c, tied))
			continue;
		uint32_t first = ra_range_first_[c], past = ra_range_first_[c + 1];
		Reg reg = choose_register(c, first, past, group_reg[find(c)], tied);
		// Else one outside the loops it skips, or the blocks clobbering it (a phi: not in its block or predecessors).
		const int32_t capacity = kGprCapacity + (rbp_free_ ? 1 : 0) - (reserved_gprs(*out_) != 0 ? 1 : 0);
		const uint32_t temp = ra_candidates_[c];
		std::vector<uint32_t> conflicts;
		bool homes = false;
		bool planned = reg == kNoReg && loop_splitter_.candidates(*this, c) &&
			split_choice(*this, c, split_registers(c), capacity, false, conflicts) &&
			loop_splitter_.plan(*this, c, &conflicts);
		if (reg == kNoReg && !planned && temps_[temp].cls == VClass::Gpr)
		{
			conflicts.clear();
			planned = homes = split_choice(*this, c, split_registers(c), capacity, true, conflicts) &&
				loop_splitter_.plan_homes(*this, c, conflicts);
		}
		if (planned)
		{
			first = static_cast<uint32_t>(ra_ranges_.size());
			ra_ranges_.insert(ra_ranges_.end(), loop_splitter_.ranges.begin(), loop_splitter_.ranges.end());
			ra_range_block_.insert(ra_range_block_.end(), loop_splitter_.blocks.begin(), loop_splitter_.blocks.end());
			past = static_cast<uint32_t>(ra_ranges_.size());
			reg = choose_register(c, first, past, group_reg[find(c)], tied);
			if (reg == kNoReg)
			{
				ra_ranges_.resize(first);
				ra_range_block_.resize(first);
			}
			else
			{
				split[c] = std::make_pair(first, past);
				for (SplitMove move : loop_splitter_.moves)
				{
					move.reg = reg;
					split_moves_.push_back(move);
				}
				if (homes)
					for (uint32_t b : loop_splitter_.homed)
						homed.push_back(std::make_pair(b, ra_candidates_[c]));
				++(homes ? stats_.home_splits : stats_.loop_splits);
			}
		}
		if (reg != kNoReg)
			commit(c, reg, first, past);
	}
	if (!split_moves_.empty() || !homed.empty())
		install_splits(split, homed);
}

// The split candidates' ranges (split, by candidate: their range in
// ra_ranges_, else none) replace their whole ones in the CSR; the values
// homed in some blocks, as (block, temp), go to split_homes_ by block.
void FunctionSelector::install_splits(
	const std::vector<std::pair<uint32_t, uint32_t>>& split, std::vector<std::pair<uint32_t, uint32_t>>& homed)
{
	split_homes_.first.assign(blocks_.size() + 1, 0);
	split_homes_.temps.clear();
	std::sort(homed.begin(), homed.end());
	for (const std::pair<uint32_t, uint32_t>& h : homed)
	{
		++split_homes_.first[h.first + 1];
		split_homes_.temps.push_back(h.second);
	}
	for (size_t b = 0; b < blocks_.size(); ++b)
		split_homes_.first[b + 1] += split_homes_.first[b];
	std::vector<uint32_t> range_first(1, 0), range_block;
	std::vector<std::pair<int32_t, int32_t>> ranges;
	for (size_t c = 0; c < ra_candidates_.size(); ++c)
	{
		const bool was_split = split[c].second != 0;
		const uint32_t first = was_split ? split[c].first : ra_range_first_[c];
		const uint32_t past = was_split ? split[c].second : ra_range_first_[c + 1];
		ranges.insert(ranges.end(), ra_ranges_.begin() + first, ra_ranges_.begin() + past);
		range_block.insert(range_block.end(), ra_range_block_.begin() + first, ra_range_block_.begin() + past);
		range_first.push_back(static_cast<uint32_t>(ranges.size()));
	}
	ra_range_first_.swap(range_first);
	ra_range_block_.swap(range_block);
	ra_ranges_.swap(ranges);
	std::sort(split_moves_.begin(), split_moves_.end(),
		[](const SplitMove& a, const SplitMove& b) { return a.from != b.from ? a.from < b.from : a.to < b.to; });
}

// Candidate c's register for its ranges [first, past): one no instruction
// inside them clobbers, no assigned range overlaps, with room in the
// pressure tree; the web's register, the tied operand's, a parameter's
// incoming register, then caller-saved before callee-saved (already saved
// ones first). kNoReg: a frame home.
Reg FunctionSelector::choose_register(uint32_t c, uint32_t first, uint32_t past, Reg web, Reg tied)
{
	const lowir::FunctionBody& body = *body_;
	const uint32_t temp = ra_candidates_[c];
	const TempInfo& t = temps_[temp];
	const bool gpr = t.cls == VClass::Gpr;
	uint32_t forbid = 0;
	for (uint32_t r = first; r < past; ++r)
	{
		const int32_t s = ra_ranges_[r].first, e = ra_ranges_[r].second;
		forbid |= ra_clobber_.query(s + 1, e - 1) | ra_early_.query(s + 1, e);
		const PressureTree& tree = gpr ? ra_gpr_pressure_ : ra_xmm_pressure_;
		if (tree.max(static_cast<uint32_t>(s), static_cast<uint32_t>(e)) + 1 >
			(gpr ? kGprCapacity + (rbp_free_ ? 1 : 0) - (reserved_gprs(*out_) != 0 ? 1 : 0) : kXmmCapacity))
			return kNoReg;
		// The registers an assigned range overlapping this one holds.
		forbid |= tree.regs(static_cast<uint32_t>(s), static_cast<uint32_t>(e));
	}
	// A variable shift writes its result before reading the count it
	// loaded into rcx: its result may not take that early clobber.
	if (temp >= body.parameter_count && t.def >= 0)
	{
		const lowir::InstId def = body.temps[temp].def;
		const Instruction& inst = body.instructions[def];
		const lowir::BinaryOp op = static_cast<lowir::BinaryOp>(inst.sub);
		if (inst.op == Opcode::Binary &&
			(op == lowir::BinaryOp::Shl || op == lowir::BinaryOp::Shr || op == lowir::BinaryOp::UShr))
			forbid |= early_[def];
	}
	uint32_t allowed =
		(gpr ? (kAllocatableGprs | (rbp_free_ ? 1u << RBP : 0u)) & ~reserved_gprs(*out_) : kAllocatableXmms) & ~forbid;
	// Each callee-saved register past the first costs a push and a pop per
	// call: a value worth fewer reloads than that (kOpenWeight) takes only
	// those already saved.
	if (gpr && ra_freq_[c] < (ra_looped_[c] ? kOpenWeight : kOpenWeightStraight) &&
		(used_callee_saved_ & (kCalleeSavedGprs | (1u << RBP))) != 0)
		allowed &= ~((kCalleeSavedGprs | (1u << RBP)) & ~used_callee_saved_);
	if (allowed == 0)
		return kNoReg;
	// A call's result is where the call returns it.
	Reg returned = kNoReg;
	if (temp >= body.parameter_count && body.temps[temp].def != kNoId &&
		body.instructions[body.temps[temp].def].op == Opcode::Call)
		returned = gpr ? RAX : XMM0;
	const Reg prefs[5] = { web, tied, temp < body.parameter_count ? incoming_reg(temp) : kNoReg, returned,
		ra_use_hint_[c] };
	for (unsigned k = 0; k < 5; ++k)
		if (prefs[k] != kNoReg && (allowed & (1u << prefs[k])))
			return prefs[k];
	if (!gpr)
		return static_cast<Reg>(__builtin_ctz(allowed));
	for (unsigned k = 0; k < 7; ++k)
		if (allowed & (1u << kGlobalCallerOrder[k]))
			return kGlobalCallerOrder[k];
	for (unsigned k = 0; k < 6; ++k)
		if ((allowed & used_callee_saved_) & (1u << kGlobalCalleeOrder[k]))
			return kGlobalCalleeOrder[k];
	for (unsigned k = 0; k < 6; ++k)
		if (allowed & (1u << kGlobalCalleeOrder[k]))
			return kGlobalCalleeOrder[k];
	return kNoReg;
}

// Emission facts: per block the assigned values present there (CSR in
// block_globals_first_ / block_entries_): bound on entry when live in, and
// the position after which the block frees the register (INT32_MAX when
// live out or read by the edge transfers); per block the registers of the
// values defined in it.
void FunctionSelector::emission_facts()
{
	const lowir::FunctionBody& body = *body_;
	const uint32_t n = static_cast<uint32_t>(blocks_.size());
	std::vector<std::pair<uint64_t, GlobalEntry>> entries;
	for (size_t c = 0; c < ra_candidates_.size(); ++c)
	{
		const uint32_t temp = ra_candidates_[c];
		const Reg reg = temps_[temp].greg;
		if (reg == kNoReg)
			continue;
		for (uint32_t r = ra_range_first_[c]; r < ra_range_first_[c + 1]; ++r)
		{
			const uint32_t b = ra_range_block_[r];
			const int32_t base = ra_base_[b];
			const int32_t count = static_cast<int32_t>(blocks_[b].count);
			const bool live_in = ra_ranges_[r].first == base;
			// A phi written by this predecessor's transfer only.
			if (!live_in && ra_ranges_[r].first >= write_slot(base, count))
				continue;
			GlobalEntry entry;
			entry.temp = temp;
			entry.live_in = live_in;
			entry.release = ra_ranges_[r].second >= read_slot(base, count) ? INT32_MAX
																		   : (ra_ranges_[r].second - base - 2) / 2;
			if (!live_in)
				block_def_gregs_[b] |= 1u << reg;
			entries.push_back(std::make_pair((static_cast<uint64_t>(b) << 32) | temp, entry));
		}
	}
	std::sort(entries.begin(), entries.end(),
		[](const std::pair<uint64_t, GlobalEntry>& a, const std::pair<uint64_t, GlobalEntry>& b) {
			return a.first < b.first;
		});
	block_globals_first_.assign(n + 1, 0);
	block_entries_.resize(entries.size());
	for (size_t k = 0; k < entries.size(); ++k)
	{
		++block_globals_first_[(entries[k].first >> 32) + 1];
		block_entries_[k] = entries[k].second;
	}
	for (uint32_t b = 0; b < n; ++b)
		block_globals_first_[b + 1] += block_globals_first_[b];
	// A block-local value flowing into a phi with a register is hinted to
	// that register, and so is each first operand it is computed from that
	// dies there: the edge transfer then moves nothing.
	for (uint32_t b = 0; b < n; ++b)
	{
		if (!blocks_[b].reachable)
			continue;
		const lowir::BlockId from = blocks_[b].id;
		for (uint32_t k = succ_first_[b]; k < succ_first_[b + 1]; ++k)
		{
			const lowir::BlockId to = blocks_[succs_[k]].id;
			for (lowir::InstId i = body.blocks[to].first; i != kNoId; i = body.instructions[i].next)
			{
				const Instruction& phi = body.instructions[i];
				if (phi.op != Opcode::Phi)
					break;
				const Reg reg = temps_[phi.result].greg;
				if (reg == kNoReg || !live(i) || phi.extra_count > 16)
					continue;
				for (uint32_t a = 0; a + 1 < phi.extra_count; a += 2)
					if (body.operands[phi.extra_first + a].index == from)
						hint_chain(resolve(body.operands[phi.extra_first + a + 1]), b, reg);
			}
		}
	}
	// A value allocated while emitting -- a block-local value, or the register
	// copy a value left in its frame home keeps in its defining block -- may
	// not take the register of a function-level value defined inside its
	// interval (after its definition, through its last use: the result of the
	// instruction reading it last may not share it).
	std::vector<std::pair<uint64_t, uint32_t>> defs; // (block << 32 | position, temp)
	for (size_t k = 0; k < global_temps_.size(); ++k)
	{
		const uint32_t temp = global_temps_[k];
		const TempInfo& t = temps_[temp];
		if (temp >= body.parameter_count && t.def >= 0)
			defs.push_back(std::make_pair((static_cast<uint64_t>(t.block) << 32) | static_cast<uint32_t>(t.def), temp));
	}
	if (defs.empty())
		return;
	std::sort(defs.begin(), defs.end());
	// (So does a value homed in some blocks, in its defining block when
	// homed there, as a homed value keeps its register copy.)
	std::vector<uint8_t> homes(temps_.size(), 0);
	for (uint32_t temp : split_homes_.temps)
		homes[temp] = 1;
	std::vector<uint32_t> first(n + 1, 0);
	for (size_t k = 0; k < defs.size(); ++k)
		++first[(defs[k].first >> 32) + 1];
	for (uint32_t b = 0; b < n; ++b)
		first[b + 1] += first[b];
	for (uint32_t temp = 0; temp < temps_.size(); ++temp)
	{
		TempInfo& t = temps_[temp];
		if ((t.flags & (TF_ALIAS | TF_DEAD | TF_LAZY | TF_FOLDED | TF_FUSED | TF_TESTED | TF_ROTATED | TF_RMW)) ||
			(t.greg != kNoReg && !homes[temp]) || t.block == kNoId || (t.cls != VClass::Gpr && t.cls != VClass::Xmm) ||
			first[t.block] == first[t.block + 1])
			continue;
		const int32_t last = t.last < t.def ? t.def : t.last;
		const std::pair<uint64_t, uint32_t> key(
			(static_cast<uint64_t>(t.block) << 32) | static_cast<uint32_t>(t.def + 1), 0);
		for (size_t k =
				 std::lower_bound(defs.begin() + first[t.block], defs.begin() + first[t.block + 1], key) - defs.begin();
			k < first[t.block + 1] && static_cast<int32_t>(defs[k].first & 0xffffffffu) <= last; ++k)
		{
			const uint32_t g = defs[k].second;
			// O1+: a value read last as the first operand its function-level
			// result is computed from (two-address, an extension) may be that
			// result's register, as a block-local result takes a dying
			// operand's: hinted there (and what it is computed from), the
			// definition then moves nothing.
			if (t.greg == kNoReg && t.last == temps_[g].def && t.cls == temps_[g].cls && computed_from(g, temp))
			{
				hint_chain(Value::temp(temp), t.block, temps_[g].greg);
				continue;
			}
			t.forbid |= 1u << temps_[g].greg;
		}
	}
}

// Whether value g's definition computes it in its own register from its
// first operand `from`, which it reads nowhere else: a Binary/Unary (not an
// integer division) or an integer extension or truncation.
bool FunctionSelector::computed_from(uint32_t g, uint32_t from) const
{
	const lowir::FunctionBody& body = *body_;
	const Instruction& inst = body.instructions[body.temps[g].def];
	const Value first = resolve(inst.ops[0]);
	if (first.kind != ValueKind::Temp || first.index != from)
		return false;
	if (inst.op == Opcode::Convert)
	{
		const lowir::ConvertOp op = static_cast<lowir::ConvertOp>(inst.sub);
		return (op == lowir::ConvertOp::ZExt || op == lowir::ConvertOp::SExt || op == lowir::ConvertOp::Trunc) &&
			inst.type.is_integer() && inst.type.kind() != TypeKind::I128 &&
			lowir::convert_source_type(inst).kind() != TypeKind::I128;
	}
	if (!ties_first_operand(inst))
		return false;
	if (inst.op == Opcode::Binary)
	{
		const lowir::BinaryOp op = static_cast<lowir::BinaryOp>(inst.sub);
		const Value second = resolve(inst.ops[1]);
		if ((!inst.type.is_float() &&
				(op == lowir::BinaryOp::Div || op == lowir::BinaryOp::Mod || op == lowir::BinaryOp::UDiv ||
					op == lowir::BinaryOp::UMod)) ||
			(second.kind == ValueKind::Temp && second.index == from))
			return false;
	}
	return true;
}

// Hints `reg` to block-local value v of block b and, through each
// Binary/Unary first operand that dies at its use, to the values it is
// computed from (a bounded chain).
void FunctionSelector::hint_chain(Value v, uint32_t b, Reg reg)
{
	const lowir::FunctionBody& body = *body_;
	for (unsigned step = 0; step < 8 && v.kind == ValueKind::Temp; ++step)
	{
		TempInfo& t = temps_[v.index];
		if ((t.flags &
				(TF_GLOBAL | TF_ALIAS | TF_DEAD | TF_LAZY | TF_FOLDED | TF_FUSED | TF_TESTED | TF_ROTATED | TF_RMW)) ||
			t.block != b || t.hint != kNoReg || v.index < body.parameter_count || (t.cls == VClass::Xmm) != is_xmm(reg))
			return;
		t.hint = reg;
		const lowir::InstId def = body.temps[v.index].def;
		if (def == kNoId || !ties_first_operand(body.instructions[def]))
			return;
		const Value u = resolve(body.instructions[def].ops[0]);
		if (u.kind != ValueKind::Temp || temps_[u.index].last != pos_[def])
			return;
		v = u;
	}
}

void FunctionSelector::allocate_globals_precise()
{
	const lowir::FunctionBody& body = *body_;
	precise_globals_ = true;
	block_entries_.clear();
	block_def_gregs_.assign(blocks_.size(), 0);
	if (has_eh_ && eh_model_ != EhModel::Host)
		return;
	std::vector<int32_t> index(temps_.size(), -1);
	ra_candidates_.clear();
	for (uint32_t t = 0; t < temps_.size(); ++t)
	{
		const TempInfo& info = temps_[t];
		// Vector parameters keep their parameter slots.
		if ((info.flags & TF_GLOBAL) && (info.flags & TF_DEFINED) && !(info.flags & (TF_ALIAS | TF_DEAD | TF_LAZY)) &&
			(info.cls == VClass::Gpr || info.cls == VClass::Xmm) && info.block != kNoId && info.uses != 0 &&
			!(t < body.parameter_count && info.cls == VClass::Xmm))
		{
			index[t] = static_cast<int32_t>(ra_candidates_.size());
			ra_candidates_.push_back(t);
		}
	}
	if (ra_candidates_.empty() || !exceptional_edges())
		return;
	const uint32_t n = static_cast<uint32_t>(blocks_.size());
	ra_base_.assign(n + 1, 0);
	int32_t total = 0;
	for (uint32_t b = 0; b < n; ++b)
	{
		ra_base_[b] = total;
		if (blocks_[b].reachable)
			total += 2 * static_cast<int32_t>(blocks_[b].count) + 4;
	}
	ra_base_[n] = total;
	ra_clobber_at_.assign(static_cast<size_t>(total) + 1, 0);
	ra_early_at_.assign(static_cast<size_t>(total) + 1, 0);
	for_each_inst([&](uint32_t id, uint32_t b) {
		if (!live(id))
			return;
		ra_clobber_at_[static_cast<size_t>(write_slot(ra_base_[b], pos_[id]))] |= clobber_[id];
		ra_early_at_[static_cast<size_t>(read_slot(ra_base_[b], pos_[id]))] |= early_[id];
	});
	ra_clobber_.build(ra_clobber_at_);
	ra_early_.build(ra_early_at_);
	loop_depths();
	collect_uses(index);
	live_ranges();
	assign_registers();
	emission_facts();
}

} // namespace native
