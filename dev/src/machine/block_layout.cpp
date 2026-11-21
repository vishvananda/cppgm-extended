#include "machine/block_layout.h"

namespace machine {

void find_placement_loops(const PlacementGraph& graph, PlacementLoops& loops)
{
	const uint32_t n = graph.size();
	const auto& succ_first = graph.succ_first;
	const auto& succs = graph.succs;
	loops.back.assign(succs.size(), 0);
	loops.post.clear();
	loops.innermost.assign(n, kNoBlock);
	loops.header.clear();
	loops.parent.clear();
	loops.first.assign(1, 0);
	loops.members.clear();
	if (n == 0)
		return;
	std::vector<uint8_t> state(n, 0); // 1 on the stack, 2 done
	std::vector<std::pair<uint32_t, uint32_t>> stack(1, std::make_pair(0u, succ_first[0]));
	state[0] = 1;
	while (!stack.empty())
	{
		auto& top = stack.back();
		if (top.second == succ_first[top.first + 1])
		{
			state[top.first] = 2;
			loops.post.push_back(top.first);
			stack.pop_back();
			continue;
		}
		const uint32_t k = top.second++, t = succs[k];
		if (state[t] == 1)
			loops.back[k] = 1;
		else if (state[t] == 0)
		{
			state[t] = 1;
			stack.push_back(std::make_pair(t, succ_first[t]));
		}
	}
	std::vector<uint32_t> pred_first(n + 1, 0), preds(succs.size());
	for (uint32_t s : succs)
		++pred_first[s + 1];
	for (uint32_t b = 0; b < n; ++b)
		pred_first[b + 1] += pred_first[b];
	std::vector<uint32_t> fill(pred_first.begin(), pred_first.end() - 1);
	for (uint32_t b = 0; b < n; ++b)
		for (uint32_t k = succ_first[b]; k < succ_first[b + 1]; ++k)
			preds[fill[succs[k]]++] = b;
	// One loop per header, over all its back edges; members are the
	// reachable blocks that reach a latch without passing the header. (Each
	// loop's walks run together: a mark of another loop's walk between two
	// of them would let a block in twice.)
	std::vector<uint32_t> loop_of(n, kNoBlock), mark(n, kNoBlock), work, headers;
	std::vector<std::vector<uint32_t>> found, latches;
	for (uint32_t b = 0; b < n; ++b)
		for (uint32_t k = succ_first[b]; k < succ_first[b + 1]; ++k)
		{
			if (!loops.back[k])
				continue;
			const uint32_t h = succs[k];
			if (loop_of[h] == kNoBlock)
			{
				loop_of[h] = static_cast<uint32_t>(found.size());
				found.push_back(std::vector<uint32_t>(1, h));
				latches.push_back(std::vector<uint32_t>());
				headers.push_back(h);
			}
			latches[loop_of[h]].push_back(b);
		}
	for (uint32_t l = 0; l < found.size(); ++l)
	{
		mark[headers[l]] = l;
		for (uint32_t b : latches[l])
		{
			if (mark[b] != l)
			{
				mark[b] = l;
				found[l].push_back(b);
				work.push_back(b);
			}
			while (!work.empty())
			{
				const uint32_t x = work.back();
				work.pop_back();
				for (uint32_t p = pred_first[x]; p < pred_first[x + 1]; ++p)
				{
					const uint32_t y = preds[p];
					if (state[y] != 0 && mark[y] != l)
					{
						mark[y] = l;
						found[l].push_back(y);
						work.push_back(y);
					}
				}
			}
		}
	}
	// Keep the loops whose walk stayed clear of the entry; give each block
	// its smallest one and each loop the smallest one holding its header.
	std::vector<uint32_t> kept;
	for (uint32_t l = 0; l < found.size(); ++l)
	{
		std::sort(found[l].begin(), found[l].end());
		if (found[l].front() != 0)
			kept.push_back(l);
	}
	std::stable_sort(
		kept.begin(), kept.end(), [&](uint32_t a, uint32_t b) { return found[a].size() > found[b].size(); });
	for (uint32_t l : kept)
	{
		const uint32_t id = static_cast<uint32_t>(loops.header.size());
		loops.header.push_back(headers[l]);
		loops.parent.push_back(loops.innermost[headers[l]]);
		for (uint32_t b : found[l])
		{
			loops.members.push_back(b);
			loops.innermost[b] = id;
		}
		loops.first.push_back(static_cast<uint32_t>(loops.members.size()));
	}
}

void placement_frequencies(const PlacementGraph& graph, const PlacementLoops& loops,
	const std::vector<uint32_t>& shares, std::vector<uint64_t>& frequency)
{
	const uint32_t n = graph.size();
	const uint64_t kLimit = static_cast<uint64_t>(1) << 50;
	std::vector<uint64_t> inflow(n, 0);
	frequency.assign(n, 0);
	if (n == 0)
		return;
	inflow[0] = static_cast<uint64_t>(1) << 20;
	for (uint32_t r = static_cast<uint32_t>(loops.post.size()); r-- > 0;)
	{
		const uint32_t b = loops.post[r];
		frequency[b] = std::min(loops.is_header(b) ? inflow[b] * 8 : inflow[b], kLimit);
		for (uint32_t k = graph.succ_first[b]; k < graph.succ_first[b + 1]; ++k)
			if (!loops.back[k])
				inflow[graph.succs[k]] += frequency[b] * shares[k] / 1000;
	}
}

void LoopChains::prepare(const std::vector<uint64_t>& slot_weight)
{
	const uint32_t n = graph_.size(), count = loops_.count();
	weight_ = slot_weight;
	next_.assign(n, kNoBlock);
	prev_.assign(n, kNoBlock);
	head_.resize(n);
	tail_.resize(n);
	for (uint32_t b = 0; b < n; ++b)
		head_[b] = tail_[b] = b;
	run_head_.assign(count, kNoBlock);
	side_head_.assign(count, kNoBlock);
	entering_.assign(count, kNoBlock);
	std::vector<uint64_t> entering_weight(count, 0);
	edges_.clear();
	for (uint32_t b = 0; b < n; ++b)
		for (uint32_t k = graph_.succ_first[b]; k < graph_.succ_first[b + 1]; ++k)
		{
			const uint32_t to = graph_.succs[k];
			if (!slot_weight[k] || loops_.back[k] || to == b || to == 0)
				continue;
			uint32_t level = loops_.innermost[b];
			while (level != kNoBlock && !loops_.contains(level, to))
				level = loops_.parent[level];
			edges_.push_back(Edge{ slot_weight[k], b, to, level == kNoBlock ? count : level });
			for (uint32_t l = loops_.innermost[to]; l != level && l != kNoBlock; l = loops_.parent[l])
				if (loops_.header[l] == to && slot_weight[k] > entering_weight[l])
				{
					entering_weight[l] = slot_weight[k];
					entering_[l] = b;
				}
		}
	// By level, then descending weight; a tie keeps a successor that
	// already follows its block, then the block order.
	std::stable_sort(edges_.begin(), edges_.end(), [](const Edge& a, const Edge& b) {
		if (a.level != b.level)
			return a.level < b.level;
		if (a.weight != b.weight)
			return a.weight > b.weight;
		return (a.to == a.from + 1) > (b.to == b.from + 1);
	});
	level_first_.assign(count + 2, 0);
	for (const auto& edge : edges_)
		++level_first_[edge.level + 1];
	for (uint32_t l = 0; l <= count; ++l)
		level_first_[l + 1] += level_first_[l];
}

// The member of loop l (or the function, l == count) that `b` stands for:
// itself, or the run of the loop just inside l that holds it.
uint32_t LoopChains::unit(uint32_t l, uint32_t b) const
{
	uint32_t inner = loops_.innermost[b];
	if (inner == l || inner == kNoBlock)
		return b;
	while (loops_.parent[inner] != l && loops_.parent[inner] != kNoBlock)
		inner = loops_.parent[inner];
	return graph_.size() + inner;
}

// The heaviest successor of b outside loop l, or kNoBlock.
uint32_t LoopChains::below(uint32_t l, uint32_t b) const
{
	uint32_t best = kNoBlock;
	uint64_t weight = 0;
	for (uint32_t k = graph_.succ_first[b]; k < graph_.succ_first[b + 1]; ++k)
		if (!loops_.contains(l, graph_.succs[k]) && weight_[k] > weight)
		{
			weight = weight_[k];
			best = graph_.succs[k];
		}
	return best;
}

// Join `from`, the last block of its chain, to the chain `to` starts or,
// when `to` heads an inner loop of the level, to that loop's run.
bool LoopChains::link(uint32_t from, uint32_t to, uint32_t level)
{
	if (next_[from] != kNoBlock)
		return false;
	const uint32_t inner = unit(level, to);
	uint32_t first = to;
	if (inner != to)
	{
		const uint32_t l = inner - graph_.size();
		if (loops_.header[l] != to || run_head_[l] == kNoBlock)
			return false;
		first = run_head_[l];
	}
	if (prev_[first] != kNoBlock || head_[from] == first)
		return false;
	next_[from] = first;
	prev_[first] = from;
	const uint32_t start = head_[from], last = tail_[first];
	head_[last] = start;
	tail_[start] = last;
	return true;
}

// Join the level's edges, then (for a loop) its chains into one run: the
// chains ending in a jump back to the header, the header's chain, then the
// other (side) chains, each group in block order.
void LoopChains::join(uint32_t level)
{
	const uint32_t count = loops_.count();
	for (uint32_t e = level_first_[level]; e < level_first_[level + 1]; ++e)
		link(edges_[e].from, edges_[e].to, level);
	if (level == count)
		return;
	const uint32_t header = loops_.header[level];
	uint32_t first = header;
	while (prev_[first] != kNoBlock)
		first = prev_[first];
	heads_.clear();
	sides_.clear();
	for (uint32_t i = loops_.first[level]; i < loops_.first[level + 1]; ++i)
	{
		const uint32_t b = loops_.members[i];
		if (prev_[b] != kNoBlock || b == first)
			continue;
		bool latch = false;
		for (uint32_t k = graph_.succ_first[tail_[b]]; k < graph_.succ_first[tail_[b] + 1]; ++k)
			latch |= loops_.back[k] && graph_.succs[k] == header;
		(latch ? heads_ : sides_).push_back(b);
	}
	heads_.push_back(first);
	side_head_[level] = sides_.empty() ? kNoBlock : sides_.front();
	heads_.insert(heads_.end(), sides_.begin(), sides_.end());
	for (uint32_t i = 1; i < heads_.size(); ++i)
	{
		const uint32_t last = tail_[heads_[i - 1]];
		next_[last] = heads_[i];
		prev_[heads_[i]] = last;
	}
	const uint32_t last = tail_[heads_.back()];
	head_[last] = heads_.front();
	tail_[heads_.front()] = last;
	run_head_[level] = heads_.front();
}

void LoopChains::emit(std::vector<uint32_t>& order) const
{
	const uint32_t n = graph_.size();
	order.clear();
	order.reserve(n);
	for (uint32_t b = 0; b < n; ++b)
		if (prev_[b] == kNoBlock)
			for (uint32_t x = b; x != kNoBlock; x = next_[x])
				order.push_back(x);
}

} // namespace machine
