#pragma once

// Optional typed LowIR model scaffold.
//
// LowIR text is the external compiler boundary. A compiler implementation may
// keep this shape, a richer equivalent shape, or a different internal shape, but
// every backend-relevant fact must serialize to and parse back from LowIR text.

#include <cstddef>
#include <string>
#include <vector>

namespace lowir_model {

enum LowirTopLevelKind
{
  LOWIR_TOP_LEVEL_FUNCTION_DECLARATION,
  LOWIR_TOP_LEVEL_FUNCTION_DEFINITION,
  LOWIR_TOP_LEVEL_GLOBAL_DECLARATION,
  LOWIR_TOP_LEVEL_GLOBAL_DEFINITION,
  LOWIR_TOP_LEVEL_OBJECT_ALIAS
};

enum LowirOperandKind
{
  LOWIR_OPERAND_TEMP,
  LOWIR_OPERAND_SLOT,
  LOWIR_OPERAND_GLOBAL,
  LOWIR_OPERAND_LABEL,
  LOWIR_OPERAND_INTEGER,
  LOWIR_OPERAND_FLOAT
};

enum LowirInstructionKind
{
  LOWIR_INSTRUCTION_CONST,
  LOWIR_INSTRUCTION_COPY,
  LOWIR_INSTRUCTION_ADDR,
  LOWIR_INSTRUCTION_LOAD,
  LOWIR_INSTRUCTION_STORE,
  LOWIR_INSTRUCTION_INDEX,
  LOWIR_INSTRUCTION_UNARY,
  LOWIR_INSTRUCTION_BINARY,
  LOWIR_INSTRUCTION_COMPARE,
  LOWIR_INSTRUCTION_CONVERT,
  LOWIR_INSTRUCTION_CALL,
  LOWIR_INSTRUCTION_BRANCH,
  LOWIR_INSTRUCTION_JUMP,
  LOWIR_INSTRUCTION_RETURN,
  LOWIR_INSTRUCTION_SWITCH,
  LOWIR_INSTRUCTION_COPY_OBJECT,
  LOWIR_INSTRUCTION_ZERO_INIT,
  LOWIR_INSTRUCTION_LANDING_PAD,
  LOWIR_INSTRUCTION_THROW,
  LOWIR_INSTRUCTION_RESUME
};

struct LowirType
{
  std::string text;
};

struct LowirMetadataItem
{
  std::string key;
  std::string value;
  bool value_is_symbol = false;
};

struct LowirOperand
{
  LowirOperandKind kind = LOWIR_OPERAND_TEMP;
  std::string text;
  LowirType literal_type;
};

struct LowirParameter
{
  std::string name;
  LowirType type;
  std::vector<LowirMetadataItem> metadata;
};

struct LowirInstruction
{
  LowirInstructionKind kind = LOWIR_INSTRUCTION_COPY;
  std::string result;
  std::string op;
  LowirType type;
  LowirType source_type;
  std::vector<LowirOperand> operands;
  std::vector<std::string> labels;
  std::vector<LowirMetadataItem> metadata;
};

struct LowirBlock
{
  std::string name;
  std::vector<LowirInstruction> instructions;
};

struct LowirFunction
{
  bool is_declaration = false;
  std::string name;
  LowirType return_type;
  std::vector<LowirParameter> parameters;
  std::vector<LowirMetadataItem> metadata;
  std::vector<LowirBlock> blocks;
};

struct LowirGlobalDataItem
{
  std::string bytes;
  std::string symbol;
  std::size_t addend = 0;
};

struct LowirGlobal
{
  bool is_declaration = false;
  std::string name;
  LowirType type;
  std::size_t size = 0;
  std::size_t alignment = 1;
  std::vector<LowirMetadataItem> metadata;
  std::vector<LowirGlobalDataItem> data;
};

struct LowirObjectAlias
{
  std::string object_symbol;
  std::string target_symbol;
};

struct LowirProgram
{
  std::vector<LowirGlobal> globals;
  std::vector<LowirFunction> functions;
  std::vector<LowirObjectAlias> aliases;
};

LowirProgram parse_lowir_program_text(const std::string & text,
                                      const std::string & source_name);
LowirProgram parse_lowir_program_files(const std::vector<std::string> & paths);
std::string serialize_lowir_program(const LowirProgram & program);
void write_lowir_program_file(const std::string & path,
                              const LowirProgram & program);

}  // namespace lowir_model
