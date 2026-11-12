#pragma once

#include "opt/loops.h"

namespace wasm {

struct Region
{
	lowir::BlockId header;
	std::vector<uint32_t> children;
	bool dispatch = false;
};

// A child below block_count is a block; otherwise it is a loop index plus
// block_count. Region zero is the function, region loop+1 that loop's body.
struct ControlPlan
{
	uint32_t block_count = 0;
	std::vector<Region> regions;
	// Only entries into local dispatch regions need a state number. Natural
	// loop backedges still use their active structured labels.
	std::vector<uint32_t> dispatch_region, dispatch_case;
	uint32_t dispatch_children = 0;
	lowir::BlockId entry(uint32_t child) const
	{
		return child < block_count ? child : regions[child - block_count + 1].header;
	}
};

bool order_control_region(ControlPlan& plan, uint32_t region, const std::vector<std::pair<uint32_t, uint32_t>>& edges,
	const opt::Cfg& cfg, std::vector<uint32_t>& indices);

bool structure_control(const lowir::Program& program, const lowir::FunctionBody& body, ControlPlan& result,
	const opt::Cfg* explicit_edges = nullptr);

} // namespace wasm
