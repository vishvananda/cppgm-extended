// (C) 2013 CPPGM Foundation www.cppgm.org.  All rights reserved.

#include <utility>
#include <sys/stat.h>
#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>
#include <fstream>

using namespace std;

#include "support/not_implemented.h"

// Supplied host helper for pragma-once identity; no syscall implementation
// is required in this assignment.
typedef pair<unsigned long long, unsigned long long> PreprocessorFileId;

bool GetPreprocessorFileId(const string& path, PreprocessorFileId& fileid)
{
    struct stat info;
    if (stat(path.c_str(), &info) != 0)
        return false;
    fileid = make_pair(static_cast<unsigned long long>(info.st_dev),
                      static_cast<unsigned long long>(info.st_ino));
    return true;
}

bool HasBatchStdinArg(int argc, char** argv)
{
	for (int i = 1; i < argc; i++)
	{
		if (string(argv[i]) == "--batch-stdin")
			return true;
	}
	return false;
}

int RunNotImplementedBatchMode()
{
	string line;
	while (getline(cin, line))
	{
		(void)line;
		cout << "EXIT_NOT_IMPLEMENTED" << endl;
	}
	return EXIT_SUCCESS;
}

int main(int argc, char** argv)
{
	try
	{
		if (HasBatchStdinArg(argc, argv))
			return RunNotImplementedBatchMode();

		vector<string> args;

		for (int i = 1; i < argc; i++)
			args.emplace_back(argv[i]);

		if (args.size() < 3 || args[0] != "-o")
			throw logic_error("invalid usage");

		string outfile = args[1];
		size_t nsrcfiles = args.size() - 2;

		throw NotImplementedException();

		ofstream out(outfile);

		out << "preproc " << nsrcfiles << endl;

		for (size_t i = 0; i < nsrcfiles; i++)
		{
			string srcfile = args[i+2];

			out << "sof " << srcfile << endl;

			ifstream in(srcfile);

			// TODO: implement `preproc` as described in the complete preprocessor assignment
			out << "not yet implemented" << endl;
	
			out << "eof" << endl;

		}
	}
	catch (const NotImplementedException& e)
	{
		cerr << "ERROR: " << e.what() << endl;
		return CPPGM_EXIT_NOT_IMPLEMENTED;
	}
	catch (exception& e)
	{
		cerr << "ERROR: " << e.what() << endl;
		return EXIT_FAILURE;
	}
}
