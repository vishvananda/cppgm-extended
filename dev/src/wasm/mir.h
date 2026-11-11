#pragma once

#include <cstdint>
#include <stdexcept>
#include <vector>

#include "lowir/program.h"
#include "machine_model.h"
#include "wasm_model.h"

namespace wasm {

using namespace wasm_model;

class WasmError : public std::runtime_error
{
public:
	explicit WasmError(const std::string& message) : std::runtime_error(message) {}
};

struct Relocation
{
	RelocationKind kind;
	std::uint32_t offset;
	std::uint32_t index;
	std::int64_t addend;
};

struct Instruction
{
	Op op;
	machine_model::Range operands;
	std::uint32_t location;
};

// One selected function. Emission and the diagnostic dump read this exact
// stream; immediate operands are typed by the opcode's vocabulary form.
struct Function
{
	std::vector<ValueType> locals;
	std::vector<Instruction> instructions;
	std::vector<std::uint64_t> operands;
	// Offsets here identify operands; encoding converts them to byte offsets.
	std::vector<Relocation> relocations;
	std::uint32_t parameter_count = 0;
	std::uint32_t frame_bytes = 0;
	std::uint32_t location = 0;

	void emit(Op op, std::initializer_list<std::uint64_t> values = {})
	{
		const machine_model::Range range{ static_cast<std::uint32_t>(operands.size()),
			static_cast<std::uint32_t>(values.size()) };
		operands.insert(operands.end(), values.begin(), values.end());
		instructions.push_back(Instruction{ op, range, location });
	}

	void relocate(RelocationKind kind, std::uint32_t index, std::uint32_t operand = 0, std::int64_t addend = 0)
	{
		relocations.push_back(Relocation{ kind, instructions.back().operands.first + operand, index, addend });
	}
};

ValueType value_type(lowir::Type type);

} // namespace wasm
