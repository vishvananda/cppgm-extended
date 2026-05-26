#include "cpp_toolchain.h"

#include "cpp_batch_frontend.h"

#include <chrono>
#include <cstdlib>
#include <ctime>
#include <exception>
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

using namespace std;

#include "cpp_decl_bridge.h"
#include "cpp_text_generators.h"
#include "cppast_parser.h"
#include "cpp_tool_cli.h"
#include "lowir_optimizer.h"
#include "lowir_object_backend.h"
#include "lowirgensemantic.h"
#include "optimization_level.h"
#include "posttokenizer.h"
#include "preprocessor.h"
#include "recog_parser.h"
#include "template_text_output.h"
#include "symbol_linkage.h"
#include "witness_api.h"
#include "recog_token_buffer.h"

namespace {

#ifndef CPPGM_DEFAULT_OBJECT_ROOT
#define CPPGM_DEFAULT_OBJECT_ROOT ""
#endif

bool phase_timing_enabled()
{
  static const bool enabled = []()
  {
    const char * value = std::getenv("CPPGM_PHASE_TIMING");
    return value && *value && std::string(value) != "0";
  }();
  return enabled;
}

class PhaseTimer
{
public:
  PhaseTimer(const char * name, const string & detail = string())
    : name_(name),
      detail_(detail),
      enabled_(phase_timing_enabled()),
      start_(enabled_ ? Clock::now() : Clock::time_point())
  {
  }

  ~PhaseTimer()
  {
    if(!enabled_) {
      return;
    }
    const auto elapsed =
        std::chrono::duration_cast<std::chrono::milliseconds>(Clock::now() - start_);
    std::cerr << "phase-timing"
              << " name=" << name_;
    if(!detail_.empty()) {
      std::cerr << " detail=" << detail_;
    }
    std::cerr << " ms=" << elapsed.count() << "\n";
  }

private:
  using Clock = std::chrono::steady_clock;

  const char * name_;
  string detail_;
  bool enabled_;
  Clock::time_point start_;
};

string source_count_detail(const vector<string> & srcfiles)
{
  return string("src-count=") + std::to_string(srcfiles.size());
}

string shell_quote(const string & value)
{
  if(value.empty()) {
    return "''";
  }
  string out = "'";
  for(size_t i = 0; i < value.size(); ++i) {
    if(value[i] == '\'') {
      out += "'\"'\"'";
    } else {
      out += value[i];
    }
  }
  out += "'";
  return out;
}

string run_command_capture_stdout(const string & command)
{
  FILE * pipe = popen(command.c_str(), "r");
  if(pipe == nullptr) {
    return string();
  }
  string output;
  char buffer[4096];
  while(fgets(buffer, sizeof(buffer), pipe) != nullptr) {
    output += buffer;
  }
  if(pclose(pipe) != 0) {
    return string();
  }
  return output;
}

string host_default_target_name()
{
#if defined(__APPLE__)
  return "macos";
#elif defined(__linux__)
  return "linux";
#else
  return string();
#endif
}

bool target_starts_with(const string & target, const string & prefix)
{
  return target.compare(0, prefix.size(), prefix) == 0;
}

string canonical_host_output_target_name(const string & output_target)
{
  if(output_target.empty()) {
    return host_default_target_name();
  }
  if(output_target == "macos" || output_target == "linux") {
    return output_target;
  }
  if(target_starts_with(output_target, "x86_64-apple-darwin")) {
    return "macos";
  }
  if(target_starts_with(output_target, "x86_64-") &&
     output_target.find("linux") != string::npos) {
    return "linux";
  }
  return output_target;
}

string host_cxx_command()
{
  const char * explicit_host = std::getenv("CPPGM_HOST_CXX");
  if(explicit_host && *explicit_host) {
    return explicit_host;
  }
  const char * generic_cxx = std::getenv("CXX");
  if(generic_cxx && *generic_cxx) {
    return generic_cxx;
  }
  return "c++";
}

bool file_exists(const string & path)
{
  ifstream in(path.c_str(), ios::binary);
  return static_cast<bool>(in);
}

string & cpp_tool_program_path_storage()
{
  static string path;
  return path;
}

bool ends_with(const string & value, const string & suffix)
{
  return value.size() >= suffix.size() &&
      value.compare(value.size() - suffix.size(), suffix.size(), suffix) == 0;
}

string path_dirname(const string & path)
{
  const string::size_type pos = path.rfind('/');
  if(pos == string::npos) {
    return string();
  }
  if(pos == 0) {
    return "/";
  }
  return path.substr(0, pos);
}

string path_basename(const string & path)
{
  const string::size_type pos = path.rfind('/');
  return pos == string::npos ? path : path.substr(pos + 1);
}

string resolved_path(const string & path)
{
  if(path.empty()) {
    return string();
  }
  char * resolved = realpath(path.c_str(), nullptr);
  if(resolved == nullptr) {
    return string();
  }
  const string out = resolved;
  std::free(resolved);
  return out;
}

string runtime_object_variant_for_program(const string & program_basename)
{
  if(ends_with(program_basename, "-debug")) {
    return "debug";
  }
  if(ends_with(program_basename, "-profile")) {
    return "profile";
  }
  if(ends_with(program_basename, "-asan")) {
    return "asan";
  }
  return "release";
}

vector<string> hosted_runtime_support_objects()
{
  string obj_root;
  const char * explicit_root = std::getenv("CPPGM_OBJECT_ROOT");
  if(explicit_root && *explicit_root) {
    obj_root = explicit_root;
  } else if(*CPPGM_DEFAULT_OBJECT_ROOT != '\0') {
    obj_root = CPPGM_DEFAULT_OBJECT_ROOT;
  } else {
    const string resolved_program = resolved_path(cpp_tool_program_path_storage());
    if(resolved_program.empty()) {
      return vector<string>();
    }
    const string program_dir = path_dirname(resolved_program);
    const string repo_root = path_dirname(program_dir);
    if(repo_root.empty()) {
      return vector<string>();
    }
    obj_root = repo_root + "/obj";
  }

  const string resolved_program = resolved_path(cpp_tool_program_path_storage());
  const string program_basename =
      resolved_program.empty() ? string("cppgm++") : path_basename(resolved_program);
  const string obj_dir =
      obj_root + "/" + runtime_object_variant_for_program(program_basename);
  vector<string> out;
  const string support_objects[] = {
    obj_dir + "/host_builtin_runtime.o",
    obj_dir + "/eh_runtime.o",
  };
  for(size_t i = 0; i < sizeof(support_objects) / sizeof(support_objects[0]); ++i) {
    if(file_exists(support_objects[i])) {
      out.push_back(support_objects[i]);
    }
  }
  return out;
}

void run_host_cpp_command(const vector<string> & args)
{
  ostringstream command;
  command << host_cxx_command();
  for(size_t i = 0; i < args.size(); ++i) {
    command << " " << shell_quote(args[i]);
  }
  const int status = std::system(command.str().c_str());
  if(status != 0) {
    throw logic_error("host toolchain invocation failed");
  }
}

int run_tool_command_status(const string & program,
                            const vector<string> & args)
{
  ostringstream command;
  command << program;
  for(size_t i = 0; i < args.size(); ++i) {
    command << " " << shell_quote(args[i]);
  }
  return std::system(command.str().c_str());
}

bool command_exists_in_path(const string & program)
{
  const string probe =
      string("command -v ") + shell_quote(program) + " >/dev/null 2>&1";
  return std::system(probe.c_str()) == 0;
}

string effective_host_output_target(const string & output_target)
{
  return canonical_host_output_target_name(output_target);
}

bool emit_macos_dsym_bundle(const string & outfile)
{
  if(!command_exists_in_path("dsymutil")) {
    return false;
  }
  vector<string> args;
  args.push_back("-o");
  args.push_back(outfile + ".dSYM");
  args.push_back(outfile);
  return run_tool_command_status("dsymutil", args) == 0;
}

HostToolchainFamily detect_host_toolchain_family()
{
  const string command = host_cxx_command();
  const string basename =
      command.rfind('/') == string::npos ? command : command.substr(command.rfind('/') + 1);
  if(basename.find("clang") != string::npos) {
    return HOST_TOOLCHAIN_CLANG;
  }
  if(basename.find("g++") != string::npos ||
     basename.find("gcc") != string::npos) {
    return HOST_TOOLCHAIN_GNU;
  }

  const string probe =
      command + " -std=gnu++11 -dM -E -x c++ - < /dev/null 2>/dev/null";
  const string output = run_command_capture_stdout(probe);
  if(output.find("__clang__") != string::npos) {
    return HOST_TOOLCHAIN_CLANG;
  }
  if(output.find("__GNUC__") != string::npos) {
    return HOST_TOOLCHAIN_GNU;
  }
  return HOST_TOOLCHAIN_UNKNOWN;
}

}  // namespace

