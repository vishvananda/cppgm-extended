#pragma once

#include <utility>
#include <vector>

#include "native/mir/mir.h"

namespace native {

// Cheap guard before the register-home dataflow.
bool has_direct_frame_read(const Function& function);

// Replace an integer load with an extraction from a covering carrier.
// The caller proves the carrier still holds the bytes and flags are dead
// when a shift is needed. Insertions preserve the original load location.
bool extract_integer_carrier(
	Inst& inst, uint32_t index, Reg holder, unsigned shift, std::vector<std::pair<uint32_t, Inst>>& extractions);
// Entries are ordered by original instruction index, including prefix order.
void insert_frame_extractions(Function& function, std::vector<std::pair<uint32_t, Inst>>& extractions);

// Whether the existing frame policy permits removing unused storage.
// With dead_stores, plain frame stores may remain if no frame read/address does.
bool frame_storage_elision_allowed(const Function& function);
bool removable_frame_storage(const Function& function, bool dead_stores);

} // namespace native
