// Student-facing scaffold for the PA8 `lowir` binary.

#include "support/not_implemented.h"
#include "support/tool_help_text.h"

#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

using namespace std;

namespace {

vector<string> collect_args(int argc, char ** argv)
{
  vector<string> args;
  for(int i = 1; i < argc; ++i) {
    args.push_back(argv[i]);
  }
  return args;
}

bool has_help_arg(const vector<string> & args)
{
  for(size_t i = 0; i < args.size(); ++i) {
    if(args[i] == "--help" || args[i] == "-h") {
      return true;
    }
  }
  return false;
}

bool has_batch_stdin_arg(const vector<string> & args)
{
  for(size_t i = 0; i < args.size(); ++i) {
    if(args[i] == "--batch-stdin") {
      return true;
    }
  }
  return false;
}

int run_not_implemented_batch_mode()
{
  string line;
  while(getline(cin, line)) {
    (void)line;
    cout << "EXIT_NOT_IMPLEMENTED" << endl;
  }
  return EXIT_SUCCESS;
}

struct Invocation
{
  string outfile;
  vector<string> inputs;
  string exercise;
};

Invocation parse_invocation(const vector<string> & args)
{
  Invocation invocation;
  for(size_t i = 0; i < args.size(); ++i) {
    const string option = args[i];
    if(option == "-o" || option == "--exercise") {
      if(++i == args.size()) throw logic_error("missing option value");
      string & value = option == "-o" ? invocation.outfile : invocation.exercise;
      if(!value.empty()) throw logic_error("repeated option");
      value = args[i];
    } else if(!option.empty() && option[0] == '-') {
      throw logic_error("unknown option");
    } else {
      invocation.inputs.push_back(option);
    }
  }
  if(invocation.outfile.empty() ||
     (invocation.inputs.empty() == invocation.exercise.empty()))
    throw logic_error("expected input files or one exercise");
  if(!invocation.exercise.empty() && invocation.exercise != "sum" &&
     invocation.exercise != "swap" && invocation.exercise != "call")
    throw logic_error("unknown exercise");
  return invocation;
}

int run_lowir_mode(const vector<string> & args)
{
  if(has_batch_stdin_arg(args)) {
    return run_not_implemented_batch_mode();
  }

  if(has_help_arg(args)) {
    cout << lowir_help_text();
    return EXIT_SUCCESS;
  }

  const Invocation invocation = parse_invocation(args);
  // TODO: read and validate invocation.inputs, or construct invocation.exercise.
  // Both paths build your LowIR model and serialize it to invocation.outfile.
  (void)invocation;

  throw NotImplementedException();
}

}  // namespace

int main(int argc, char ** argv)
{
  try
  {
    return run_lowir_mode(collect_args(argc, argv));
  }
  catch(const NotImplementedException & e)
  {
    cerr << "ERROR: " << e.what() << endl;
    return CPPGM_EXIT_NOT_IMPLEMENTED;
  }
  catch(const exception & e)
  {
    cerr << "ERROR: " << e.what() << endl;
    return EXIT_FAILURE;
  }
}
