#pragma once

#include <string>
#include <vector>

#include "asm_model.h"

namespace assembly {

template<typename T>
struct View
{
	const T* data = nullptr;
	uint32_t size = 0;
};

struct InputConstant
{
	asm_model::Bits bits = {};
	bool present = false;
};

// A frontend's arena-owned checked packet; value types stay with that
// frontend until it constructs the corresponding typed LowIR operands.
struct Packet
{
	asm_model::Template definition;
	const char* text = nullptr;
	uint32_t text_size = 0;
	View<asm_model::Operand> operands;
	View<asm_model::Alternative> alternatives;
	View<asm_model::RegisterGroup> groups;
	View<asm_model::Immediate> immediates;
	View<asm_model::Piece> pieces;
	View<asm_model::Register> registers;
	View<asm_model::Register> clobbers;
	View<InputConstant> constants;
};

struct PacketBuilder
{
	asm_model::Template definition;
	std::string text;
	std::vector<asm_model::Operand> operands;
	std::vector<asm_model::Alternative> alternatives;
	std::vector<asm_model::RegisterGroup> groups;
	std::vector<asm_model::Immediate> immediates;
	std::vector<asm_model::Piece> pieces;
	std::vector<asm_model::Register> registers;
	std::vector<asm_model::Register> clobbers;
	std::vector<InputConstant> constants;
};

struct GnuOperand
{
	std::string name;
	std::string constraint;
	unsigned bits = 0;
	bool output = false;
	bool is_volatile = false;
};

// Source-level placeholder/constraint parsing. No C++ AST or LowIR types,
// and no attempt to parse instructions before physical operands are bound.
PacketBuilder gnu_packet(const std::string& source, const std::vector<GnuOperand>& operands,
	const std::vector<std::string>& clobbers, bool is_volatile, bool basic);

} // namespace assembly
