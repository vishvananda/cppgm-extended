#pragma once

// The --help texts of the course tools, as the course solution prints them.
// The scaffolds print the same text; the "diagnostic" and "audit" flags are
// the course solution's own and are not part of any assignment's contract.
// The course solution builds on this header exactly as it is shipped.

inline const char* abimangle_help_text()
{
	return "usage: abimangle -o <outfile> <abi-facts-file>...\n"
		   "\n"
		   "query flags:\n"
		   "  --help, -h\n"
		   "\n"
		   "diagnostic flags:\n"
		   "  --stats  work counters, mangle time and peak RSS on standard error\n";
}

inline const char* cppgm_help_text()
{
	return "usage: cppgm++ --emit-ast -o <outfile> <srcfile>...\n"
		   "   or: cppgm++ --emit-types -o <outfile> <srcfile>...\n"
		   "   or: cppgm++ --emit-semantics -o <outfile> <srcfile>...\n"
		   "   or: cppgm++ --emit-lowir -o <outfile> <srcfile>...\n"
		   "   or: cppgm++ -c [options] <srcfile-or-lowirfile>...\n"
		   "   or: cppgm++ -E [options] <srcfile>...\n"
		   "   or: cppgm++ [options] <input>...\n"
		   "\n"
		   "emit-mode flags (every --emit-* mode):\n"
		   "  -I <dir> / -I<dir>  user include search path\n"
		   "  -D <macro> / -D<macro>\n"
		   "  -U <macro> / -U<macro>\n"
		   "  --hosted            the host compiler's include paths and predefined macros, as -c uses\n"
		   "  -O0 / -O1 / -O2 / -O3, --inline-limit  (--emit-lowir) the LowIR optimizer\n"
		   "  -g0 / -gline-tables-only  (--emit-lowir) with an output named *.lowir, LowIR in the\n"
		   "                      object dialect `cppgm++ -c` reads back (else the course dialect)\n"
		   "\n"
		   "query flags:\n"
		   "  --help, -h\n"
		   "  --version\n"
		   "  -v\n"
		   "  -dumpmachine\n"
		   "  -dumpversion\n"
		   "  -print-search-dirs\n"
		   "\n"
		   "driver flags:\n"
		   "  -E                  preprocess only\n"
		   "  -c                  compile only\n"
		   "  -o <outfile>        explicit output path\n"
		   "  -I <dir> / -I<dir>  user include search path\n"
		   "  -isystem <dir>      system include search path\n"
		   "  -D <macro> / -D<macro>\n"
		   "  -U <macro> / -U<macro>\n"
		   "  -include <file>\n"
		   "  -L <dir> / -L<dir>\n"
		   "  -l <name> / -l<name>\n"
		   "  -O0 / -O1 / -O2 / -O3 (default: -O0)\n"
		   "  --inline-limit <name>=<positive-integer>  an inliner limit (also --emit-lowir)\n"
		   "  -g0 / -gline-tables-only\n"
		   "  -stdlib <name> / -stdlib=<name>\n"
		   "  --target <target>\n"
		   "\n"
		   "diagnostic flags:\n"
		   "  --stats             work counters, phase times and peak RSS on standard error\n"
		   "  --stats-functions   each compiled function's native census on standard error\n";
}

inline const char* lowir_help_text()
{
	return "usage: lowir -o <outfile> <lowirfile>...\n"
		   "   or: lowir --exercise <sum|swap|call> -o <outfile>\n"
		   "\n"
		   "query flags:\n"
		   "  --help, -h\n"
		   "\n"
		   "required flags:\n"
		   "  -o <outfile>\n"
		   "\n"
		   "diagnostic flags:\n"
		   "  --stats  work counters, phase times and peak RSS on standard error\n";
}

inline const char* lowir2native_help_text()
{
	return "usage: lowir2native -o <outfile> <lowirfile>...\n"
		   "   or: lowir2native -O0 -o <outfile> <lowirfile>...\n"
		   "   or: lowir2native -O1 -o <outfile> <lowirfile>...\n"
		   "   or: lowir2native -O2 -o <outfile> <lowirfile>...\n"
		   "   or: lowir2native -O3 -o <outfile> <lowirfile>...\n"
		   "   or: lowir2native --dump-machine-ir <mirfile> <lowirfile>...\n"
		   "   or: lowir2native -o <outfile> --dump-machine-ir <mirfile> <lowirfile>...\n"
		   "\n"
		   "query flags:\n"
		   "  --help, -h\n"
		   "\n"
		   "driver flags:\n"
		   "  -O0 / -O1 / -O2 / -O3\n"
		   "  -o <outfile>\n"
		   "  --target <target>\n"
		   "  --stats\n"
		   "  --dump-machine-ir <mirfile>\n"
		   "  --dump-native-plan <mirfile>\n";
}

inline const char* lowiropt_help_text()
{
	return "usage: lowiropt -O0 -o <outfile> <lowirfile>...\n"
		   "   or: lowiropt -O1 -o <outfile> <lowirfile>...\n"
		   "   or: lowiropt -O2 -o <outfile> <lowirfile>...\n"
		   "   or: lowiropt -O3 -o <outfile> <lowirfile>...\n"
		   "\n"
		   "query flags:\n"
		   "  --help, -h\n"
		   "\n"
		   "required flags:\n"
		   "  -O0 / -O1 / -O2 / -O3\n"
		   "  -o <outfile>\n"
		   "\n"
		   "diagnostic flags:\n"
		   "  --inline-limit <name>=<positive-integer>  caller-budget, once-cap, once-caller-budget, hint-late-cap\n"
		   "  --stats\n"
		   "\n"
		   "audit flags:\n"
		   "  --validate        validate the optimized program before writing it\n"
		   "  --verify-passes   validate the program after every optimization pass\n";
}
