#pragma once

#include <string>
#include <vector>

namespace abi_mangle {

struct Invocation
{
  std::string outfile;
  std::vector<std::string> inputs;
};

std::string mangle_fact_files(const std::vector<std::string> & input_paths);

}  // namespace abi_mangle
