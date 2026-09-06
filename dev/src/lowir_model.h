#pragma once

// Optional typed LowIR model scaffold.
//
// This header is a teaching shape, not the course solution's own model: it
// is deliberately simple (names are plain strings) so it can be read whole.
//
// LowIR text is the durable compiler boundary introduced in PA13. This header
// gives one possible in-memory shape for that text. You may use it directly,
// adapt it, or replace it with your own equivalent model, but backend-visible
// facts must still serialize to and parse back from LowIR text.

#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "ir_symbol_model.h"

namespace lowir_model {

// The metadata vocabulary is shared with the machine-IR scaffold and the
// course solution; see ir_symbol_model.h.
using namespace ir_model;

struct ParseError : std::runtime_error
{
  explicit ParseError(const std::string & message)
    : std::runtime_error(message)
  {}
};

struct LowType
{
  std::string text;
};

struct Operand
{
  enum Kind
  {
    OP_TEMP,
    OP_SLOT,
    OP_GLOBAL,
    OP_LABEL,
    OP_INTEGER,
    OP_FLOAT
  } kind = OP_INTEGER;

  std::string text;
  long long int_value = 0;
  long double float_value = 0.0L;
  LowType literal_type;
};

struct SymbolMetadata
{
  SymbolRole role = SR_NONE;
  LanguageLinkageMode linkage = LLM_DEFAULT;
  SymbolBindingMode binding = SBM_DEFAULT;
  std::string object_symbol;
  std::string tls_for_symbol;
  std::string section_segment;
  std::string section_name;
  bool keep_internal_alias = false;
  bool prefer_local_object_binding = false;
  bool object_output_root = false;
  bool force_inline = false;
};

struct ParameterMetadata
{
  ParamPassingMode passing = PPM_DIRECT;
  ParamAliasMode alias = PALM_DEFAULT;
};

struct Parameter
{
  std::string name;
  LowType type;
  ParameterMetadata metadata;
};

struct InstructionDebugLocation
{
  std::string file;
  std::size_t line = 0;
  std::size_t column = 0;

  bool present() const
  {
    return !file.empty() && line != 0 && column != 0;
  }
};

struct GlobalDeclaration
{
  std::string name;
  bool has_type = false;
  LowType type;
  GlobalStorageMode storage = GSM_DEFAULT;
  SymbolMetadata metadata;
};

struct GlobalDefinition
{
  struct DataItem
  {
    enum Kind
    {
      ITEM_INTEGER,
      ITEM_ADDR,
      ITEM_ZERO
    } kind = ITEM_INTEGER;

    LowType type;
    Operand literal_operand;
    std::string symbol;
    long long addr_addend = 0;
    std::size_t zero_bytes = 0;
  };

  std::string name;
  bool structured = false;
  GlobalStorageMode storage = GSM_DEFAULT;
  LowType type;
  enum InitKind
  {
    INIT_ZERO,
    INIT_INTEGER,
    INIT_ADDR
  } init_kind = INIT_ZERO;
  Operand init_operand;
  long long addr_addend = 0;
  std::vector<DataItem> data_items;
  SymbolMetadata metadata;
};

struct Instruction
{
  enum Kind
  {
    IK_CONST,
    IK_COPY,
    IK_ADDR,
    IK_LOAD,
    IK_ATOMIC_LOAD,
    IK_STORE,
    IK_ATOMIC_STORE,
    IK_ATOMIC_EXCHANGE,
    IK_INDEX,
    IK_UNARY,
    IK_BINARY,
    IK_CMP,
    IK_CONVERT,
    IK_ATOMIC_ADD_FETCH,
    IK_ATOMIC_COMPARE_EXCHANGE,
    IK_ATOMIC_THREAD_FENCE,
    IK_ATOMIC_SIGNAL_FENCE,
    IK_VA_START,
    IK_VA_ARG,
    IK_STACK_ALLOC,
    IK_CALL,
    IK_COPYOBJ,
    IK_ZEROINIT,
    IK_EH_TRY,
    IK_EH_CLEANUP,
    IK_EH_CLEANUP_CLAUSE,
    IK_EH_CATCH,
    IK_EH_FILTER,
    IK_EH_CATCH_ALL,
    IK_EH_END,
    IK_THROW,
    IK_EXCEPTION,
    IK_EXCEPTION_SELECTOR,
    IK_RESUME,
    IK_JUMP,
    IK_BRANCH,
    IK_SWITCH,
    IK_RETURN
  } kind = IK_CONST;

  std::string dest;
  LowType type;
  LowType source_type;
  std::string op;
  std::size_t byte_count = 0;
  std::size_t byte_alignment = 1;
  bool has_eh_selector = false;
  long long eh_selector = 0;
  IndexProjectionKind index_projection = IPK_NONE;
  Operand first;
  Operand second;
  Operand third;
  std::vector<Operand> args;
  bool call_returns_void = false;
  bool has_call_signature = false;
  std::vector<Parameter> call_params;
  LowType call_return_type;
  FunctionBoundaryMetadata call_boundary;
  InstructionDebugLocation debug_location;
};

struct Block
{
  std::string label;
  std::vector<Instruction> instructions;
};

struct Function
{
  std::string name;
  std::vector<Parameter> params;
  LowType return_type;
  std::vector<std::pair<std::string, LowType> > slots;
  std::vector<Block> blocks;
  InstructionDebugLocation debug_location;
  FunctionBoundaryMetadata boundary;
  SymbolMetadata metadata;
};

struct FunctionDeclaration
{
  std::string name;
  std::vector<Parameter> params;
  LowType return_type;
  FunctionBoundaryMetadata boundary;
  SymbolMetadata metadata;
};

struct ObjectAlias
{
  std::string object_symbol;
  std::string target;
};

struct Program
{
  std::vector<GlobalDeclaration> global_declarations;
  std::vector<GlobalDefinition> globals;
  std::vector<FunctionDeclaration> function_declarations;
  std::vector<Function> functions;
  std::vector<ObjectAlias> object_aliases;
  std::vector<ir_model::ExportedSymbol> exported_symbols;
};

using LowirType = LowType;
using LowirOperand = Operand;
using LowirParameter = Parameter;
using LowirInstruction = Instruction;
using LowirBlock = Block;
using LowirFunction = Function;
using LowirFunctionDeclaration = FunctionDeclaration;
using LowirGlobalDeclaration = GlobalDeclaration;
using LowirGlobalDefinition = GlobalDefinition;
using LowirObjectAlias = ObjectAlias;
using LowirProgram = Program;

LowirProgram parse_lowir_program_text(const std::string & text,
                                      const std::string & source_name = std::string("<memory>"));
LowirProgram parse_lowir_program_files(const std::vector<std::string> & paths);
std::string serialize_lowir_program(const LowirProgram & program);

}  // namespace lowir_model
