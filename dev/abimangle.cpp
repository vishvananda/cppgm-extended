// abimangle: reads normalized ABI fact files and writes one Itanium mangled
// name per case through the shared typed ABI graph and encoder.

#include "abi/facts/abi_fact_reader.h"
#include "abi/itanium/abi_encoder.h"
#include "support/file_handle.h"
#include "support/internal_error.h"
#include "support/telemetry.h"
#include "support/tool_errors.h"
#include "support/tool_help_text.h"

#include <sys/resource.h>

#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

using namespace std;

namespace {

struct Invocation
{
	string outfile;
	vector<string> inputs;
	bool stats = false;
};

bool has_help_arg(int argc, char** argv)
{
	for (int i = 1; i < argc; ++i)
	{
		const string arg = argv[i];
		if (arg == "--help" || arg == "-h")
			return true;
	}
	return false;
}

Invocation parse_invocation(int argc, char** argv)
{
	Invocation invocation;
	for (int i = 1; i < argc; ++i)
	{
		const string arg = argv[i];
		if (arg == "-o")
		{
			if (i + 1 >= argc)
				throw InvocationError("missing output file after -o");
			if (!invocation.outfile.empty())
				throw InvocationError("repeated option: -o");
			invocation.outfile = argv[++i];
		}
		else if (arg == "--stats")
		{
			if (!telemetry::kEnabled)
				throw InvocationError(telemetry::kUnavailable);
			invocation.stats = true;
		}
		else if (arg.size() > 1 && arg[0] == '-')
			throw InvocationError("unknown option: " + arg);
		else
			invocation.inputs.push_back(arg);
	}
	if (invocation.outfile.empty() || invocation.inputs.empty())
		throw InvocationError("invalid usage");
	return invocation;
}

// `--stats` (spec section 9): work counters, graph size, phase time and
// peak RSS on standard error, read from state the run already keeps.
void report_stats(const abi_mangle::AbiMangleStats& stats, double mangle_ms)
{
#if CPPGM_TELEMETRY_ENABLED
	struct rusage usage;
	getrusage(RUSAGE_SELF, &usage);
	cerr << "abimangle stats: source_files=" << stats.source_files << " source_bytes=" << stats.source_bytes
		 << " cases=" << stats.cases << " records=" << stats.records << " definitions=" << stats.definitions
		 << " peak_graph_nodes=" << stats.peak_graph_nodes << " peak_graph_bytes=" << stats.peak_graph_bytes
		 << " substitution_lookups=" << stats.substitution_lookups << " substitution_hits=" << stats.substitution_hits
		 << " substitution_entries=" << stats.substitution_entries << " entity_references=" << stats.entity_references
		 << " output_bytes=" << stats.output_bytes << " mangle_ms=" << mangle_ms << " peak_rss_kb=" << usage.ru_maxrss
		 << endl;
#endif
}

int run_abimangle(int argc, char** argv)
{
	if (has_help_arg(argc, argv))
	{
		cout << abimangle_help_text();
		return EXIT_SUCCESS;
	}
	const Invocation invocation = parse_invocation(argc, argv);
	const auto start = chrono::steady_clock::now();
	abi_mangle::AbiMangleStats stats;
	FileHandle out(fopen(invocation.outfile.c_str(), "wb"));
	if (out.get() == nullptr)
		throw InputOutputError("unable to open output file '" + invocation.outfile + "'");
	abi_facts::mangle_fact_files(invocation.inputs, out.get(), &stats);
	if (!out.close())
		throw InputOutputError("unable to write output file '" + invocation.outfile + "'");
	const double mangle_ms = chrono::duration<double, milli>(chrono::steady_clock::now() - start).count();
	if (invocation.stats)
		report_stats(stats, mangle_ms);
	return EXIT_SUCCESS;
}

} // namespace

int main(int argc, char** argv)
{
	try
	{
		return run_abimangle(argc, argv);
	}
	catch (const InternalError& e)
	{
		cerr << "internal compiler error: " << e.what() << endl;
		return CPPGM_EXIT_INTERNAL_ERROR;
	}
	catch (const exception& e)
	{
		cerr << "abimangle: " << e.what() << "\n";
		return EXIT_FAILURE;
	}
}
