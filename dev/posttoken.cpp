// (C) 2013 CPPGM Foundation www.cppgm.org.  All rights reserved.
// Revised for cppgm-extended; see NOTICE.

#include <iostream>
#include <stdexcept>
#include <string>

using namespace std;

#include "lex/post_token_writer.h"
#include "lex/post_tokenizer.h"
#include "preprocess/lexer/pp_lexer.h"
#include "preprocess/source/source_buffer.h"
#include "support/internal_error.h"

// posttoken: translation phases 1-6 and the tokenization part of phase 7 of
// a C++ source file read from standard input, describing the resulting
// tokens on standard output.

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
		PPLexer lexer(source);
		PostTokenizer tokens(lexer);
		PostTokenWriter output(cout);
		try
		{
			for (;;)
			{
				const PostToken& token = tokens.next();
				output.write(token);
				if (token.kind == PostTokenKind::EndOfFile)
					break;
			}
		}
		catch (const SourceError& e)
		{
			// Phases 1-3 failed; the tokens before the error are kept.
			output.flush();
			cerr << "ERROR: <stdin>:" << FormatSourceError(source, e) << endl;
			return EXIT_FAILURE;
		}
		output.flush();
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
