#pragma once

#include <string>
#include <vector>

struct CppPreprocessOptions
{
  std::vector<std::string> include_paths;
  std::vector<std::string> system_include_paths;
  std::vector<std::string> macro_definitions;
  std::vector<std::string> macro_undefinitions;
  std::vector<std::string> forced_include_files;
  bool emit_insignificant_whitespace = true;
};
