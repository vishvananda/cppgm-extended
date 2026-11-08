#pragma once

#include "asm/parser.h"
#include "native/x86/assembler.h"

namespace native {
void encode_assembly_mask_transfer(
	Assembler& out, const asm_model::Register& reg, const RM& memory, bool store, unsigned bytes);
bool encode_mask_form(Assembler& out, const assembly::Instruction& instruction);
} // namespace native
