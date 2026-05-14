#include "cli_batch_frontend.h"
#include "lowir_optimizer.h"
#include "optimization_level.h"
#include "tool_help_text.h"

#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

using namespace std;

namespace {

struct LowIROptInvocation
{
  bool explicit_optimization_level = false;
  int optimization_level = 0;
  string outfile;
  vector<string> inputs;
};

bool has_help_arg(int argc, char ** argv)
{
  for(int i = 1; i < argc; ++i) {
    const string arg = argv[i];
    if(arg == "--help" || arg == "-h") {
      return true;
    }
  }
  return false;
}

LowIROptInvocation parse_lowiropt_invocation(const vector<string> & args)
{
  LowIROptInvocation invocation;
  for(size_t i = 0; i < args.size(); ++i) {
    int optimization_level = invocation.optimization_level;
    if(parse_optimization_level_arg(args[i], optimization_level)) {
      invocation.explicit_optimization_level = true;
      invocation.optimization_level = optimization_level;
      continue;
    }
    if(args[i] == "-o") {
      if(i + 1 >= args.size()) {
        throw logic_error("missing output file after -o");
      }
      invocation.outfile = args[++i];
      continue;
    }
    invocation.inputs.push_back(args[i]);
  }

  if(!invocation.explicit_optimization_level ||
     invocation.outfile.empty() ||
     invocation.inputs.empty()) {
    throw logic_error("invalid usage");
  }
  return invocation;
}

int run_lowiropt_impl(const vector<string> & args)
{
  const LowIROptInvocation invocation = parse_lowiropt_invocation(args);
  ofstream out(invocation.outfile.c_str());
  if(!out) {
    throw logic_error("unable to open output file");
  }
  out << optimize_lowir_text(invocation.inputs, invocation.optimization_level);
  return EXIT_SUCCESS;
}

}  // namespace

int main(int argc, char ** argv)
{
  if(has_help_arg(argc, argv)) {
    cout << lowiropt_help_text();
    return EXIT_SUCCESS;
  }
  return run_cli_frontend(argc, argv, run_lowiropt_impl);
}
