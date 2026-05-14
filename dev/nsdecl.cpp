// (C) 2013 CPPGM Foundation www.cppgm.org.  All rights reserved.

#include <ctime>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

using namespace std;

#include "nsdecl_semantic.h"
#include "posttokenizer.h"
#include "preprocessor.h"
#include "recog_parser.h"
#include "recog_token_buffer.h"

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
		size_t nsrcfiles = args.size() - 2;
		time_t now = time(nullptr);

		ofstream out(outfile);

		out << nsrcfiles << " translation units" << endl;

		for (size_t i = 0; i < nsrcfiles; i++)
		{
			string srcfile = args[i+2];

			out << "start translation unit " << srcfile << endl;

			Preprocessor preprocessor(srcfile, now);
			SourceLocationTable source_locations;
			PostTokenizer posttokenizer(preprocessor, &source_locations, &preprocessor);
			RecogTokenizer tokenizer(posttokenizer);
				RecogTokenBuffer tokens(tokenizer, srcfile, &source_locations);

				out << describe_nsdecl_translation_unit(tokens);
				out << "end translation unit" << endl;
		}

		return EXIT_SUCCESS;
	}
	catch (exception& e)
	{
		cerr << "ERROR: " << e.what() << endl;
		return EXIT_FAILURE;
	}
}
