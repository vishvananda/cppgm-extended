#include "driver/toolchain.h"

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sys/stat.h>
#include <utility>
#include <vector>

#include "abi/itanium/abi_encoding_error.h"
#include "driver/host_environment.h"
#include "driver/wasm_environment.h"
#include "driver/wasm_link_options.h"
#include "lex/post_token_writer.h"
#include "lex/language_mode.h"
#include "lex/post_tokenizer.h"
#include "lower/program_lowering.h"
#include "lowir/text_reader.h"
#include "native/backend_error.h"
#include "native/link/linker.h"
#include "native/object/elf_object.h"
#include "native/object/macho_object.h"
#include "native/object/object_compiler.h"
#include "opt/optimizer.h"
#include "sema/analyzer.h"
#include "support/internal_error.h"
#include "support/telemetry.h"
#include "support/tool_errors.h"
#include "syntax/syntax_unit.h"
#include "wasm/backend.h"

#if CPPGM_TELEMETRY_ENABLED
#include <sys/resource.h>
#endif

namespace driver {

// Source dialect and feature gates share the token tools' standard names.
void language_standard(const std::string& standard, PreprocessorOptions& options)
{
	const LanguageMode mode = ReadLanguageMode(standard);
	options.cplusplus = mode.cplusplus;
	options.strict = mode.strict;
}

bool sse_option(const std::string& option, uint8_t& level)
{
	const bool disable = option.compare(0, 5, "-mno-") == 0;
	if (!disable && option.compare(0, 2, "-m") != 0)
		return false;
	const std::string feature = option.substr(disable ? 5 : 2);
	const char* names[] = { "sse3", "ssse3", "sse4.1", "sse4.2" };
	for (uint8_t i = 0; i < 4; ++i)
		if (feature == names[i] || (feature == "sse4" && i == (disable ? 2 : 3)))
		{
			level = disable ? std::min(level, i) : std::max(level, static_cast<uint8_t>(i + 1));
			return true;
		}
	return false;
}

namespace {

// An input in command-line order: a source, serialized LowIR (`.lowir`,
// PA33), an object, or a -l library.
struct Input
{
	enum class Kind
	{
		Source,
		LowIR,
		Object,
		Library
	};

