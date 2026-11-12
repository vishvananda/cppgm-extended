#pragma once

#include "lowir/instruction.h"

namespace opt {

// Approximate expansion work, excluding operand reads and local assignment.
// Unknown or data-dependent expansions remain unpriced (UINT32_MAX), never
// silently charged as one native instruction. Required inlining is separate.
uint32_t wasm_inline_instruction_cost(const lowir::Instruction& inst);

} // namespace opt
