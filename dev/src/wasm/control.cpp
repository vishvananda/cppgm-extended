#include "wasm/control.h"

#include <algorithm>
#include <functional>
#include <queue>

namespace wasm {

namespace {

using lowir::kNoId;

uint32_t project(const opt::LoopForest& forest, uint32_t block, uint32_t owner, uint32_t blocks)
{
	uint32_t inner = forest.innermost[block];
	if (inner == owner)
		return block;
	while (inner != kNoId && forest.loops[inner].parent != owner)
		inner = forest.loops[inner].parent;
	return inner == kNoId ? kNoId : blocks + inner;
}

bool order_region(Region& region, std::vector<std::pair<uint32_t, uint32_t>>& edges, const opt::Cfg& cfg,
	const opt::LoopForest& forest, uint32_t blocks, std::vector<uint32_t>& incoming)
{
	for (uint32_t child : region.children)
		incoming[child] = 0;
	for (const auto& edge : edges)
		++incoming[edge.second];
	std::sort(edges.begin(), edges.end());
	using Ranked = std::pair<uint32_t, uint32_t>;
	std::priority_queue<Ranked, std::vector<Ranked>, std::greater<Ranked>> ready;
	auto enqueue = [&](uint32_t child) {
		const uint32_t entry = child < blocks ? child : forest.loops[child - blocks].header;
		ready.emplace(cfg.rpo_index[entry], child);
	};
	for (uint32_t child : region.children)
		if (incoming[child] == 0)
			enqueue(child);
	std::vector<uint32_t> ordered;
	while (!ready.empty())
	{
		const uint32_t child = ready.top().second;
		ready.pop();
		ordered.push_back(child);
		auto edge = std::lower_bound(edges.begin(), edges.end(), std::make_pair(child, uint32_t(0)));
		while (edge != edges.end() && edge->first == child)
		{
			if (--incoming[edge->second] == 0)
				enqueue(edge->second);
			++edge;
		}
	}
	if (ordered.size() != region.children.size())
		return false;
	region.children.swap(ordered);
	return true;
}

} // namespace

bool structure_control(const lowir::Program& program, const lowir::FunctionBody& body, ControlPlan& result)
{
	opt::Cfg cfg;
	opt::build_cfg(program, body, cfg);
	opt::LoopForest forest;
	opt::find_loops(body, cfg, forest);
	if (forest.partial || std::find(forest.dropped.begin(), forest.dropped.end(), uint8_t(1)) != forest.dropped.end())
		return false;
	const uint32_t blocks = static_cast<uint32_t>(body.blocks.size());
	result.block_count = blocks;
	result.regions.resize(forest.loops.size() + 1);
	result.regions[0].header = body.layout[0];
	for (uint32_t i = 0; i < forest.loops.size(); ++i)
	{
		result.regions[i + 1].header = forest.loops[i].header;
		const uint32_t parent = forest.loops[i].parent == kNoId ? 0 : forest.loops[i].parent + 1;
		result.regions[parent].children.push_back(blocks + i);
	}
	for (uint32_t block : cfg.rpo)
	{
		const uint32_t owner = forest.innermost[block];
		result.regions[owner == kNoId ? 0 : owner + 1].children.push_back(block);
	}
	std::vector<std::vector<std::pair<uint32_t, uint32_t>>> edges(result.regions.size());
	for (uint32_t block : cfg.rpo)
	{
		uint32_t owner = forest.innermost[block];
		for (;;)
		{
			const uint32_t region = owner == kNoId ? 0 : owner + 1;
			const uint32_t from = project(forest, block, owner, blocks);
			for (uint32_t i = cfg.succ_first[block]; i < cfg.succ_first[block + 1]; ++i)
			{
				const uint32_t successor = cfg.succs[i];
				if (owner != kNoId && successor == forest.loops[owner].header)
					continue;
				const uint32_t to = project(forest, successor, owner, blocks);
				if (to != kNoId && to != from)
					edges[region].emplace_back(from, to);
			}
			if (owner == kNoId)
				break;
			owner = forest.loops[owner].parent;
		}
	}
	std::vector<uint32_t> incoming(blocks + forest.loops.size());
	for (uint32_t i = 0; i < result.regions.size(); ++i)
		if (!order_region(result.regions[i], edges[i], cfg, forest, blocks, incoming))
			return false;
	return true;
}

} // namespace wasm
