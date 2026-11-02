#pragma once

#include <functional>

#include "opt/call_graph.h"

// Interprocedural argument propagation (PA32 O2+): an internal, unobserved,
// nonrecursive, fixed-arity definition whose every direct call passes the
// same literal or symbol for a parameter reads that value instead; a
// parameter its body then never reads leaves the signature and every call
// (internal: no other unit sees the boundary). One graph build and one scan
// of the call sites, O(instructions); changed callees and callers are
// simplified once (`simplify`).
namespace opt {

// Marks each defined function whose calls leave memory as it was
// (Program::read_only_functions); true when it marks one.
bool infer_read_only_functions(lowir::Program& program);

bool specialize_arguments(
	lowir::Program& program, OptimizerStats& stats, const std::function<void(lowir::SymbolId)>& simplify);

// Callee copies (O2+): a call passing a defined function for a parameter its
// callee (a fixed-arity, inlinable definition of at most 400 instructions)
// calls through calls an internal copy that reads the function instead, as
// do the copy's calls passing it on (a recursive callee's included), so the
// inliner may then inline it. True when it made a copy.
bool copy_for_callees(lowir::Program& program, OptimizerStats& stats);

} // namespace opt
