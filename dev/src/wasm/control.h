#pragma once

#include "opt/loops.h"

namespace wasm {

struct Region
{
	lowir::BlockId header;
	std::vector<uint32_t> children;
};

// A child below block_count is a block; otherwise it is a loop index plus
// block_count. Region zero is the function, region loop+1 that loop's body.
struct ControlPlan
{
	uint32_t block_count = 0;
	std::vector<Region> regions;
};

bool structure_control(const lowir::Program& program, const lowir::FunctionBody& body, ControlPlan& result);

} // namespace wasm