void set_cpp_tool_program_path(const string & path)
{
  cpp_tool_program_path_storage() = path;
}

string cpp_tool_program_path()
{
  return cpp_tool_program_path_storage();
}

HostToolchainFamily host_toolchain_family()
{
  static const HostToolchainFamily family = detect_host_toolchain_family();
  return family;
}

bool can_use_host_toolchain_for_output_target(const string & output_target)
{
  const string host_target = host_default_target_name();
  return !host_target.empty() &&
      canonical_host_output_target_name(output_target) == host_target;
}

void run_host_cpp_query(const vector<string> & args)
{
  run_host_cpp_command(args);
}

bool link_host_objects_to_native(const vector<string> & objfiles,
                                 const string & outfile,
                                 const string & output_target,
                                 int debug_info_level,
                                 const string & stdlib_flag)
{
  if(objfiles.empty()) {
    throw logic_error("host object link requires at least one object file");
  }
  if(!can_use_host_toolchain_for_output_target(output_target)) {
    throw logic_error("host object link target is not supported");
  }

  vector<string> args;
  if(debug_info_level >= 1) {
    args.push_back("-g");
  }
  if(effective_host_output_target(output_target) == "linux") {
    // Generated x86_64 host objects are not PIC; Linux distributions commonly
    // default the host linker to PIE, which leaves absolute vtable relocations
    // as runtime text relocations.
    args.push_back("-no-pie");
  }
  if(!stdlib_flag.empty()) {
    args.push_back(stdlib_flag);
  } else {
    const char * env_stdlib = std::getenv("CPPGM_STDLIB_FLAGS");
    if(env_stdlib && *env_stdlib) {
      args.push_back(env_stdlib);
    }
  }
  args.push_back("-o");
  args.push_back(outfile);
  for(size_t i = 0; i < objfiles.size(); ++i) {
    args.push_back(objfiles[i]);
  }
  const vector<string> support_objects = hosted_runtime_support_objects();
  for(size_t i = 0; i < support_objects.size(); ++i) {
    args.push_back(support_objects[i]);
  }
  run_host_cpp_command(args);

  if(debug_info_level < 1) {
    return false;
  }
  if(effective_host_output_target(output_target) != "macos") {
    return false;
  }

  return !emit_macos_dsym_bundle(outfile);
}

vector<CallSemNode> analyze_cpp_sources(const vector<string> & srcfiles,
                                        const CppPreprocessOptions & options,
                                        bool expand_output_closure,
                                        vector<string> * dependency_files,
                                        vector<witness::TemplateWitnessSession> *
                                            witness_sessions,
                                        bool exact_source_locations)
{
  PhaseTimer timer("analyze_cpp_sources", source_count_detail(srcfiles));
  time_t now = time(nullptr);
  vector<CallSemNode> translation_units;
  if(witness_sessions != nullptr) {
    witness_sessions->clear();
  }
  set<string> seen_dependencies;
  const bool file_only_source_locations =
      !exact_source_locations && witness_sessions == nullptr;
  CppPreprocessOptions analysis_options = options;
  analysis_options.emit_insignificant_whitespace = false;
  for(size_t i = 0; i < srcfiles.size(); ++i) {
    PhaseTimer translation_unit_timer("analyze_translation_unit", srcfiles[i]);
    Preprocessor preprocessor(srcfiles[i], now, analysis_options);
    SourceLocationTable source_locations;
    PostTokenizer posttokenizer(preprocessor,
                                &source_locations,
                                &preprocessor,
                                file_only_source_locations);
    RecogTokenizer tokenizer(posttokenizer);
    RecogTokenBuffer tokens(tokenizer, srcfiles[i], &source_locations);
    witness::TemplateWitnessSession * witness_session = nullptr;
    if(witness_sessions != nullptr) {
      witness_sessions->push_back(witness::create_template_witness_session());
      witness_session = &witness_sessions->back();
    }
    translation_units.push_back(analyze_calls_translation_unit(tokens,
                                                               expand_output_closure,
                                                               false,
                                                               witness_session));
    if(dependency_files != nullptr) {
      const vector<string> & local_dependencies = preprocessor.dependency_files();
      for(size_t j = 0; j < local_dependencies.size(); ++j) {
        if(seen_dependencies.insert(local_dependencies[j]).second) {
          dependency_files->push_back(local_dependencies[j]);
        }
      }
    }
  }
  return translation_units;
}

void clear_lowir_program_debug_locations(lowir_internal::Program & program)
{
  for(size_t fi = 0; fi < program.functions.size(); ++fi) {
    program.functions[fi].debug_location = lowir_internal::InstructionDebugLocation();
    for(size_t bi = 0; bi < program.functions[fi].blocks.size(); ++bi) {
      for(size_t ii = 0; ii < program.functions[fi].blocks[bi].instructions.size(); ++ii) {
        program.functions[fi].blocks[bi].instructions[ii].debug_location =
            lowir_internal::InstructionDebugLocation();
      }
    }
  }
}

bool object_symbol_definition_is_root(const lowir_internal::SymbolMetadata & metadata)
{
  if(metadata.object_output_root) {
    return true;
  }
  if(metadata.role != lowir_internal::SR_NONE) {
    return true;
  }
  return metadata.binding != lowir_internal::SBM_WEAK &&
         metadata.binding != lowir_internal::SBM_INTERNAL;
}

bool object_symbol_definition_is_prunable(const lowir_internal::SymbolMetadata & metadata)
{
  return metadata.role == lowir_internal::SR_NONE &&
         (metadata.binding == lowir_internal::SBM_WEAK ||
          metadata.binding == lowir_internal::SBM_INTERNAL);
}

bool object_symbol_definition_is_inline_canonicalizable(
    const lowir_internal::SymbolMetadata & metadata)
{
  return object_symbol_definition_is_prunable(metadata) &&
         (metadata.prefer_local_object_binding ||
          metadata.binding == lowir_internal::SBM_INTERNAL);
}

bool lowir_operand_equals(const lowir_internal::Operand & lhs,
                          const lowir_internal::Operand & rhs)
{
  if(lhs.kind != rhs.kind) {
    return false;
  }

  switch(lhs.kind) {
    case lowir_internal::Operand::OP_TEMP:
    case lowir_internal::Operand::OP_SLOT:
    case lowir_internal::Operand::OP_GLOBAL:
    case lowir_internal::Operand::OP_LABEL:
      return lhs.text == rhs.text;
    case lowir_internal::Operand::OP_INTEGER:
      return lhs.int_value == rhs.int_value &&
             lhs.literal_type.text == rhs.literal_type.text;
    case lowir_internal::Operand::OP_FLOAT:
      return lhs.float_value == rhs.float_value &&
             lhs.literal_type.text == rhs.literal_type.text;
  }
  return false;
}

