// Student-facing scaffold for the PA9 `abimangle` binary.
//
// abi/itanium/abi_mangle.h gives the vocabulary: the Itanium codes and the
// fact-file words as enums.  The typed ABI model, the fact reader that
// builds it, the substitution dictionary and the encoder are yours; keep
// the model and encoder under dev/src/ so the compiler (PA10 on) builds
// typed targets and calls the same encoder without writing fact text.

#include "abi/itanium/abi_mangle.h"
#include "support/not_implemented.h"
#include "support/tool_help_text.h"

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

using namespace std;

namespace {

// Reads the fact files in order and returns one mangled name per case,
// each on its own line.
string mangle_fact_files(const vector<string>& input_paths)
{
	(void)input_paths;
	throw NotImplementedException();
}

struct AbimangleInvocation
{
	string outfile;
	vector<string> inputs;
};

bool has_help_arg(int argc, char** argv)
{
	for (int i = 1; i < argc; ++i)
	{
		const string arg = argv[i];
		if (arg == "--help" || arg == "-h")
		{
			return true;
		}
	}
	return false;
}

void print_help()
{
	cout << abimangle_help_text();
}

AbimangleInvocation parse_invocation(int argc, char** argv)
{
	AbimangleInvocation invocation;
	for (int i = 1; i < argc; ++i)
	{
		const string arg = argv[i];
		if (arg == "-o")
		{
			if (i + 1 >= argc)
			{
				throw logic_error("missing output file after -o");
			}
			invocation.outfile = argv[++i];
			continue;
		}
		invocation.inputs.push_back(arg);
	}
	if (invocation.outfile.empty() || invocation.inputs.empty())
	{
		throw logic_error("invalid usage");
	}
	return invocation;
}

int run_abimangle(int argc, char** argv)
{
	if (has_help_arg(argc, argv))
	{
		print_help();
		return EXIT_SUCCESS;
	}
	const AbimangleInvocation invocation = parse_invocation(argc, argv);
	ofstream out(invocation.outfile.c_str());
	if (!out)
	{
		throw logic_error("unable to open output file '" + invocation.outfile + "'");
	}
	out << mangle_fact_files(invocation.inputs);
	return EXIT_SUCCESS;
}

} // namespace

int main(int argc, char** argv)
{
	try
	{
		return run_abimangle(argc, argv);
	}
	catch (const NotImplementedException& e)
	{
		cerr << "ERROR: " << e.what() << endl;
		return CPPGM_EXIT_NOT_IMPLEMENTED;
	}
	catch (const exception& e)
	{
		cerr << "abimangle: " << e.what() << "\n";
		return EXIT_FAILURE;
	}
}
