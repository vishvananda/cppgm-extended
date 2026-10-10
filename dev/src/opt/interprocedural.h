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

bool specialize_arguments(
	lowir::Program& program, OptimizerStats& stats, const std::function<void(lowir::SymbolId)>& simplify);

} // namespace opt
