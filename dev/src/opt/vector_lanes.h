#pragma once

#include "lowir/instruction.h"

namespace opt {

// Common lane semantics for loop widening and straight-line packing. A false
// result needs an expansion or a target capability, rather than a single op.
unsigned lane_bytes(lowir::Type type);
bool lane_operation(lowir::BinaryOp op, lowir::Type type, lowir::VectorOp& out);

} // namespace opt
