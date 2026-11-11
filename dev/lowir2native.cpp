// lowir2native: reads LowIR units, lowers them to x86-64 machine IR and
// writes a static Linux executable and/or the machine-IR dump.

#include "lowir/program.h"
#include "lowir/text_reader.h"
#include "backend/compile.h"
#include "native/backend.h"
#include "native/link/linker.h"
#include "native/object/object_compiler.h"
#include "support/internal_error.h"
#include "support/telemetry.h"
#include "support/tool_errors.h"
#include "support/tool_help_text.h"

#include <cstdio>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

using namespace std;

namespace {

struct LowIR2NativeInvocation
{
	bool has_optimization_level = false;
	int optimization_level = 0;
	native::Target target = native::host_target();
	string output_target;
	string outfile;
	string machine_ir_file;
	bool stats = false;
	vector<string> srcfiles;
	vector<string> link_inputs;
	vector<string> exports;
	string sysroot;
	string resource_directory;
	bool no_standard_libraries = false;
	bool no_startfiles = false;
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

bool is_optimization_level(const string& arg, int& level)
{
	if (arg.size() == 3 && arg[0] == '-' && arg[1] == 'O' && arg[2] >= '0' && arg[2] <= '3')
	{
		level = arg[2] - '0';
		return true;
	}
	return false;
}

bool starts_with_dash(const string& arg)
{
	return !arg.empty() && arg[0] == '-';
}

// Batch requests are served by the linked test runner; a build without it
// cannot run them.
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

LowIR2NativeInvocation parse_lowir2native_invocation(const vector<string>& args)
{
	LowIR2NativeInvocation invocation;

	for (size_t i = 0; i < args.size(); ++i)
	{
		int optimization_level = 0;
		if (is_optimization_level(args[i], optimization_level))
		{
			if (invocation.has_optimization_level)
			{
				throw InvocationError("multiple optimization levels provided");
			}
			invocation.has_optimization_level = true;
			invocation.optimization_level = optimization_level;
			continue;
		}
		if (args[i] == "--stats")
		{
			if (!telemetry::kEnabled)
				throw InvocationError(telemetry::kUnavailable);
			invocation.stats = true;
			continue;
		}
		if (args[i] == "--sysroot" || args[i].compare(0, 10, "--sysroot=") == 0 || args[i] == "-resource-dir" ||
			args[i].compare(0, 14, "-resource-dir=") == 0)
		{
			const bool root = args[i].compare(0, 9, "--sysroot") == 0;
			const size_t equal = args[i].find('=');
			if (equal == string::npos && i + 1 >= args.size())
				throw InvocationError("missing target directory");
			const string directory = equal == string::npos ? args[++i] : args[i].substr(equal + 1);
			if (directory.empty())
				throw InvocationError("empty target directory");
			(root ? invocation.sysroot : invocation.resource_directory) = directory;
			continue;
		}
		if (args[i] == "-nostdlib" || args[i] == "-nodefaultlibs" || args[i] == "-nostartfiles")
		{
			invocation.no_standard_libraries = args[i] != "-nostartfiles" || invocation.no_standard_libraries;
			invocation.no_startfiles = args[i] != "-nodefaultlibs" || invocation.no_startfiles;
			continue;
		}
		if (args[i] == "--export" || args[i].compare(0, 9, "--export=") == 0)
		{
			if (args[i] == "--export")
			{
				if (i + 1 >= args.size())
					throw InvocationError("missing name after --export");
				invocation.exports.push_back(args[++i]);
			}
			else
				invocation.exports.push_back(args[i].substr(9));
			if (invocation.exports.back().empty())
				throw InvocationError("empty export name");
			continue;
		}
		if (args[i] == "--target" || args[i].compare(0, 9, "--target=") == 0)
		{
			if (args[i] == "--target" && i + 1 >= args.size())
			{
				throw InvocationError("missing target after --target");
			}
			if (!invocation.output_target.empty())
			{
				throw InvocationError("multiple --target options provided");
			}
			invocation.output_target = args[i] == "--target" ? args[++i] : args[i].substr(9);
			if (!native::parse_target(invocation.output_target, invocation.target))
			{
				throw InvocationError("unsupported target: " + invocation.output_target);
			}
			continue;
		}
		if (args[i] == "--dump-machine-ir" || args[i] == "--dump-native-plan")
		{
			if (i + 1 >= args.size())
			{
				throw InvocationError("missing output file after --dump-machine-ir");
			}
			if (!invocation.machine_ir_file.empty())
			{
				throw InvocationError("multiple machine IR dump paths provided");
			}
			invocation.machine_ir_file = args[++i];
			continue;
		}
		if (args[i] == "-o")
		{
			if (i + 1 >= args.size())
			{
				throw InvocationError("missing output file after -o");
			}
			if (!invocation.outfile.empty())
			{
				throw InvocationError("multiple output files provided");
			}
			invocation.outfile = args[++i];
			continue;
		}
		if (starts_with_dash(args[i]))
		{
			throw InvocationError("unknown option: " + args[i]);
		}
		const string& path = args[i];
		const size_t dot = path.rfind('.');
		const string extension = dot == string::npos ? "" : path.substr(dot);
		if (extension == ".o" || extension == ".obj" || extension == ".a")
			invocation.link_inputs.push_back(path);
		else
			invocation.srcfiles.push_back(path);
	}

	if ((invocation.outfile.empty() && invocation.machine_ir_file.empty()) || invocation.srcfiles.empty())
	{
		throw InvocationError("invalid usage");
	}

	return invocation;
}

// An executable needing what the standalone runtime lacks (catch clauses,
// the C++ ABI runtime, RTTI objects, i128 helpers): the program compiled
// as a course object and linked with link mode's runtime library, as
// cppgm++ links a LowIR input.
bool link_with_runtime(lowir::Program& program, const LowIR2NativeInvocation& invocation, string& error)
{
	native::recover_jump_thunks(program, native::EhModel::Course);
	vector<native::object::ObjectFile> objects(1);
	objects[0].path = invocation.srcfiles[0];
	error.clear();
	if (!native::compile_object(program, objects[0], error, nullptr, native::EhModel::Course,
			invocation.optimization_level, false, invocation.target) ||
		!native::link_executable(objects, invocation.outfile, error, nullptr, invocation.target))
	{
		remove(invocation.outfile.c_str());
		return false;
	}
	return true;
}

int run_lowir2native_mode(const vector<string>& args)
{
	if (has_arg(args, "--batch-stdin"))
	{
		return run_not_implemented_batch_mode();
	}

	if (has_help_arg(args))
	{
		cout << lowir2native_help_text();
		return EXIT_SUCCESS;
	}

	const LowIR2NativeInvocation invocation = parse_lowir2native_invocation(args);
	lowir::Program program;
	string error;
	if (!lowir::read_program_files(program, invocation.srcfiles, error))
	{
		cerr << "ERROR: " << error << endl;
		return EXIT_FAILURE;
	}
	native::BackendOptions options;
	options.target = invocation.target;
	if (invocation.output_target.empty() && program.explicit_target)
		options.target = program.target;
	options.exports = invocation.exports;
	options.link_inputs = invocation.link_inputs;
	options.sysroot = invocation.sysroot;
	options.resource_directory = invocation.resource_directory;
	options.no_standard_libraries = invocation.no_standard_libraries;
	options.no_startfiles = invocation.no_startfiles;
	options.output = invocation.outfile;
	options.machine_ir = invocation.machine_ir_file;
	options.optimization_level = invocation.optimization_level;
	options.stats = invocation.stats;
	bool undefined = false;
	options.undefined_symbol = &undefined;
	string stats_line;
	if (!backend::compile_program(program, options, error, invocation.stats ? &stats_line : nullptr))
	{
		if (!undefined || !link_with_runtime(program, invocation, error))
		{
			cerr << "ERROR: " << error << endl;
			return EXIT_FAILURE;
		}
	}
	if (invocation.stats)
	{
		cerr << stats_line << endl;
	}
	return EXIT_SUCCESS;
}

} // namespace

int main(int argc, char** argv)
{
	try
	{
		return run_lowir2native_mode(collect_args(argc, argv));
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
