#pragma once

#include "asm/parser.h"
#include "native/x86/assembler.h"

namespace native {
void encode_evex_vector_transfer(Assembler& out, const asm_model::Register& reg, const RM& memory, bool store);
bool encode_evex_form(Assembler& out, const assembly::Instruction& instruction);
} // namespace native
