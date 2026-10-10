#pragma once

#include <cstdint>
#include <string>

#include "lowir/program.h"
#include "support/telemetry.h"

// The LowIR optimizer: one pipeline over the typed program model,
// shared by lowiropt, `cppgm++ --emit-lowir -O<n>` and the object path
// (`cppgm++ -c -O<n>` from source or from LowIR text), so an object
// depends only on facts that serialized LowIR carries. Levels select
// policies (spec section 7): O0 transforms nothing; O1 runs local
// simplification, propagation, control-flow cleanup, slot promotion,
// bounded inlining and pruning; O2 adds bounded memory and loop work; O3
// targeted unrolling. Every pass is bounded (pa32/plan.md lists scopes,
// complexities and budgets).
namespace opt {

// Optional limits of the inliner (`--inline-limit <name>=<n>`); 0 keeps
// the default policy.
struct InlineLimits
{
	uint32_t caller_budget = 0;
	uint32_t once_cap = 0;
	uint32_t once_caller_budget = 0;
	uint32_t hint_late_cap = 0;
};

struct OptimizerOptions
{
	int level = 0;
	InlineLimits limits;
	// Audit mode (lowiropt --verify-passes): the whole program is
	// validated after every pass of every function, and the first pass
	// leaving it invalid is named on standard error (exit status 3). With
	// a path (--verify-passes=<path>) the program is written there after
	// each pass leaving it invalid, or leaving a use of a temporary whose
	// definition is no longer in the layout, and the run continues.
	bool verify_passes = false;
	std::string verify_dump;
	// Course objects (cppgm++ -c to a .obj, course link mode): the object
	// back end compiles a non-variadic this-adjusting thunk as a function
	// (host objects encode each forwarding thunk as a jump:
	// native::recover_jump_thunks), so the inliner copies the target into
	// it and a dispatch makes one call; otherwise thunks stay as lowered.
	bool inline_thunk_targets = false;
};

// Work counters (`--stats`): observational only.
struct OptimizerStats
{
	telemetry::Counter functions = 0;
	// The program's size before and after (the inline budgets scale with
	// instructions_in, so these are counted in every build).
	uint64_t instructions_in = 0;
	uint64_t instructions_out = 0;
	uint64_t blocks_in = 0;
	uint64_t blocks_out = 0;
	telemetry::Counter folded = 0;
	telemetry::Counter propagated = 0;
	telemetry::Counter reused = 0;
	telemetry::Counter dead = 0;
	telemetry::Counter branches_folded = 0;
	telemetry::Counter blocks_removed = 0;
	telemetry::Counter blocks_merged = 0;
	telemetry::Counter slots_promoted = 0;
	telemetry::Counter phis_inserted = 0;
	telemetry::Counter calls_inlined = 0;
	telemetry::Counter inlined_instructions = 0;
	telemetry::Counter functions_pruned = 0;
	telemetry::Counter loads_reused = 0;
	telemetry::Counter stores_removed = 0;
	telemetry::Counter loops_removed = 0;
	telemetry::Counter loops_unrolled = 0;
	telemetry::Counter specialized = 0;
	telemetry::Milliseconds optimize_ms = 0;
};

// Parses `name=value` of --inline-limit into `limits`; false for an
// unknown name or a zero or malformed value.
bool parse_inline_limit(const std::string& text, InlineLimits& limits);

// Optimizes a valid program in place at options.level.
void optimize_program(lowir::Program& program, const OptimizerOptions& options, OptimizerStats* stats);

// One `pa32_opt_stats` line of the counters.
std::string describe_stats(const OptimizerStats& stats);

} // namespace opt
