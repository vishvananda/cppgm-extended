// Student-facing scaffold for the PA5+ `cppgm++` binary.

#include "driver/host_environment.h"
#include "driver/toolchain.h"
#include "lower/program_lowering.h"
#include "lowir/text_writer.h"
#include "lowir/validator.h"
#include "opt/optimizer.h"
#include "sema/analyzer.h"
#include "sema/semantic_dump.h"
#include "sema/semantics_dump.h"
#include "support/internal_error.h"
#include "support/telemetry.h"
#include "support/tool_errors.h"
#include "support/tool_usage.h"
#include "syntax/ast_printer.h"
#include "syntax/syntax_unit.h"

#include <chrono>
#include <cstdlib>
#include <ctime>
#include <sys/resource.h>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

using namespace std;

namespace {

enum class EmitMode
{
	None,
	Ast,
	Types,
	Semantics,
	LowIR,
};

enum class DriverMode
{
	Query,
	Preprocess,
	Compile,
	Link,
};

struct DriverInvocation
{
	DriverMode mode;

	DriverInvocation() : mode(DriverMode::Link) {}
};

vector<string> collect_args(int argc, char** argv)
{
	vector<string> args;
	for (int i = 1; i < argc; ++i)
	{
		args.push_back(argv[i]);
	}
	return args;
}

bool has_arg(const vector<string>& args, const string& needle)
{
	for (size_t i = 0; i < args.size(); ++i)
	{
		if (args[i] == needle)
		{
			return true;
		}
	}
	return false;
}

bool has_help_arg(const vector<string>& args)
{
	return has_arg(args, "--help") || has_arg(args, "-h");
}

bool starts_with(const string& value, const string& prefix)
{
	return value.size() >= prefix.size() && value.compare(0, prefix.size(), prefix) == 0;
}

bool ends_with(const string& value, const string& suffix)
{
	return value.size() >= suffix.size() && value.compare(value.size() - suffix.size(), suffix.size(), suffix) == 0;
}

bool is_query_driver_flag(const string& arg)
{
	return arg == "--version" || arg == "-v" || arg == "-dumpmachine" || arg == "-dumpversion" ||
		arg == "-print-search-dirs";
}

bool is_optimization_flag(const string& arg)
{
	return starts_with(arg, "-O");
}

bool is_debug_info_flag(const string& arg)
{
	return arg == "-g0" || arg == "-gline-tables-only" || arg == "-g" || starts_with(arg, "-g");
}

// What a debug-information flag asks for, as GCC and Clang read it: none
// (-g0, 0), some (-g, -g1..3, -ggdb, -gdwarf-N, -gline-tables-only, 1), or
// the level unchanged (a modifier: -gcolumn-info, -gno-*, -gz, -1).
int debug_info_request(const string& arg)
{
	if (arg == "-g0" || arg == "-ggdb0")
		return 0;
	if (starts_with(arg, "-gno-") || starts_with(arg, "-gz") || arg == "-gcolumn-info" || arg == "-gpubnames" ||
		arg == "-gsplit-dwarf" || starts_with(arg, "-gstrict-dwarf") || arg == "-grecord-gcc-switches")
		return -1;
	return 1;
}

bool is_benign_driver_flag(const string& arg)
{
	return arg == "-Wall" || arg == "-Winvalid-offsetof" || arg == "-pipe" || arg == "-w" || arg == "-pg" ||
		arg == "-pedantic" || arg == "-pedantic-errors" || starts_with(arg, "-W") || starts_with(arg, "-f") ||
		starts_with(arg, "-m") || starts_with(arg, "-std=");
}

void consume_required_option_argument(
	const vector<string>& args, size_t& i, const string& option, const string& expected)
{
	if (i + 1 >= args.size())
	{
		throw InvocationError("missing " + expected + " after " + option);
	}
	++i;
}

bool consume_joined_or_separate_option(
	const vector<string>& args, size_t& i, const string& option, const string& expected)
{
	if (args[i] == option)
	{
		consume_required_option_argument(args, i, option, expected);
		return true;
	}
	if (starts_with(args[i], option) && args[i].size() > option.size())
	{
		return true;
	}
	return false;
}

int run_not_implemented_batch_mode()
{
	string line;
	while (getline(cin, line))
	{
		(void)line;
		cout << "EXIT_NOT_IMPLEMENTED" << endl;
	}
	return EXIT_SUCCESS;
}

void consume_emit_flag(vector<string>& args, const string& flag, EmitMode value, EmitMode& out)
{
	vector<string> kept;
	bool found = false;
	for (size_t i = 0; i < args.size(); ++i)
	{
		if (args[i] == flag)
		{
			found = true;
			continue;
		}
		kept.push_back(args[i]);
	}

	if (!found)
	{
		return;
	}

	if (out != EmitMode::None)
	{
		throw InvocationError("multiple --emit-* options provided");
	}
	out = value;
	args.swap(kept);
}

EmitMode parse_emit_mode(vector<string>& args)
{
	EmitMode mode = EmitMode::None;
	consume_emit_flag(args, "--emit-ast", EmitMode::Ast, mode);
	consume_emit_flag(args, "--emit-types", EmitMode::Types, mode);
	consume_emit_flag(args, "--emit-semantics", EmitMode::Semantics, mode);
	consume_emit_flag(args, "--emit-lowir", EmitMode::LowIR, mode);
	return mode;
}

struct SourceOutputInvocation
{
	string outfile;
	vector<string> inputs;
	// `--hosted`: the host environment -c uses (include paths, predefined
	// macros, builtin declarations, hosted analysis; --emit-lowir also lowers
	// in host-object mode), so a hosted source's views are those of the
	// program -c compiles. `-I`, `-D` and `-U` as the driver takes them.
	bool hosted = false;
	vector<string> include_paths;
	string command_line;
	// --emit-lowir -O<n>: the optimizer's level and limits.
	opt::OptimizerOptions optimizer;
	// A debug-information level (-g0, -gline-tables-only, ...) was stated,
	// and the last asks for debug information.
	bool debug_level = false;
	bool debug_info = false;
};

// -O<level> as GCC reads it (see driver/toolchain.cpp); -1 for another
// option.
int optimization_level(const string& arg)
{
	if (!starts_with(arg, "-O"))
	{
		return -1;
	}
	const string level = arg.substr(2);
	if (level.empty() || level == "g")
	{
		return 1;
	}
	if (level == "s" || level == "z")
	{
		return 2;
	}
	if (level == "fast")
	{
		return 3;
	}
	for (size_t i = 0; i < level.size(); ++i)
	{
		if (level[i] < '0' || level[i] > '9')
		{
			throw InvocationError("unsupported option: " + arg);
		}
	}
	return level.size() > 1 || level[0] > '3' ? 3 : level[0] - '0';
}

SourceOutputInvocation parse_source_output_invocation(const vector<string>& args, bool allow_lowir_options)
{
	bool explicit_outfile = false;
	SourceOutputInvocation invocation;
	vector<string>& inputs = invocation.inputs;

	for (size_t i = 0; i < args.size(); ++i)
	{
		if (args[i] == "-o")
		{
			consume_required_option_argument(args, i, "-o", "output file");
			invocation.outfile = args[i];
			explicit_outfile = true;
			continue;
		}
		if (args[i] == "--hosted")
		{
			invocation.hosted = true;
			continue;
		}
		const size_t option = i;
		if (consume_joined_or_separate_option(args, i, "-I", "path"))
		{
			invocation.include_paths.push_back(i == option ? args[i].substr(2) : args[i]);
			continue;
		}
		if (consume_joined_or_separate_option(args, i, "-D", "macro definition"))
		{
			const string value = i == option ? args[i].substr(2) : args[i];
			const size_t equals = value.find('=');
			invocation.command_line += "#define " +
				(equals == string::npos ? value + " 1" : value.substr(0, equals) + " " + value.substr(equals + 1)) +
				"\n";
			continue;
		}
		if (consume_joined_or_separate_option(args, i, "-U", "macro name"))
		{
			invocation.command_line += "#undef " + (i == option ? args[i].substr(2) : args[i]) + "\n";
			continue;
		}
		if (allow_lowir_options && is_optimization_flag(args[i]))
		{
			invocation.optimizer.level = optimization_level(args[i]);
			continue;
		}
		if (allow_lowir_options && args[i] == "--inline-limit")
		{
			consume_required_option_argument(args, i, "--inline-limit", "limit");
			if (!opt::parse_inline_limit(args[i], invocation.optimizer.limits))
			{
				throw InvocationError("invalid --inline-limit: " + args[i]);
			}
			continue;
		}
		if (allow_lowir_options && is_debug_info_flag(args[i]))
		{
			invocation.debug_level = true;
			const int request = debug_info_request(args[i]);
			if (request >= 0)
				invocation.debug_info = request != 0;
			continue;
		}
		if (allow_lowir_options && (args[i] == "--witness" || args[i] == "--witness-debug"))
		{
			consume_required_option_argument(args, i, args[i], "output file");
			continue;
		}
		if (args[i] == "-c" || args[i] == "-E" || is_query_driver_flag(args[i]))
		{
			throw InvocationError("invalid usage");
		}
		if (starts_with(args[i], "-"))
		{
			throw InvocationError("unsupported option in emit mode: " + args[i]);
		}
		inputs.push_back(args[i]);
	}

	if (!explicit_outfile || inputs.empty())
	{
		throw InvocationError("invalid usage");
	}
	// GCC publishes __OPTIMIZE__ at -O1 and above (driver/toolchain.cpp).
	if (invocation.optimizer.level > 0)
	{
		invocation.command_line.insert(0, "#define __OPTIMIZE__ 1\n");
	}
	return invocation;
}

bool consume_preprocess_option(const vector<string>& args, size_t& i)
{
	if (consume_joined_or_separate_option(args, i, "-D", "macro definition"))
	{
		return true;
	}
	if (consume_joined_or_separate_option(args, i, "-U", "macro name"))
	{
		return true;
	}
	if (args[i] == "-include")
	{
		consume_required_option_argument(args, i, "-include", "file");
		return true;
	}
	if (args[i] == "-nostdinc")
	{
		return true;
	}
	if (args[i] == "-x")
	{
		consume_required_option_argument(args, i, "-x", "language");
		return true;
	}
	return false;
}

bool consume_search_option(const vector<string>& args, size_t& i)
{
	if (consume_joined_or_separate_option(args, i, "-I", "path"))
	{
		return true;
	}
	if (consume_joined_or_separate_option(args, i, "-isystem", "path"))
	{
		return true;
	}
	if (consume_joined_or_separate_option(args, i, "-L", "path"))
	{
		return true;
	}
	if (consume_joined_or_separate_option(args, i, "-l", "library name"))
	{
		return true;
	}
	return false;
}

bool consume_dependency_option(const vector<string>& args, size_t& i)
{
	if (args[i] == "-MMD" || args[i] == "-MD" || args[i] == "-MP")
	{
		return true;
	}
	if (consume_joined_or_separate_option(args, i, "-MF", "depfile path"))
	{
		return true;
	}
	if (consume_joined_or_separate_option(args, i, "-MT", "target"))
	{
		return true;
	}
	if (consume_joined_or_separate_option(args, i, "-MQ", "target"))
	{
		return true;
	}
	return false;
}

bool consume_toolchain_option(const vector<string>& args, size_t& i)
{
	if (is_debug_info_flag(args[i]))
	{
		return true;
	}
	if (is_optimization_flag(args[i]))
	{
		return true;
	}
	// The optimizer's limits (driver/toolchain.cpp parses the value).
	if (args[i] == "--inline-limit")
	{
		consume_required_option_argument(args, i, "--inline-limit", "limit");
		return true;
	}
	if (args[i] == "--target")
	{
		consume_required_option_argument(args, i, "--target", "target");
		return true;
	}
	if (starts_with(args[i], "--target="))
	{
		if (args[i].size() == string("--target=").size())
		{
			throw InvocationError("missing target after --target");
		}
		return true;
	}
	if (args[i] == "-std")
	{
		consume_required_option_argument(args, i, "-std", "language standard");
		return true;
	}
	if (args[i] == "-stdlib")
	{
		consume_required_option_argument(args, i, "-stdlib", "standard library name");
		return true;
	}
	if (starts_with(args[i], "-stdlib="))
	{
		return true;
	}
	// -pthread: _REENTRANT for the preprocessor (driver/toolchain.cpp).
	if (args[i] == "-pthread")
	{
		return true;
	}
	return false;
}

DriverInvocation parse_driver_invocation(const vector<string>& args)
{
	if (args.empty())
	{
		throw InvocationError("invalid usage");
	}

	DriverInvocation invocation;
	if (is_query_driver_flag(args[0]))
	{
		if (args.size() != 1)
		{
			throw InvocationError("query flag must be used as a direct invocation");
		}
		invocation.mode = DriverMode::Query;
		return invocation;
	}

	bool compile_only = false;
	bool preprocess_only = false;
	bool explicit_outfile = false;
	vector<string> inputs;

	for (size_t i = 0; i < args.size(); ++i)
	{
		if (is_query_driver_flag(args[i]))
		{
			throw InvocationError("query flag must be used as a direct invocation");
		}
		if (args[i] == "-c")
		{
			compile_only = true;
			continue;
		}
		if (args[i] == "--stats" || args[i] == "--stats-functions" || args[i].compare(0, 15, "--verify-passes") == 0)
		{
			continue;
		}
		if (args[i] == "-E")
		{
			preprocess_only = true;
			continue;
		}
		if (args[i] == "-o")
		{
			consume_required_option_argument(args, i, "-o", "output file");
			explicit_outfile = true;
			continue;
		}
		if (consume_preprocess_option(args, i) || consume_search_option(args, i) ||
			consume_dependency_option(args, i) || consume_toolchain_option(args, i) || is_benign_driver_flag(args[i]))
		{
			continue;
		}
		if (starts_with(args[i], "-"))
		{
			throw InvocationError("unsupported driver option: " + args[i]);
		}
		inputs.push_back(args[i]);
	}

	if (compile_only && preprocess_only)
	{
		throw InvocationError("cannot combine -c and -E");
	}
	if (inputs.empty())
	{
		throw InvocationError("invalid usage");
	}
	if ((compile_only || preprocess_only) && explicit_outfile && inputs.size() != 1)
	{
		throw InvocationError("cannot specify -o when generating multiple output files");
	}

	invocation.mode = preprocess_only ? DriverMode::Preprocess : compile_only ? DriverMode::Compile : DriverMode::Link;
	return invocation;
}

// __DATE__ and __TIME__ from one asctime call per run: "Www Mmm dd hh:mm:ss yyyy\n".
PreprocessorOptions source_preprocessor_options()
{
	PreprocessorOptions options;
	const time_t now = time(nullptr);
	const string stamp = asctime(localtime(&now));
	options.date = stamp.substr(4, 7) + stamp.substr(20, 4);
	options.time = stamp.substr(11, 8);
	options.author = "Vish Abrams";
	options.literals = NumberDialect::Gnu;
	return options;
}

PreprocessorOptions source_preprocessor_options(const SourceOutputInvocation& invocation)
{
	PreprocessorOptions options = source_preprocessor_options();
	options.include_paths = invocation.include_paths;
	options.command_line = invocation.command_line;
	if (invocation.hosted)
		driver::add_host_environment(options, false);
	return options;
}

double milliseconds_since(chrono::steady_clock::time_point start)
{
	return chrono::duration<double, milli>(chrono::steady_clock::now() - start).count();
}

// `--stats` (spec section 9): one line of work counters, phase times and
// the process's peak RSS so far per translation unit, on standard error.
void report_ast_stats(const string& path, const syntax::SyntaxUnit& unit, double parse_ms, double print_ms)
{
#if CPPGM_TELEMETRY_ENABLED
	const syntax::ParserCounters parser = unit.counters();
	const PreprocessorCounters preprocessor = unit.preprocessor_counters();
	struct rusage usage;
	getrusage(RUSAGE_SELF, &usage);
	cerr << "cppgm++ stats " << path << ": source_bytes=" << preprocessor.source_bytes
		 << " pp_tokens=" << preprocessor.source_tokens << " invocations=" << preprocessor.expansion.invocations
		 << " tokens=" << parser.tokens << " peak_window=" << parser.peak_window
		 << " checkpoints=" << parser.checkpoints << " rewinds=" << parser.rewinds
		 << " deferred_bodies=" << parser.deferred_bodies << " deferred_tokens=" << parser.deferred_tokens
		 << " deferred_lambdas=" << parser.deferred_lambdas << " peak_run_bytes=" << parser.peak_run_bytes
		 << " lookups=" << parser.lookups << " bindings=" << parser.bindings
		 << " kept_searches=" << parser.kept_searches << " peak_nesting=" << parser.peak_nesting
		 << " syntax_arena_bytes=" << parser.arena_bytes << " spelling_bytes=" << unit.spelling_bytes()
		 << " pp_arena_bytes=" << preprocessor.arena_bytes << " parse_ms=" << parse_ms << " print_ms=" << print_ms
		 << " peak_rss_kb=" << usage.ru_maxrss << endl;
#endif
}

// --emit-ast: each source file is a translation unit, parsed and dumped in
// command-line order. Any preprocessing, lexical or syntax error
// fails the whole run.
int run_emit_ast_mode(vector<string> args)
{
	bool stats = false;
	for (size_t i = 0; i < args.size(); ++i)
	{
		if (args[i] == "--stats")
		{
			stats = true;
			args.erase(args.begin() + i);
			break;
		}
	}
	const SourceOutputInvocation invocation = parse_source_output_invocation(args, false);
	const PreprocessorOptions options = source_preprocessor_options(invocation);
	// The output file's contents are unspecified after a failure.
	ofstream out(invocation.outfile, ios::binary);
	out << invocation.inputs.size() << " translation units\n";
	for (size_t i = 0; i < invocation.inputs.size(); ++i)
	{
		try
		{
			const chrono::steady_clock::time_point parse_start = chrono::steady_clock::now();
			syntax::SyntaxUnit unit(options, invocation.inputs[i]);
			if (!unit.parse())
			{
				cerr << "ERROR: " << unit.error() << endl;
				if (stats)
					report_ast_stats(invocation.inputs[i], unit, milliseconds_since(parse_start), 0);
				return EXIT_FAILURE;
			}
			const double parse_ms = milliseconds_since(parse_start);
			const chrono::steady_clock::time_point print_start = chrono::steady_clock::now();
			out << "start translation unit " << (i + 1) << "\n";
			syntax::PrintTranslationUnit(unit.tree(), out);
			out << "end translation unit\n";
			if (stats)
				report_ast_stats(invocation.inputs[i], unit, parse_ms, milliseconds_since(print_start));
		}
		catch (const PreprocessError& e)
		{
			cerr << "ERROR: " << invocation.inputs[i] << ": " << e.what() << endl;
			return EXIT_FAILURE;
		}
	}
	out.flush();
	return out ? EXIT_SUCCESS : EXIT_FAILURE;
}

bool consume_stats_flag(vector<string>& args)
{
	for (size_t i = 0; i < args.size(); ++i)
	{
		if (args[i] == "--stats")
		{
			args.erase(args.begin() + i);
			return true;
		}
	}
	return false;
}

// `--stats` for --emit-types: the semantic graph's work counters, phase
// times and peak RSS per translation unit, on standard error.
void report_types_stats(const string& path, const syntax::SyntaxUnit& unit, const sema::SemanticGraph& graph,
	double parse_ms, double analyze_ms, double print_ms)
{
#if CPPGM_TELEMETRY_ENABLED
	struct rusage usage;
	getrusage(RUSAGE_SELF, &usage);
	cerr << "cppgm++ stats " << path << ": tokens=" << unit.counters().tokens << " scopes=" << graph.scopes
		 << " bindings=" << graph.bindings << " entities=" << graph.entities << " types=" << graph.types.size()
		 << " names=" << graph.names.names() << " lookups=" << graph.names.lookups()
		 << " member_cache=" << graph.names.cached_member_lookups()
		 << " parser_kept_searches=" << unit.counters().kept_searches
		 << " constant_evaluations=" << graph.constant_evaluations << " constant_steps=" << graph.constant_steps
		 << " syntax_arena_bytes=" << unit.counters().arena_bytes
		 << " semantic_arena_bytes=" << graph.arena.bytes_reserved() << " parse_ms=" << parse_ms
		 << " analyze_ms=" << analyze_ms << " print_ms=" << print_ms << " peak_rss_kb=" << usage.ru_maxrss << endl;
#endif
}

// --emit-types: each source file is a translation unit, parsed, analyzed
// into its semantic graph and dumped in command-line order. Any
// preprocessing, syntax or semantic error fails the whole run.
int run_emit_types_mode(vector<string> args)
{
	const bool stats = consume_stats_flag(args);
	const SourceOutputInvocation invocation = parse_source_output_invocation(args, false);
	const PreprocessorOptions options = source_preprocessor_options(invocation);
	// The output file's contents are unspecified after a failure.
	ofstream out(invocation.outfile, ios::binary);
	out << invocation.inputs.size() << " translation units\n";
	for (size_t i = 0; i < invocation.inputs.size(); ++i)
	{
		try
		{
			const chrono::steady_clock::time_point parse_start = chrono::steady_clock::now();
			syntax::SyntaxUnit unit(options, invocation.inputs[i]);
			if (!unit.parse())
			{
				cerr << "ERROR: " << unit.error() << endl;
				return EXIT_FAILURE;
			}
			const double parse_ms = milliseconds_since(parse_start);
			const chrono::steady_clock::time_point analyze_start = chrono::steady_clock::now();
			sema::SemanticGraph graph;
			// The parser's name bindings estimate the unit's (scope, name) pairs.
			graph.names.reserve(unit.counters().bindings);
			try
			{
				sema::Analyzer analyzer(graph, unit.identifiers());
				analyzer.set_hosted(options.hosted);
				analyzer.set_source_positions(&unit.positions());
				analyzer.analyze(unit.tree());
			}
			catch (const sema::SemanticError& e)
			{
				cerr << "ERROR: " << unit.describe(e.location(), e.what()) << endl;
				return EXIT_FAILURE;
			}
			catch (const InternalError& e)
			{
				throw unit.locate(e);
			}
			const double analyze_ms = milliseconds_since(analyze_start);
			const chrono::steady_clock::time_point print_start = chrono::steady_clock::now();
			out << "start translation unit " << (i + 1) << "\n";
			sema::PrintTypesDump(graph, out);
			out << "end translation unit\n";
			if (stats)
				report_types_stats(
					invocation.inputs[i], unit, graph, parse_ms, analyze_ms, milliseconds_since(print_start));
		}
		catch (const PreprocessError& e)
		{
			cerr << "ERROR: " << invocation.inputs[i] << ": " << e.what() << endl;
			return EXIT_FAILURE;
		}
	}
	out.flush();
	return out ? EXIT_SUCCESS : EXIT_FAILURE;
}

// `--stats` for --emit-semantics: the semantics mode's work counters.
void report_semantics_stats(const string& path, const sema::SemanticGraph& graph)
{
#if CPPGM_TELEMETRY_ENABLED
	cerr << "cppgm++ semantics " << path << ": expressions=" << graph.expressions
		 << " conversions=" << graph.conversions << " resolutions=" << graph.resolutions
		 << " candidates=" << graph.candidates << " specializations=" << graph.specializations
		 << " demands=" << graph.demands << " override_steps=" << graph.override_steps
		 << " virtual_slots=" << graph.virtual_slots << " instantiated_classes=" << graph.instantiated_classes
		 << " instantiated_functions=" << graph.instantiated_functions
		 << " instantiated_arguments=" << graph.instantiated_arguments
		 << " instantiated_variables=" << graph.instantiated_variables
		 << " value_substitutions=" << graph.value_substitutions << " partial_candidates=" << graph.partial_candidates
		 << " substitution_failures=" << graph.substitution_failures
		 << " limited_lookups=" << graph.names.limited_lookups() << " base_steps=" << graph.base_steps << endl;
#endif
}

// --emit-semantics: each source file is a translation unit, parsed,
// analyzed in semantics mode (typed expressions, conversions, calls and
// statement rules on top of the PA6 graph) and dumped in command-line order
//. Any preprocessing, syntax or semantic error fails the whole run.
int run_emit_semantics_mode(vector<string> args)
{
	const bool stats = consume_stats_flag(args);
	const SourceOutputInvocation invocation = parse_source_output_invocation(args, false);
	const PreprocessorOptions options = source_preprocessor_options(invocation);
	// The output file's contents are unspecified after a failure.
	ofstream out(invocation.outfile, ios::binary);
	out << invocation.inputs.size() << " translation units\n";
	for (size_t i = 0; i < invocation.inputs.size(); ++i)
	{
		try
		{
			const chrono::steady_clock::time_point parse_start = chrono::steady_clock::now();
			syntax::SyntaxUnit unit(options, invocation.inputs[i]);
			if (!unit.parse())
			{
				cerr << "ERROR: " << unit.error() << endl;
				return EXIT_FAILURE;
			}
			const double parse_ms = milliseconds_since(parse_start);
			const chrono::steady_clock::time_point analyze_start = chrono::steady_clock::now();
			sema::SemanticGraph graph;
			graph.names.reserve(unit.counters().bindings);
			try
			{
				sema::Analyzer analyzer(graph, unit.identifiers(), sema::AnalysisMode::Semantics);
				analyzer.set_hosted(options.hosted);
				analyzer.set_source_positions(&unit.positions());
				analyzer.analyze(unit.tree());
			}
			catch (const sema::SemanticError& e)
			{
				cerr << "ERROR: " << unit.describe(e.location(), e.what()) << endl;
				return EXIT_FAILURE;
			}
			catch (const InternalError& e)
			{
				throw unit.locate(e);
			}
			const double analyze_ms = milliseconds_since(analyze_start);
			const chrono::steady_clock::time_point print_start = chrono::steady_clock::now();
			out << "start translation unit " << (i + 1) << "\n";
			sema::PrintSemanticsDump(unit.tree(), graph, out);
			out << "end translation unit\n";
			if (stats)
			{
				report_types_stats(
					invocation.inputs[i], unit, graph, parse_ms, analyze_ms, milliseconds_since(print_start));
				report_semantics_stats(invocation.inputs[i], graph);
			}
		}
		catch (const PreprocessError& e)
		{
			cerr << "ERROR: " << invocation.inputs[i] << ": " << e.what() << endl;
			return EXIT_FAILURE;
		}
	}
	out.flush();
	return out ? EXIT_SUCCESS : EXIT_FAILURE;
}

bool consume_flag(vector<string>& args, const char* flag)
{
	for (size_t i = 0; i < args.size(); ++i)
	{
		if (args[i] == flag)
		{
			args.erase(args.begin() + i);
			return true;
		}
	}
	return false;
}

// `--stats` for --emit-lowir: the lowering's work counters and the
// program's size, phase times and peak RSS for the whole run.
void report_lowir_stats(const lower::LoweringCounters& counters, const lowir::Program& program, double front_ms,
	double lower_ms, double write_ms)
{
#if CPPGM_TELEMETRY_ENABLED
	struct rusage usage;
	getrusage(RUSAGE_SELF, &usage);
	cerr << "cppgm++ lowir stats: units=" << counters.units << " functions=" << counters.functions
		 << " declarations=" << counters.declarations << " globals=" << counters.globals
		 << " instructions=" << counters.instructions << " blocks=" << counters.blocks << " slots=" << counters.slots
		 << " mangled_names=" << counters.mangled_names << " vtables=" << counters.vtables
		 << " virtual_slots=" << counters.virtual_slots << " override_steps=" << counters.override_steps
		 << " specializations=" << counters.specializations << " instantiated_classes=" << counters.instantiated_classes
		 << " instantiated_functions=" << counters.instantiated_functions
		 << " instantiated_arguments=" << counters.instantiated_arguments
		 << " instantiated_variables=" << counters.instantiated_variables
		 << " value_substitutions=" << counters.value_substitutions
		 << " partial_candidates=" << counters.partial_candidates
		 << " substitution_failures=" << counters.substitution_failures
		 << " limited_lookups=" << counters.limited_lookups << " constant_evaluations=" << counters.constant_evaluations
		 << " constant_steps=" << counters.constant_steps << " base_steps=" << counters.base_steps
		 << " symbols=" << program.symbols.size() << " names=" << program.names.size()
		 << " lowir_arena_bytes=" << program.arena.bytes_reserved() << " front_ms=" << front_ms
		 << " lower_ms=" << lower_ms << " write_ms=" << write_ms << " peak_rss_kb=" << usage.ru_maxrss << endl;
#endif
}

// --emit-lowir: each source file is a translation unit, parsed, analyzed
// in semantics mode and lowered into one LowIR program built from
// the semantic facts; the unit's tree and graph are released before the
// next. The text writer is the output view. `--validate` checks the
// constructed program first (audit builds).
int run_emit_lowir_mode(vector<string> args)
{
	const bool stats = consume_stats_flag(args);
	const bool validate = consume_flag(args, "--validate");
	SourceOutputInvocation invocation = parse_source_output_invocation(args, true);
	// The LowIR half of an object compile: a `.lowir` output (the
	// toolchain's LowIR object input, `-c x.lowir`) with the object
	// compile's debug-information level stated (-g0, -gline-tables-only)
	// is the program `-c -o x.o` compiles, lowered in host-object mode as
	// --hosted asks, so the object it gives is the direct compile's (PA32
	// object roundtrip). Otherwise the output is the course's LowIR view,
	// which the LowIR lanes compare and the supplied course back end runs
	// (PA12 controls: `--emit-lowir -O0 -o x.lowir`), as `.obj` names the
	// course's object.
	// Both state the instruction facts the lowering proved (lowir.md
	// "Instruction Facts"), which the object compile reads.
	const bool object_half = invocation.debug_level && ends_with(invocation.outfile, ".lowir");
	if (object_half)
		invocation.hosted = true;
	PreprocessorOptions options = source_preprocessor_options(invocation);
	// The LowIR view preprocesses in the host environment -c uses (PA32: a
	// source the driver lanes compile includes host headers); --hosted also
	// lowers in host-object mode.
	if (!options.hosted)
		driver::add_host_environment(options, false);
	lowir::Program program;
	lower::ProgramLowering lowering(program);
	// Definitions are written as they complete and their bodies released,
	// except when the whole program is validated first.
	lowir::DefinitionStream definitions;
	const bool optimizing = invocation.optimizer.level > 0;
	if (!validate && !optimizing)
		lowering.set_definition_stream(&definitions);
	lowering.set_object_mode(invocation.hosted);
	lowering.set_host_objects(invocation.hosted);
	// The LowIR view carries the source's debug locations when debug
	// information is asked for (lowir.md "Debug Locations"; not the
	// object half, whose object takes none).
	lowering.set_debug_locations(invocation.debug_info && !object_half);
	double front_ms = 0, lower_ms = 0;
	for (size_t i = 0; i < invocation.inputs.size(); ++i)
	{
		try
		{
			const chrono::steady_clock::time_point front_start = chrono::steady_clock::now();
			syntax::SyntaxUnit unit(options, invocation.inputs[i]);
			if (!unit.parse())
			{
				cerr << "ERROR: " << unit.error() << endl;
				return EXIT_FAILURE;
			}
			sema::SemanticGraph graph;
			graph.names.reserve(unit.counters().bindings);
			sema::Analyzer analyzer(graph, unit.identifiers(), sema::AnalysisMode::Semantics);
			analyzer.set_hosted(options.hosted);
			analyzer.set_source_positions(&unit.positions());
			try
			{
				analyzer.analyze(unit.tree());
			}
			catch (const sema::SemanticError& e)
			{
				cerr << "ERROR: " << unit.describe(e.location(), e.what()) << endl;
				return EXIT_FAILURE;
			}
			catch (const InternalError& e)
			{
				throw unit.locate(e);
			}
			front_ms += milliseconds_since(front_start);
			const chrono::steady_clock::time_point lower_start = chrono::steady_clock::now();
			try
			{
				lowering.lower_unit(unit.tree(), graph, analyzer);
			}
			catch (const lower::LoweringError& e)
			{
				cerr << "ERROR: " << unit.describe(e.location(), e.what()) << endl;
				return EXIT_FAILURE;
			}
			catch (const sema::SemanticError& e)
			{
				cerr << "ERROR: " << unit.describe(e.location(), e.what()) << endl;
				return EXIT_FAILURE;
			}
			catch (const InternalError& e)
			{
				throw unit.locate(e);
			}
			lowering.counters().override_steps += graph.override_steps;
			lowering.counters().virtual_slots += graph.virtual_slots;
			lowering.counters().specializations += graph.specializations;
			lowering.counters().instantiated_classes += graph.instantiated_classes;
			lowering.counters().instantiated_functions += graph.instantiated_functions;
			lowering.counters().instantiated_arguments += graph.instantiated_arguments;
			lowering.counters().instantiated_variables += graph.instantiated_variables;
			lowering.counters().value_substitutions += graph.value_substitutions;
			lowering.counters().partial_candidates += graph.partial_candidates;
			lowering.counters().substitution_failures += graph.substitution_failures;
			lowering.counters().limited_lookups += graph.names.limited_lookups();
			lowering.counters().constant_evaluations += graph.constant_evaluations;
			lowering.counters().constant_steps += graph.constant_steps;
			lowering.counters().base_steps += graph.base_steps;
			lower_ms += milliseconds_since(lower_start);
		}
		catch (const PreprocessError& e)
		{
			cerr << "ERROR: " << invocation.inputs[i] << ": " << e.what() << endl;
			return EXIT_FAILURE;
		}
	}
	lowering.finish();
	if (validate)
	{
		lowir::Diagnostic diagnostic;
		if (!lowir::validate_constructed_program(program, diagnostic))
			throw InternalError("invalid LowIR: " + lowir::describe(program, diagnostic));
	}
	opt::OptimizerStats optimizer_stats;
	if (optimizing)
	{
		opt::optimize_program(program, invocation.optimizer, &optimizer_stats);
		lowir::Diagnostic diagnostic;
		if (validate && !lowir::validate_constructed_program(program, diagnostic))
			throw InternalError("optimized LowIR is invalid: " + lowir::describe(program, diagnostic));
	}
	const chrono::steady_clock::time_point write_start = chrono::steady_clock::now();
	const bool written = validate || optimizing ? lowir::write_program_file(program, invocation.outfile)
												: lowir::write_program_file(program, definitions, invocation.outfile);
	if (!written)
	{
		cerr << "ERROR: cannot write " << invocation.outfile << endl;
		return EXIT_FAILURE;
	}
	if (stats)
	{
		report_lowir_stats(lowering.counters(), program, front_ms, lower_ms, milliseconds_since(write_start));
		if (optimizing)
			cerr << opt::describe_stats(optimizer_stats) << endl;
	}
	return EXIT_SUCCESS;
}

int run_driver_mode(const vector<string>& args)
{
	const DriverInvocation invocation = parse_driver_invocation(args);
	switch (invocation.mode)
	{
	case DriverMode::Query:
		return driver::run_query(args[0]);
	case DriverMode::Preprocess:
	case DriverMode::Compile:
	case DriverMode::Link:
		return driver::run_toolchain(args, source_preprocessor_options());
	}
	throw InternalError("unreachable driver mode");
}

int run_cppgm(const vector<string>& raw_args)
{
	if (!telemetry::kEnabled && (has_arg(raw_args, "--stats") || has_arg(raw_args, "--stats-functions")))
		throw InvocationError(telemetry::kUnavailable);

	if (has_arg(raw_args, "--batch-stdin"))
	{
		return run_not_implemented_batch_mode();
	}

	if (has_help_arg(raw_args))
	{
		cout << cppgm_help_text();
		return EXIT_SUCCESS;
	}

	vector<string> args = raw_args;
	const EmitMode mode = parse_emit_mode(args);

	switch (mode)
	{
	case EmitMode::Ast:
		return run_emit_ast_mode(args);
	case EmitMode::Types:
		return run_emit_types_mode(args);
	case EmitMode::Semantics:
		return run_emit_semantics_mode(args);
	case EmitMode::LowIR:
		return run_emit_lowir_mode(args);
	case EmitMode::None:
		return run_driver_mode(args);
	}

	throw InternalError("unreachable emit mode");
}

} // namespace

int main(int argc, char** argv)
{
	try
	{
		return run_cppgm(collect_args(argc, argv));
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
