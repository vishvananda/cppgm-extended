// lowiropt: reads LowIR units, runs the optimization pipeline of one
// level (opt/optimizer.h) and writes the result as canonical LowIR text.

#include "lowir/program.h"
#include "lowir/text_reader.h"
#include "lowir/text_writer.h"
#include "lowir/validator.h"
#include "opt/optimizer.h"
#include "support/internal_error.h"
#include "support/telemetry.h"
#include "support/tool_errors.h"
#include "support/tool_help_text.h"

#include <sys/resource.h>

#include <chrono>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

using namespace std;

namespace {

vector<string> collect_args(int argc, char** argv)
{
	vector<string> args;
	for (int i = 1; i < argc; ++i)
		args.push_back(argv[i]);
	return args;
}

bool has_arg(const vector<string>& args, const char* first, const char* second)
{
	for (size_t i = 0; i < args.size(); ++i)
	{
		if (args[i] == first || (second != nullptr && args[i] == second))
			return true;
	}
	return false;
}

// Batch requests are served by the linked test runner; a build without it
// cannot run them.
int run_not_implemented_batch_mode()
{
	string line;
	while (getline(cin, line))
		cout << "EXIT_NOT_IMPLEMENTED" << endl;
	return EXIT_SUCCESS;
}

struct Invocation
{
	bool has_level = false;
	opt::OptimizerOptions options;
	string outfile;
	vector<string> inputs;
	bool stats = false;
	// --validate (audit builds): the optimized program is checked before
	// it is written; --verify-passes checks it after every pass
	// (OptimizerOptions::verify_passes).
	bool validate = false;
};

Invocation parse_invocation(const vector<string>& args)
{
	Invocation invocation;
	for (size_t i = 0; i < args.size(); ++i)
	{
		const string& option = args[i];
		if (option.size() == 3 && option[0] == '-' && option[1] == 'O' && option[2] >= '0' && option[2] <= '3')
		{
			if (invocation.has_level)
				throw InvocationError("multiple optimization levels provided");
			invocation.has_level = true;
			invocation.options.level = option[2] - '0';
		}
		else if (option == "-o")
		{
			if (++i == args.size())
				throw InvocationError("missing output file after -o");
			if (!invocation.outfile.empty())
				throw InvocationError("multiple output files provided");
			invocation.outfile = args[i];
		}
		else if (option == "--inline-limit")
		{
			if (++i == args.size() || !opt::parse_inline_limit(args[i], invocation.options.limits))
				throw InvocationError("invalid --inline-limit");
		}
		else if (option == "--stats")
		{
			if (!telemetry::kEnabled)
				throw InvocationError(telemetry::kUnavailable);
			invocation.stats = true;
		}
		else if (option == "--validate")
			invocation.validate = true;
		else if (option == "--verify-passes")
			invocation.options.verify_passes = true;
		else if (option.compare(0, 16, "--verify-passes=") == 0)
		{
			invocation.options.verify_passes = true;
			invocation.options.verify_dump = option.substr(16);
		}
		else if (!option.empty() && option[0] == '-')
			throw InvocationError("unknown option: " + option);
		else
			invocation.inputs.push_back(option);
	}
	if (!invocation.has_level || invocation.outfile.empty() || invocation.inputs.empty())
		throw InvocationError("usage: lowiropt -O<0-3> -o <outfile> <lowirfile>...");
	return invocation;
}

#if CPPGM_TELEMETRY_ENABLED
double milliseconds_since(chrono::steady_clock::time_point start)
{
	return chrono::duration<double, milli>(chrono::steady_clock::now() - start).count();
}
#endif

int run_lowiropt_mode(const vector<string>& args)
{
	if (has_arg(args, "--batch-stdin", nullptr))
		return run_not_implemented_batch_mode();
	if (has_arg(args, "--help", "-h"))
	{
		cout << lowiropt_help_text();
		return EXIT_SUCCESS;
	}
	const Invocation invocation = parse_invocation(args);
	lowir::Program program;
	lowir::ReadStatistics read;
	string error;
	if (!lowir::read_program_files(program, invocation.inputs, error, &read))
	{
		cerr << "ERROR: " << error << endl;
		return EXIT_FAILURE;
	}
	opt::OptimizerStats stats;
	opt::optimize_program(program, invocation.options, &stats);
	lowir::Diagnostic diagnostic;
	if (invocation.validate && !lowir::validate_constructed_program(program, diagnostic))
		throw InternalError("optimized LowIR is invalid: " + lowir::describe(program, diagnostic));
#if CPPGM_TELEMETRY_ENABLED
	const chrono::steady_clock::time_point start = chrono::steady_clock::now();
#endif
	if (!lowir::write_program_file(program, invocation.outfile))
	{
		cerr << "ERROR: cannot write " << invocation.outfile << endl;
		return EXIT_FAILURE;
	}
#if CPPGM_TELEMETRY_ENABLED
	if (invocation.stats)
	{
		struct rusage usage;
		getrusage(RUSAGE_SELF, &usage);
		cerr << opt::describe_stats(stats) << " parse_ms=" << read.parse_ms << " validate_ms=" << read.validate_ms
			 << " write_ms=" << milliseconds_since(start) << " peak_rss_kb=" << usage.ru_maxrss << endl;
	}
#endif
	return EXIT_SUCCESS;
}

} // namespace

int main(int argc, char** argv)
{
	try
	{
		return run_lowiropt_mode(collect_args(argc, argv));
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
