#pragma once

#include <vector>

#include "opt/function_ir.h"

// Natural loops of one body (PA32 loop transforms): back edges into a
// dominating header, bodies by reverse search from the latches, nesting by
// containment. Loops whose bodies hold handler blocks are not reported, nor
// those found once the memberships pass eight per block (a deep nest).
// O(blocks + edges + reported memberships).
namespace opt {

struct Loop
{
	lowir::BlockId header;
	std::vector<lowir::BlockId> blocks; // header first
	std::vector<lowir::BlockId> latches;
	lowir::BlockId preheader; // the one outside predecessor jumping only here, or kNoId
	uint32_t parent;          // enclosing loop index or kNoId
};

struct LoopForest
{
	std::vector<Loop> loops;            // outer loops before the loops they contain
	std::vector<uint32_t> innermost;    // per block: innermost loop or kNoId
	std::vector<uint32_t> enter, leave; // per loop: preorder interval in the loop tree
	std::vector<uint32_t> mark;         // scratch membership marks
	uint32_t epoch = 0;
};

void find_loops(const lowir::FunctionBody& body, const Cfg& cfg, LoopForest& forest);

// Whether block `b` belongs to loop `l` (including nested loops). O(1).
bool in_loop(const LoopForest& forest, uint32_t l, lowir::BlockId b);

} // namespace opt
