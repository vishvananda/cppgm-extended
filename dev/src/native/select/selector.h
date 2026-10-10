#pragma once

#include <cstdint>

#include "lowir/program.h"
#include "native/mir/mir.h"
#include "support/telemetry.h"

// LowIR -> MIR for one function definition: analysis, instruction
// selection and block-local register allocation (native/select/*.cpp).
namespace native {

// Work counters of one compilation (`--stats`).
struct SelectStats
{
	telemetry::Counter functions = 0;
	telemetry::Counter lowir_instructions = 0;
	telemetry::Counter mir_instructions = 0;
	telemetry::Counter spills = 0;
	telemetry::Counter homes = 0;
	telemetry::Counter lazy_addresses = 0;
	telemetry::Counter folded_loads = 0;
	telemetry::Counter fused_compares = 0;
	telemetry::Counter promoted_slots = 0;
	telemetry::Counter dead_slots = 0;
	telemetry::Counter global_registers = 0;
	telemetry::Counter liveness_steps = 0; // uses and predecessor edges the hull walks visit
	telemetry::Counter hull_fallbacks = 0; // cross-block values left in frame homes by the walk budget
	telemetry::Counter edge_blocks = 0;
	telemetry::Counter eh_walks = 0;          // host EH: block walks to the states' fixed point
	telemetry::Counter eh_states = 0;         // host EH: interned record stacks
	telemetry::Counter threaded_blocks = 0;   // O1+: jump-only MIR blocks removed
	telemetry::Counter layout_branches = 0;   // O1+: fall-through jumps and branches removed
	telemetry::Counter strlen_prefixes = 0;   // O1+: strlen calls given the inline probe
	telemetry::Counter dynamic_copies = 0;    // unused-result builtin memcpy calls copied in place
	telemetry::Counter jump_tables = 0;       // O1+: switches dispatched through a jump table
	telemetry::Counter rotated_loops = 0;     // O1+: back edges given a copy of their loop test
	telemetry::Counter loop_splits = 0;       // O1+: values given a register outside loops they skip
	telemetry::Counter home_splits = 0;       // O1+: values given a register outside the blocks clobbering it
	telemetry::Counter ordered_frames = 0;    // O1+: frames whose objects order_frame moved
	telemetry::Counter dead_frame_stores = 0; // O1+: frame stores no path reads (drop_dead_frame_stores)
	// The block-local allocator (allocation.cpp): registers bound to values
	// and freed, and allocations that found every register of the class
	// held (by a parameter's value among them, or by other values only).
	telemetry::Counter register_grants = 0;
	telemetry::Counter register_releases = 0;
	telemetry::Counter grant_busy = 0;
	telemetry::Counter grant_busy_parameters = 0;
	telemetry::Counter grant_busy_values = 0;
	// Definitions of cross-block values: in their function-level register,
	// in a block-local register (stored to the frame home), or in scratch
	// stored to the home when no register was free.
	telemetry::Counter defined_in_plan = 0;
	telemetry::Counter defined_other_register = 0;
	telemetry::Counter defined_frame = 0;
};

class FunctionSelector;

// How a host object's code reaches `symbol` (pa27 README 1, 5): directly
// (pc-relative), through its GOT entry, or a thread-local form; from the
// symbol's facts, or from the binding a streaming producer states for a
// global referenced before it is declared.
SymbolAccess host_symbol_access(const lowir::Program& program, lowir::SymbolId symbol);

class Selector
{
public:
	explicit Selector(const lowir::Program& program);
	~Selector();

	Selector(const Selector&) = delete;
	Selector& operator=(const Selector&) = delete;

	// Selects the definition of `function` into `out` (cleared first).
	// Throws BackendError for LowIR outside the supported subset
	// and InternalError for a broken invariant of the selector.
	void select(lowir::SymbolId function, Function& out);
	// The exception model of the functions selected from now on (the
	// course's runtime ABI unless set).
	void set_eh_model(EhModel model);
	// The optimization level of the functions selected from now on (0
	// unless set): O1 and above lay out branches in the MIR itself
	// (branch_layout.cpp), select the builtin strlen call prefix
	// (emit_calls.cpp) and, in the course EH model, push handler records
	// below the frame (runtime/eh_record.h).
	void set_optimization_level(int level);

	const SelectStats& stats() const { return stats_; }

private:
	const lowir::Program& program_;
	SelectStats stats_;
	FunctionSelector* state_;
};

} // namespace native
