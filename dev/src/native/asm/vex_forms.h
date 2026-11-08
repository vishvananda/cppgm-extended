#pragma once

#include "asm/parser.h"
#include "native/x86/assembler.h"

namespace native {
// Literal VEX register views use their architectural numbers. They do not
// widen the ordinary MIR register sets or fabricate scalar value carriers.
bool encode_vex_form(Assembler& out, const assembly::Instruction& instruction);
void encode_assembly_vector_transfer(Assembler& out, const asm_model::Register& reg, const RM& memory, bool store);
} // namespace native
