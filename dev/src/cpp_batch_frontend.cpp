#include "cpp_batch_frontend.h"

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

using namespace std;

namespace {

int write_text_output(const vector<string> & srcfiles,
                      const string & outfile,
                      const CppTextGenerator & generator,
                      ostream & err)
{
  try
  {
    ofstream out(outfile.c_str());
    if(!out) {
      throw logic_error("unable to open output file");
    }
    out << generator(srcfiles);
    return EXIT_SUCCESS;
  }
  catch(const exception & e)
  {
    err << "ERROR: " << e.what() << endl;
    return EXIT_FAILURE;
  }
}

int run_text_request(const vector<string> & args,
                     const CppTextGenerator & generator,
                     ostream & err)
{
  if(args.size() < 3 || args[0] != "-o") {
    err << "ERROR: invalid usage" << endl;
    return EXIT_FAILURE;
  }

  return write_text_output(vector<string>(args.begin() + 2, args.end()),
                           args[1],
                           generator,
                           err);
}

}

int run_cpp_text_frontend(int argc,
                          char ** argv,
                          const CppTextGenerator & generator)
{
  try
  {
    vector<string> args;
    for(int i = 1; i < argc; ++i) {
      args.emplace_back(argv[i]);
    }
    return run_text_request(args, generator, cerr);
  }
  catch(const exception & e)
  {
    cerr << "ERROR: " << e.what() << endl;
    return EXIT_FAILURE;
  }
}
