#pragma once

// Optional typed machine-IR model scaffold.
//
// PA28/PA38 MIR dumps should serialize the same machine-level program that the
// native backend is using, not a second display-only representation.

#include <string>
#include <vector>

namespace mir_model {

enum MirOperandKind
{
  MIR_OPERAND_REGISTER,
  MIR_OPERAND_VIRTUAL_REGISTER,
  MIR_OPERAND_STACK_SLOT,
  MIR_OPERAND_GLOBAL,
  MIR_OPERAND_LABEL,
  MIR_OPERAND_IMMEDIATE,
  MIR_OPERAND_MEMORY
};

enum MirInstructionKind
{
  MIR_INSTRUCTION_MOVE,
  MIR_INSTRUCTION_LOAD,
  MIR_INSTRUCTION_STORE,
  MIR_INSTRUCTION_ADDRESS,
  MIR_INSTRUCTION_UNARY,
  MIR_INSTRUCTION_BINARY,
  MIR_INSTRUCTION_COMPARE,
  MIR_INSTRUCTION_CONVERT,
  MIR_INSTRUCTION_CALL,
  MIR_INSTRUCTION_BRANCH,
  MIR_INSTRUCTION_JUMP,
  MIR_INSTRUCTION_RETURN,
  MIR_INSTRUCTION_PSEUDO
};

struct MirType
{
  std::string text;
};

struct MirOperand
{
  MirOperandKind kind = MIR_OPERAND_REGISTER;
  std::string text;
  MirType type;
};

struct MirInstruction
{
  MirInstructionKind kind = MIR_INSTRUCTION_PSEUDO;
  std::string opcode;
  std::vector<MirOperand> outputs;
  std::vector<MirOperand> inputs;
  std::vector<std::string> annotations;
};

struct MirBlock
{
  std::string name;
  std::vector<MirInstruction> instructions;
};

struct MirFunction
{
  std::string name;
  std::vector<MirBlock> blocks;
};

struct MirProgram
{
  std::string target;
  std::vector<MirFunction> functions;
};

MirProgram parse_mir_program_text(const std::string & text,
                                  const std::string & source_name);
MirProgram parse_mir_program_files(const std::vector<std::string> & paths);
std::string serialize_mir_program(const MirProgram & program);
void write_mir_program_file(const std::string & path,
                            const MirProgram & program);

}  // namespace mir_model
