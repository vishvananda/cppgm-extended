#pragma once

#include <cstdint>
#include <vector>

#include "native/mir/mir.h"

namespace native {

// One frame allocation (FunctionSelector::frame_alloc): bytes at offset.
struct FrameObject
{
	int32_t offset;
	uint32_t bytes;
	uint32_t align;
};

// O1+, after the MIR cleanups: the frame objects reordered by the
// estimated frequency of their accesses, the hottest nearest the frame
// register (below RBP, or above RSP's preserve area without a frame
// pointer) where more accesses take a one-byte displacement. Every frame
// operand must start inside an object; the objects one access spans move
// as one. A function with operands carrying frame offsets elsewhere
// (va_start, course EH records), or with an address formed outside its
// object (the selector's frame_pinned_), keeps its layout. Returns whether
// it moved any.
bool order_frame(Function& function, const std::vector<FrameObject>& objects);

// O1+, after the MIR cleanups: stores (and bulk copies or zeroing) into
// frame objects whose bytes no path reads before they are overwritten or
// the function returns go. Only objects every access of which is a direct
// load, store, compare, test or bulk copy of known bytes are tracked (no
// address taken, no index, not the scratch area or the EH slot); a
// function with course EH records or a variable argument list keeps its
// stores. Host landing reads join liveness at each throwing instruction,
// before later normal-path overwrites. Backward liveness runs to a fixed
// point within bounded scratch/work; exhaustion keeps all stores.
// Returns the stores dropped.
uint32_t drop_dead_frame_stores(Function& function, const std::vector<FrameObject>& objects);

// The frame object holding a frame offset (lowir::kNoId: none).
class FrameObjectIndex
{
public:
	explicit FrameObjectIndex(const std::vector<FrameObject>& objects);
	uint32_t find(int32_t disp) const;
	const std::vector<FrameObject>& objects() const { return objects_; }

private:
	const std::vector<FrameObject>& objects_;
	std::vector<uint32_t> by_offset_;
};

// Per object, whether drop_dead_frame_stores tracks it: every access a
// direct load, store, compare, test or bulk copy of known bytes inside it
// (so no address of it exists for a pointer to write through).
std::vector<uint8_t> tracked_frame_objects(const Function& function, const FrameObjectIndex& index);

} // namespace native
