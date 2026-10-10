#pragma once

#include <functional>

#include "opt/call_graph.h"
#include "opt/passes.h"

// Bounded inlining of direct calls (PA32 O1+). Three waves over the
// callee-first call graph (opt/call_graph.h):
//   ordinary   small nonrecursive callees within a per-caller growth budget
//              (trivial leaves exempt), plus single-call discardable
//              callees within their own caller and unit budgets;
//   once       after pruning, the single-call policy alone with fresh
//              budgets (pruning dead callers exposes new single calls);
//   late       a fresh budget for small optimized bodies.
// A callee with EH instructions, va_start or stack_alloc, a variadic or
// mismatched boundary, recursion, no_inline or GNU replaceability stays a
// call; a call in handler code needs a callee that cannot unwind. Each
// caller is simplified (`simplify`) after it changes, before its own
// callers decide, so a whole wave is one traversal. Budgets are listed in
// pa32/plan.md.
namespace opt {

enum class InlineWave
{
	Ordinary,
	Once,
	Late
};

struct InlineBudgets
{
	uint64_t once_unit = 0;   // remaining single-call work for the unit
	uint64_t growth_unit = 0; // remaining instructions the other policies may copy into the unit
	uint64_t force_unit = 0;  // remaining instructions force-inline callees may copy
};

// Runs one wave. `simplify` cleans one function after inlining (and, in
// the ordinary wave, every function once in callee-first order);
// `original_size` holds each definition's input size by symbol and
// `nounwind` the definitions proved nonthrowing so far.
void run_inline_wave(lowir::Program& program, const OptimizerOptions& options, OptimizerStats& stats, InlineWave wave,
	InlineBudgets& budgets, const std::vector<uint32_t>& original_size, const std::vector<uint8_t>& nounwind,
	const std::function<void(lowir::SymbolId)>& simplify);

} // namespace opt
