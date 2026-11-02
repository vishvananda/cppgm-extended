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
	std::vector<uint8_t> dropped;       // per block: in a cycle not reported (a handler, not reducible)
	bool partial = false;               // the budget ended the search: more loops exist
	std::vector<uint32_t> enter, leave; // per loop: preorder interval in the loop tree
	std::vector<uint32_t> mark;         // scratch membership marks
	uint32_t epoch = 0;
};

void find_loops(const lowir::FunctionBody& body, const Cfg& cfg, LoopForest& forest);

// Whether block `b` belongs to loop `l` (including nested loops). O(1).
bool in_loop(const LoopForest& forest, uint32_t l, lowir::BlockId b);

// Whether a branch leads to `b` only on a floating value that is not a number,
// or on a negative one under a square root: a rare path, around whose calls
// the allocator saves what a loop keeps in registers.
bool rare_float_path(const lowir::FunctionBody& body, const Cfg& cfg, lowir::BlockId b);

// Gives loop `loop` a preheader: a new block of one jump to its header, between
// the predecessors outside the loop and the header. False when one is a
// handler. The graph changes.
bool make_loop_preheader(lowir::FunctionBody& body, const Cfg& cfg, const LoopForest& forest, uint32_t loop);

// For a pass that rewrites several loops between finding them: marks in
// `touched` (per block) the blocks a rewrite of `l` may change -- the loop, the
// blocks that enter it and those it leaves to -- and returns true, or returns
// false with nothing marked when one is marked already (`l` waits for the next
// round). Loops a pass rewrites under this rule do not see each other's changes.
bool claim_loop(const Cfg& cfg, const Loop& l, std::vector<uint8_t>& touched);

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
// The same up to the first step by a value: that step's result is the root.
Located locate_steps(const lowir::FunctionBody& body, lowir::Value v);

// An address as a base, a constant offset and a multiple of `stride` (0: no
// multiple), through `index` steps, pointer copies and a pointer phi stepping
// by a literal. Two accesses from one base whose ranges do not meet modulo the
// common stride lie in different fields of an array's elements.
struct Strided
{
	lowir::Value base = lowir::Value::none();
	int64_t offset = 0;
	uint64_t stride = 0;
};

Strided strided_address(const lowir::FunctionBody& body, lowir::Value v);
bool strided_apart(const Strided& a, uint64_t a_bytes, const Strided& b, uint64_t b_bytes);

} // namespace opt