lowir_internal::Operand lowir_temp_operand(const string & name)
{
  lowir_internal::Operand operand;
  operand.kind = lowir_internal::Operand::OP_TEMP;
  operand.text = name;
  return operand;
}

lowir_internal::Operand lowir_slot_operand(const string & name)
{
  lowir_internal::Operand operand;
  operand.kind = lowir_internal::Operand::OP_SLOT;
  operand.text = name;
  return operand;
}

bool lowir_operand_is_copyable_value(const lowir_internal::Operand & operand)
{
  return operand.kind == lowir_internal::Operand::OP_TEMP ||
         operand.kind == lowir_internal::Operand::OP_GLOBAL ||
         operand.kind == lowir_internal::Operand::OP_INTEGER ||
         operand.kind == lowir_internal::Operand::OP_FLOAT;
}

lowir_internal::Instruction make_lowir_copy_instruction(
    const string & dest,
    const string & type,
    const lowir_internal::Operand & value,
    const lowir_internal::Instruction & source)
{
  lowir_internal::Instruction instruction;
  instruction.kind = lowir_internal::Instruction::IK_COPY;
  instruction.dest = dest;
  instruction.type.text = type;
  instruction.first = value;
  instruction.debug_location = source.debug_location;
  return instruction;
}

struct IdentityWrapperInfo
{
  string return_type;
  size_t arg_index = 0;
};

struct ConstantWrapperInfo
{
  string return_type;
  lowir_internal::Operand value;
};

struct CompareWrapperInfo
{
  string return_type;
  string compare_type;
  string op;
  size_t lhs_arg_index = 0;
  size_t rhs_arg_index = 0;
};

string make_object_inline_name(const string & original, size_t inline_site_id)
{
  if(original.empty()) {
    return original;
  }
  return string(1, original[0]) + "__objinl" + to_string(inline_site_id) +
         "__" + original.substr(1);
}

bool lowir_instruction_returns_operand(const lowir_internal::Instruction & instruction,
                                       const string & type,
                                       const lowir_internal::Operand & operand)
{
  return instruction.kind == lowir_internal::Instruction::IK_RETURN &&
         instruction.type.text == type &&
         lowir_operand_equals(instruction.first, operand);
}

bool object_function_is_direct_identity_wrapper(
    const lowir_internal::Function & function,
    size_t & arg_index)
{
  if(!function.slots.empty() ||
     function.blocks.size() != 1 ||
     function.blocks[0].instructions.size() != 1) {
    return false;
  }

  for(size_t i = 0; i < function.params.size(); ++i) {
    if(function.params[i].type.text == function.return_type.text &&
       lowir_instruction_returns_operand(function.blocks[0].instructions[0],
                                         function.return_type.text,
                                         lowir_temp_operand(function.params[i].name))) {
      arg_index = i;
      return true;
    }
  }
  return false;
}

bool object_function_is_copy_identity_wrapper(
    const lowir_internal::Function & function,
    size_t & arg_index)
{
  if(!function.slots.empty() ||
     function.blocks.size() != 1 ||
     function.blocks[0].instructions.size() != 2) {
    return false;
  }

  const lowir_internal::Instruction & copy = function.blocks[0].instructions[0];
  if(copy.kind != lowir_internal::Instruction::IK_COPY ||
     copy.type.text != function.return_type.text) {
    return false;
  }

  for(size_t i = 0; i < function.params.size(); ++i) {
    if(function.params[i].type.text == function.return_type.text &&
       lowir_operand_equals(copy.first, lowir_temp_operand(function.params[i].name)) &&
       lowir_instruction_returns_operand(function.blocks[0].instructions[1],
                                         function.return_type.text,
                                         lowir_temp_operand(copy.dest))) {
      arg_index = i;
      return true;
    }
  }
  return false;
}

bool object_function_is_store_load_identity_wrapper(
    const lowir_internal::Function & function,
    size_t & arg_index)
{
  if(function.slots.size() != function.params.size() ||
     function.blocks.size() != 1 ||
     function.blocks[0].instructions.size() != function.params.size() + 2) {
    return false;
  }

  for(size_t i = 0; i < function.params.size(); ++i) {
    const lowir_internal::Instruction & store = function.blocks[0].instructions[i];
    if(function.slots[i].second.text != function.params[i].type.text ||
       store.kind != lowir_internal::Instruction::IK_STORE ||
       store.type.text != function.params[i].type.text ||
       !lowir_operand_equals(store.first, lowir_temp_operand(function.params[i].name)) ||
       !lowir_operand_equals(store.second, lowir_slot_operand(function.slots[i].first))) {
      return false;
    }
  }

  const lowir_internal::Instruction & load =
      function.blocks[0].instructions[function.params.size()];
  if(load.kind != lowir_internal::Instruction::IK_LOAD ||
     load.type.text != function.return_type.text) {
    return false;
  }

  for(size_t i = 0; i < function.params.size(); ++i) {
    if(function.params[i].type.text == function.return_type.text &&
       function.slots[i].second.text == function.return_type.text &&
       lowir_operand_equals(load.first, lowir_slot_operand(function.slots[i].first)) &&
       lowir_instruction_returns_operand(
           function.blocks[0].instructions[function.params.size() + 1],
           function.return_type.text,
           lowir_temp_operand(load.dest))) {
      arg_index = i;
      return true;
    }
  }
  return false;
}

bool object_function_is_trivial_identity_wrapper(const lowir_internal::Function & function,
                                                 IdentityWrapperInfo & info)
{
  if(!object_symbol_definition_is_inline_canonicalizable(function.metadata) ||
     function.boundary.arity != lowir_internal::CAM_FIXED ||
     function.boundary.unwind != lowir_internal::CUM_NO ||
     function.params.empty() ||
     function.return_type.text == "void" ||
     lowir_internal::is_object_type(function.return_type)) {
    return false;
  }

  size_t arg_index = 0;
  if(!object_function_is_direct_identity_wrapper(function, arg_index) &&
     !object_function_is_copy_identity_wrapper(function, arg_index) &&
     !object_function_is_store_load_identity_wrapper(function, arg_index)) {
    return false;
  }

  info.return_type = function.return_type.text;
  info.arg_index = arg_index;
  return true;
}

bool object_function_is_constant_wrapper(const lowir_internal::Function & function,
                                         ConstantWrapperInfo & info)
{
  if(!object_symbol_definition_is_inline_canonicalizable(function.metadata) ||
     function.boundary.arity != lowir_internal::CAM_FIXED ||
     !function.params.empty() ||
     !function.slots.empty() ||
     function.return_type.text == "void" ||
     lowir_internal::is_object_type(function.return_type) ||
     function.blocks.size() != 1 ||
     function.blocks[0].instructions.size() != 2) {
    return false;
  }

  const lowir_internal::Instruction & constant = function.blocks[0].instructions[0];
  if(constant.kind != lowir_internal::Instruction::IK_CONST ||
     !lowir_operand_is_copyable_value(constant.first)) {
    return false;
  }

  if(!lowir_instruction_returns_operand(function.blocks[0].instructions[1],
                                        function.return_type.text,
                                        lowir_temp_operand(constant.dest))) {
    return false;
  }

  info.return_type = function.return_type.text;
  info.value = constant.first;
  return true;
}

bool resolve_compare_wrapper_operand(
    const lowir_internal::Operand & operand,
    const map<string, size_t> & temp_param_indices,
    size_t & arg_index)
{
  if(operand.kind != lowir_internal::Operand::OP_TEMP) {
    return false;
  }
  map<string, size_t>::const_iterator found =
      temp_param_indices.find(operand.text);
  if(found == temp_param_indices.end()) {
    return false;
  }
  arg_index = found->second;
  return true;
}

