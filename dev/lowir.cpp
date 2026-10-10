// lowir: reads, validates and writes LowIR units, and builds the PA8
// construction exercises through the same model and writer.

#include "lowir/exercises.h"
#include "lowir/program.h"
#include "lowir/text_reader.h"
#include "lowir/text_writer.h"
#include "lowir/validator.h"
#include "support/internal_error.h"
#include "support/telemetry.h"
#include "support/tool_errors.h"
#include "support/tool_usage.h"

#include <sys/resource.h>

#include <chrono>
#include <cstdio>
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
	string outfile;
	vector<string> inputs;
	string exercise;
	bool stats = false;
};

Invocation parse_invocation(const vector<string>& args)
{
	Invocation invocation;
	for (size_t i = 0; i < args.size(); ++i)
	{
		const string option = args[i];
		if (option == "-o" || option == "--exercise")
		{
			if (++i == args.size())
				throw InvocationError("missing value after " + option);
			string& value = option == "-o" ? invocation.outfile : invocation.exercise;
			if (!value.empty())
				throw InvocationError("repeated option: " + option);
			value = args[i];
		}
		else if (option == "--stats")
		{
			if (!telemetry::kEnabled)
				throw InvocationError(telemetry::kUnavailable);
			invocation.stats = true;
		}
		else if (!option.empty() && option[0] == '-')
			throw InvocationError("unknown option: " + option);
		else
			invocation.inputs.push_back(option);
	}
	if (invocation.outfile.empty() || invocation.inputs.empty() == invocation.exercise.empty())
		throw InvocationError("expected input files or one exercise");
	return invocation;
}

double milliseconds_since(chrono::steady_clock::time_point start)
{
	return chrono::duration<double, milli>(chrono::steady_clock::now() - start).count();
}

// `--stats` (spec section 9): one line of model sizes, phase times and the
// process's peak RSS, on standard error. The counts are read from the
// finished model; nothing is computed for the report alone.
void report_stats(const lowir::Program& program, const lowir::ReadStatistics& read, double build_ms, double write_ms)
{
#if CPPGM_TELEMETRY_ENABLED
	size_t blocks = 0, instructions = 0, temps = 0, literals = 0, operands = 0;
	for (const lowir::Function& function : program.functions)
	{
		if (function.body == nullptr)
			continue;
		blocks += function.body->blocks.size();
		instructions += function.body->instructions.size();
		temps += function.body->temps.size();
		literals += function.body->literals.size();
		operands += function.body->operands.size();
	}
	struct rusage usage;
	getrusage(RUSAGE_SELF, &usage);
	cerr << "lowir stats: source_bytes=" << read.source_bytes << " symbols=" << program.symbols.size()
		 << " functions=" << program.functions.size() << " globals=" << program.globals.size()
		 << " data_items=" << program.data_items.size() << " signatures=" << program.signatures.size()
		 << " blocks=" << blocks << " instructions=" << instructions << " temps=" << temps << " literals=" << literals
		 << " extra_operands=" << operands << " names=" << program.names.size()
		 << " name_arena_bytes=" << program.arena.bytes_reserved() << " parse_ms=" << read.parse_ms
		 << " validate_ms=" << read.validate_ms << " build_ms=" << build_ms << " write_ms=" << write_ms
		 << " peak_rss_kb=" << usage.ru_maxrss << endl;
#endif
}

int run_lowir_mode(const vector<string>& args)
{
	if (has_arg(args, "--batch-stdin", nullptr))
		return run_not_implemented_batch_mode();
	if (has_arg(args, "--help", "-h"))
	{
		cout << lowir_help_text();
		return EXIT_SUCCESS;
	}
	const Invocation invocation = parse_invocation(args);
	lowir::Program program;
	lowir::ReadStatistics read;
	double build_ms = 0;
	if (!invocation.exercise.empty())
	{
		const chrono::steady_clock::time_point start = chrono::steady_clock::now();
		if (!lowir::build_exercise(program, invocation.exercise))
			throw InvocationError("unknown LowIR exercise: " + invocation.exercise);
		// Audit the constructed program before writing it.
		lowir::Diagnostic diagnostic;
		if (!lowir::validate_constructed_program(program, diagnostic))
			throw InternalError("invalid exercise program: " + lowir::describe(program, diagnostic));
		build_ms = milliseconds_since(start);
	}
	else
	{
		string error;
		if (!lowir::read_program_files(program, invocation.inputs, error, &read))
		{
			cerr << "ERROR: " << error << endl;
			return EXIT_FAILURE;
		}
	}
	const chrono::steady_clock::time_point start = chrono::steady_clock::now();
	if (!lowir::write_program_file(program, invocation.outfile))
		throw InputOutputError("cannot write " + invocation.outfile);
	if (invocation.stats)
		report_stats(program, read, build_ms, milliseconds_since(start));
	return EXIT_SUCCESS;
}

} // namespace

int main(int argc, char** argv)
{
	try
	{
		return run_lowir_mode(collect_args(argc, argv));
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
