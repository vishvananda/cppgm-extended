#pragma once

#include <stdexcept>
#include <string>
#include <vector>

#include "symbol_linkage.h"

namespace lowir_internal {

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

enum SymbolRole
{
  SR_NONE,
  SR_ENTRY,
  SR_INIT,
  SR_FINI,
  SR_EH_TOP,
  SR_EH_VALUE,
  SR_EH_TYPE,
  SR_EH_UNHANDLED,
  SR_EH_ALLOCATE_EXCEPTION,
  SR_EH_BEGIN_CATCH,
  SR_EH_CALL_UNEXPECTED,
  SR_EH_CURRENT_EXCEPTION_TYPE,
  SR_EH_END_CATCH,
  SR_EH_RETHROW,
  SR_EH_THROW,
  SR_EH_PERSONALITY,
  SR_EH_RESUME
};

enum LanguageLinkageMode
{
  LLM_DEFAULT,
  LLM_C,
  LLM_CPP
};

enum SymbolBindingMode
{
  SBM_DEFAULT,
  SBM_INTERNAL,
  SBM_STRONG,
  SBM_WEAK
};

enum ParamPassingMode
{
  PPM_DIRECT,
  PPM_INDIRECT_RESULT,
  PPM_BY_ADDRESS,
  PPM_REFERENCE,
  PPM_DECAY
};

enum ParamCaptureMode
{
  PCM_DEFAULT,
  PCM_NOCAPTURE,
  PCM_MAYCAPTURE
};

enum ParamAccessMode
{
  PAM_DEFAULT,
  PAM_NONE,
  PAM_READ,
  PAM_WRITE,
  PAM_READWRITE
};

enum ParamAliasMode
{
  PALM_DEFAULT,
  PALM_NOALIAS
};

enum CallArityMode
{
  CAM_FIXED,
  CAM_VARIADIC,
  CAM_PROTOTYPE_RELAXED
};

enum CallEffectsMode
{
  CFXM_DEFAULT,
  CFXM_READNONE,
  CFXM_READONLY,
  CFXM_READWRITE
};

enum CallUnwindMode
{
  CUM_DEFAULT,
  CUM_MAY,
  CUM_NO
};

enum CallReturnMode
{
  CRM_DEFAULT,
  CRM_RETURNS,
  CRM_NORETURN
};

enum GlobalStorageMode
{
  GSM_DEFAULT,
  GSM_WRITABLE,
  GSM_READONLY,
  GSM_THREAD_LOCAL
};

enum IndexProjectionKind
{
  IPK_NONE,
  IPK_ARRAY_ELEMENT,
  IPK_FIELD,
  IPK_BASE_SUBOBJECT,
  IPK_REFERENCE_FIELD
};

struct SymbolMetadata
{
  SymbolRole role = SR_NONE;
  LanguageLinkageMode linkage = LLM_DEFAULT;
  SymbolBindingMode binding = SBM_DEFAULT;
  std::string object_symbol;
  bool keep_internal_alias = false;
  bool prefer_local_object_binding = false;
  bool object_trivial_lifecycle = false;
};

struct FunctionBoundaryMetadata
{
  CallArityMode arity = CAM_FIXED;
  CallEffectsMode effects = CFXM_DEFAULT;
  CallUnwindMode unwind = CUM_DEFAULT;
  CallReturnMode returns = CRM_DEFAULT;
};

struct ParameterMetadata
{
  ParamPassingMode passing = PPM_DIRECT;
  ParamCaptureMode capture = PCM_DEFAULT;
  ParamAccessMode access = PAM_DEFAULT;
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

struct Program
{
  std::vector<GlobalDeclaration> global_declarations;
  std::vector<GlobalDefinition> globals;
  std::vector<FunctionDeclaration> function_declarations;
  std::vector<Function> functions;
  std::vector<symbol_linkage::SymbolIdentity> exported_symbols;
};

Program parse_program(const std::vector<std::string> & srcfiles);
Program parse_program_text(const std::string & text,
                           const std::string & label = std::string("<memory>"));
void canonicalize_program_export_metadata(Program & program);
LowType parse_type_text(const std::string & text,
                        const std::string & label = std::string("<memory>"));
Operand parse_operand_text(const std::string & text,
                           const std::string & label = std::string("<memory>"));
GlobalDefinition::DataItem parse_global_data_item_text(
    const std::string & text,
    const std::string & label = std::string("<memory>"));
Instruction parse_instruction_text(const std::string & text,
                                   const std::string & label = std::string("<memory>"));
std::string dump_program(const Program & program);
std::string mangle_name(const std::string & qualified);
bool is_plain_identifier_text(const std::string & text);
std::string lowir_debug_value_temp_name(const std::string & source_name,
                                        std::size_t version);
bool lowir_debug_value_source_name(const std::string & temp_name,
                                   std::string & source_name);
bool is_object_type(const LowType & type);
std::size_t type_size(const LowType & type);
std::size_t type_alignment(const LowType & type);
bool is_sign_extended_integer_type(const LowType & type);
const char * symbol_role_text(SymbolRole role);
const char * language_linkage_text(LanguageLinkageMode linkage);
const char * symbol_binding_text(SymbolBindingMode binding);
const char * global_storage_text(GlobalStorageMode storage);
const char * index_projection_text(IndexProjectionKind kind);
const char * param_passing_mode_text(ParamPassingMode mode);
const char * param_capture_mode_text(ParamCaptureMode mode);
const char * param_access_mode_text(ParamAccessMode mode);
const char * param_alias_mode_text(ParamAliasMode mode);
const char * call_arity_mode_text(CallArityMode mode);
const char * call_effects_mode_text(CallEffectsMode mode);
const char * call_unwind_mode_text(CallUnwindMode mode);
const char * call_return_mode_text(CallReturnMode mode);
bool is_function_symbol_role(SymbolRole role);
bool is_global_symbol_role(SymbolRole role);
bool is_host_eh_symbol_role(SymbolRole role);

}  // namespace lowir_internal
