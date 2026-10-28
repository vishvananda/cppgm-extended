#pragma once

#include "lowir/instruction.h"

namespace native {

// A source location at a byte offset from an encoded function's start.
struct CodeLocation
{
	uint32_t offset;
	lowir::DebugLocation location;
};

} // namespace native
