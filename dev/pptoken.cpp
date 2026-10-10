#include <iostream>
#include <stdexcept>
#include <string>

using namespace std;

#include "preprocess/source/source_buffer.h"
#include "preprocess/tokens/DebugPPTokenStream.h"
#include "preprocess/tokens/pp_token_emitter.h"
#include "support/internal_error.h"

// pptoken: translation phases 1-3 of a C++ source file read from standard
// input, describing the resulting preprocessing tokens on standard output.

bool HasBatchStdinArg(int argc, char** argv)
{
	for (int i = 1; i < argc; i++)
	{
		if (string(argv[i]) == "--batch-stdin")
			return true;
	}
	return false;
}

// Batch requests are served by the linked test runner; a build without it
// cannot run them.
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

		SourceBuffer source = SourceBuffer::ReadFrom(cin);
		DebugPPTokenStream output;
		try
		{
			EmitPPTokens(source, output);
		}
		catch (const SourceError& e)
		{
			cout.flush();
			cerr << "ERROR: <stdin>:" << FormatSourceError(source, e) << endl;
			return EXIT_FAILURE;
		}
		return EXIT_SUCCESS;
	}
	catch (const InternalError& e)
	{
		cerr << "internal compiler error: " << e.what() << endl;
		return CPPGM_EXIT_INTERNAL_ERROR;
	}
	catch (const exception& e)
	{
		cerr << "ERROR: " << e.what() << endl;
		return EXIT_FAILURE;
	}
}
