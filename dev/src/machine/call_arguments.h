#pragma once

#include "lowir/program.h"

namespace machine {

void apply_argument_types(
	const lowir::FunctionBody& body, const lowir::Instruction& instruction, std::vector<lowir::Type>& types);

ir_model::AggregateShape argument_shape(
	const lowir::FunctionBody& body, const lowir::Instruction& instruction, uint32_t argument);

// Boundary formals determine named argument types; unnamed arguments retain
// their explicitly recorded scalar view or their LowIR operand's storage type.
void call_argument_types(const lowir::FunctionBody& body, const lowir::Instruction& inst,
	const lowir::Signature& signature, const lowir::Parameter* parameters, std::vector<lowir::Type>& output);

} // namespace machine
