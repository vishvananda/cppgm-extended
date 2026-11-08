#pragma once
#include "asm/parser.h"
#include "native/x86/assembler.h"
#include "x86_asm_model.h"

namespace native {
void encode_integer_form(
	Assembler& out, const assembly::Instruction& inst, const x86_asm_model::Form& form, unsigned bits);
} // namespace native
