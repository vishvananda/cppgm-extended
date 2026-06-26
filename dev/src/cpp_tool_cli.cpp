#include "cpp_tool_cli.h"

#include <stdexcept>

using namespace std;

#include "optimization_level.h"

namespace {

bool ends_with(const string & path, const string & suffix)
{
  return path.size() >= suffix.size() &&
      path.compare(path.size() - suffix.size(), suffix.size(), suffix) == 0;
}

string basename_only(const string & path)
{
  const string::size_type slash = path.find_last_of("/\\");
  if(slash == string::npos) {
    return path;
  }
  return path.substr(slash + 1);
}

string default_object_output_path(const string & input)
{
  string base = basename_only(input);
  const string::size_type dot = base.find_last_of('.');
  if(dot != string::npos) {
    base.erase(dot);
  }
  return base + ".o";
}

bool starts_with(const string & value, const string & prefix)
{
  return value.size() >= prefix.size() &&
      value.compare(0, prefix.size(), prefix) == 0;
}

bool is_query_driver_flag(const string & arg)
{
  return arg == "--version" ||
      arg == "--help" ||
      arg == "-h" ||
      arg == "-v" ||
      arg == "-dumpmachine" ||
      arg == "-dumpversion" ||
      arg == "-print-search-dirs";
}

bool is_benign_driver_flag(const string & arg)
{
  return arg == "-Wall" ||
      arg == "-Winvalid-offsetof" ||
      arg == "-pipe" ||
      arg == "-w" ||
      arg == "-pg" ||
      arg == "-pedantic" ||
      arg == "-pedantic-errors" ||
      starts_with(arg, "-W") ||
      starts_with(arg, "-f") ||
      starts_with(arg, "-m") ||
      starts_with(arg, "-std=");
}

logic_error missing_option_argument(const string & option,
                                    const string & expected)
{
  return logic_error("missing " + expected + " after " + option);
}

bool consume_query_family(const vector<string> & args,
                          size_t i)
{
  if(!is_query_driver_flag(args[i])) {
    return false;
  }
  throw logic_error("query flag must appear first: " + args[i]);
}

bool consume_phase_output_flag(CppToolInvocation & invocation,
                               const vector<string> & args,
                               size_t & i)
{
  if(args[i] == "-c") {
    invocation.compile_only = true;
    return true;
  }
  if(args[i] == "-E") {
    invocation.preprocess_only = true;
    return true;
  }
  if(args[i] == "-o") {
    if(i + 1 >= args.size()) {
      throw missing_option_argument("-o", "output file");
    }
    invocation.explicit_outfile = true;
    invocation.outfile = args[++i];
    return true;
  }
  return false;
}

bool consume_macro_include_flag(CppToolInvocation & invocation,
                                const vector<string> & args,
                                size_t & i)
{
  if(args[i] == "-D") {
    if(i + 1 >= args.size()) {
      throw missing_option_argument("-D", "macro definition");
    }
    invocation.preprocess_options.macro_definitions.push_back(args[++i]);
    return true;
  }
  if(starts_with(args[i], "-D") && args[i].size() > 2) {
    invocation.preprocess_options.macro_definitions.push_back(args[i].substr(2));
    return true;
  }
  if(args[i] == "-U") {
    if(i + 1 >= args.size()) {
      throw missing_option_argument("-U", "macro name");
    }
    invocation.preprocess_options.macro_undefinitions.push_back(args[++i]);
    return true;
  }
  if(starts_with(args[i], "-U") && args[i].size() > 2) {
    invocation.preprocess_options.macro_undefinitions.push_back(args[i].substr(2));
    return true;
  }
  if(args[i] == "-include") {
    if(i + 1 >= args.size()) {
      throw missing_option_argument("-include", "file");
    }
    invocation.preprocess_options.forced_include_files.push_back(args[++i]);
    return true;
  }
  return false;
}

bool consume_search_flag(CppToolInvocation & invocation,
                         const vector<string> & args,
                         size_t & i)
{
  if(args[i] == "-I") {
    if(i + 1 >= args.size()) {
      throw missing_option_argument("-I", "path");
    }
    invocation.preprocess_options.include_paths.push_back(args[++i]);
    return true;
  }
  if(starts_with(args[i], "-I") && args[i].size() > 2) {
    invocation.preprocess_options.include_paths.push_back(args[i].substr(2));
    return true;
  }
  if(args[i] == "-isystem") {
    if(i + 1 >= args.size()) {
      throw missing_option_argument("-isystem", "path");
    }
    invocation.preprocess_options.system_include_paths.push_back(args[++i]);
    return true;
  }
  if(starts_with(args[i], "-isystem") && args[i].size() > 8) {
    invocation.preprocess_options.system_include_paths.push_back(args[i].substr(8));
    return true;
  }
  if(args[i] == "-L") {
    if(i + 1 >= args.size()) {
      throw missing_option_argument("-L", "path");
    }
    invocation.library_paths.push_back(args[++i]);
    return true;
  }
  if(starts_with(args[i], "-L") && args[i].size() > 2) {
    invocation.library_paths.push_back(args[i].substr(2));
    return true;
  }
  if(args[i] == "-l") {
    if(i + 1 >= args.size()) {
      throw missing_option_argument("-l", "library name");
    }
    invocation.libraries.push_back(args[++i]);
    return true;
  }
  if(starts_with(args[i], "-l") && args[i].size() > 2) {
    invocation.libraries.push_back(args[i].substr(2));
    return true;
  }
  return false;
}

bool consume_depfile_flag(CppToolInvocation & invocation,
                          const vector<string> & args,
                          size_t & i)
{
  if(args[i] == "-MMD" || args[i] == "-MD") {
    invocation.generate_depfile = true;
    return true;
  }
  if(args[i] == "-MP") {
    invocation.depfile_phony_targets = true;
    return true;
  }
  if(args[i] == "-MF") {
    if(i + 1 >= args.size()) {
      throw missing_option_argument("-MF", "depfile path");
    }
    invocation.depfile = args[++i];
    return true;
  }
  if(starts_with(args[i], "-MF") && args[i].size() > 3) {
    invocation.depfile = args[i].substr(3);
    return true;
  }
  if(args[i] == "-MT" || args[i] == "-MQ") {
    if(i + 1 >= args.size()) {
      throw missing_option_argument(args[i], "target");
    }
    invocation.dep_targets.push_back(args[++i]);
    return true;
  }
  if((starts_with(args[i], "-MT") || starts_with(args[i], "-MQ")) &&
     args[i].size() > 3) {
    invocation.dep_targets.push_back(args[i].substr(3));
    return true;
  }
  return false;
}

bool consume_witness_flag(CppToolInvocation & invocation,
                          const vector<string> & args,
                          size_t & i)
{
  if(args[i] == "--witness") {
    if(i + 1 >= args.size()) {
      throw missing_option_argument("--witness", "output file");
    }
    invocation.witness_output = args[++i];
    return true;
  }
  if(args[i] == "--witness-debug") {
    if(i + 1 >= args.size()) {
      throw missing_option_argument("--witness-debug", "output file");
    }
    invocation.witness_debug_output = args[++i];
    return true;
  }
  return false;
}

bool consume_toolchain_flag(CppToolInvocation & invocation,
                            const vector<string> & args,
                            size_t & i)
{
  if(args[i] == "-g0") {
    invocation.debug_info_level = 0;
    return true;
  }
  if(args[i] == "-gline-tables-only") {
    invocation.debug_info_level = 1;
    return true;
  }
  if(args[i] == "-g" || starts_with(args[i], "-g")) {
    invocation.debug_info_level = 2;
    return true;
  }
  int optimization_level = invocation.optimization_level;
  if(parse_optimization_level_arg(args[i], optimization_level)) {
    invocation.optimization_level = optimization_level;
    return true;
  }
  if(args[i] == "--target") {
    if(i + 1 >= args.size()) {
      throw missing_option_argument("--target", "target");
    }
    invocation.output_target = args[++i];
    return true;
  }
  if(starts_with(args[i], "--target=")) {
    if(args[i].size() == string("--target=").size()) {
      throw missing_option_argument("--target", "target");
    }
    invocation.output_target = args[i].substr(string("--target=").size());
    return true;
  }
  if(args[i] == "-std") {
    if(i + 1 >= args.size()) {
      throw missing_option_argument("-std", "language standard");
    }
    ++i;
    return true;
  }
  if(args[i] == "-stdlib") {
    if(i + 1 >= args.size()) {
      throw missing_option_argument("-stdlib", "standard library name");
    }
    invocation.stdlib_flag = "-stdlib=" + args[++i];
    return true;
  }
  if(starts_with(args[i], "-stdlib=")) {
    invocation.stdlib_flag = args[i];
    return true;
  }
  if(args[i] == "-pthread") {
    throw logic_error("option not yet supported: -pthread");
  }
  return false;
}

bool consume_benign_flag(const vector<string> & args,
                         size_t i)
{
  return is_benign_driver_flag(args[i]);
}

}  // namespace

