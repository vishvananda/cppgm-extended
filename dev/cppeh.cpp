#include "lowir_driver_frontend.h"
#include "tool_help_text.h"

#include <iostream>
#include <string>

using namespace std;

namespace {

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

int run_cppeh_mode(int argc, char ** argv)
{
  if(has_help_arg(argc, argv)) {
    cout << cppeh_help_text();
    return EXIT_SUCCESS;
  }
  return run_cppeh_frontend(argc, argv);
}

}  // namespace

int main(int argc, char ** argv)
{
  return run_cppeh_mode(argc, argv);
}
