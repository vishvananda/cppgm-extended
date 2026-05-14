#include "lowir_tool_cli.h"

#include <stdexcept>

using namespace std;

LowIRToolInvocation parse_lowir_tool_invocation(const vector<string> & args)
{
  LowIRToolInvocation invocation;
  for(size_t i = 0; i < args.size(); ++i) {
    if(args[i] == "-c") {
      invocation.compile_only = true;
      continue;
    }
    if(args[i] == "--target") {
      if(i + 1 >= args.size()) {
        throw logic_error("missing target after --target");
      }
      invocation.output_target = args[++i];
      continue;
    }
    if(args[i] == "--dump-link-map") {
      if(i + 1 >= args.size()) {
        throw logic_error("missing output file after --dump-link-map");
      }
      invocation.mapfile = args[++i];
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

  if(invocation.outfile.empty() || invocation.inputs.empty()) {
    throw logic_error("invalid usage");
  }
  if(invocation.compile_only && !invocation.mapfile.empty()) {
    throw logic_error("--dump-link-map is not valid with -c");
  }
  if(!invocation.compile_only && !invocation.output_target.empty()) {
    throw logic_error("--target is only valid with -c");
  }
  return invocation;
}

LowIRToolInvocation parse_lowir_tool_invocation(int argc, char ** argv)
{
  vector<string> args;
  for(int i = 1; i < argc; ++i) {
    args.push_back(argv[i]);
  }
  return parse_lowir_tool_invocation(args);
}