CppToolInvocation parse_cpp_tool_invocation(const vector<string> & args)
{
  CppToolInvocation invocation;
  if(!args.empty() && is_query_driver_flag(args[0])) {
    invocation.query_args = args;
    return invocation;
  }

  for(size_t i = 0; i < args.size(); ++i) {
    if(consume_query_family(args, i)) {
      continue;
    }
    if(consume_benign_flag(args, i)) {
      continue;
    }
    if(consume_phase_output_flag(invocation, args, i)) {
      continue;
    }
    if(consume_macro_include_flag(invocation, args, i)) {
      continue;
    }
    if(consume_search_flag(invocation, args, i)) {
      continue;
    }
    if(consume_depfile_flag(invocation, args, i)) {
      continue;
    }
    if(consume_witness_flag(invocation, args, i)) {
      continue;
    }
    if(consume_toolchain_flag(invocation, args, i)) {
      continue;
    }
    invocation.inputs.push_back(args[i]);
  }

  if(invocation.query_only()) {
    return invocation;
  }
  if(invocation.inputs.empty()) {
    throw logic_error("invalid usage");
  }
  if(invocation.compile_only && invocation.preprocess_only) {
    throw logic_error("cannot combine -c and -E");
  }
  if(invocation.compile_only && invocation.explicit_outfile &&
     invocation.inputs.size() != 1) {
    throw logic_error("cannot specify -o when generating multiple output files");
  }
  if(invocation.preprocess_only && invocation.explicit_outfile &&
     invocation.inputs.size() != 1) {
    throw logic_error("cannot specify -o when generating multiple output files");
  }
  if(invocation.compile_only && invocation.inputs.size() != 1 &&
     (!invocation.depfile.empty() || !invocation.dep_targets.empty())) {
    throw logic_error("explicit depfile output requires exactly one compile input");
  }
  if((!invocation.preprocess_options.macro_undefinitions.empty() ||
      !invocation.preprocess_options.forced_include_files.empty()) &&
     invocation.inputs.empty()) {
    throw logic_error("invalid usage");
  }
  if(invocation.compile_only && !invocation.explicit_outfile &&
     invocation.inputs.size() == 1) {
    invocation.outfile = default_object_output_path(invocation.inputs[0]);
  }
  if(!invocation.compile_only && !invocation.preprocess_only &&
     !invocation.explicit_outfile) {
    invocation.outfile = "a.out";
  }
  return invocation;
}

CppToolInvocation parse_cpp_tool_invocation(int argc, char ** argv)
{
  vector<string> args;
  for(int i = 1; i < argc; ++i) {
    args.push_back(argv[i]);
  }
  return parse_cpp_tool_invocation(args);
}

bool path_looks_like_object_file(const string & path)
{
  return ends_with(path, ".o") ||
      ends_with(path, ".obj") ||
      ends_with(path, ".a");
}

bool path_looks_like_lowir_file(const string & path)
{
  return ends_with(path, ".lowir");
}
