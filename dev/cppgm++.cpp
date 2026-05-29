// (C) 2013 CPPGM Foundation www.cppgm.org.  All rights reserved.

#include "cli_batch_frontend.h"
#include "abi_mangle.h"
#include "cpp_batch_frontend.h"
#include "cpp_driver_frontend.h"
#include "cpp_text_generators.h"
#include "cpp_tool_cli.h"
#include "cpp_toolchain.h"
#include "file_timing.h"
#include "template_text_output.h"
#include "tool_help_text.h"
#include "witness_api.h"

#include <cctype>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <set>
#include <string>
#include <utility>
#include <vector>

using namespace std;

namespace {

enum class EmitMode
{
  None,
  Ast,
  Types,
  Semantics,
  LowIR,
  AbiFacts,
};

bool has_arg(const vector<string> & args, const string & needle)
{
  for(size_t i = 0; i < args.size(); ++i) {
    if(args[i] == needle) {
      return true;
    }
  }
  return false;
}

bool has_help_arg(const vector<string> & args)
{
  return has_arg(args, "--help") || has_arg(args, "-h");
}

void consume_emit_flag(vector<string> & args,
                       const string & flag,
                       EmitMode value,
                       EmitMode & out)
{
  vector<string> kept;
  bool found = false;
  for(size_t i = 0; i < args.size(); ++i) {
    if(args[i] == flag) {
      found = true;
      continue;
    }
    kept.push_back(args[i]);
  }

  if(!found) {
    return;
  }

  if(out != EmitMode::None) {
    throw logic_error("multiple --emit-* options provided");
  }
  out = value;
  args.swap(kept);
}

vector<string> collect_args(int argc, char ** argv)
{
  vector<string> args;
  for(int i = 1; i < argc; ++i) {
    args.push_back(argv[i]);
  }
  return args;
}

EmitMode parse_emit_mode(vector<string> & args)
{
  EmitMode mode = EmitMode::None;
  consume_emit_flag(args, "--emit-ast", EmitMode::Ast, mode);
  consume_emit_flag(args, "--emit-types", EmitMode::Types, mode);
  consume_emit_flag(args, "--emit-semantics", EmitMode::Semantics, mode);
  consume_emit_flag(args, "--emit-lowir", EmitMode::LowIR, mode);
  consume_emit_flag(args, "--emit-abi-facts", EmitMode::AbiFacts, mode);
  return mode;
}

template<typename Fn>
int run_main_with_args(const vector<string> & args, const Fn & fn)
{
  vector<string> argv_storage;
  argv_storage.push_back("cppgm++");
  argv_storage.insert(argv_storage.end(), args.begin(), args.end());

  vector<char *> argv;
  for(size_t i = 0; i < argv_storage.size(); ++i) {
    argv.push_back(const_cast<char *>(argv_storage[i].c_str()));
  }

  return fn(static_cast<int>(argv.size()), argv.data());
}

int run_text_mode(const vector<string> & args,
                  const CppTextGenerator & generator)
{
  return run_main_with_args(
      args,
      [&](int argc, char ** argv) {
        return run_cpp_text_frontend(argc, argv, generator);
      });
}

int run_emit_ast_mode(const vector<string> & args)
{
  return run_text_mode(args, generate_cppast_translation_units);
}

int run_emit_types_mode(const vector<string> & args)
{
  return run_text_mode(args, generate_types_translation_units);
}

int run_emit_semantics_mode(const vector<string> & args)
{
  return run_text_mode(
      args,
      [](const vector<string> & srcfiles) {
        return generate_calls_translation_units(srcfiles);
      });
}

int run_emit_lowir_mode(const vector<string> & args)
{
  try
  {
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
                            witness_sink);
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

string abi_fact_label_component(string text)
{
  if(text.empty()) {
    return "case";
  }
  for(size_t i = 0; i < text.size(); ++i) {
    const unsigned char ch = static_cast<unsigned char>(text[i]);
    if(!isalnum(ch)) {
      text[i] = '_';
    }
  }
  return text;
}

bool node_is_ordinary_abi_function(const CallSemNode & node)
{
  if(node.kind != CallSemKind::function_definition &&
     node.kind != CallSemKind::function_declaration) {
    return false;
  }
  if(node.is_constructor ||
     node.is_destructor ||
     node.has_special_member_entry_point_kind) {
    return false;
  }
  return symbol_linkage::has_exported_object_symbol(callsem_symbol(node));
}

bool append_abi_symbol_fact_cases(
    const symbol_linkage::SymbolIdentity & symbol,
    abi_mangle::AbiMangleTargetKind target_kind,
    abi_mangle::AbiFactFile & file,
    set<string> & seen)
{
  if(symbol.linkage == symbol_linkage::SL_INTERNAL) {
    return false;
  }
  if(!symbol.abi_mangle_facts) {
    return false;
  }
  bool appended = false;
  for(size_t i = 0; i < symbol.abi_mangle_facts->size(); ++i) {
    const symbol_linkage::SymbolIdentity::AbiMangleFactEntry & entry =
        (*symbol.abi_mangle_facts)[i];
    if(entry.target.kind != target_kind || entry.object_symbol.empty()) {
      continue;
    }
    const string key =
        to_string(static_cast<int>(entry.target.kind)) + ":" + entry.object_symbol;
    if(!seen.insert(key).second) {
      appended = true;
      continue;
    }
    abi_mangle::AbiFactCase fact_case;
    fact_case.label = abi_fact_label_component(
        symbol.internal_symbol.empty() ? entry.object_symbol : symbol.internal_symbol);
    fact_case.target = entry.target;
    file.cases.push_back(std::move(fact_case));
    appended = true;
  }
  return appended;
}

bool append_abi_function_case(const CallSemNode & node,
                              abi_mangle::AbiFactFile & file,
                              set<string> & seen)
{
  return node_is_ordinary_abi_function(node) &&
         append_abi_symbol_fact_cases(callsem_symbol(node),
                                      abi_mangle::ABI_MANGLE_FUNCTION,
                                      file,
                                      seen);
}

bool append_abi_variable_case(const CallSemNode & node,
                              abi_mangle::AbiFactFile & file,
                              set<string> & seen)
{
  return node.kind == CallSemKind::variable &&
         append_abi_symbol_fact_cases(callsem_symbol(node),
                                      abi_mangle::ABI_MANGLE_VARIABLE,
                                      file,
                                      seen);
}

void collect_abi_fact_cases(const CallSemNode & node,
                            abi_mangle::AbiFactFile & file,
                            set<string> & seen)
{
  append_abi_function_case(node, file, seen);
  append_abi_variable_case(node, file, seen);
  for(size_t i = 0; i < node.children.size(); ++i) {
    collect_abi_fact_cases(node.children[i], file, seen);
  }
}

string generate_abi_fact_text_from_cpp_sources(const vector<string> & inputs,
                                               const CppPreprocessOptions & options)
{
  symbol_linkage::AbiMangleFactCaptureScope abi_fact_capture(true);
  const vector<CallSemNode> translation_units =
      analyze_cpp_sources(inputs, options, true);
  abi_mangle::AbiFactFile file;
  set<string> seen;
  for(size_t i = 0; i < translation_units.size(); ++i) {
    collect_abi_fact_cases(translation_units[i], file, seen);
  }
  return abi_mangle::serialize_fact_file(file);
}

int run_emit_abi_facts_mode(const vector<string> & args)
{
  try
  {
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
    out << generate_abi_fact_text_from_cpp_sources(invocation.inputs,
                                                   invocation.preprocess_options);
    return EXIT_SUCCESS;
  }
  catch(const exception & e)
  {
    cerr << "ERROR: " << e.what() << endl;
    return EXIT_FAILURE;
  }
}

int run_compile_or_link_mode(const vector<string> & args)
{
  if(has_arg(args, "-E")) {
    return run_main_with_args(args, run_cpphostcompat_frontend);
  }
  return run_main_with_args(args, run_cpptoolchain_frontend);
}

int run_cppgm(const vector<string> & raw_args)
{
  if(has_help_arg(raw_args)) {
    cout << cppgm_help_text();
    return EXIT_SUCCESS;
  }

  vector<string> args = raw_args;
  const EmitMode mode = parse_emit_mode(args);

  switch(mode) {
  case EmitMode::Ast:
    return run_emit_ast_mode(args);
  case EmitMode::Types:
    return run_emit_types_mode(args);
  case EmitMode::Semantics:
    return run_emit_semantics_mode(args);
  case EmitMode::LowIR:
    return run_emit_lowir_mode(args);
  case EmitMode::AbiFacts:
    return run_emit_abi_facts_mode(args);
  case EmitMode::None:
    return run_compile_or_link_mode(args);
  }

  throw logic_error("unreachable emit mode");
}

}  // namespace

int main(int argc, char ** argv)
{
  file_timing::startup_mark("main.enter");
  const vector<string> args = collect_args(argc, argv);
  file_timing::startup_mark("main.args_collected");
  if(argc > 0 && argv[0]) {
    set_cpp_tool_program_path(argv[0]);
  }
  file_timing::startup_mark("main.program_path_set");
  const int status = run_cppgm(args);
  file_timing::startup_mark("main.exit");
  return status;
}
