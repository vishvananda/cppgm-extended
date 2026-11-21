#pragma once

#include <vector>

#include "opt/function_ir.h"

// Natural loops of one body (PA33 loop transforms): back edges into a
// dominating header, bodies by reverse search from the latches, nesting by
// containment. Loops whose bodies hold handler blocks are not reported, nor
// those found once the memberships pass eight per block (a deep nest).
// O(blocks + edges + reported memberships).
namespace opt {

struct FunctionContext;

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

// Whether a loop's multiplication reads a table of row pointers with no
// volatile accesses or explicit stores. Used to budget full unrolling.
bool indirect_word_reduction(const lowir::FunctionBody& body, const Loop& loop);
bool from_induction(const lowir::FunctionBody& body, const std::vector<lowir::BlockId>& block_of,
	const std::vector<uint8_t>& member, lowir::TempId iv, lowir::Value value, unsigned depth);
bool indexes_local_array(const lowir::FunctionBody& body, const Loop& loop, lowir::TempId iv);
bool has_contiguous_float_work(const lowir::FunctionBody& body, const Loop& loop);
bool loop_uses_vectors(const lowir::FunctionBody& body, const Loop& loop);

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
// Gives a loop of several back edges one latch: a new block of one jump to
// its header that every back edge passes through, where a phi merges what
// the header's phis took from the latches when those differ. False for a
// loop of one latch or with a handler latch. The graph changes.
bool make_loop_latch(lowir::FunctionBody& body, const Cfg& cfg, const LoopForest& forest, uint32_t loop);
// Gives a loop nested in loop `loop` at its header (a do-while loop that
// begins the outer body) a header of its own: the back edges from latches
// whose cycle through the header holds no other latch stay; the others and
// the entries go to a new outer header in front, whose phis merge what they
// bring. The inner loop must be counted (each inner latch tests a counter),
// leave only into the outer loop, and keep a value that an outer latch
// steps. False when every latch or none is such, when that is not so (the
// paths are then one loop's), or for a handler. The graph changes.
bool separate_nested_loop(lowir::FunctionBody& body, const Cfg& cfg, const LoopForest& forest, uint32_t loop);

// For a pass that rewrites several loops between finding them: marks in
// `touched` (per block) the blocks a rewrite of `l` may change -- the loop, the
// blocks that enter it and those it leaves to -- and returns true, or returns
// false with nothing marked when one is marked already (`l` waits for the next
// round). Loops a pass rewrites under this rule do not see each other's changes.
bool claim_loop(const Cfg& cfg, const Loop& l, std::vector<uint8_t>& touched);
// Whether claim_loop would refuse `l`. A pass that reads a loop before it
// claims it asks first: a refused loop may hold what a rewrite in this round
// added, which the round's facts (the graph, `block_of`) do not cover.
bool loop_claimed(const Cfg& cfg, const Loop& l, const std::vector<uint8_t>& touched);

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
// The slot or global an address stays in, or none: through `addr`, pointer
// copies, `index` steps of any size and phis whose every input stays in that
// one object (a pointer that walks an array).
lowir::Value address_object(const lowir::FunctionBody& body, lowir::Value v);

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

// A loop of one pass becomes straight code: its header's test, of literals
// and the header's phis alone, continues with the entering values and
// leaves with the latch's (opt/loop_unrolling.cpp). `context.block_of` maps
// the instructions.
bool straighten_single_pass_loop(FunctionContext& context, const LoopForest& forest, uint32_t loop);

// The instructions the copies of a counted loop keep when each pass has the
// induction `iv` (a header phi entered with a literal) as a literal: per
// pass, the blocks its folded tests reach, less the instructions that are
// constants there; then the header once more. `trips` passes; `member` and
// `owned` are the loop's blocks and its private exits. UINT64_MAX when the
// induction has no literal on some pass (opt/loop_unrolling.cpp).
uint64_t unrolled_size(const FunctionContext& context, const Cfg& cfg, const Loop& loop,
	const std::vector<uint8_t>& member, const std::vector<uint8_t>& owned, lowir::TempId iv, int64_t trips);

// A second signed 32-bit bound in a counted loop. It tests a header phi
// that steps by one, and both tests leave to `exit`. The fast copy can
// bypass this second test when all its passes fit below `bound`.
struct CountedUnrollGuard
{
	lowir::BlockId block = lowir::kNoId;
	lowir::BlockId exit = lowir::kNoId;
	lowir::Value start;
	lowir::Value bound;
};

void clone_counted_loop(FunctionContext& context, const Loop& loop, int64_t trips, const std::vector<uint8_t>& member,
	const std::vector<uint8_t>& owned, const CountedUnrollGuard& guard);

// The test number (0-based) at which a counted loop leaves: the induction
// starts at a literal and steps by a literal, the bound is a literal, and
// the tested values move from continuing to leaving without leaving the
// comparison's integer range (no wrap). `next` tests the stepped value.
bool counted_trips(lowir::Type type, lowir::ComparePredicate pred, bool continue_on_true, __int128 init, __int128 step,
	__int128 bound, bool next, int64_t& trips, __int128& last);

} // namespace opt
