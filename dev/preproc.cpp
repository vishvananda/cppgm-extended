// (C) 2013 CPPGM Foundation www.cppgm.org.  All rights reserved.
// Revised for cppgm-extended; see NOTICE.

#include <ctime>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

using namespace std;

#include "lex/post_token_writer.h"
#include "lex/post_tokenizer.h"
#include "preprocess/preprocessor.h"
#include "support/internal_error.h"
#include "support/telemetry.h"
#include "support/tool_errors.h"

// preproc: translation phases 1-6 and the tokenization part of phase 7 of
// each primary source file, written as `sof`/PA2 post-token/`eof` records.
// Each source file is a translation unit with its own preprocessing state.
// The host file identity that the scaffold's GetPreprocessorFileId gives
// #pragma once is GetFileIdentity in preprocess/source/source_files.

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

// __DATE__ and __TIME__ from one asctime call at the start of main
// (directives.md): "Www Mmm dd hh:mm:ss yyyy\n".
void SetBuildTime(PreprocessorOptions& options)
{
	const time_t now = time(nullptr);
	const string stamp = asctime(localtime(&now));
	options.date = stamp.substr(4, 7) + stamp.substr(20, 4);
	options.time = stamp.substr(11, 8);
}

// `--stats` (before -o) reports the work counters of each translation unit
// on standard error, also when preprocessing fails.
void ReportCounters(const string& path, const PreprocessorCounters& counters)
{
#if CPPGM_TELEMETRY_ENABLED
	cerr << "preproc stats " << path << ": source_tokens=" << counters.source_tokens
		 << " directives=" << counters.directives << " skipped_directives=" << counters.skipped_directives
		 << " includes=" << counters.includes << " guarded_skips=" << counters.guarded_skips
		 << " files=" << counters.files << " source_bytes=" << counters.source_bytes
		 << " identifiers=" << counters.identifiers << " arena_bytes=" << counters.arena_bytes
		 << " invocations=" << counters.expansion.invocations
		 << " produced_tokens=" << counters.expansion.produced_tokens
		 << " argument_tokens=" << counters.expansion.argument_tokens
		 << " argument_expansions=" << counters.expansion.argument_expansions
		 << " nested_collections=" << counters.expansion.nested_collections << " pastes=" << counters.expansion.pastes
		 << " stringizes=" << counters.expansion.stringizes
		 << " raw_operands_hidden=" << counters.expansion.raw_operands_hidden
		 << " peak_contexts=" << counters.expansion.peak_contexts
		 << " peak_context_tokens=" << counters.expansion.peak_context_tokens
		 << " peak_invocations=" << counters.expansion.peak_invocations << endl;
#endif
}

// Preprocesses one source file into the writer; false after an error.
bool PreprocessSource(const PreprocessorOptions& options, const string& path, PostTokenWriter& output, bool stats)
{
	output.write_line("sof " + path);
	try
	{
		Preprocessor preprocessor(options, path);
		try
		{
			PostTokenizer tokens(preprocessor);
			for (;;)
			{
				const PostToken& token = tokens.next();
				if (token.kind == PostTokenKind::Invalid)
				{
					cerr << "ERROR: " << path << ": invalid phase-7 token" << endl;
					if (stats)
						ReportCounters(path, preprocessor.counters());
					return false;
				}
				output.write(token);
				if (token.kind == PostTokenKind::EndOfFile)
					break;
			}
		}
		catch (const PreprocessError& e)
		{
			cerr << "ERROR: " << preprocessor.describe(e) << endl;
			if (stats)
				ReportCounters(path, preprocessor.counters());
			return false;
		}
		catch (const SourceError& e)
		{
			cerr << "ERROR: " << preprocessor.describe(e) << endl;
			if (stats)
				ReportCounters(path, preprocessor.counters());
			return false;
		}
		if (stats)
			ReportCounters(path, preprocessor.counters());
	}
	catch (const PreprocessError& e)
	{
		cerr << "ERROR: " << path << ": " << e.what() << endl;
		return false;
	}
	return true;
}

int main(int argc, char** argv)
{
	try
	{
		if (HasBatchStdinArg(argc, argv))
			return RunNotImplementedBatchMode();

		PreprocessorOptions options;
		SetBuildTime(options);
		options.author = "Course Solution";

		vector<string> args;
		for (int i = 1; i < argc; i++)
			args.emplace_back(argv[i]);
		const bool stats = !args.empty() && args[0] == "--stats";
		if (stats && !telemetry::kEnabled)
			throw InvocationError(telemetry::kUnavailable);
		if (stats)
			args.erase(args.begin());

		if (args.size() < 3 || args[0] != "-o")
			throw InvocationError("invalid usage");

		const string outfile = args[1];
		const size_t nsrcfiles = args.size() - 2;
		ofstream out(outfile);
		PostTokenWriter output(out);
		output.write_line("preproc " + to_string(nsrcfiles));
		for (size_t i = 0; i < nsrcfiles; i++)
		{
			if (!PreprocessSource(options, args[i + 2], output, stats))
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
