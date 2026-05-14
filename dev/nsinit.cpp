// (C) 2013 CPPGM Foundation www.cppgm.org.  All rights reserved.

#include <ctime>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

using namespace std;

#include "nsinit_semantic.h"
#include "posttokenizer.h"
#include "preprocessor.h"
#include "recog_parser.h"
#include "recog_token_buffer.h"

int main(int argc, char** argv)
{
  try
  {
    vector<string> args;

    for(int i = 1; i < argc; ++i)
      args.emplace_back(argv[i]);

    if(args.size() < 3 || args[0] != "-o")
      throw logic_error("invalid usage");

    string outfile = args[1];
    size_t nsrcfiles = args.size() - 2;
    time_t now = time(nullptr);
    vector<unique_ptr<NSTranslationUnitInput>> units;

    for(size_t i = 0; i < nsrcfiles; ++i)
    {
      string srcfile = args[i + 2];
      units.push_back(unique_ptr<NSTranslationUnitInput>(
          new NSTranslationUnitInput(srcfile, now)));
    }

    vector<char> program_image = build_nsinit_program_image(units);

    ofstream out(outfile, ios::binary);
    out.write(program_image.data(), program_image.size());
    return EXIT_SUCCESS;
  }
  catch(exception& e)
  {
    cerr << "ERROR: " << e.what() << endl;
    return EXIT_FAILURE;
  }
}
