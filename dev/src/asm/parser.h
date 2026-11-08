#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "asm_model.h"

namespace assembly {

enum class OperandKind : uint8_t
{
	Register,
	Memory,
	Immediate,
	Label,
	Symbol
};

enum class Relocation : uint8_t
{
	Address,
	Got,
	GotTpOff,
	Tlv
};

struct SymbolUse
{
	uint32_t first;
	uint32_t count;
	asm_model::SymbolReference reference;
};

struct Address
{
	asm_model::Register base = {};
	asm_model::Register index = {};
	bool has_base = false;
	bool has_index = false;
	uint8_t scale = 1;
	int32_t displacement = 0;
	bool rip = false;
	asm_model::SymbolReference symbol;
	Relocation relocation = Relocation::Address;
};

struct Operand
{
	OperandKind kind = OperandKind::Immediate;
	asm_model::Register reg = {};
	Address address;
	uint64_t immediate = 0;
	uint16_t bits = 0;
	std::string target;
	uint32_t label = 0;
};

struct Instruction
{
	bool label = false;
	uint8_t prefix = 0;
	std::string mnemonic;
	std::vector<Operand> operands; // destination first in either source dialect
	uint32_t line = 1;
	uint32_t column = 1;
	uint16_t bits = 0; // explicit mnemonic suffix, or zero
};

struct AssemblyError
{
	std::string message;
	uint32_t line;
	uint32_t column;
};

// The input is user assembly after its frontend's placeholder expansion.
// This is its only instruction parse; ordinary compiler code never uses it.
std::vector<Instruction> parse(
	const std::string& source, asm_model::Dialect dialect, const std::vector<SymbolUse>& symbols = {});

} // namespace assembly
