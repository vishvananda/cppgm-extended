#pragma once

#include <string>
#include <vector>

#include "asm_model.h"
#include "lowir/instruction.h"

namespace lowir {

using AssemblyDialect = asm_model::Dialect;
using AssemblyMemory = asm_model::Memory;
using AssemblyRegister = asm_model::Register;
using AssemblyRegisterGroup = asm_model::RegisterGroup;
using AssemblyAlternative = asm_model::Alternative;
using AssemblyImmediate = asm_model::Immediate;
using AssemblyPiece = asm_model::Piece;

// Source-independent constraints are shared with the frontends. Actual values
// and storage addresses belong to an invocation's ordinary LowIR operand list.
struct AssemblyOperand : asm_model::Operand
{
	Type type = Type::scalar(TypeKind::Void);
};

struct AssemblyTemplate : asm_model::Template
{
	NameId name = 0;
	DebugLocation location = {};
};

// Allocated only for a program containing assembly. Flat tables retain source
// ownership and keep the ordinary instruction and function-body records small.
struct AssemblyModule
{
	std::string text;
	std::vector<AssemblyTemplate> templates;
	std::vector<AssemblyOperand> operands;
	std::vector<AssemblyAlternative> alternatives;
	std::vector<AssemblyRegisterGroup> groups;
	std::vector<AssemblyImmediate> immediates;
	std::vector<AssemblyPiece> pieces;
	std::vector<AssemblyRegister> registers;
	std::vector<AssemblyRegister> clobbers;
};

} // namespace lowir
