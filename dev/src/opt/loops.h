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

// Whether two located accesses name the same place at the same type.
bool same_place(const Located& a, lowir::Type a_type, const Located& b, lowir::Type b_type);

// Whether a place is read or written at its type before `loop`: in a block
// that dominates the header from outside (the nearest few), with no call
// after it there or in the blocks between it and the loop, which might free
// the place.
bool read_before_loop(const lowir::FunctionBody& body, const Cfg& cfg, const LoopForest& forest, uint32_t loop,
	const Located& at, lowir::Type type);

// Whether `bytes` at `address` lie in a slot for any value of its index: a
// table lookup by a byte reads the table, whatever the byte.
bool table_entry(const lowir::FunctionBody& body, lowir::Value address, uint64_t bytes);

// A nonnegative index's largest possible value; UINT64_MAX when unknown.
uint64_t index_upper_bound(const lowir::FunctionBody& body, lowir::Value value);

// What a load moved to `pre` reads when `pre`, or the one block before it,
// already loaded or stored that address at its type with no write after it:
// that value (None otherwise).
lowir::Value read_in_preheader(const lowir::Program& program, const lowir::FunctionBody& body, const Cfg& cfg,
	lowir::BlockId pre, const lowir::Instruction& load);

// Replaces each phi of `header` that `v` enters from every edge (or that
// reads only itself otherwise) with `v`, moving its counts in `uses`.
void fold_header_phis(lowir::FunctionBody& body, lowir::BlockId header, lowir::Value v, std::vector<uint32_t>& uses);

// The test number (0-based) at which a counted loop leaves: the induction
// starts at a literal and steps by a literal, the bound is a literal, and
// the tested values move from continuing to leaving without leaving the
// comparison's integer range (no wrap). `next` tests the stepped value.
bool counted_trips(lowir::Type type, lowir::ComparePredicate pred, bool continue_on_true, __int128 init, __int128 step,
	__int128 bound, bool next, int64_t& trips, __int128& last);

} // namespace opt
