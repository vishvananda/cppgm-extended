#pragma once
#include "lowir/model/program.h"

namespace lowir_opt {

// Whether a binary operation's operands may be exchanged.
bool commutative(lowir_model::LowOperation op);
// Fold one instruction with literal operands into `*result`; false when an
// operand is not a literal or the fold is not defined for the type.
bool fold_unary(const lowir_model::Instruction & ins, lowir_model::Operand * result);
bool fold_binary(const lowir_model::Instruction & ins, lowir_model::Operand * result);
bool fold_compare(const lowir_model::Instruction & ins, lowir_model::Operand * result);
bool fold_convert(const lowir_model::Instruction & ins, lowir_model::Operand * result);
// x + 0, x * 1, x & ~0 and their mirrors: the operand the operation returns.
bool algebraic_identity(const lowir_model::Instruction & ins, lowir_model::Operand * result);

}  // namespace lowir_opt
