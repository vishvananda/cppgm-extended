#pragma once

#include <string>
#include <vector>

#include "cpp_preprocess_options.h"

struct CppToolInvocation
{
  bool compile_only = false;
  bool preprocess_only = false;
  bool generate_depfile = false;
  bool depfile_phony_targets = false;
  bool explicit_outfile = false;
  int optimization_level = 0;
  int debug_info_level = 0;
  std::string output_target;
  std::string stdlib_flag;
  std::string outfile;
  std::string witness_output;
  std::string witness_debug_output;
  std::string depfile;
  std::vector<std::string> query_args;
  std::vector<std::string> library_paths;
  std::vector<std::string> libraries;
  std::vector<std::string> dep_targets;
  std::vector<std::string> inputs;
  CppPreprocessOptions preprocess_options;

  bool query_only() const
  {
    return !query_args.empty();
  }
};

CppToolInvocation parse_cpp_tool_invocation(const std::vector<std::string> & args);
bool path_looks_like_object_file(const std::string & path);
bool path_looks_like_lowir_file(const std::string & path);
