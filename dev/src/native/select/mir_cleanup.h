#pragma once

#include <vector>

#include "native/mir/mir.h"
#include "native/select/frame_layout.h"

// MIR cleanups after selection, before branch layout (O1+).
namespace native {

// Runs of consecutive stores of immediates to adjacent bytes of one base
// become the fewest aligned stores of the combined bytes (the rest nops),
// so a wider load of them forwards from one store.
void merge_constant_stores(Function& function);

// A register copy followed by an add, a subtract of an immediate, a shift
// left by 1 to 3 or a lea of its destination becomes one lea of the
// copied register. Returns the copies removed.
uint32_t fuse_copies_into_lea(Function& function);

// `add b, imm` and `add b, c` adjacent (either order, the flags dead)
// become one lea; a 32-bit zero-extension of a register whose upper half
// the block already zeroed becomes a nop. Returns the instructions removed.
uint32_t fuse_adds_and_extensions(Function& function);

// A `test r, r` repeating the flags the instruction before it set from r
// goes (the readers' conditions permitting). Returns the tests removed.
uint32_t drop_redundant_tests(Function& function);

// A 64-bit register copy between registers an earlier copy in the block
// made equal (neither named since) goes. Returns the copies removed.
uint32_t drop_redundant_copies(Function& function);

// Reloads of a temp's frame home a register still holds go or become
// register copies (feeding the scratch's readers directly). Returns the
// instructions removed.
uint32_t forward_home_reloads(Function& function);

// A load from a frame object a bulk copy of known bytes just filled reads
// the copy's source instead, while nothing since wrote memory a pointer
// could reach or the source's registers (a by-value argument copied into
// its slot and loaded into its register: drop_dead_frame_stores then
// drops the copy when the object is one it tracks). Returns the loads
// rewritten.
uint32_t forward_frame_copies(Function& function, const std::vector<FrameObject>& objects);

// A return's loads of rax/rdx from the frame take what its block stored
// there directly, and the block's frame writes nothing reads before the
// return go. Returns the writes removed.
uint32_t forward_return_slots(Function& function);

// A frame-pointer-free leaf whose instructions no longer use its frame
// reserves none. Returns whether it dropped it.
bool drop_unused_frame(Function& function);

// Loop rotation by tail duplication: a block jumping back to an earlier
// short loop test ends with a copy of it instead. Returns the back edges
// rotated.
uint32_t rotate_loops(Function& function);

// A jump to a test block that only decides between two successors on a
// register the jumping block set to a known value goes to the decided
// successor. Returns the jumps retargeted.
uint32_t thread_decided_tests(Function& function, const lowir::Program& program);

} // namespace native