bool object_function_is_compare_wrapper(const lowir_internal::Function & function,
                                        CompareWrapperInfo & info)
{
  if(!object_symbol_definition_is_inline_canonicalizable(function.metadata) ||
     function.boundary.arity != lowir_internal::CAM_FIXED ||
     function.boundary.unwind != lowir_internal::CUM_NO ||
     function.params.empty() ||
     function.return_type.text == "void" ||
     lowir_internal::is_object_type(function.return_type) ||
     function.blocks.size() != 1 ||
     function.blocks[0].instructions.size() < 2) {
    return false;
  }

  const vector<lowir_internal::Instruction> & instructions =
      function.blocks[0].instructions;
  const lowir_internal::Instruction & cmp =
      instructions[instructions.size() - 2];
  if(cmp.kind != lowir_internal::Instruction::IK_CMP ||
     cmp.dest.empty() ||
     !lowir_instruction_returns_operand(instructions.back(),
                                        function.return_type.text,
                                        lowir_temp_operand(cmp.dest))) {
    return false;
  }

  map<string, size_t> temp_param_indices;
  map<string, size_t> slot_param_indices;
  for(size_t i = 0; i < function.params.size(); ++i) {
    temp_param_indices[function.params[i].name] = i;
  }

  for(size_t i = 0; i + 2 < instructions.size(); ++i) {
    const lowir_internal::Instruction & instruction = instructions[i];
    if(instruction.kind == lowir_internal::Instruction::IK_STORE &&
       instruction.first.kind == lowir_internal::Operand::OP_TEMP &&
       instruction.second.kind == lowir_internal::Operand::OP_SLOT) {
      map<string, size_t>::const_iterator found =
          temp_param_indices.find(instruction.first.text);
      if(found == temp_param_indices.end() ||
         instruction.type.text != function.params[found->second].type.text) {
        return false;
      }
      slot_param_indices[instruction.second.text] = found->second;
      continue;
    }

    if(instruction.kind == lowir_internal::Instruction::IK_LOAD &&
       instruction.first.kind == lowir_internal::Operand::OP_SLOT &&
       !instruction.dest.empty()) {
      map<string, size_t>::const_iterator found =
          slot_param_indices.find(instruction.first.text);
      if(found == slot_param_indices.end() ||
         instruction.type.text != function.params[found->second].type.text) {
        return false;
      }
      temp_param_indices[instruction.dest] = found->second;
      continue;
    }

    if(instruction.kind == lowir_internal::Instruction::IK_COPY &&
       instruction.first.kind == lowir_internal::Operand::OP_TEMP &&
       !instruction.dest.empty()) {
      map<string, size_t>::const_iterator found =
          temp_param_indices.find(instruction.first.text);
      if(found == temp_param_indices.end() ||
         instruction.type.text != function.params[found->second].type.text) {
        return false;
      }
      temp_param_indices[instruction.dest] = found->second;
      continue;
    }

    return false;
  }

  size_t lhs_arg_index = 0;
  size_t rhs_arg_index = 0;
  if(!resolve_compare_wrapper_operand(cmp.first, temp_param_indices, lhs_arg_index) ||
     !resolve_compare_wrapper_operand(cmp.second, temp_param_indices, rhs_arg_index) ||
     cmp.type.text != function.params[lhs_arg_index].type.text ||
     cmp.type.text != function.params[rhs_arg_index].type.text) {
    return false;
  }

  info.return_type = function.return_type.text;
  info.compare_type = cmp.type.text;
  info.op = cmp.op;
  info.lhs_arg_index = lhs_arg_index;
  info.rhs_arg_index = rhs_arg_index;
  return true;
}

bool object_function_has_noop_void_body(const lowir_internal::Function & function)
{
  if(function.boundary.arity != lowir_internal::CAM_FIXED ||
     function.return_type.text != "void" ||
     function.blocks.size() != 1 ||
     function.slots.size() != function.params.size() ||
     function.blocks[0].instructions.size() != function.params.size() + 1) {
    return false;
  }

  for(size_t i = 0; i < function.params.size(); ++i) {
    const lowir_internal::Instruction & store = function.blocks[0].instructions[i];
    if(function.slots[i].second.text != function.params[i].type.text ||
       store.kind != lowir_internal::Instruction::IK_STORE ||
       store.type.text != function.params[i].type.text ||
       !lowir_operand_equals(store.first, lowir_temp_operand(function.params[i].name)) ||
       !lowir_operand_equals(store.second, lowir_slot_operand(function.slots[i].first))) {
      return false;
    }
  }

  const lowir_internal::Instruction & ret = function.blocks[0].instructions.back();
  return ret.kind == lowir_internal::Instruction::IK_RETURN &&
         ret.type.text == "void";
}

bool object_function_is_noop_void_wrapper(const lowir_internal::Function & function)
{
  return object_symbol_definition_is_inline_canonicalizable(function.metadata) &&
         object_function_has_noop_void_body(function);
}

bool object_function_is_trivial_lifecycle_noop_wrapper(
    const lowir_internal::Function & function)
{
  return object_symbol_definition_is_prunable(function.metadata) &&
         function.metadata.object_trivial_lifecycle &&
         object_function_has_noop_void_body(function);
}

bool internal_object_function_definition_is_explicit_root(
    const lowir_internal::Function & function)
{
  const lowir_internal::SymbolMetadata & metadata = function.metadata;
  return metadata.role == lowir_internal::SR_NONE &&
         metadata.binding == lowir_internal::SBM_INTERNAL &&
         !metadata.object_symbol.empty() &&
         !metadata.prefer_local_object_binding &&
         !metadata.object_trivial_lifecycle;
}

bool simple_void_object_inline_instruction_is_supported(
    const lowir_internal::Instruction & instruction,
    bool final_instruction)
{
  if(final_instruction) {
    return instruction.kind == lowir_internal::Instruction::IK_RETURN &&
           instruction.type.text == "void";
  }

  switch(instruction.kind) {
    case lowir_internal::Instruction::IK_CONST:
    case lowir_internal::Instruction::IK_COPY:
    case lowir_internal::Instruction::IK_ADDR:
    case lowir_internal::Instruction::IK_LOAD:
    case lowir_internal::Instruction::IK_STORE:
    case lowir_internal::Instruction::IK_INDEX:
    case lowir_internal::Instruction::IK_UNARY:
    case lowir_internal::Instruction::IK_COPYOBJ:
    case lowir_internal::Instruction::IK_ZEROINIT:
      return true;
    default:
      return false;
  }
}

bool object_function_is_simple_void_inline_candidate(
    const lowir_internal::Function & function)
{
  if(!object_symbol_definition_is_inline_canonicalizable(function.metadata) ||
     function.boundary.arity != lowir_internal::CAM_FIXED ||
     function.return_type.text != "void" ||
     function.blocks.size() != 1 ||
     function.blocks[0].instructions.empty() ||
     function.blocks[0].instructions.size() > 10) {
    return false;
  }

  for(size_t i = 0; i < function.blocks[0].instructions.size(); ++i) {
    const bool final_instruction = i + 1 == function.blocks[0].instructions.size();
    if(!simple_void_object_inline_instruction_is_supported(
           function.blocks[0].instructions[i],
           final_instruction)) {
      return false;
    }
  }
  return true;
}

void rewrite_simple_object_inlined_operand(
    lowir_internal::Operand & operand,
    const map<string, lowir_internal::Operand> & parameter_operands,
    const map<string, string> & renamed_temps,
    const map<string, string> & renamed_slots)
{
  switch(operand.kind) {
    case lowir_internal::Operand::OP_TEMP: {
      const map<string, lowir_internal::Operand>::const_iterator param_found =
          parameter_operands.find(operand.text);
      if(param_found != parameter_operands.end()) {
        operand = param_found->second;
        return;
      }
      const map<string, string>::const_iterator temp_found =
          renamed_temps.find(operand.text);
      if(temp_found != renamed_temps.end()) {
        operand.text = temp_found->second;
      }
      return;
    }
    case lowir_internal::Operand::OP_SLOT: {
      const map<string, string>::const_iterator slot_found =
          renamed_slots.find(operand.text);
      if(slot_found != renamed_slots.end()) {
        operand.text = slot_found->second;
      }
      return;
    }
    default:
      return;
  }
}

