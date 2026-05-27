// (C) 2013 CPPGM Foundation www.cppgm.org.  All rights reserved.

#include "cli_batch_frontend.h"
#include "abi_mangle.h"
#include "cpp_batch_frontend.h"
#include "cpp_decl_model.h"
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

string abi_qualified_name_text(const cpp_decl::QualifiedName & qualified)
{
  string out = qualified.rooted ? "::" : string();
  for(size_t i = 0; i < qualified.qualifiers.size(); ++i) {
    out += qualified.qualifiers[i];
    out += "::";
  }
  out += qualified.name;
  return out;
}

string abi_node_qualified_name_text(const CallSemNode & node)
{
  if(!callsem_resolved_name(node).empty()) {
    return callsem_resolved_name(node);
  }
  const shared_ptr<cpp_decl::QualifiedName> & qualified =
      callsem_qualified_name_syntax(node);
  if(qualified) {
    return abi_qualified_name_text(*qualified);
  }
  return node.text.str();
}

bool abi_builtin_type(EFundamentalType type, abi_mangle::AbiBuiltinType & out)
{
  switch(type) {
  case FT_SIGNED_CHAR: out = abi_mangle::ABI_BUILTIN_SIGNED_CHAR; return true;
  case FT_SHORT_INT: out = abi_mangle::ABI_BUILTIN_SHORT; return true;
  case FT_INT: out = abi_mangle::ABI_BUILTIN_INT; return true;
  case FT_LONG_INT: out = abi_mangle::ABI_BUILTIN_LONG; return true;
  case FT_LONG_LONG_INT: out = abi_mangle::ABI_BUILTIN_LONG_LONG; return true;
  case FT_INT128: out = abi_mangle::ABI_BUILTIN_INT128; return true;
  case FT_UNSIGNED_CHAR: out = abi_mangle::ABI_BUILTIN_UNSIGNED_CHAR; return true;
  case FT_UNSIGNED_SHORT_INT: out = abi_mangle::ABI_BUILTIN_UNSIGNED_SHORT; return true;
  case FT_UNSIGNED_INT: out = abi_mangle::ABI_BUILTIN_UNSIGNED_INT; return true;
  case FT_UNSIGNED_LONG_INT: out = abi_mangle::ABI_BUILTIN_UNSIGNED_LONG; return true;
  case FT_UNSIGNED_LONG_LONG_INT: out = abi_mangle::ABI_BUILTIN_UNSIGNED_LONG_LONG; return true;
  case FT_UINT128: out = abi_mangle::ABI_BUILTIN_UINT128; return true;
  case FT_WCHAR_T: out = abi_mangle::ABI_BUILTIN_WCHAR; return true;
  case FT_CHAR: out = abi_mangle::ABI_BUILTIN_CHAR; return true;
  case FT_CHAR16_T: out = abi_mangle::ABI_BUILTIN_CHAR16; return true;
  case FT_CHAR32_T: out = abi_mangle::ABI_BUILTIN_CHAR32; return true;
  case FT_BOOL: out = abi_mangle::ABI_BUILTIN_BOOL; return true;
  case FT_FLOAT: out = abi_mangle::ABI_BUILTIN_FLOAT; return true;
  case FT_DOUBLE: out = abi_mangle::ABI_BUILTIN_DOUBLE; return true;
  case FT_LONG_DOUBLE: out = abi_mangle::ABI_BUILTIN_LONG_DOUBLE; return true;
  case FT_VOID: out = abi_mangle::ABI_BUILTIN_VOID; return true;
  case FT_NULLPTR_T: out = abi_mangle::ABI_BUILTIN_NULLPTR; return true;
  }
  return false;
}

