#pragma once

#include <string>
#include <vector>

#include "preprocess/preprocessor.h"

// cppgm++'s compile (-c) and link modes: each C++ source is its own
// translation unit, compiled through the whole pipeline (preprocessing,
// parsing, semantic analysis, LowIR lowering, the native back end) into a
// relocatable object; link mode compiles its source inputs the same way,
// reads its object inputs (.o, .obj) and the -l libraries found in the -L
// directories (lib<name>.o, lib<name>.obj), and links them all with the
// runtime library into a static executable (native/link/linker.h).
//
// -E preprocesses each source in hosted mode into the PA4 preproc
// stream (`preproc <n>`, then `sof`/posttoken/`eof` records per source).
//
// Options: -c, -E, -o <file>, -I <dir>, -isystem <dir>, -D <name>[=<value>],
// -U <name>, -L <dir>, -l <name> (each also joined), -include <file>,
// -nostdinc, -x c++, --target <target> (or =; the Linux x86-64 names only),
// -std=<c++11, c++14 or c++17, and their gnu and draft names> (__cplusplus;
// c++NN the strict ISO dialect's host macros, __STRICT_ANSI__),
// -stdlib=<the build's selection>, -f(no-)exceptions and -f(no-)rtti (the
// hosted predefined macros and feature probes), --stats; -O*, -g*, -W*,
// other -f* and -m*, -pedantic(-errors), -w, -pipe and dependency-file
// options are accepted and do not change the output.
//
// Objects: -c writes a course object for an output named *.obj (the
// course's EH runtime ABI, PA25's link mode input) and a host object
// otherwise (host EH tables and runtime names, pa26: linked by the host
// toolchain); link mode compiles its sources as course objects.
namespace driver {

// Runs a preprocess (-E), compile or link invocation; `base` supplies
// __DATE__ and friends. Returns the process exit status.
int run_toolchain(const std::vector<std::string>& args, const PreprocessorOptions& base);

// A direct driver query (--version, -v, -dumpmachine, -dumpversion,
// -print-search-dirs, PA29), answered on standard output.
int run_query(const std::string& flag);

} // namespace driver
