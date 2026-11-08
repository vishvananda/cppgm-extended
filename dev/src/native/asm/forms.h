#pragma once

#include "x86_asm_model.h"
#include <string>

namespace native {
namespace asm_detail {
int branch_condition(const std::string& mnemonic);

using x86_asm_model::Encoding;
using x86_asm_model::Form;

const Form* assembly_form(const std::string& mnemonic, uint16_t& suffix);

} // namespace asm_detail
} // namespace native
