#pragma once

#include <cstdint>

#include "lex/gnu_builtins.h"
#include "lowir/program.h"

namespace lower {

// An SSE builtin's types.
struct SseForm
{
	bool scalar = false;        // leaves the first operand's other lanes
	bool vector_result = false; // a sixteen-byte vector, not a scalar
	bool floating = false;      // the result lanes
	uint32_t lane_bytes = 0;    // the result lanes
	bool source_floating = false;
	uint32_t source_bytes = 0;
	bool second_vector = false; // the second operand is a vector
};

struct SseInstruction
{
	lowir::VectorOp op = lowir::VectorOp::Pzero;
	bool swapped = false;         // the operands in the other order
	bool repeated = false;        // the one operand as both operands
	int32_t immediate = -1;       // a fixed immediate
	int32_t literal_operand = -1; // the argument that is the immediate
};

// Whether a `vector` instruction computes the builtin.
bool SseInstructionOf(VectorLanewiseOp operation, const SseForm& form, SseInstruction& out);

} // namespace lower
