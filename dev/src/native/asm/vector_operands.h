#pragma once

#include "asm/parser.h"
#include "native/x86/assembler.h"
#include "x86_asm_model.h"

namespace native {
struct VectorEncodingPlan
{
	unsigned bits, memory_bits, reg, source;
	RM other;
	uint8_t opcode, control;
	bool has_immediate;
};
VectorEncodingPlan prepare_vector_encoding(
	const assembly::Instruction& inst, const x86_asm_model::VexForm& form, unsigned register_count = 16);
} // namespace native
