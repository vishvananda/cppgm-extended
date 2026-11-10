#pragma once

#include <functional>

#include "opt/call_graph.h"

// Interprocedural argument propagation (PA33 O2+): an internal, unobserved,
// nonrecursive, fixed-arity definition whose every direct call passes the
// same literal or symbol for a parameter reads that value instead; a
// parameter its body then never reads leaves the signature and every call
// (internal: no other unit sees the boundary). One graph build and one scan
// of the call sites, O(instructions); changed callees and callers are
// simplified once (`simplify`).
namespace opt {

// Marks definitions that leave caller memory alone and nonrecursive
// definitions whose writes stay in one bounded pointer parameter's span
// (Program::read_only_functions and function_writes). One call graph;
// bounded summaries are built in callee-first order. True when one is found.
bool infer_function_memory(lowir::Program& program);

// Record a conservative read-only fact after one callee is simplified in
// bottom-up order. Unknown or replaceable calls prevent a proof. No
// repeatability, bounded-write or parameter-position facts are inferred.
bool infer_optimized_readonly(lowir::Program& program, lowir::SymbolId symbol);

bool specialize_arguments(
	lowir::Program& program, OptimizerStats& stats, const std::function<void(lowir::SymbolId)>& simplify);

// Callee copies (O2+): a call passing a defined function for a parameter its
// callee (a fixed-arity, inlinable definition of at most 400 instructions)
// calls through calls an internal copy that reads the function instead, as
// do the copy's calls passing it on (a recursive callee's included), so the
// inliner may then inline it. True when it made a copy.
bool copy_for_callees(lowir::Program& program, OptimizerStats& stats);

// O2+: redirect calls through call-and-return wrappers, composing parameter
// permutations and constants without copying a body or changing signatures.
// Keep argument evaluation, materialization and cleanup, and honor no-inline
// and replacement metadata. Resolve each wrapper once; the final argument
// count may not exceed the original boundary's count. No CFG invalidation.
bool forward_calls(lowir::Program& program, OptimizerStats& stats);

// Speculative devirtualization (O2+): a call through a slot of an object's
// vtable, passing the object first, where a store of a readonly table's
// address into the object (its construction) dominates the call, compares
// the loaded callee with the function the table holds in that slot (a
// definition of at most 64 instructions) and calls that function directly
// when they match, so the later waves may inline it; otherwise the call
// goes through the slot as before. True when it rewrote a call.
bool speculate_virtual_calls(lowir::Program& program, OptimizerStats& stats);

// Static initialization (O1+): the stores of constants into internal
// globals that open the unit's initialization function (its entry block:
// literals, readonly data and what it stored there, as the native back end
// computes them) become those globals' data, as [basic.start.init]p3
// permits, when the global starts zero and nothing else writes it
// (find_unwritten_globals). Its loads then fold. One scan of the entry
// block and, when it records a store, one of the unit, which fills
// `unwritten` as find_unwritten_globals would after the move (an empty
// `unwritten` is filled in any case). `moved`, when given, marks the
// globals whose stores it moved; true when it moved one.
bool initialize_statically(
	lowir::Program& program, std::vector<uint8_t>& unwritten, std::vector<uint8_t>* moved = nullptr);

} // namespace opt