void rewrite_simple_object_inlined_instruction(
    lowir_internal::Instruction & instruction,
    const map<string, lowir_internal::Operand> & parameter_operands,
    const map<string, string> & renamed_temps,
    const map<string, string> & renamed_slots)
{
  if(!instruction.dest.empty()) {
    const map<string, string>::const_iterator dest_found =
        renamed_temps.find(instruction.dest);
    if(dest_found != renamed_temps.end()) {
      instruction.dest = dest_found->second;
    }
  }

  rewrite_simple_object_inlined_operand(instruction.first,
                                        parameter_operands,
                                        renamed_temps,
                                        renamed_slots);
  rewrite_simple_object_inlined_operand(instruction.second,
                                        parameter_operands,
                                        renamed_temps,
                                        renamed_slots);
  rewrite_simple_object_inlined_operand(instruction.third,
                                        parameter_operands,
                                        renamed_temps,
                                        renamed_slots);
  for(size_t i = 0; i < instruction.args.size(); ++i) {
    rewrite_simple_object_inlined_operand(instruction.args[i],
                                          parameter_operands,
                                          renamed_temps,
                                          renamed_slots);
  }
}

vector<lowir_internal::Instruction> simple_object_inlined_body(
    const lowir_internal::Function & callee,
    const vector<lowir_internal::Operand> & call_args,
    size_t inline_site_id)
{
  map<string, lowir_internal::Operand> parameter_operands;
  for(size_t i = 0; i < callee.params.size(); ++i) {
    parameter_operands[callee.params[i].name] = call_args[i];
  }

  map<string, string> renamed_slots;
  for(size_t i = 0; i < callee.slots.size(); ++i) {
    renamed_slots[callee.slots[i].first] =
        make_object_inline_name(callee.slots[i].first, inline_site_id);
  }

  map<string, string> renamed_temps;
  const lowir_internal::Block & block = callee.blocks.front();
  for(size_t i = 0; i < block.instructions.size(); ++i) {
    const lowir_internal::Instruction & instruction = block.instructions[i];
    if(!instruction.dest.empty()) {
      renamed_temps[instruction.dest] =
          make_object_inline_name(instruction.dest, inline_site_id);
    }
  }

  vector<lowir_internal::Instruction> out;
  out.reserve(block.instructions.size());
  for(size_t i = 0; i < block.instructions.size(); ++i) {
    const lowir_internal::Instruction & instruction = block.instructions[i];
    if(instruction.kind == lowir_internal::Instruction::IK_RETURN) {
      continue;
    }
    lowir_internal::Instruction cloned = instruction;
    rewrite_simple_object_inlined_instruction(cloned,
                                              parameter_operands,
                                              renamed_temps,
                                              renamed_slots);
    out.push_back(cloned);
  }
  return out;
}

bool rewrite_trivial_identity_object_wrapper_calls(
    lowir_internal::Function & function,
    const map<string, IdentityWrapperInfo> & identity_wrappers)
{
  bool changed = false;
  for(size_t bi = 0; bi < function.blocks.size(); ++bi) {
    vector<lowir_internal::Instruction> rewritten;
    rewritten.reserve(function.blocks[bi].instructions.size());
    bool block_changed = false;
    for(size_t ii = 0; ii < function.blocks[bi].instructions.size(); ++ii) {
      const lowir_internal::Instruction & instruction =
          function.blocks[bi].instructions[ii];
      const map<string, IdentityWrapperInfo>::const_iterator identity_wrapper =
          instruction.first.kind == lowir_internal::Operand::OP_GLOBAL
              ? identity_wrappers.find(instruction.first.text)
              : identity_wrappers.end();
      if(instruction.kind == lowir_internal::Instruction::IK_CALL &&
         !instruction.call_returns_void &&
         !instruction.dest.empty() &&
         !instruction.has_call_signature &&
         identity_wrapper != identity_wrappers.end() &&
         instruction.type.text == identity_wrapper->second.return_type &&
         identity_wrapper->second.arg_index < instruction.args.size() &&
         lowir_operand_is_copyable_value(
             instruction.args[identity_wrapper->second.arg_index])) {
        const lowir_internal::Operand & replacement =
            instruction.args[identity_wrapper->second.arg_index];
        if(replacement.kind == lowir_internal::Operand::OP_TEMP &&
           replacement.text == instruction.dest) {
          block_changed = true;
          changed = true;
          continue;
        }
        rewritten.push_back(make_lowir_copy_instruction(instruction.dest,
                                                        instruction.type.text,
                                                        replacement,
                                                        instruction));
        block_changed = true;
        changed = true;
        continue;
      }

      rewritten.push_back(instruction);
    }
    if(block_changed) {
      function.blocks[bi].instructions.swap(rewritten);
    }
  }
  return changed;
}

bool rewrite_constant_object_wrapper_calls(
    lowir_internal::Function & function,
    const map<string, ConstantWrapperInfo> & constant_wrappers)
{
  bool changed = false;
  for(size_t bi = 0; bi < function.blocks.size(); ++bi) {
    vector<lowir_internal::Instruction> rewritten;
    rewritten.reserve(function.blocks[bi].instructions.size());
    bool block_changed = false;
    for(size_t ii = 0; ii < function.blocks[bi].instructions.size(); ++ii) {
      const lowir_internal::Instruction & instruction =
          function.blocks[bi].instructions[ii];
      const map<string, ConstantWrapperInfo>::const_iterator constant_wrapper =
          instruction.first.kind == lowir_internal::Operand::OP_GLOBAL
              ? constant_wrappers.find(instruction.first.text)
              : constant_wrappers.end();
      if(instruction.kind == lowir_internal::Instruction::IK_CALL &&
         !instruction.call_returns_void &&
         !instruction.dest.empty() &&
         !instruction.has_call_signature &&
         constant_wrapper != constant_wrappers.end() &&
         instruction.type.text == constant_wrapper->second.return_type &&
         instruction.args.empty()) {
        rewritten.push_back(make_lowir_copy_instruction(instruction.dest,
                                                        instruction.type.text,
                                                        constant_wrapper->second.value,
                                                        instruction));
        block_changed = true;
        changed = true;
        continue;
      }

      rewritten.push_back(instruction);
    }
    if(block_changed) {
      function.blocks[bi].instructions.swap(rewritten);
    }
  }
  return changed;
}

bool rewrite_compare_object_wrapper_calls(
    lowir_internal::Function & function,
    const map<string, CompareWrapperInfo> & compare_wrappers)
{
  bool changed = false;
  for(size_t bi = 0; bi < function.blocks.size(); ++bi) {
    vector<lowir_internal::Instruction> rewritten;
    rewritten.reserve(function.blocks[bi].instructions.size());
    bool block_changed = false;
    for(size_t ii = 0; ii < function.blocks[bi].instructions.size(); ++ii) {
      const lowir_internal::Instruction & instruction =
          function.blocks[bi].instructions[ii];
      const map<string, CompareWrapperInfo>::const_iterator compare_wrapper =
          instruction.first.kind == lowir_internal::Operand::OP_GLOBAL
              ? compare_wrappers.find(instruction.first.text)
              : compare_wrappers.end();
      if(instruction.kind == lowir_internal::Instruction::IK_CALL &&
         !instruction.call_returns_void &&
         !instruction.dest.empty() &&
         !instruction.has_call_signature &&
         compare_wrapper != compare_wrappers.end() &&
         instruction.type.text == compare_wrapper->second.return_type &&
         compare_wrapper->second.lhs_arg_index < instruction.args.size() &&
         compare_wrapper->second.rhs_arg_index < instruction.args.size() &&
         lowir_operand_is_copyable_value(
             instruction.args[compare_wrapper->second.lhs_arg_index]) &&
         lowir_operand_is_copyable_value(
             instruction.args[compare_wrapper->second.rhs_arg_index])) {
        lowir_internal::Instruction cmp;
        cmp.kind = lowir_internal::Instruction::IK_CMP;
        cmp.dest = instruction.dest;
        cmp.type.text = compare_wrapper->second.compare_type;
        cmp.op = compare_wrapper->second.op;
        cmp.first = instruction.args[compare_wrapper->second.lhs_arg_index];
        cmp.second = instruction.args[compare_wrapper->second.rhs_arg_index];
        cmp.debug_location = instruction.debug_location;
        rewritten.push_back(cmp);
        block_changed = true;
        changed = true;
        continue;
      }

      rewritten.push_back(instruction);
    }
    if(block_changed) {
      function.blocks[bi].instructions.swap(rewritten);
    }
  }
  return changed;
}

