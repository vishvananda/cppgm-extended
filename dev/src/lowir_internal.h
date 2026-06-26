#pragma once

#include "lowir_model.h"

namespace lowir_internal {

using namespace lowir_model;

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
