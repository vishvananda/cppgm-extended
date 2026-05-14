// (C) 2013 CPPGM Foundation www.cppgm.org.  All rights reserved.

#include <ctime>
#include <utility>
#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>
#include <fstream>

using namespace std;

#include "preproc_output.h"

int main(int argc, char** argv)
{
  try
  {
    vector<string> args;

    for (int i = 1; i < argc; i++)
      args.emplace_back(argv[i]);

    if (args.size() < 3 || args[0] != "-o")
      throw logic_error("invalid usage");

    string outfile = args[1];

	    write_preprocessed_posttoken_output_file(
	        outfile,
	        vector<string>(args.begin() + 2, args.end()));
	    return EXIT_SUCCESS;
	  }
	  catch (exception& e)
	  {
	    cerr << "ERROR: " << e.what() << '\n';
	    return EXIT_FAILURE;
	  }
}
