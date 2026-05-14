#pragma once

#include <string>
#include <vector>

struct LowIRToolInvocation
{
  bool compile_only = false;
  std::string output_target;
  std::string outfile;
  std::string mapfile;
  std::vector<std::string> inputs;
};

LowIRToolInvocation parse_lowir_tool_invocation(const std::vector<std::string> & args);
LowIRToolInvocation parse_lowir_tool_invocation(int argc, char ** argv);
