#include "native/select/loop_split.h"

#include <algorithm>

#include "native/select/selector_state.h"

// Spilling around loops: see loop_split.h.
namespace native {

namespace {

struct BlockRange
{
	uint32_t block;
	int32_t first;
	int32_t last;
};

// A block's weight as the allocator counts uses: its estimated
// executions, none when cold.
uint64_t block_weight(const SelectorState& s, uint32_t b)
{
	return s.loop_splitter_.weights[b];
}

} // namespace

void LoopSplitter::reset(const SelectorState& s)
{
	const uint32_t n = static_cast<uint32_t>(s.blocks_.size());
	std::vector<uint8_t> eh(n, 0);
	for (uint32_t b = 0; b < n; ++b)
		if (s.ra_exc_first_[b] != s.ra_exc_first_[b + 1] || s.blocks_[b].handler)
			eh[b] = 1;
	for (uint32_t q : s.ra_exc_preds_)
		eh[q] = 1;
	// The landings each block's calls and resumes enter (CSR).
	exc_succ_first_.assign(n + 1, 0);
	exc_succs_.resize(s.ra_exc_preds_.size());
	for (uint32_t q : s.ra_exc_preds_)
		++exc_succ_first_[q + 1];
	for (uint32_t b = 0; b < n; ++b)
		exc_succ_first_[b + 1] += exc_succ_first_[b];
	{
		std::vector<uint32_t> fill(exc_succ_first_.begin(), exc_succ_first_.end() - 1);
		for (uint32_t l = 0; l < n; ++l)
			for (uint32_t k = s.ra_exc_first_[l]; k < s.ra_exc_first_[l + 1]; ++k)
				exc_succs_[fill[s.ra_exc_preds_[k]]++] = l;
	}
	order_.clear();
	for (uint32_t l = 0; l + 1 < loop_first.size(); ++l)
	{
		bool ok = true;
		for (uint32_t k = loop_first[l]; k < loop_first[l + 1] && ok; ++k)
			ok = loop_blocks[k] != 0 && !eh[loop_blocks[k]];
		if (ok)
			order_.push_back(l);
	}
	std::sort(order_.begin(), order_.end(), [&](uint32_t a, uint32_t b) {
		const uint32_t sa = loop_first[a + 1] - loop_first[a], sb = loop_first[b + 1] - loop_first[b];
		return sa != sb ? sa > sb : a < b;
	});
	if (range_at_.size() < n)
	{
		range_at_.assign(n, 0);
		used_at_.assign(n, 0);
		inside_at_.assign(n, 0);
		reload_at_.assign(n, 0);
		loop_at_.assign(n, 0);
		split_at_.assign(n, 0);
		epoch_ = 0;
		split_epoch_ = 0;
	}
}

bool LoopSplitter::candidates(const SelectorState& s, uint32_t c)
{
	const uint32_t temp = s.ra_candidates_[c];
	const TempInfo& t = s.temps_[temp];
	chosen_.clear();
	if (order_.empty() || t.cls != VClass::Gpr)
		return false;
	const uint32_t e = ++epoch_;
	for (uint32_t r = s.ra_range_first_[c]; r < s.ra_range_first_[c + 1]; ++r)
		if (s.ra_ranges_[r].first == s.ra_base_[s.ra_range_block_[r]])
			range_at_[s.ra_range_block_[r]] = e;
	for (uint32_t u = s.ra_use_first_[c]; u < s.ra_use_first_[c + 1]; ++u)
		used_at_[s.ra_uses_[u].first] = e;
	if (temp >= s.body_->parameter_count)
		used_at_[t.block] = e;
	// The outermost loops the value is live into without a use or its
	// definition: largest first, skipping those inside a chosen one.
	bool any = false;
	for (uint32_t l : order_)
	{
		const uint32_t first = loop_first[l], past = loop_first[l + 1];
		const uint32_t header = loop_blocks[first];
		if (range_at_[header] != e || inside_at_[header] == e)
			continue;
		bool clean = true;
		for (uint32_t k = first; k < past && clean; ++k)
			clean = used_at_[loop_blocks[k]] != e;
		if (!clean)
			continue;
		for (uint32_t k = first; k < past; ++k)
		{
			inside_at_[loop_blocks[k]] = e;
			loop_at_[loop_blocks[k]] = l;
		}
		chosen_.push_back(l);
		any = true;
	}
	return any;
}

bool LoopSplitter::plan(const SelectorState& s, uint32_t c, const std::vector<uint32_t>* conflicts)
{
	const uint32_t temp = s.ra_candidates_[c];
	// The loops kept: those holding a conflict, or all candidates.
	const uint32_t e = ++split_epoch_;
	bool any = false;
	for (uint32_t l : chosen_)
	{
		bool keep = conflicts == nullptr;
		for (size_t k = 0; conflicts != nullptr && k < conflicts->size() && !keep; ++k)
			keep = inside((*conflicts)[k]) && loop_at_[(*conflicts)[k]] == l;
		if (keep)
			for (uint32_t k = loop_first[l]; k < loop_first[l + 1]; ++k)
				split_at_[loop_blocks[k]] = e;
		any = any || keep;
	}
	if (!any)
		return false;
	auto kept = [&](uint32_t b) { return split_at_[b] == e; };
	// The transfers: on each edge into a block the value is live into,
	// from outside the loops to inside (a store) or back (a reload).
	moves.clear();
	cost = 0;
	for (uint32_t r = s.ra_range_first_[c]; r < s.ra_range_first_[c + 1]; ++r)
	{
		const uint32_t x = s.ra_range_block_[r];
		if (s.ra_ranges_[r].first != s.ra_base_[x])
			continue;
		for (uint32_t k = s.preds_first_[x]; k < s.preds_first_[x + 1]; ++k)
		{
			const uint32_t q = s.preds_[k];
			const bool from_inside = kept(q), to_inside = kept(x);
			if (from_inside == to_inside)
				continue;
			SplitMove move;
			move.from = q;
			move.to = x;
			move.temp = temp;
			move.store = to_inside;
			moves.push_back(move);
			cost += block_weight(s, to_inside ? q : x);
		}
	}
	if (cost >= s.ra_freq_[c])
		return false;
	std::sort(moves.begin(), moves.end(),
		[](const SplitMove& a, const SplitMove& b) { return a.from != b.from ? a.from < b.from : a.to < b.to; });
	moves.erase(std::unique(moves.begin(), moves.end(),
					[](const SplitMove& a, const SplitMove& b) { return a.from == b.from && a.to == b.to; }),
		moves.end());
	// The ranges outside the loops, and each reload's on its predecessor's
	// transfer write slot, by block.
	ranges.clear();
	blocks.clear();
	std::vector<BlockRange> split;
	for (uint32_t r = s.ra_range_first_[c]; r < s.ra_range_first_[c + 1]; ++r)
		if (!kept(s.ra_range_block_[r]))
			split.push_back(BlockRange{ s.ra_range_block_[r], s.ra_ranges_[r].first, s.ra_ranges_[r].second });
	for (const SplitMove& move : moves)
	{
		// (A predecessor ending in a conditional terminator reloads in the
		// edge block, where only what the successor takes is live: the
		// value meets it at the successor's entry.)
		if (move.store || reload_at_[move.from] == e || s.succ_first_[move.from + 1] - s.succ_first_[move.from] > 1)
			continue;
		reload_at_[move.from] = e; // split stamps never repeat
		const int32_t written = s.ra_base_[move.from] + 3 + 2 * static_cast<int32_t>(s.blocks_[move.from].count);
		split.push_back(BlockRange{ move.from, written, written });
	}
	std::stable_sort(
		split.begin(), split.end(), [](const BlockRange& a, const BlockRange& b) { return a.block < b.block; });
	for (const BlockRange& range : split)
	{
		blocks.push_back(range.block);
		ranges.push_back(std::make_pair(range.first, range.last));
	}
	return true;
}

bool LoopSplitter::plan_homes(
	const SelectorState& s, uint32_t c, const std::vector<uint32_t>& conflicts, uint64_t bound)
{
	const uint32_t temp = s.ra_candidates_[c];
	const TempInfo& t = s.temps_[temp];
	const uint32_t e = ++split_epoch_;
	for (uint32_t r = s.ra_range_first_[c]; r < s.ra_range_first_[c + 1]; ++r)
		reload_at_[s.ra_range_block_[r]] = e; // here: the blocks it is live in
	homed.clear();
	auto add = [&](uint32_t b) {
		if (split_at_[b] != e)
		{
			split_at_[b] = e;
			homed.push_back(b);
		}
	};
	for (uint32_t b : conflicts)
		add(b);
	// Host EH: no transfer runs on an exceptional edge, so a call block
	// and a landing it enters with the value live there are homed alike.
	for (size_t k = 0; k < homed.size(); ++k)
	{
		const uint32_t b = homed[k];
		for (uint32_t i = exc_succ_first_[b]; i < exc_succ_first_[b + 1]; ++i)
			if (reload_at_[exc_succs_[i]] == e)
				add(exc_succs_[i]);
		for (uint32_t i = s.ra_exc_first_[b]; i < s.ra_exc_first_[b + 1]; ++i)
			if (reload_at_[s.ra_exc_preds_[i]] == e)
				add(s.ra_exc_preds_[i]);
	}
	if (homed.empty())
		return false;
	auto kept = [&](uint32_t b) { return split_at_[b] == e; };
	const uint32_t def_block = temp < s.body_->parameter_count ? 0 : t.block;
	const bool defined_inside = kept(def_block);
	// A phi is written by its predecessors' transfers, in parallel with
	// the split's own: neither its block nor those may hold its home alone.
	if (temp >= s.body_->parameter_count && s.body_->instructions[s.body_->temps[temp].def].op == lowir::Opcode::Phi)
	{
		if (defined_inside)
			return false;
		for (uint32_t k = s.preds_first_[def_block]; k < s.preds_first_[def_block + 1]; ++k)
			if (kept(s.preds_[k]))
				return false;
	}
	// The home's writes: its definition's (homed there), or a store on
	// each edge in.
	cost = defined_inside ? (temp < s.body_->parameter_count ? kWeightScale : block_weight(s, def_block)) : 0;
	// (Every term is nonnegative: past the limit the plan fails.)
	const uint64_t limit = std::min(bound, s.ra_freq_[c]);
	if (cost >= limit)
		return false;
	moves.clear();
	for (uint32_t r = s.ra_range_first_[c]; r < s.ra_range_first_[c + 1]; ++r)
	{
		const uint32_t x = s.ra_range_block_[r];
		if (s.ra_ranges_[r].first != s.ra_base_[x])
			continue;
		for (uint32_t k = s.preds_first_[x]; k < s.preds_first_[x + 1]; ++k)
		{
			const uint32_t q = s.preds_[k];
			const bool from_inside = kept(q), to_inside = kept(x);
			if (from_inside == to_inside || (to_inside && defined_inside))
				continue;
			SplitMove move;
			move.from = q;
			move.to = x;
			move.temp = temp;
			move.store = to_inside;
			moves.push_back(move);
			cost += std::min(block_weight(s, q), block_weight(s, x));
			if (cost >= limit)
				return false;
		}
	}
	for (uint32_t u = s.ra_use_first_[c]; u < s.ra_use_first_[c + 1]; ++u)
		if (kept(s.ra_uses_[u].first))
		{
			cost += block_weight(s, s.ra_uses_[u].first);
			if (cost >= limit)
				return false;
		}
	std::sort(moves.begin(), moves.end(),
		[](const SplitMove& a, const SplitMove& b) { return a.from != b.from ? a.from < b.from : a.to < b.to; });
	moves.erase(std::unique(moves.begin(), moves.end(),
					[](const SplitMove& a, const SplitMove& b) { return a.from == b.from && a.to == b.to; }),
		moves.end());
	// The ranges outside the blocks. A reload needs none in its homed
	// predecessor: it writes the register on its own edge (an edge block
	// after a conditional terminator), where only the values live into
	// the successor are, and the value's range there starts at its entry.
	ranges.clear();
	blocks.clear();
	for (uint32_t r = s.ra_range_first_[c]; r < s.ra_range_first_[c + 1]; ++r)
		if (!kept(s.ra_range_block_[r]))
		{
			blocks.push_back(s.ra_range_block_[r]);
			ranges.push_back(s.ra_ranges_[r]);
		}
	return true;
}

} // namespace native
