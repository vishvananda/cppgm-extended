#pragma once

#include "asm/parser.h"
#include "native/x86/assembler.h"

namespace native {
bool encode_mask_form(Assembler& out, const assembly::Instruction& instruction);
} // namespace native