bool abi_type_from_cpp_type(const cpp_decl::TypePtr & type,
                            abi_mangle::AbiType & out)
{
  if(!type) {
    return false;
  }

  switch(type->kind) {
  case cpp_decl::Type::TK_FUNDAMENTAL:
    out.kind = abi_mangle::ABI_TYPE_BUILTIN;
    return abi_builtin_type(type->fundamental, out.builtin_type);

  case cpp_decl::Type::TK_NAMED:
    if(type->named_display.empty() || type->named_lambda_mangle) {
      return false;
    }
    out.kind = abi_mangle::ABI_TYPE_NAMED;
    out.name = type->named_display;
    return true;

  case cpp_decl::Type::TK_CV: {
    abi_mangle::AbiType inner;
    if(!abi_type_from_cpp_type(type->inner, inner)) {
      return false;
    }
    if(type->cv_volatile) {
      abi_mangle::AbiType wrapper;
      wrapper.kind = abi_mangle::ABI_TYPE_VOLATILE;
      wrapper.child_types.push_back(std::move(inner));
      inner = std::move(wrapper);
    }
    if(type->cv_const) {
      out.kind = abi_mangle::ABI_TYPE_CONST;
      out.child_types.push_back(std::move(inner));
    } else {
      out = std::move(inner);
    }
    return true;
  }

  case cpp_decl::Type::TK_ATOMIC:
    out.kind = abi_mangle::ABI_TYPE_VENDOR_QUALIFIED;
    out.name = "_Atomic";
    out.vendor_qualifier = abi_mangle::ABI_VENDOR_QUALIFIER_ATOMIC;
    out.child_types.push_back(abi_mangle::AbiType());
    return abi_type_from_cpp_type(type->inner, out.child_types.back());

  case cpp_decl::Type::TK_POINTER:
    out.kind = abi_mangle::ABI_TYPE_POINTER;
    out.child_types.push_back(abi_mangle::AbiType());
    return abi_type_from_cpp_type(type->inner, out.child_types.back());

  case cpp_decl::Type::TK_LVALUE_REFERENCE:
    out.kind = abi_mangle::ABI_TYPE_LVALUE_REFERENCE;
    out.child_types.push_back(abi_mangle::AbiType());
    return abi_type_from_cpp_type(type->inner, out.child_types.back());

  case cpp_decl::Type::TK_RVALUE_REFERENCE:
    out.kind = abi_mangle::ABI_TYPE_RVALUE_REFERENCE;
    out.child_types.push_back(abi_mangle::AbiType());
    return abi_type_from_cpp_type(type->inner, out.child_types.back());

  case cpp_decl::Type::TK_ARRAY:
    if(!type->has_bound) {
      return false;
    }
    out.kind = abi_mangle::ABI_TYPE_ARRAY;
    out.array_bound.kind = abi_mangle::ABI_ARRAY_BOUND_INTEGER;
    out.array_bound.integer_value = type->bound;
    out.child_types.push_back(abi_mangle::AbiType());
    return abi_type_from_cpp_type(type->inner, out.child_types.back());

  case cpp_decl::Type::TK_FUNCTION:
    out.kind = abi_mangle::ABI_TYPE_FUNCTION;
    out.variadic = type->variadic;
    out.child_types.reserve(type->params.size() + 1);
    out.child_types.push_back(abi_mangle::AbiType());
    if(!abi_type_from_cpp_type(type->inner, out.child_types.back())) {
      return false;
    }
    for(size_t i = 0; i < type->params.size(); ++i) {
      out.child_types.push_back(abi_mangle::AbiType());
      if(!abi_type_from_cpp_type(type->params[i], out.child_types.back())) {
        return false;
      }
    }
    return true;

  case cpp_decl::Type::TK_MEMBER_POINTER:
    out.kind = abi_mangle::ABI_TYPE_MEMBER_POINTER;
    out.child_types.push_back(abi_mangle::AbiType());
    out.child_types.push_back(abi_mangle::AbiType());
    return abi_type_from_cpp_type(type->owner, out.child_types[0]) &&
           abi_type_from_cpp_type(type->inner, out.child_types[1]);

  case cpp_decl::Type::TK_BLOCK_POINTER:
    return false;
  }
  return false;
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

bool append_abi_function_case(const CallSemNode & node,
                              abi_mangle::AbiFactFile & file,
                              set<string> & seen)
{
  if(!node_is_ordinary_abi_function(node)) {
    return false;
  }
  cpp_decl::TypePtr type = strip_top_level_cv(node.semantic_type);
  if(!type || type->kind != cpp_decl::Type::TK_FUNCTION) {
    return false;
  }
  const string qualified_name = abi_node_qualified_name_text(node);
  if(qualified_name.empty() ||
     qualified_name == "main" ||
     callsem_symbol(node).internal_symbol == "@main" ||
     qualified_name.find("operator") != string::npos) {
    return false;
  }
  const string key = string("function:") + callsem_symbol(node).object_symbol;
  if(!seen.insert(key).second) {
    return true;
  }

  abi_mangle::AbiFactCase fact_case;
  fact_case.label = abi_fact_label_component(callsem_symbol(node).internal_symbol);
  fact_case.target.kind = abi_mangle::ABI_MANGLE_FUNCTION;
  fact_case.target.function.form = abi_mangle::ABI_FUNCTION_PATH;
  fact_case.target.function.c_linkage = node.is_c_linkage;
  fact_case.target.function.qualified_name = qualified_name;
  fact_case.target.function.variadic = type->variadic;
  fact_case.target.function.nested_const = node.is_const_method || type->function_const;
  fact_case.target.function.nested_volatile =
      node.is_volatile_method || type->function_volatile;
  fact_case.target.function.abi_tags = callsem_abi_tags(node);
  fact_case.target.function.parameter_types.reserve(type->params.size());
  for(size_t i = 0; i < type->params.size(); ++i) {
    fact_case.target.function.parameter_types.push_back(abi_mangle::AbiType());
    if(!abi_type_from_cpp_type(type->params[i],
                               fact_case.target.function.parameter_types.back())) {
      return false;
    }
  }
  file.cases.push_back(std::move(fact_case));
  return true;
}

bool append_abi_variable_case(const CallSemNode & node,
                              abi_mangle::AbiFactFile & file,
                              set<string> & seen)
{
  if(node.kind != CallSemKind::variable ||
     !symbol_linkage::has_exported_object_symbol(callsem_symbol(node))) {
    return false;
  }
  const string qualified_name = abi_node_qualified_name_text(node);
  if(qualified_name.empty()) {
    return false;
  }
  const string key = string("variable:") + callsem_symbol(node).object_symbol;
  if(seen.insert(key).second) {
    abi_mangle::AbiFactCase fact_case;
    fact_case.label = abi_fact_label_component(callsem_symbol(node).internal_symbol);
    fact_case.target.kind = abi_mangle::ABI_MANGLE_VARIABLE;
    fact_case.target.qualified_name = qualified_name;
    fact_case.target.c_linkage = node.is_c_linkage;
    file.cases.push_back(std::move(fact_case));
  }
  if(node.is_thread_local &&
     !callsem_symbol(node).thread_local_wrapper_object_symbol.empty()) {
    const string tls_key =
        string("tls-wrapper:") +
        callsem_symbol(node).thread_local_wrapper_object_symbol;
    if(seen.insert(tls_key).second) {
      abi_mangle::AbiFactCase fact_case;
      fact_case.label =
          abi_fact_label_component(callsem_symbol(node).internal_symbol + "__tls_wrapper");
      fact_case.target.kind = abi_mangle::ABI_MANGLE_THREAD_LOCAL_WRAPPER;
      fact_case.target.qualified_name = qualified_name;
      file.cases.push_back(std::move(fact_case));
    }
  }
  return true;
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