bool remove_noop_void_object_wrapper_calls(
    lowir_internal::Function & function,
    const map<string, size_t> & noop_wrapper_param_counts)
{
  bool changed = false;
  for(size_t bi = 0; bi < function.blocks.size(); ++bi) {
    vector<lowir_internal::Instruction> rewritten;
    rewritten.reserve(function.blocks[bi].instructions.size());
    bool block_changed = false;
    for(size_t ii = 0; ii < function.blocks[bi].instructions.size(); ++ii) {
      const lowir_internal::Instruction & instruction =
          function.blocks[bi].instructions[ii];
      const map<string, size_t>::const_iterator noop_wrapper =
          instruction.first.kind == lowir_internal::Operand::OP_GLOBAL
              ? noop_wrapper_param_counts.find(instruction.first.text)
              : noop_wrapper_param_counts.end();
      if(instruction.kind == lowir_internal::Instruction::IK_CALL &&
         instruction.call_returns_void &&
         instruction.dest.empty() &&
         !instruction.has_call_signature &&
         noop_wrapper != noop_wrapper_param_counts.end() &&
         instruction.args.size() == noop_wrapper->second) {
        block_changed = true;
        changed = true;
        continue;
      }

      rewritten.push_back(instruction);
    }
    if(block_changed) {
      function.blocks[bi].instructions.swap(rewritten);
    }
  }
  return changed;
}

bool inline_simple_void_object_wrapper_calls(
    lowir_internal::Function & function,
    const map<string, const lowir_internal::Function *> & inline_candidates,
    size_t & next_inline_site_id)
{
  bool changed = false;
  for(size_t bi = 0; bi < function.blocks.size(); ++bi) {
    vector<lowir_internal::Instruction> rewritten;
    rewritten.reserve(function.blocks[bi].instructions.size());
    bool block_changed = false;
    for(size_t ii = 0; ii < function.blocks[bi].instructions.size(); ++ii) {
      const lowir_internal::Instruction & instruction =
          function.blocks[bi].instructions[ii];
      const map<string, const lowir_internal::Function *>::const_iterator found =
          instruction.first.kind == lowir_internal::Operand::OP_GLOBAL
              ? inline_candidates.find(instruction.first.text)
              : inline_candidates.end();
      if(instruction.kind == lowir_internal::Instruction::IK_CALL &&
         instruction.call_returns_void &&
         instruction.dest.empty() &&
         !instruction.has_call_signature &&
         found != inline_candidates.end() &&
         found->second != nullptr &&
         found->second->name != function.name &&
         instruction.args.size() == found->second->params.size()) {
        const size_t inline_site_id = next_inline_site_id++;
        for(size_t si = 0; si < found->second->slots.size(); ++si) {
          function.slots.push_back(
              make_pair(make_object_inline_name(found->second->slots[si].first,
                                                inline_site_id),
                        found->second->slots[si].second));
        }
        const vector<lowir_internal::Instruction> inlined =
            simple_object_inlined_body(*found->second,
                                       instruction.args,
                                       inline_site_id);
        rewritten.insert(rewritten.end(), inlined.begin(), inlined.end());
        block_changed = true;
        changed = true;
        continue;
      }

      rewritten.push_back(instruction);
    }
    if(block_changed) {
      function.blocks[bi].instructions.swap(rewritten);
    }
  }
  return changed;
}

lowir_internal::Program inline_trivial_identity_object_wrappers(
    const lowir_internal::Program & program)
{
  map<string, IdentityWrapperInfo> identity_wrappers;
  for(size_t i = 0; i < program.functions.size(); ++i) {
    IdentityWrapperInfo info;
    if(object_function_is_trivial_identity_wrapper(program.functions[i], info)) {
      identity_wrappers[program.functions[i].name] = info;
    }
  }
  if(identity_wrappers.empty()) {
    return program;
  }

  lowir_internal::Program rewritten = program;
  bool changed = false;
  for(size_t i = 0; i < rewritten.functions.size(); ++i) {
    if(rewrite_trivial_identity_object_wrapper_calls(rewritten.functions[i],
                                                     identity_wrappers)) {
      changed = true;
    }
  }
  return changed ? rewritten : program;
}

lowir_internal::Program inline_constant_object_wrappers(
    const lowir_internal::Program & program)
{
  map<string, ConstantWrapperInfo> constant_wrappers;
  for(size_t i = 0; i < program.functions.size(); ++i) {
    ConstantWrapperInfo info;
    if(object_function_is_constant_wrapper(program.functions[i], info)) {
      constant_wrappers[program.functions[i].name] = info;
    }
  }
  if(constant_wrappers.empty()) {
    return program;
  }

  lowir_internal::Program rewritten = program;
  bool changed = false;
  for(size_t i = 0; i < rewritten.functions.size(); ++i) {
    if(rewrite_constant_object_wrapper_calls(rewritten.functions[i],
                                             constant_wrappers)) {
      changed = true;
    }
  }
  return changed ? rewritten : program;
}

lowir_internal::Program inline_compare_object_wrappers(
    const lowir_internal::Program & program)
{
  map<string, CompareWrapperInfo> compare_wrappers;
  for(size_t i = 0; i < program.functions.size(); ++i) {
    CompareWrapperInfo info;
    if(object_function_is_compare_wrapper(program.functions[i], info)) {
      compare_wrappers[program.functions[i].name] = info;
    }
  }
  if(compare_wrappers.empty()) {
    return program;
  }

  lowir_internal::Program rewritten = program;
  bool changed = false;
  for(size_t i = 0; i < rewritten.functions.size(); ++i) {
    if(rewrite_compare_object_wrapper_calls(rewritten.functions[i],
                                            compare_wrappers)) {
      changed = true;
    }
  }
  return changed ? rewritten : program;
}

lowir_internal::Program inline_simple_void_object_wrappers(
    const lowir_internal::Program & program)
{
  map<string, const lowir_internal::Function *> inline_candidates;
  for(size_t i = 0; i < program.functions.size(); ++i) {
    if(object_function_is_simple_void_inline_candidate(program.functions[i])) {
      inline_candidates[program.functions[i].name] = &program.functions[i];
    }
  }
  if(inline_candidates.empty()) {
    return program;
  }

  lowir_internal::Program rewritten = program;
  bool changed = false;
  size_t next_inline_site_id = 1;
  for(size_t i = 0; i < rewritten.functions.size(); ++i) {
    if(inline_simple_void_object_wrapper_calls(rewritten.functions[i],
                                               inline_candidates,
                                               next_inline_site_id)) {
      changed = true;
    }
  }
  return changed ? rewritten : program;
}