	Kind kind;
	std::string name;
};

struct Invocation
{
	native::Target target = native::host_target();
	bool compile_only = false;
	bool position_independent = false;
	bool red_zone = true;
	// -ffunction-sections: each function in a section of its own.
	bool function_sections = false;
	bool preprocess_only = false;
	// -fsyntax-only: each source analyzed, nothing lowered or written.
	bool syntax_only = false;
	bool ignore_narrowing = false;
	bool debug_info = false;
	bool debug_types = false;
	bool debug_optimization = false;
	bool stats = false;
	bool strict_aliasing = true;
	// --stats-functions: the object back end's per-function census
	// (native/object/function_census.h), after each unit's compilation.
	bool function_census = false;
	// -O<n>: the LowIR optimization level before object
	// generation; omitted, -O0.
	opt::OptimizerOptions optimizer;
	std::string output;
	std::string sysroot;
	std::string resource_directory;
	bool no_standard_libraries = false;
	bool no_startfiles = false;
	uint32_t wasm_stack_size = 0;
	bool wasm_atomics = true;
	bool wasm_atomics_explicit = false;
	backend::WasmLinkOptions wasm_link;
	std::vector<Input> inputs;
	std::vector<std::string> library_paths;
	std::vector<std::string> exports;
	PreprocessorOptions preprocessor;
};

double milliseconds_since(std::chrono::steady_clock::time_point start)
{
	return std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - start).count();
}

#if CPPGM_TELEMETRY_ENABLED
long peak_rss_kb()
{
	struct rusage usage;
	getrusage(RUSAGE_SELF, &usage);
	return usage.ru_maxrss;
}
#endif

bool starts_with(const std::string& value, const char* prefix)
{
	return value.compare(0, std::char_traits<char>::length(prefix), prefix) == 0;
}

bool ends_with(const std::string& value, const char* suffix)
{
	const size_t length = std::char_traits<char>::length(suffix);
	return value.size() >= length && value.compare(value.size() - length, length, suffix) == 0;
}

bool object_name(const std::string& name)
{
	return ends_with(name, ".o") || ends_with(name, ".obj") || ends_with(name, ".a");
}

// -O<level> as GCC reads it: -O is -O1, -Os, -Oz and -Og keep their nearest
// level, -Ofast and -O4 and beyond are -O3. Returns -1 for no -O option.
int optimization_level(const std::string& arg)
{
	if (!starts_with(arg, "-O"))
		return -1;
	const std::string level = arg.substr(2);
	if (level.empty() || level == "g")
		return 1;
	if (level == "s" || level == "z")
		return 2;
	if (level == "fast")
		return 3;
	for (char c : level)
		if (c < '0' || c > '9')
			throw InvocationError("unsupported option: " + arg);
	const unsigned long n = std::strtoul(level.c_str(), nullptr, 10);
	return n > 3 ? 3 : static_cast<int>(n);
}

// The value of an option given as `-X value` or `-Xvalue`; false when
// args[i] is not the option.
bool option_value(const std::vector<std::string>& args, size_t& i, const char* option, std::string& value)
{
	const std::string& arg = args[i];
	if (arg == option)
	{
		if (i + 1 >= args.size())
			throw InvocationError(std::string("missing argument after ") + option);
		value = args[++i];
		return true;
	}
	if (starts_with(arg, option) && arg.size() > std::char_traits<char>::length(option))
	{
		value = arg.substr(std::char_traits<char>::length(option));
		return true;
	}
	return false;
}

// GCC flags that change the program's types, layout, ABI or observable
// behavior, which this compiler does not honor: refused rather than
// accepted with another meaning (README: build-system flags are honored or
// harmless).
bool refused_flag(const std::string& arg)
{
	static const char* const kRefused[] = { "-fshort-wchar", "-fshort-enums", "-fpack-struct", "-funsigned-bitfields",
		"-fno-signed-bitfields", "-fno-elide-constructors", "-ffreestanding", "-fno-hosted", "-m32", "-mx32", "-m16",
		"-mlong-double-64", "-mlong-double-128", "-msoft-float", "-mno-sse", "-mno-sse2", "-mno-80387",
		"-mno-fp-ret-in-387", "-mfpmath=387" };
	for (const char* refused : kRefused)
	{
		if (arg == refused)
			return true;
	}
	return starts_with(arg, "-fpack-struct=") || starts_with(arg, "-mabi=");
}

bool accepted_flag(const std::string& arg)
{
	return starts_with(arg, "-O") || starts_with(arg, "-g") || starts_with(arg, "-W") || starts_with(arg, "-f") ||
		starts_with(arg, "-m") || arg == "-pedantic" || arg == "-pedantic-errors" || arg == "-w" || arg == "-pipe" ||
		arg == "-MMD" || arg == "-MD" || arg == "-MP";
}

// GCC's include chain (cpplib's remove_duplicates): a directory named
// twice is searched at its first position only, and an -I directory that
// is also a system directory (-isystem or a standard one) is searched as
// that system directory ("the -I option is ignored"), so <stdlib.h> under
// -I/usr/include still finds libstdc++'s wrapper first. Directories are
// compared by device and inode.
void prune_include_paths(PreprocessorOptions& options)
{
	std::vector<std::pair<dev_t, ino_t>> seen;
	const auto identity = [](const std::string& path, std::pair<dev_t, ino_t>& id) {
		struct stat status;
		if (stat(path.c_str(), &status) != 0 || !S_ISDIR(status.st_mode))
			return false;
		id = std::make_pair(status.st_dev, status.st_ino);
		return true;
	};
	const auto first = [&](const std::string& path) {
		std::pair<dev_t, ino_t> id;
		if (!identity(path, id))
			return true;
		for (const std::pair<dev_t, ino_t>& other : seen)
		{
			if (other == id)
				return false;
		}
		seen.push_back(id);
		return true;
	};
	std::vector<std::string> system;
	for (const std::string& path : options.system_include_paths)
	{
		if (first(path))
			system.push_back(path);
	}
	std::vector<std::string> user;
	for (const std::string& path : options.include_paths)
	{
		if (first(path))
			user.push_back(path);
	}
	options.system_include_paths.swap(system);
	options.include_paths.swap(user);
}

void configure_target_runtime(Invocation& invocation, bool nostdinc)
{
	configure_target_environment(invocation.preprocessor, invocation.target);
	if (target_model::is_wasm(invocation.target))
	{
		if (invocation.wasm_atomics)
			invocation.preprocessor.command_line.insert(0, "#define __wasm_atomics__ 1\n");
		if (!nostdinc)
			add_wasm_headers(invocation.preprocessor, invocation.sysroot, invocation.resource_directory);
		if (!invocation.sysroot.empty())
			invocation.library_paths.push_back(invocation.sysroot + "/lib/wasm32-wasip1");
	}
	else if (!invocation.sysroot.empty() || !invocation.resource_directory.empty() ||
		invocation.no_standard_libraries || invocation.no_startfiles || invocation.wasm_stack_size != 0 ||
		(invocation.wasm_atomics_explicit && invocation.wasm_atomics) || invocation.wasm_link.requested())
		throw InvocationError("target runtime directory/options currently require a Wasm target");
}

Invocation parse(const std::vector<std::string>& args, const PreprocessorOptions& base)
{
	Invocation invocation;
	invocation.preprocessor = base;
	std::string value;
	std::string forced_includes;
	bool nostdinc = false;
	bool pthread = false;
	bool optimize_size = false, popcnt_explicit = false;
	unsigned pic_level = 0, pie_level = 0;
	const size_t base_defines = invocation.preprocessor.command_line.size();
	for (size_t i = 0; i < args.size(); ++i)
	{
		const std::string& arg = args[i];
		if (arg == "-c")
			invocation.compile_only = true;
		else if (arg == "-fPIC" || arg == "-fpic" || arg == "-fPIE" || arg == "-fpie")
		{
			invocation.position_independent = true;
			pic_level = arg == "-fPIC" || arg == "-fPIE" ? 2 : 1;
			pie_level = arg == "-fPIE" || arg == "-fpie" ? pic_level : 0;
		}
		else if (arg == "-fno-PIC" || arg == "-fno-pic" || arg == "-fno-PIE" || arg == "-fno-pie")
		{
			invocation.position_independent = false;
			pic_level = pie_level = 0;
		}
		else if (arg == "-mred-zone" || arg == "-mno-red-zone")
			invocation.red_zone = arg == "-mred-zone";
		else if (arg == "-ffunction-sections" || arg == "-fno-function-sections")
			invocation.function_sections = arg == "-ffunction-sections";
		else if (arg == "-pthread")
			pthread = true;
		else if (parse_wasm_link_option(args, i, invocation.wasm_link))
			continue;
		else if (arg == "-matomics" || arg == "-mno-atomics")
		{
			invocation.wasm_atomics = arg == "-matomics";
			invocation.wasm_atomics_explicit = true;
		}
		else if (arg == "-E")
			invocation.preprocess_only = true;
		else if (arg == "-nostdinc")
			nostdinc = true;
		else if (starts_with(arg, "-Wl,-z,stack-size="))
		{
			const std::string size = arg.substr(18);
			uint64_t bytes = 0;
			for (char digit : size)
			{
				if (digit < '0' || digit > '9' || bytes > 0x7fffffffu / 10)
					throw InvocationError("invalid Wasm stack size");
				bytes = bytes * 10 + static_cast<unsigned>(digit - '0');
			}
			if (bytes == 0 || bytes > 0x7fffffffu || bytes % 16 != 0)
				throw InvocationError("Wasm stack size must be a positive multiple of 16 below 2 GiB");
			invocation.wasm_stack_size = static_cast<uint32_t>(bytes);
		}
		else if (arg == "--sysroot" || starts_with(arg, "--sysroot=") || arg == "-resource-dir" ||
			starts_with(arg, "-resource-dir="))
		{
			const bool root = starts_with(arg, "--sysroot");
			const size_t equal = arg.find('=');
			if (equal == std::string::npos && i + 1 >= args.size())
				throw InvocationError("missing directory after " + arg);
			const std::string directory = equal == std::string::npos ? args[++i] : arg.substr(equal + 1);
			if (directory.empty())
				throw InvocationError("empty target directory");
			(root ? invocation.sysroot : invocation.resource_directory) = directory;
		}
		else if (arg == "-nostdlib" || arg == "-nodefaultlibs")
		{
			invocation.no_standard_libraries = true;
			invocation.no_startfiles = invocation.no_startfiles || arg == "-nostdlib";
		}
		else if (arg == "-nostartfiles")
			invocation.no_startfiles = true;
		else if (arg == "-include")
		{
			if (i + 1 >= args.size())
				throw InvocationError("missing file after -include");
			// GCC: read after every -D and -U, found from the working
			// directory first.
			forced_includes += "#include \"" + args[++i] + "\"\n";
		}
		else if (starts_with(arg, "-std="))
			language_standard(arg.substr(5), invocation.preprocessor);
		else if (starts_with(arg, "-stdlib="))
		{
			if (!host_stdlib_selected(arg.substr(8)))
				throw InvocationError("this compiler was not built for " + arg + " (CPPGM_STDLIB_FLAGS)");
		}
		else if (arg == "-x")
		{
			if (i + 1 >= args.size())
				throw InvocationError("missing language after -x");
			if (args[++i] != "c++")
				throw InvocationError("unsupported language: " + args[i]);
		}
		else if (arg == "-fno-exceptions" || arg == "-fexceptions")
			invocation.preprocessor.modes.exceptions = arg == "-fexceptions";
		else if (arg == "-fno-strict-aliasing" || arg == "-fstrict-aliasing")
			invocation.strict_aliasing = arg == "-fstrict-aliasing";
		else if (arg == "-mpopcnt" || arg == "-mno-popcnt")
		{
			invocation.preprocessor.popcnt = arg == "-mpopcnt";
			popcnt_explicit = true;
		}
		else if (sse_option(arg, invocation.preprocessor.sse_level))
		{
		}
		else if (arg == "-fno-rtti" || arg == "-frtti")
			invocation.preprocessor.modes.rtti = arg == "-frtti";
		else if (arg == "-funsigned-char" || arg == "-fno-signed-char" || arg == "-fsigned-char" ||
			arg == "-fno-unsigned-char")
			invocation.preprocessor.plain_char_unsigned = arg == "-funsigned-char" || arg == "-fno-signed-char";
		else if (arg == "-Wno-narrowing" || arg == "-Wnarrowing" || arg == "-Werror=narrowing")
			invocation.ignore_narrowing = arg == "-Wno-narrowing";
		else if (arg == "--stats")
			invocation.stats = true;
		else if (arg == "--stats-functions")
			invocation.function_census = true;
		else if (starts_with(arg, "-g"))
		{
			const int request = debug_info_request(arg);
			if (request >= 0)
			{
				invocation.debug_info = request != 0;
				invocation.debug_types = debug_type_request(arg) != 0;
			}
		}
		else if (optimization_level(arg) >= 0)
		{
			invocation.optimizer.level = optimization_level(arg);
			optimize_size = arg == "-Os" || arg == "-Oz";
			invocation.debug_optimization = arg == "-Og";
		}
		else if (arg == "-fno-inline" || arg == "-finline")
			invocation.optimizer.optional_inlining = arg == "-finline";
		else if (arg == "--inline-limit")
		{
			if (i + 1 >= args.size() || !opt::parse_inline_limit(args[++i], invocation.optimizer.limits))
				throw InvocationError("invalid --inline-limit");
		}
		else if (arg == "--verify-passes")
			invocation.optimizer.verify_passes = true;
		else if (arg.compare(0, 16, "--verify-passes=") == 0)
		{
			invocation.optimizer.verify_passes = true;
			invocation.optimizer.verify_dump = arg.substr(16);
		}
		else if (arg == "-fsyntax-only")
			invocation.syntax_only = true;
		else if (arg == "-o")
		{
			if (i + 1 >= args.size())
				throw InvocationError("missing output file after -o");
			invocation.output = args[++i];
		}
		else if (arg == "--export" || starts_with(arg, "--export="))
		{
			if (arg == "--export" && i + 1 >= args.size())
				throw InvocationError("missing export name");
			invocation.exports.push_back(arg == "--export" ? args[++i] : arg.substr(9));
		}
		else if (arg == "--target" || starts_with(arg, "--target="))
		{
			if (arg == "--target" && i + 1 >= args.size())
				throw InvocationError("missing target after --target");
			const std::string target = arg == "--target" ? args[++i] : arg.substr(9);
			if (!native::parse_target(target, invocation.target))
				throw InvocationError("unsupported target: " + target);
		}
		else if (option_value(args, i, "-isystem", value))
			invocation.preprocessor.system_include_paths.push_back(value);
		else if (option_value(args, i, "-I", value))
			invocation.preprocessor.include_paths.push_back(value);
		else if (option_value(args, i, "-D", value))
		{
			const size_t equals = value.find('=');
			invocation.preprocessor.command_line += "#define " +
				(equals == std::string::npos ? value + " 1"
											 : value.substr(0, equals) + " " + value.substr(equals + 1)) +
				"\n";
		}
		else if (option_value(args, i, "-U", value))
			invocation.preprocessor.command_line += "#undef " + value + "\n";
		else if (option_value(args, i, "-L", value))
			invocation.library_paths.push_back(value);
		else if (option_value(args, i, "-l", value))
			invocation.inputs.push_back(Input{ Input::Kind::Library, value });
		else if (option_value(args, i, "-MF", value) || option_value(args, i, "-MT", value) ||
			option_value(args, i, "-MQ", value))
			continue;
		else if (refused_flag(arg))
			throw InvocationError("unsupported option (it changes the program's meaning): " + arg);
		else if (accepted_flag(arg))
			continue;
		else if (starts_with(arg, "-"))
			throw InvocationError("unsupported option: " + arg);
		else
			invocation.inputs.push_back(Input{ object_name(arg) ? Input::Kind::Object
					: ends_with(arg, ".lowir")                  ? Input::Kind::LowIR
																: Input::Kind::Source,
				arg });
	}
	if (invocation.inputs.empty())
		throw InvocationError("no input files");
	// GCC's -pthread defines _REENTRANT ahead of every -D and -U wherever
	// it is written (the host link adds the thread library; link mode's
	// runtime needs nothing).
	if (pthread)
		invocation.preprocessor.command_line.insert(base_defines, "#define _REENTRANT 1\n");
	invocation.preprocessor.command_line += forced_includes;
	if (!popcnt_explicit)
		invocation.preprocessor.popcnt = invocation.preprocessor.sse_level >= 4;
	// These macros describe this invocation. Explicit -D/-U retain their
	// precedence over target and optimization defaults.
	std::string mode_macros = invocation.optimizer.level > 0 ? "#define __OPTIMIZE__ 1\n" : "";
	if (invocation.optimizer.level == 0 || !invocation.optimizer.optional_inlining)
		mode_macros += "#define __NO_INLINE__ 1\n";
	if (optimize_size)
		mode_macros += "#define __OPTIMIZE_SIZE__ 1\n";
	if (invocation.target == native::Target::MacOS && pic_level == 0)
		pic_level = 2;
	if (pic_level != 0)
		mode_macros += "#define __PIC__ " + std::to_string(pic_level) + "\n#define __pic__ " +
			std::to_string(pic_level) + "\n";
	if (pie_level != 0)
		mode_macros += "#define __PIE__ " + std::to_string(pie_level) + "\n#define __pie__ " +
			std::to_string(pie_level) + "\n";
	invocation.preprocessor.command_line.insert(0, mode_macros);
	// Preserve named objects and statement boundaries in the debugging level.
	// Preprocessing still observes GCC's __OPTIMIZE__ for -Og.
	if (invocation.debug_optimization && invocation.debug_types)
		invocation.optimizer.level = 0;
	add_host_environment(invocation.preprocessor, nostdinc || invocation.target != native::host_target());
	configure_target_runtime(invocation, nostdinc);
	invocation.optimizer.has_gnu_sincos = target_model::has_gnu_sincos(invocation.target);
	// Darwin objects are position independent even without an explicit -fPIC.
	if (invocation.target == native::Target::MacOS)
		invocation.position_independent = true;
	prune_include_paths(invocation.preprocessor);
	if (invocation.preprocess_only)
		invocation.preprocessor.builtin_declarations.clear();
	return invocation;
}

// The semantic graph's template and evaluation work for a unit's stats
// line (hosted compilations have no --emit-lowir view of it).
struct UnitWork
{
	telemetry::Counter specializations = 0;
	telemetry::Counter instantiated_classes = 0;
	telemetry::Counter instantiated_functions = 0;
	telemetry::Counter instantiated_variables = 0;
	telemetry::Counter constant_evaluations = 0;
	telemetry::Counter resolutions = 0;
	telemetry::Counter candidates = 0;
};

void report_unit(const std::string& path, double front_ms, double back_ms, const native::ObjectStats& stats,
	const UnitWork& work, const native::object::ObjectFile& object)
{
#if CPPGM_TELEMETRY_ENABLED
	uint64_t bytes = 0;
	for (const native::object::Section& section : object.sections)
		bytes += section.bytes.size();
	std::cerr << "cppgm++ compile stats " << path << ": functions=" << stats.select.functions
			  << " lowir_instructions=" << stats.select.lowir_instructions
			  << " mir_instructions=" << stats.select.mir_instructions << " code_bytes=" << stats.emit.code_bytes
			  << " sections=" << stats.sections << " symbols=" << stats.symbols << " relocations=" << stats.relocations
			  << " landings=" << stats.landings << " lsdas=" << stats.lsdas << " unwind_bytes=" << stats.unwind_bytes
			  << " eh_walks=" << stats.select.eh_walks << " eh_states=" << stats.select.eh_states
			  << " pruned_functions=" << stats.pruned_functions << " specializations=" << work.specializations
			  << " instantiated_classes=" << work.instantiated_classes
			  << " instantiated_functions=" << work.instantiated_functions
			  << " instantiated_variables=" << work.instantiated_variables
			  << " constant_evaluations=" << work.constant_evaluations << " resolutions=" << work.resolutions
			  << " candidates=" << work.candidates << " section_bytes=" << bytes << " front_ms=" << front_ms
			  << " lower_and_object_ms=" << back_ms << " select_ms=" << stats.select_ms
			  << " encode_ms=" << stats.encode_ms << " peak_rss_kb=" << peak_rss_kb() << std::endl;
#endif
}

// --stats-functions: the unit's census lines on standard error.
void report_census(const native::ObjectStats& stats)
{
	if (stats.census_lines.empty())
		return;
	for (const std::string& line : stats.census_lines)
		std::cerr << line << '\n';
	std::cerr.flush();
}

// One translation unit through the whole pipeline into `object`: each
// function definition is encoded as the lowering completes it and its
// LowIR body released; the unit's syntax and semantic graph are released
// before the remaining definitions are encoded and the object is built.
// Optimizes `program` at the invocation's level (a course object's
// thunks given their targets: OptimizerOptions::inline_thunk_targets). A
// jump thunk is then what its optimized body says: Program::jump_thunks,
// not in the LowIR text, is recognized again from the bodies as an object
// compile of the optimized LowIR recognizes it
// (native::recover_jump_thunks), so a thunk whose target was inlined into
// it is encoded as written, and a host object is the one compiled from
// the optimized text (lowiropt and --emit-lowir keep thunks as lowered).
void optimize_unit(lowir::Program& program, const Invocation& invocation, native::EhModel eh_model)
{
	if (invocation.optimizer.level == 0)
		return;
	opt::OptimizerOptions options = invocation.optimizer;
	options.inline_thunk_targets = eh_model == native::EhModel::Course;
	opt::optimize_program(program, options, nullptr);
	program.jump_thunks.clear();
	native::recover_jump_thunks(program, eh_model);
}

bool lower_source(const std::string& path, const Invocation& invocation, lower::ProgramLowering& lowering,
	UnitWork& work, double& front_ms, std::chrono::steady_clock::time_point start)
{
	{
		syntax::SyntaxUnit unit(invocation.preprocessor, path);
		if (!unit.parse())
		{
			std::cerr << "ERROR: " << unit.error() << std::endl;
			return false;
		}
		sema::SemanticGraph graph(invocation.preprocessor.plain_char_unsigned, invocation.target);
		graph.names.reserve(unit.counters().bindings);
		sema::Analyzer analyzer(graph, unit.identifiers(), sema::AnalysisMode::Semantics);
		analyzer.set_hosted(invocation.preprocessor.hosted);
		analyzer.set_cxx14(invocation.preprocessor.cplusplus != "201103L");
		analyzer.set_cxx17(invocation.preprocessor.cplusplus == "201703L");
		analyzer.set_exceptions(invocation.preprocessor.modes.exceptions);
		analyzer.set_ignore_narrowing(invocation.ignore_narrowing);
		analyzer.set_source_positions(&unit.positions());
		try
		{
			analyzer.analyze(unit.tree());
			front_ms = milliseconds_since(start);
			if (invocation.syntax_only)
				return true;
			lowering.lower_unit(unit.tree(), graph, analyzer);
			work.specializations = graph.specializations;
			work.instantiated_classes = graph.instantiated_classes;
			work.instantiated_functions = graph.instantiated_functions;
			work.instantiated_variables = graph.instantiated_variables;
			work.constant_evaluations = graph.constant_evaluations;
			work.resolutions = graph.resolutions;
			work.candidates = graph.candidates;
		}
		catch (const sema::SemanticError& e)
		{
			std::cerr << "ERROR: " << unit.describe(e.location(), e.what()) << std::endl;
			return false;
		}
		catch (const lower::LoweringError& e)
		{
			std::cerr << "ERROR: " << unit.describe(e.location(), e.what()) << std::endl;
			return false;
		}
		catch (const InternalError& e)
		{
			throw unit.locate(e);
		}
	}
	return true;
}

bool compile_unit(
	const std::string& path, const Invocation& invocation, native::EhModel eh_model, native::object::ObjectFile& object)
{
	const std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
	lowir::Program program;
	lower::ProgramLowering lowering(program);
	native::ObjectStats stats;
	stats.clocks = invocation.stats;
	stats.census = invocation.function_census;
	native::ObjectBuilder builder(program, stats, eh_model, invocation.optimizer.level, invocation.position_independent,
		invocation.target, invocation.red_zone, invocation.function_sections);
	// At -O0 each definition is encoded as the lowering completes it; an
	// optimizing level keeps the program until the optimizer has run.
	if (invocation.optimizer.level == 0)
		lowering.set_definition_stream(&builder);
	lowering.set_object_mode(true);
	lowering.set_debug_locations(invocation.debug_info);
	lowering.set_debug_types(invocation.debug_types);
	lowering.set_host_objects(eh_model == native::EhModel::Host);
	lowering.set_macho_objects(invocation.target == native::Target::MacOS);
	lowering.set_optimizing(true);
	lowering.set_strict_aliasing(invocation.strict_aliasing);
	lowering.set_popcnt(invocation.preprocessor.popcnt);
	lowering.set_exceptions(invocation.preprocessor.modes.exceptions);
	double front_ms = 0;
	UnitWork work;
	try
	{
		if (!lower_source(path, invocation, lowering, work, front_ms, start))
			return false;
		if (invocation.syntax_only)
			return true;
		lowering.finish();
		optimize_unit(program, invocation, eh_model);
		object.path = path;
		builder.finish(object);
	}
	// The unit's errors without a source position. An InternalError (a
	// compiler bug, not an error of the program) leaves for the tool to
	// report.
	catch (const PreprocessError& e)
	{
		std::cerr << "ERROR: " << path << ": " << e.what() << std::endl;
		return false;
	}
	catch (const lower::LoweringError& e)
	{
		std::cerr << "ERROR: " << path << ": " << e.what() << std::endl;
		return false;
	}
	catch (const native::BackendError& e)
	{
		std::cerr << "ERROR: " << path << ": " << e.what() << std::endl;
		return false;
	}
	catch (const abi_mangle::EncodingError& e)
	{
		std::cerr << "ERROR: " << path << ": " << e.what() << std::endl;
		return false;
	}
	catch (const LimitError& e)
	{
		std::cerr << "ERROR: " << path << ": " << e.what() << std::endl;
		return false;
	}
	if (invocation.stats)
		report_unit(path, front_ms, milliseconds_since(start) - front_ms, stats, work, object);
	report_census(stats);
	return true;
}

// One serialized LowIR unit (PA33's LowIR object input): read and
// validated, optimized at the invocation's level, then compiled by the same
// object back end as a source's program.
bool compile_lowir_unit(
	const std::string& path, const Invocation& invocation, native::EhModel eh_model, native::object::ObjectFile& object)
{
	// A host object's LowIR may state what course LowIR cannot (classes of
	// x87 and SSEUP data, arguments to `...`, NaN payloads; see
	// lowir::Program::object_facts).
	lowir::Program program;
	program.object_facts = true;
	std::string error;
	if (!lowir::read_program_files(program, std::vector<std::string>(1, path), error))
	{
		std::cerr << "ERROR: " << error << std::endl;
		return false;
	}
	native::recover_jump_thunks(program, eh_model);
	optimize_unit(program, invocation, eh_model);
	native::ObjectStats stats;
	stats.clocks = invocation.stats;
	stats.census = invocation.function_census;
	object.path = path;
	if (!native::compile_object(program, object, error, &stats, eh_model, invocation.optimizer.level,
			invocation.position_independent, invocation.target, invocation.red_zone, invocation.function_sections))
	{
		std::cerr << "ERROR: " << path << ": " << error << std::endl;
		return false;
	}
	report_census(stats);
	return true;
}

bool compile_input(
	const Input& input, const Invocation& invocation, native::EhModel eh_model, native::object::ObjectFile& object)
{
	return input.kind == Input::Kind::LowIR ? compile_lowir_unit(input.name, invocation, eh_model, object)
											: compile_unit(input.name, invocation, eh_model, object);
}

bool regular_file(const std::string& path)
{
	struct stat status;
	return stat(path.c_str(), &status) == 0 && S_ISREG(status.st_mode);
}

// An output that names one of the inputs would destroy it (host compilers
// refuse too).
void check_output(const Invocation& invocation, const std::string& output)
{
	struct stat out;
	if (stat(output.c_str(), &out) != 0)
		return;
	for (const Input& input : invocation.inputs)
	{
		struct stat in;
		if (input.kind != Input::Kind::Library && stat(input.name.c_str(), &in) == 0 && in.st_dev == out.st_dev &&
			in.st_ino == out.st_ino)
			throw InvocationError("input file " + input.name + " is the same as the output file");
	}
}

std::string default_object_name(const std::string& source)
{
	std::string name = source.substr(source.rfind('/') == std::string::npos ? 0 : source.rfind('/') + 1);
	const size_t dot = name.rfind('.');
	if (dot != std::string::npos && dot != 0)
		name.erase(dot);
	return name + ".o";
}

// The exception model of a -c object: a course object (`.obj`, PA25's
// encoding for link mode) keeps the course's runtime ABI; any other
// object is a host object the host toolchain links, with host EH tables.
native::EhModel object_eh_model(const std::string& output)
{
	return ends_with(output, ".obj") ? native::EhModel::Course : native::EhModel::Host;
}

// Libraries are link inputs: -c compiles the sources and ignores them.
int compile_mode(const Invocation& invocation)
{
	size_t sources = 0;
	for (const Input& input : invocation.inputs)
		sources += input.kind != Input::Kind::Library ? 1 : 0;
	if (sources != 1 && !invocation.output.empty())
		throw InvocationError("cannot specify -o with -c and several inputs");
	for (const Input& input : invocation.inputs)
	{
		if (input.kind == Input::Kind::Library)
			continue;
		if (input.kind != Input::Kind::Source && input.kind != Input::Kind::LowIR)
			throw InvocationError("-c needs C++ source or LowIR inputs: " + input.name);
		native::object::ObjectFile object;
		const std::string output = invocation.output.empty() ? default_object_name(input.name) : invocation.output;
		if (!compile_input(input, invocation, object_eh_model(output), object))
			return EXIT_FAILURE;
		std::string error;
		check_output(invocation, output);
		if (!(invocation.target == native::Target::MacOS ? native::object::write_macho_object(object, output, error)
														 : native::object::write_elf_object(object, output, error)))
		{
			std::cerr << "ERROR: " << error << std::endl;
			return EXIT_FAILURE;
		}
	}
	return EXIT_SUCCESS;
}

// -l<name>: lib<name>.o or lib<name>.obj in the first -L directory that has one.
std::string find_library(const Invocation& invocation, const std::string& name)
{
	for (const std::string& directory : invocation.library_paths)
	{
		const std::string base = directory + (ends_with(directory, "/") ? "" : "/") + "lib" + name;
		for (const char* suffix : { ".o", ".obj", ".a" })
		{
			if ((std::string(suffix) != ".a" || target_model::is_wasm(invocation.target)) &&
				regular_file(base + suffix))
				return base + suffix;
		}
	}
	throw InvocationError("library not found: -l" + name);
}

int link_mode(const Invocation& invocation)
{
	const std::string output = invocation.output.empty() ? "a.out" : invocation.output;
	check_output(invocation, output);
	std::vector<native::object::ObjectFile> objects;
	objects.reserve(invocation.inputs.size() + 2);
	for (const Input& input : invocation.inputs)
	{
		objects.push_back(native::object::ObjectFile());
		native::object::ObjectFile& object = objects.back();
		if (input.kind == Input::Kind::Source || input.kind == Input::Kind::LowIR)
		{
			if (!compile_input(input, invocation, native::EhModel::Course, object))
				return EXIT_FAILURE;
			continue;
		}
		const std::string path = input.kind == Input::Kind::Library ? find_library(invocation, input.name) : input.name;
		std::string error;
		if (!(invocation.target == native::Target::MacOS ? native::object::read_macho_object(path, object, error)
														 : native::object::read_elf_object(path, object, error)))
		{
			std::cerr << "ERROR: " << error << std::endl;
			return EXIT_FAILURE;
		}
	}
	native::LinkStats stats;
	std::string error;
	if (!native::link_executable(objects, output, error, &stats, invocation.target))
	{
		// No stale executable is left behind (as the host linker removes it).
		std::remove(output.c_str());
		std::cerr << "ERROR: " << error << std::endl;
		return EXIT_FAILURE;
	}
#if CPPGM_TELEMETRY_ENABLED
	if (invocation.stats)
		std::cerr << "cppgm++ link stats: objects=" << stats.objects << " sections=" << stats.sections
				  << " discarded_groups=" << stats.discarded_groups << " symbols=" << stats.symbols
				  << " relocations=" << stats.relocations << " got_slots=" << stats.got_slots
				  << " runtime_names=" << stats.runtime_members << " text_bytes=" << stats.text_bytes
				  << " rodata_bytes=" << stats.rodata_bytes << " data_bytes=" << stats.data_bytes
				  << " bss_bytes=" << stats.bss_bytes << " runtime_ms=" << stats.runtime_ms
				  << " link_ms=" << stats.link_ms << " peak_rss_kb=" << peak_rss_kb() << std::endl;
#endif
	return EXIT_SUCCESS;
}

// One translation unit's posttoken records (`sof`, the PA2 tokens, `eof`);
// false after an error.
bool wasm_input(const Input& input, const Invocation& invocation, const backend::Options& options,
	wasm::LinkInputs& inputs, std::string& error)
{
	if (input.kind == Input::Kind::Object || input.kind == Input::Kind::Library)
	{
		if (invocation.compile_only)
			throw InvocationError("-c requires a source or LowIR input");
		try
		{
			wasm::read_link_inputs(
				input.kind == Input::Kind::Library ? find_library(invocation, input.name) : input.name, inputs);
			return true;
		}
		catch (const wasm::WasmError& failure)
		{
			error = failure.what();
			return false;
		}
	}
	lowir::Program program;
	if (input.kind == Input::Kind::LowIR)
	{
		if (!lowir::read_program_files(program, { input.name }, error))
			return false;
	}
	else
	{
		program.target = invocation.target;
		program.explicit_target = true;
		lower::ProgramLowering lowering(program);
		lowering.set_optimizing(invocation.optimizer.level >= 2);
		lowering.set_exceptions(invocation.preprocessor.modes.exceptions);
		lowering.set_strict_aliasing(invocation.strict_aliasing);
		UnitWork work;
		double front_ms = 0;
		const auto start = std::chrono::steady_clock::now();
		if (!lower_source(input.name, invocation, lowering, work, front_ms, start))
			return false;
		lowering.finish();
	}
	std::string dump = "machine_ir wasm32 1\n";
	wasm::Object object;
	if (!wasm::compile_object(program, options, object, dump, error))
		return false;
	inputs.objects.push_back(std::move(object));
	return true;
}

int wasm_mode(const Invocation& invocation)
{
	if (invocation.compile_only && invocation.inputs.size() != 1 && !invocation.output.empty())
		throw InvocationError("cannot specify -o with -c and several inputs");
	backend::Options options;
	options.target = invocation.target;
	options.output = invocation.output.empty() ? "a.wasm" : invocation.output;
	options.optimization_level = invocation.optimizer.level;
	options.exports = invocation.exports;
	options.sysroot = invocation.sysroot;
	options.resource_directory = invocation.resource_directory;
	options.cxx_runtime = true;
	options.exceptions = invocation.preprocessor.modes.exceptions;
	options.no_standard_libraries = invocation.no_standard_libraries;
	options.no_startfiles = invocation.no_startfiles;
	options.wasm_atomics = invocation.wasm_atomics;
	options.wasm_link = invocation.wasm_link;
	if (invocation.wasm_stack_size != 0)
		options.wasm_stack_size = invocation.wasm_stack_size;
	options.optimizer = &invocation.optimizer;
	std::string error;
	wasm::LinkInputs objects;
	for (const Input& input : invocation.inputs)
	{
		if (!wasm_input(input, invocation, options, objects, error))
		{
			std::cerr << "ERROR: " << error << std::endl;
			return EXIT_FAILURE;
		}
		if (invocation.compile_only)
		{
			options.output = invocation.output.empty() ? default_object_name(input.name) : invocation.output;
			check_output(invocation, options.output);
			if (!wasm::output_objects(objects, options, true, error))
			{
				std::cerr << "ERROR: " << error << std::endl;
				return EXIT_FAILURE;
			}
			objects.objects.clear();
		}
	}
	if (!invocation.compile_only)
	{
		check_output(invocation, options.output);
		if (!wasm::output_objects(objects, options, false, error))
		{
			std::cerr << "ERROR: " << error << std::endl;
			return EXIT_FAILURE;
		}
	}
	return EXIT_SUCCESS;
}

bool preprocess_unit(const std::string& path, const Invocation& invocation, PostTokenWriter& output)
{
	output.write_line("sof " + path);
	try
	{
		Preprocessor preprocessor(invocation.preprocessor, path);
		try
		{
			// The stream types literals as the course's preprocessor does.
			PostTokenizer tokens(preprocessor, nullptr,
				invocation.preprocessor.literals == NumberDialect::Gnu ? NumberDialect::GnuStream
																	   : invocation.preprocessor.literals);
			for (;;)
			{
				const PostToken& token = tokens.next();
				if (token.kind == PostTokenKind::Invalid)
				{
					std::cerr << "ERROR: " << path << ": invalid phase-7 token" << std::endl;
					return false;
				}
				output.write(token);
				if (token.kind == PostTokenKind::EndOfFile)
					return true;
			}
		}
		catch (const PreprocessError& e)
		{
			std::cerr << "ERROR: " << preprocessor.describe(e) << std::endl;
		}
		catch (const SourceError& e)
		{
			std::cerr << "ERROR: " << preprocessor.describe(e) << std::endl;
		}
	}
	catch (const PreprocessError& e)
	{
		std::cerr << "ERROR: " << path << ": " << e.what() << std::endl;
	}
	return false;
}

// -E: the PA4 preproc stream of every source, to the -o file or standard
// output; libraries and objects are link inputs.
int preprocess_mode(const Invocation& invocation)
{
	std::vector<std::string> sources;
	for (const Input& input : invocation.inputs)
	{
		if (input.kind == Input::Kind::Object || input.kind == Input::Kind::LowIR)
			throw InvocationError("-E needs C++ source inputs: " + input.name);
		if (input.kind == Input::Kind::Source)
			sources.push_back(input.name);
	}
	if (sources.size() != 1 && !invocation.output.empty())
		throw InvocationError("cannot specify -o with -E and several inputs");
	std::ofstream file;
	if (!invocation.output.empty())
	{
		check_output(invocation, invocation.output);
		file.open(invocation.output, std::ios::binary);
		if (!file)
			throw InputOutputError("cannot write " + invocation.output);
	}
	std::ostream& out = invocation.output.empty() ? std::cout : file;
	PostTokenWriter output(out);
	output.write_line("preproc " + std::to_string(sources.size()));
	for (const std::string& source : sources)
	{
		if (!preprocess_unit(source, invocation, output))
			return EXIT_FAILURE;
	}
	output.flush();
	out.flush();
	return out ? EXIT_SUCCESS : EXIT_FAILURE;
}

} // namespace

