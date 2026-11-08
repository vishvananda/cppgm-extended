#pragma once

#include <cstddef>
#include <string>

#include "asm_model.h"

namespace assembly {

// Target register vocabulary, independent of a source frontend and of MIR's
// compact register numbering. Names have no percent sigil. The caller decides
// whether a known architectural view is legal in a particular operand role.
bool parse_register(const char* name, std::size_t size, asm_model::Register& reg);
std::string register_name(const asm_model::Register& reg);

} // namespace assembly