lowir_internal::Program remove_noop_void_object_wrappers(
    const lowir_internal::Program & program)
{
  map<string, size_t> noop_wrapper_param_counts;
  for(size_t i = 0; i < program.functions.size(); ++i) {
    if(object_function_is_noop_void_wrapper(program.functions[i])) {
      noop_wrapper_param_counts[program.functions[i].name] =
          program.functions[i].params.size();
    }
  }
  if(noop_wrapper_param_counts.empty()) {
    return program;
  }

  lowir_internal::Program rewritten = program;
  bool changed = false;
  for(size_t i = 0; i < rewritten.functions.size(); ++i) {
    if(remove_noop_void_object_wrapper_calls(rewritten.functions[i],
                                             noop_wrapper_param_counts)) {
      changed = true;
    }
  }
  return changed ? rewritten : program;
}

lowir_internal::Program remove_trivial_lifecycle_object_wrappers(
    const lowir_internal::Program & program)
{
  map<string, size_t> noop_wrapper_param_counts;
  for(size_t i = 0; i < program.functions.size(); ++i) {
    if(object_function_is_trivial_lifecycle_noop_wrapper(program.functions[i])) {
      noop_wrapper_param_counts[program.functions[i].name] =
          program.functions[i].params.size();
    }
  }
  if(noop_wrapper_param_counts.empty()) {
    return program;
  }

  lowir_internal::Program rewritten = program;
  bool changed = false;
  for(size_t i = 0; i < rewritten.functions.size(); ++i) {
    if(remove_noop_void_object_wrapper_calls(rewritten.functions[i],
                                             noop_wrapper_param_counts)) {
      changed = true;
    }
  }
  return changed ? rewritten : program;
}

void note_live_symbol_name(const string & symbol,
                           const set<string> & function_names,
                           const set<string> & global_names,
                           set<string> & live_functions,
                           set<string> & live_globals)
{
  if(function_names.count(symbol) != 0) {
    live_functions.insert(symbol);
  }
  if(global_names.count(symbol) != 0) {
    live_globals.insert(symbol);
  }
}

void note_live_symbol_operand(const lowir_internal::Operand & operand,
                              const set<string> & function_names,
                              const set<string> & global_names,
                              set<string> & live_functions,
                              set<string> & live_globals)
{
  if(operand.kind == lowir_internal::Operand::OP_GLOBAL) {
    note_live_symbol_name(operand.text,
                          function_names,
                          global_names,
                          live_functions,
                          live_globals);
  }
}

void note_live_symbol_references(const lowir_internal::Instruction & instruction,
                                 const set<string> & function_names,
                                 const set<string> & global_names,
                                 set<string> & live_functions,
                                 set<string> & live_globals)
{
  note_live_symbol_operand(instruction.first,
                           function_names,
                           global_names,
                           live_functions,
                           live_globals);
  note_live_symbol_operand(instruction.second,
                           function_names,
                           global_names,
                           live_functions,
                           live_globals);
  note_live_symbol_operand(instruction.third,
                           function_names,
                           global_names,
                           live_functions,
                           live_globals);
  for(size_t i = 0; i < instruction.args.size(); ++i) {
    note_live_symbol_operand(instruction.args[i],
                             function_names,
                             global_names,
                             live_functions,
                             live_globals);
  }
}

void note_live_symbol_references(const lowir_internal::Function & function,
                                 const set<string> & function_names,
                                 const set<string> & global_names,
                                 set<string> & live_functions,
                                 set<string> & live_globals)
{
  if(!function.metadata.tls_for_symbol.empty()) {
    note_live_symbol_name(function.metadata.tls_for_symbol,
                          function_names,
                          global_names,
                          live_functions,
                          live_globals);
  }
  for(size_t bi = 0; bi < function.blocks.size(); ++bi) {
    for(size_t ii = 0; ii < function.blocks[bi].instructions.size(); ++ii) {
      note_live_symbol_references(function.blocks[bi].instructions[ii],
                                  function_names,
                                  global_names,
                                  live_functions,
                                  live_globals);
    }
  }
}

void note_live_symbol_references(const lowir_internal::GlobalDefinition & global,
                                 const set<string> & function_names,
                                 const set<string> & global_names,
                                 set<string> & live_functions,
                                 set<string> & live_globals)
{
  if(!global.structured &&
     global.init_kind == lowir_internal::GlobalDefinition::INIT_ADDR) {
    note_live_symbol_operand(global.init_operand,
                             function_names,
                             global_names,
                             live_functions,
                             live_globals);
  }
  for(size_t i = 0; i < global.data_items.size(); ++i) {
    const lowir_internal::GlobalDefinition::DataItem & item = global.data_items[i];
    if(item.kind == lowir_internal::GlobalDefinition::DataItem::ITEM_ADDR) {
      note_live_symbol_name(item.symbol,
                            function_names,
                            global_names,
                            live_functions,
                            live_globals);
    }
  }
}

lowir_internal::Program prune_unreferenced_object_symbol_definitions(
    const lowir_internal::Program & program)
{
  set<string> function_names;
  set<string> global_names;
  map<string, const lowir_internal::Function *> function_by_name;
  map<string, const lowir_internal::GlobalDefinition *> global_by_name;
  for(size_t i = 0; i < program.functions.size(); ++i) {
    function_names.insert(program.functions[i].name);
    function_by_name[program.functions[i].name] = &program.functions[i];
  }
  for(size_t i = 0; i < program.globals.size(); ++i) {
    global_names.insert(program.globals[i].name);
    global_by_name[program.globals[i].name] = &program.globals[i];
  }

  set<string> live_functions;
  set<string> live_globals;
  for(size_t i = 0; i < program.functions.size(); ++i) {
    if(object_symbol_definition_is_root(program.functions[i].metadata) ||
       internal_object_function_definition_is_explicit_root(program.functions[i])) {
      live_functions.insert(program.functions[i].name);
    }
  }
  for(size_t i = 0; i < program.globals.size(); ++i) {
    if(object_symbol_definition_is_root(program.globals[i].metadata)) {
      live_globals.insert(program.globals[i].name);
    }
  }

  bool changed = true;
  while(changed) {
    changed = false;
    set<string> next_live_functions = live_functions;
    set<string> next_live_globals = live_globals;
    for(set<string>::const_iterator it = live_functions.begin();
        it != live_functions.end();
        ++it) {
      map<string, const lowir_internal::Function *>::const_iterator found =
          function_by_name.find(*it);
      if(found == function_by_name.end()) {
        continue;
      }
      note_live_symbol_references(*found->second,
                                  function_names,
                                  global_names,
                                  next_live_functions,
                                  next_live_globals);
    }
    for(set<string>::const_iterator it = live_globals.begin();
        it != live_globals.end();
        ++it) {
      map<string, const lowir_internal::GlobalDefinition *>::const_iterator found =
          global_by_name.find(*it);
      if(found == global_by_name.end()) {
        continue;
      }
      note_live_symbol_references(*found->second,
                                  function_names,
                                  global_names,
                                  next_live_functions,
                                  next_live_globals);
    }
    if(next_live_functions.size() != live_functions.size() ||
       next_live_globals.size() != live_globals.size()) {
      live_functions.swap(next_live_functions);
      live_globals.swap(next_live_globals);
      changed = true;
    }
  }

  lowir_internal::Program pruned = program;
  pruned.globals.clear();
  pruned.function_declarations.clear();
  pruned.functions.clear();
  set<string> removed_globals;
  set<string> removed_functions;
  for(size_t i = 0; i < program.globals.size(); ++i) {
    if(object_symbol_definition_is_prunable(program.globals[i].metadata) &&
       live_globals.count(program.globals[i].name) == 0) {
      removed_globals.insert(program.globals[i].name);
      continue;
    }
    pruned.globals.push_back(program.globals[i]);
  }
  for(size_t i = 0; i < program.functions.size(); ++i) {
    const lowir_internal::Function & function = program.functions[i];
    const bool removed_tls_target =
        !function.metadata.tls_for_symbol.empty() &&
        removed_globals.count(function.metadata.tls_for_symbol) != 0;
    if(removed_tls_target ||
       (object_symbol_definition_is_prunable(function.metadata) &&
        live_functions.count(function.name) == 0)) {
      removed_functions.insert(program.functions[i].name);
      continue;
    }
    pruned.functions.push_back(program.functions[i]);
  }

  if(removed_globals.empty() && removed_functions.empty()) {
    return program;
  }

  pruned.exported_symbols.clear();
  for(size_t i = 0; i < program.exported_symbols.size(); ++i) {
    if(removed_functions.count(program.exported_symbols[i].internal_symbol) != 0 ||
       removed_globals.count(program.exported_symbols[i].internal_symbol) != 0) {
      continue;
    }
    pruned.exported_symbols.push_back(program.exported_symbols[i]);
  }
  pruned.object_aliases.clear();
  for(size_t i = 0; i < program.object_aliases.size(); ++i) {
    if(removed_functions.count(program.object_aliases[i].target) != 0 ||
       removed_globals.count(program.object_aliases[i].target) != 0) {
      continue;
    }
    pruned.object_aliases.push_back(program.object_aliases[i]);
  }
  for(size_t i = 0; i < program.function_declarations.size(); ++i) {
    const lowir_internal::FunctionDeclaration & declaration =
        program.function_declarations[i];
    if(!declaration.metadata.tls_for_symbol.empty() &&
       removed_globals.count(declaration.metadata.tls_for_symbol) != 0) {
      continue;
    }
    pruned.function_declarations.push_back(declaration);
  }
  return pruned;
}

