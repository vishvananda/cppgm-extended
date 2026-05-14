// (C) 2013 CPPGM Foundation www.cppgm.org.  All rights reserved.

#include <ctime>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

using namespace std;

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

    ofstream out(outfile);
    out << "recog " << nsrcfiles << '\n';

    for(size_t i = 0; i < nsrcfiles; ++i)
    {
      string srcfile = args[i + 2];

      try
      {
        Preprocessor preprocessor(srcfile, now);
        SourceLocationTable source_locations;
        PostTokenizer posttokenizer(preprocessor, &source_locations, &preprocessor);
        RecogTokenizer tokenizer(posttokenizer);
        RecogTokenBuffer tokens(tokenizer, srcfile, &source_locations);
        RecogParser parser(tokens);
        if(!parser.parse_translation_unit())
          throw logic_error(parser.error());

        if(tokens.size() == 1)
          cout << "empty" << '\n';
        else
          cout << "translation-unit" << '\n';

        out << srcfile << " OK" << '\n';
      }
      catch(exception& e)
      {
        cerr << srcfile << ": " << e.what() << '\n';
	        out << srcfile << " BAD" << '\n';
	      }
	    }
	    return EXIT_SUCCESS;
	  }
	  catch(exception& e)
	  {
	    cerr << "ERROR: " << e.what() << '\n';
	    return EXIT_FAILURE;
	  }
}