int debug_info_request(const std::string& arg)
{
	if (arg == "-g0" || arg == "-ggdb0")
		return 0;
	if (starts_with(arg, "-gno-") || starts_with(arg, "-gz") || arg == "-gcolumn-info" || arg == "-gpubnames" ||
		arg == "-gsplit-dwarf" || starts_with(arg, "-gstrict-dwarf") || arg == "-grecord-gcc-switches")
		return -1;
	return 1;
}

int debug_type_request(const std::string& arg)
{
	const int level = debug_info_request(arg);
	if (level <= 0)
		return level;
	return arg == "-gline-tables-only" || arg == "-g1" || arg == "-ggdb1" ? 0 : 1;
}

int run_toolchain(const std::vector<std::string>& args, const PreprocessorOptions& base)
{
	const Invocation invocation = parse(args, base);
	if (!target_model::is_wasm(invocation.target) && !invocation.exports.empty())
		throw InvocationError("explicit function exports require a Wasm target");
	if (invocation.preprocess_only)
		return preprocess_mode(invocation);
	if (invocation.syntax_only)
	{
		for (const Input& input : invocation.inputs)
		{
			native::object::ObjectFile object;
			if (input.kind == Input::Kind::Source &&
				!compile_unit(input.name, invocation, native::EhModel::Host, object))
				return EXIT_FAILURE;
		}
		return EXIT_SUCCESS;
	}
	if (target_model::is_wasm(invocation.target))
		return wasm_mode(invocation);
	return invocation.compile_only ? compile_mode(invocation) : link_mode(invocation);
}

int run_query(const std::string& flag)
{
	std::cout << host_query(flag);
	std::cout.flush();
	return std::cout ? EXIT_SUCCESS : EXIT_FAILURE;
}

} // namespace driver
