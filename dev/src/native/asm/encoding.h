#pragma once

#include "asm/parser.h"
#include "native/x86/assembler.h"

namespace native {

// Source assembly uses the same prefix/ModRM/fixup encoder as selected MIR.
// A failure names an unsupported or invalid instruction form; no host
// assembler is consulted by production compilation.
void encode_assembly(Assembler& out, const std::vector<assembly::Instruction>& instructions);

} // namespace native
