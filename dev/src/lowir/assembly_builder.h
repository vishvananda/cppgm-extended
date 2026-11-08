#pragma once

#include "asm/packet.h"
#include "lowir/program.h"

namespace lowir {

// Imports one checked frontend packet. The types correspond to its source
// operands; invocation Values are attached separately by FunctionBuilder.
uint32_t add_assembly(Program& program, const assembly::Packet& packet, const Type* types, DebugLocation location);

} // namespace lowir
