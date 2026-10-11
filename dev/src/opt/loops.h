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

// An address as its root (a slot, a global, or the temporary or parameter
// it steps from) and a constant byte offset when known, through literal
// `index` steps, `addr` and pointer copies.
struct Located
{
	lowir::Value root;
	int64_t offset = 0;
	bool known = true; // offset known
};

Located locate(const lowir::FunctionBody& body, lowir::Value v);

} // namespace opt
