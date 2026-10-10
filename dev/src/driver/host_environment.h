#pragma once

#include "preprocess/preprocessor.h"

namespace driver {

// The hosted environment of cppgm++'s preprocess, compile and link modes
// (PA26's hosted headers, PA29): the host C++ compiler's predefined macros
// (gnu++11) and standard include directories, discovered once when cppgm++
// was built (dev/gen_builtin_host_config.pl) and baked in, so the answer
// does not depend on the compiler installed where cppgm++ runs. Turns on
// hosted mode, sets the builtin declarations and, unless `nostdinc`, appends the standard directories
// after any -isystem ones. The host's macros announcing exceptions
// (__EXCEPTIONS, __cpp_exceptions) and RTTI (__GXX_RTTI, __cpp_rtti) are
// read only in the modes that have them (options.modes).
void add_host_environment(PreprocessorOptions& options, bool nostdinc);

// Whether `-stdlib=<name>` agrees with the standard library selection
// cppgm++ was built with (CPPGM_STDLIB_FLAGS): only a selection made then
// can be named; a build for the host's default library names none.
bool host_stdlib_selected(const std::string& name);

// The direct driver queries (--version, -v, -dumpmachine, -dumpversion,
// -print-search-dirs): this compiler's version line, then the host
// toolchain's answers as cppgm++ was built against them.
std::string host_query(const std::string& flag);

} // namespace driver
