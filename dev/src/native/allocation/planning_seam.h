#pragma once
#include "native/allocation/location_planning.h"
#include "native/analysis/function.h"
#include <cstddef>
#include <utility>
#include <vector>

// The seam between the facts of a function and an allocator's plan.
//
// `plan_value_locations` (location_planning.h) computes the layout facts
// below from the function, then asks an allocator to decide which values
// live in which registers over which positions.  The function it sees is
// the post-split layout: critical edges have been split into synthesized
// `__phi_edge_N` blocks appended after the original blocks, whose jumps
// back into the body are layout backedges.  "Loop-carried" below means
// exactly "has a layout-backedge predecessor", nothing about source loops.
// Positions number every instruction of that layout in block order, from
// 0, and every interval and span here is inclusive at both ends: a call at
// an interval's begin or end is inside it.
//
// `FunctionFacts` (native/analysis/function.h) gives each value's
// definition position, first and last use, its flags (live across a call,
// live across a block edge, a parameter, a loop invariant, only a call
// argument, only a storage address, ...), the positions of the calls, and
// for each register the positions at which the function clobbers it.
// `facts.uses[value]` is a use count; `facts.calls` and every
// `facts.clobber_positions[register]` are sorted ascending; a clobber at
// exactly a segment's begin or end position forbids the register.  A use
// of a value by a phi is recorded at the position of the predecessor
// block's terminator, where the transfer happens, not at the phi.
//
// A plan is a FunctionLocationTimeline: for each value, zero or more
// segments [begin, end] with a location kind and, for register kinds, the
// register index.  The reactive walk that lowers the function honours a
// PLK_GPR segment by keeping the value in that register from `begin` to
// `end` and by not handing the register to anything else there.  What a
// correct plan must respect:
//
// - A value may be planned only in a general register of the callee-saved
//   pool (rbx, r12, r13, r14, r15) or, when its whole segment contains no
//   call and no clobber of that register, of the caller pool (r8, r9).
//   No other register may be planned.
// - A register may not be planned for a value over any position at which
//   the function clobbers it (`facts.clobber_positions[register]`), nor
//   for a value whose `facts.live_across_clobbers[value]` mask names it.
// - rbx may not be planned when `facts.has_i128_atomic`.
// - Two values planned in one register must not have overlapping segments.
// - A segment must cover the value's whole life: from its definition to its
//   last use, extended to the end of every span in `layout.spans` that
//   contains the last use, repeatedly, when the value is live across a
//   block edge or is a phi.  For a loop-carried phi the segment written
//   into the timeline begins at position 0, because the walk reserves a
//   phi's home at construction and the home receives a predecessor's
//   transfer before the phi's own position; for deciding conflicts between
//   values, the phi's interval may be taken to begin at
//   `layout.phi_transfer_start`.  A value whose extended interval contains
//   a call may only use the callee-saved pool.
// - Merge phis (in `layout.phi_transfer_start` but not loop-carried) are
//   not plannable: they are written before their linear position by every
//   predecessor.  Parameters, values with no use, values used only as a
//   storage address, object-typed values, and values of a type wider than
//   64 bits or of floating type are not plannable either.
//
// `register_spans[register]` receives the planned [begin, end] intervals
// of each callee-saved register, so the walk can steer its own scratch
// choices away from them; the spans are advisory, and leaving them empty
// only costs later reactive choices, never correctness.
//
// The course fixtures judge a plan by the program's behaviour and by each
// fixture's `x.ref.expect`: a size envelope, and contract lines that name
// the exception regions and the calls a function keeps.  They do not name
// which values are in registers.
//
// Which candidates are worth a register is the allocator's business, but
// two facts save an experiment.  The reactive walk already places well,
// without a plan: parameters, a value used only as a call argument whose
// life contains no call, and a value used only as a storage address (it
// becomes a memory operand).  Planning every other eligible value, with
// the callee-saved pool for anything whose life contains a call, meets the
// performance envelope on the course fixtures.
namespace lowir_native {
namespace location_planning {

// One pass over the layout, computed by plan_value_locations.
struct LayoutScan
{
  // For each value: the position of the earliest predecessor terminator
  // that writes it, when the value is a phi; otherwise
  // FunctionFacts::missing_position().
  std::vector<std::size_t> phi_transfer_start;
  // For each value: nonzero when the value is a loop-carried phi (one of
  // its incoming edges is a layout backedge).
  std::vector<unsigned char> phi_loop_carried;
  // For each value: the position of its loop header when loop-carried.
  std::vector<std::size_t> phi_header_start;
  // Every layout backedge span and backward exception span, as
  // [target position, source position]: an interval that ends inside one
  // is live to its end.
  std::vector<std::pair<std::size_t, std::size_t> > spans;
  // Parallel to `spans`: nonzero for a loop backedge, zero for an
  // exception span.
  std::vector<unsigned char> span_is_backedge;
  // Every layout-forward jump as [source position, target position].
  std::vector<std::pair<std::size_t, std::size_t> > forward_edges;
};

// A second allocator at the seam, written from this header and the PA38
// README alone (dev/src/native/allocation/linear_scan.cpp); selected by
// CPPGM_BACKEND_VARIANT=linear-scan.  `register_spans` has sixteen entries,
// indexed by X64Register, cleared on entry.
FunctionLocationTimeline plan_value_locations_linear_scan(
    const lowir_model::LowirFunction & function,
    const analysis::FunctionFacts & facts,
    const LayoutScan & layout,
    std::vector<std::pair<std::size_t, std::size_t> > * register_spans,
    Stats * stats);

}  // namespace location_planning
}  // namespace lowir_native
