// Student-facing scaffold for the PA24 `cpplink` binary.

#include "exceptions.h"
#include "tool_help_text.h"

#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

using namespace std;

namespace {

struct LinkToolInvocation
{
  bool compile_only = false;
  string output_target;
  string outfile;
  string mapfile;
  vector<string> inputs;
};

vector<string> collect_args(int argc, char ** argv)
{
  vector<string> args;
  for(int i = 1; i < argc; ++i) {
    args.push_back(argv[i]);
  }
  return args;
}

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

int run_not_implemented_batch_mode()
{
  string line;
  while(getline(cin, line)) {
    (void)line;
    cout << "EXIT_NOT_IMPLEMENTED" << endl;
  }
  return EXIT_SUCCESS;
}

string consume_option_argument(const vector<string> & args,
                               size_t & index,
                               const string & option,
                               const string & description)
{
  if(index + 1 >= args.size()) {
    throw logic_error("missing " + description + " after " + option);
  }
  ++index;
  return args[index];
}

LinkToolInvocation parse_link_tool_invocation(const vector<string> & args)
{
  LinkToolInvocation invocation;
  for(size_t i = 0; i < args.size(); ++i) {
    if(args[i] == "-c") {
      invocation.compile_only = true;
      continue;
    }
    if(args[i] == "--target") {
      invocation.output_target =
          consume_option_argument(args, i, "--target", "target");
      continue;
    }
    if(args[i] == "--dump-link-map") {
      invocation.mapfile =
          consume_option_argument(args, i, "--dump-link-map", "output file");
      continue;
    }
    if(args[i] == "-o") {
      invocation.outfile =
          consume_option_argument(args, i, "-o", "output file");
      continue;
    }
    if(!args[i].empty() && args[i][0] == '-') {
      throw logic_error("unknown option: " + args[i]);
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

int run_compile_mode(const LinkToolInvocation & invocation)
{
  (void)invocation;
  throw NotImplementedException();
}

int run_link_mode(const LinkToolInvocation & invocation)
{
  (void)invocation;
  throw NotImplementedException();
}

int run_cpplink_mode(const vector<string> & args)
{
  if(has_arg(args, "--batch-stdin")) {
    return run_not_implemented_batch_mode();
  }

  if(has_help_arg(args)) {
    cout << cpplink_help_text();
    return EXIT_SUCCESS;
  }

  const LinkToolInvocation invocation = parse_link_tool_invocation(args);
  if(invocation.compile_only) {
    return run_compile_mode(invocation);
  }
  return run_link_mode(invocation);
}

}  // namespace

int main(int argc, char ** argv)
{
  try
  {
    return run_cpplink_mode(collect_args(argc, argv));
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
