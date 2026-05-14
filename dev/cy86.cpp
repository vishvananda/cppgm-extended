// (C) 2013 CPPGM Foundation www.cppgm.org.  All rights reserved.

#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

using namespace std;

#include "cy86_compiler.h"

int main(int argc, char ** argv)
{
  try
  {
    vector<string> args;
    for(int i = 1; i < argc; ++i) {
      args.push_back(argv[i]);
    }

    string output_target;
    string outfile;
    vector<string> srcfiles;

    for(size_t i = 0; i < args.size(); ++i) {
      if(args[i] == "--target") {
        if(i + 1 >= args.size()) {
          throw logic_error("missing target after --target");
        }
        output_target = args[++i];
        continue;
      }
      if(args[i] == "-o") {
        if(i + 1 >= args.size()) {
          throw logic_error("missing output file after -o");
        }
        outfile = args[++i];
        continue;
      }
      srcfiles.push_back(args[i]);
    }

    if(outfile.empty() || srcfiles.empty()) {
      throw logic_error("invalid usage");
    }

    build_cy86_program(srcfiles, outfile, output_target);
    return EXIT_SUCCESS;
  }
  catch(exception & e)
  {
    cerr << "ERROR: " << e.what() << endl;
    return EXIT_FAILURE;
  }
}
