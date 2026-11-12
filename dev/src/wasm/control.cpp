#include "wasm/control.h"

#include <algorithm>

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

} // namespace

bool structure_control(
	const lowir::Program& program, const lowir::FunctionBody& body, ControlPlan& result, const opt::Cfg* explicit_edges)
{
	opt::Cfg ordinary;
	if (explicit_edges == nullptr)
		opt::build_cfg(program, body, ordinary);
	const opt::Cfg& cfg = explicit_edges != nullptr ? *explicit_edges : ordinary;
	opt::LoopForest forest;
	opt::find_loops(body, cfg, forest);
	if (forest.partial || std::find(forest.dropped.begin(), forest.dropped.end(), uint8_t(1)) != forest.dropped.end())
		return false;
	const uint32_t blocks = static_cast<uint32_t>(body.blocks.size());
	result.regions.clear();
	result.dispatch_region.clear();
	result.dispatch_case.clear();
	result.dispatch_children = 0;
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
	std::vector<uint32_t> indices(blocks + forest.loops.size());
	// Newly appended local dispatch regions are cyclic by construction; only
	// the original natural-loop regions need ordering and SCC condensation.
	for (uint32_t i = 0; i < edges.size(); ++i)
		if (!order_control_region(result, i, edges[i], cfg, indices))
			return false;
	return true;
}

} // namespace wasm
