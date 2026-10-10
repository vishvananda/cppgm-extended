#pragma once

#include <string>
#include <vector>

#include "lowir/program.h"
#include "native/mir/mir.h"
#include "native/select/selector.h"

namespace native {

// `cppgm++ --stats-functions` (telemetry): for a function the object back
// end selected into `mir`, one line
//
//   function_census symbol=<name> mir=N mov_loads=N mov_stores=N
//     mov_copies=N planned=N grants=N releases=N spills=N frame_homes=N
//     grant_busy=N grant_busy_parameters=N grant_busy_values=N
//     defined_in_plan=N defined_frame=N defined_other_register=N
//
// and one `loop_census symbol=<name> header_label=<label> header=N blocks=N
// mir=N calls=N eh=N depth=N frame_operands=N frame_bindings=N
// callee_saved=N` line per natural loop of its MIR, by header block.
//
// From the MIR: its instructions; its frame loads and stores and its
// register-to-register copies (the movement); per loop, the blocks, their
// instructions, calls, exception-handling instructions and frame operands,
// the loops that strictly contain it, and the function's frame bindings
// and callee-saved registers. From the selector's counters around the
// function (`before`, `after`): the cross-block values given a
// function-level register (planned), the block-local allocator's grants
// and releases, the values spilled and the frame homes made, the
// allocations that found every register held (by a parameter's value
// among them, or by other values only), and where cross-block values were
// defined (SelectStats).
void append_function_census(const lowir::Program& program, const Function& mir, const SelectStats& before,
	const SelectStats& after, std::vector<std::string>& lines);

} // namespace native
