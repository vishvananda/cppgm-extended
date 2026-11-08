#pragma once

#include "native/asm/forms.h"
#include "native/x86/assembler.h"
#include "asm/parser.h"

namespace native {
void encode_extend_form(Assembler& out, const assembly::Instruction& inst, const asm_detail::Form& form);
} // namespace native
