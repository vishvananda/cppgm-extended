#pragma once

#include <string>
#include <vector>

#include "asm_model.h"
#include "lowir/program.h"

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

// The declaration retains the ABI; ordinary optimization never sees this body.
// Raw bodies contain assembly invocations followed by unreachable, with only
// constant or symbol arguments. The assembly owns every ABI register and stack.
struct AssemblyFunction
{
	SymbolId symbol;
	std::unique_ptr<FunctionBody> body;
};

// Ordered module source, independent of any function. Values are constants
// or typed symbol identities; no runtime storage or register setup is implied.
struct AssemblyGlobal
{
	uint32_t definition;
	std::vector<Value> arguments;
	std::vector<Literal> literals;
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
	std::vector<AssemblyFunction> functions;
	std::vector<AssemblyGlobal> globals;
};

const AssemblyFunction* find_assembly_function(const Program& program, SymbolId symbol);
void retain_assembly_function(Program& program, SymbolId symbol);
bool validate_assembly_functions(const Program& program, const char*& error);
bool validate_assembly_globals(const Program& program, const char*& error);

} // namespace lowir