string generate_lowir_from_cpp_sources(const vector<string> & srcfiles,
                                       const CppPreprocessOptions & options,
                                       int optimization_level,
                                       int debug_info_level)
{
  return generate_lowir_from_translation_units(
      analyze_cpp_sources(srcfiles,
                          options,
                          true,
                          nullptr,
                          nullptr,
                          debug_info_level >= 1),
      optimization_level,
      debug_info_level);
}

string generate_lowir_from_translation_units(const vector<CallSemNode> & translation_units,
                                             int optimization_level,
                                             int debug_info_level)
{
  PhaseTimer timer("build_lowir_program",
                   std::string("translation-units=") +
                       std::to_string(translation_units.size()));
  lowir_internal::Program program =
      build_lowir_program(translation_units, true, true, debug_info_level >= 1);
  if(normalize_optimization_level(optimization_level) > 0) {
    program = optimize_lowir_program(program, optimization_level);
  }
  if(debug_info_level < 1) {
    clear_lowir_program_debug_locations(program);
  }
  return lowir_internal::dump_program(program);
}

lowir_internal::Program build_lowir_program_from_cpp_sources(
    const vector<string> & srcfiles,
    const CppPreprocessOptions & options,
    int debug_info_level)
{
  vector<CallSemNode> translation_units =
      analyze_cpp_sources(srcfiles,
                          options,
                          true,
                          nullptr,
                          nullptr,
                          debug_info_level >= 1);
  PhaseTimer timer("build_lowir_program", source_count_detail(srcfiles));
  return build_lowir_program(translation_units, true, true, debug_info_level >= 1);
}

lowir_internal::Program prepare_object_lowir_program(lowir_internal::Program program,
                                                     int optimization_level,
                                                     int debug_info_level)
{
  if(debug_info_level < 1) {
    clear_lowir_program_debug_locations(program);
  }
  program = optimize_lowir_program(program, optimization_level);
  program = inline_trivial_identity_object_wrappers(program);
  program = inline_constant_object_wrappers(program);
  program = inline_compare_object_wrappers(program);
  if(normalize_optimization_level(optimization_level) > 0) {
    program = remove_noop_void_object_wrappers(program);
  }
  program = remove_trivial_lifecycle_object_wrappers(program);
  if(normalize_optimization_level(optimization_level) > 0) {
    program = inline_simple_void_object_wrappers(program);
  }
  return prune_unreferenced_object_symbol_definitions(program);
}

machine_object::ObjectFile build_cpp_object_file(const vector<string> & srcfiles,
                                                 const CppPreprocessOptions & options,
                                                 const string & output_target,
                                                 int optimization_level,
                                                 int debug_info_level)
{
  lowir_internal::Program program = prepare_object_lowir_program(
      build_lowir_program_from_cpp_sources(srcfiles, options, debug_info_level),
      optimization_level,
      debug_info_level);
  PhaseTimer timer("build_machine_object",
                   string("target=") + output_target + " " + source_count_detail(srcfiles));
  return build_machine_object(program,
                              effective_host_output_target(output_target),
                              true,
                              true,
                              debug_info_level);
}

void write_cpp_object_file(const vector<string> & srcfiles,
                           const CppPreprocessOptions & options,
                           const string & outfile,
                           const string & output_target,
                           int optimization_level,
                           int debug_info_level,
                           vector<string> * dependency_files)
{
  vector<string> local_dependencies;
  vector<string> * dep_sink =
      dependency_files != nullptr ? dependency_files : &local_dependencies;
  dep_sink->clear();
  vector<CallSemNode> translation_units =
      analyze_cpp_sources(srcfiles,
                          options,
                          true,
                          dep_sink,
                          nullptr,
                          debug_info_level >= 1);
  lowir_internal::Program program = prepare_object_lowir_program(
      build_lowir_program(translation_units, true, true, debug_info_level >= 1),
      optimization_level,
      debug_info_level);
  PhaseTimer timer("write_object_file",
                   string("outfile=") + outfile + " target=" + output_target + " " +
                   source_count_detail(srcfiles));
  machine_object::write_object_file(outfile,
                                    build_machine_object(program,
                                                         effective_host_output_target(output_target),
                                                         true,
                                                         true,
                                                         debug_info_level));
}

int run_cpp_to_lowir_frontend(int argc, char ** argv)
{
  try
  {
    vector<string> args;
    for(int i = 1; i < argc; ++i) {
      args.push_back(argv[i]);
    }
    const CppToolInvocation invocation = parse_cpp_tool_invocation(args);
    if(invocation.query_only() ||
       invocation.compile_only ||
       invocation.preprocess_only ||
       !invocation.explicit_outfile ||
       invocation.inputs.empty()) {
      throw logic_error("invalid usage");
    }

    ofstream out(invocation.outfile.c_str());
    if(!out) {
      throw logic_error("unable to open output file");
    }
    vector<witness::TemplateWitnessSession> witness_sessions;
    vector<witness::TemplateWitnessSession> * witness_sink =
        (invocation.witness_output.empty() &&
         invocation.witness_debug_output.empty()) ?
            nullptr : &witness_sessions;
    const vector<CallSemNode> translation_units =
        analyze_cpp_sources(invocation.inputs,
                            invocation.preprocess_options,
                            true,
                            nullptr,
                            witness_sink,
                            invocation.debug_info_level >= 1 ||
                                witness_sink != nullptr);
    out << generate_lowir_from_translation_units(translation_units,
                                                 invocation.optimization_level,
                                                 invocation.debug_info_level);
    if(!invocation.witness_output.empty()) {
      ofstream witness_output(invocation.witness_output.c_str());
      if(!witness_output) {
        throw logic_error("unable to open witness output file");
      }
      witness_output << render_witness_sessions(invocation.inputs, witness_sessions);
    }
    if(!invocation.witness_debug_output.empty()) {
      ofstream witness_debug_output(invocation.witness_debug_output.c_str());
      if(!witness_debug_output) {
        throw logic_error("unable to open witness debug output file");
      }
      witness_debug_output << render_witness_debug_sessions(invocation.inputs,
                                                            witness_sessions);
    }
    return EXIT_SUCCESS;
  }
  catch(const exception & e)
  {
    cerr << "ERROR: " << e.what() << endl;
    return EXIT_FAILURE;
  }
}
