#pragma once

#include "asm/module_text.h"

namespace assembly {

// Absolute 64-bit expressions, using GNU assembler precedence and truth values.
uint64_t module_expression(const ModuleLine& line, uint32_t first = 0);

} // namespace assembly
