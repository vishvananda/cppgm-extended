#pragma once

#include <string>
#include <vector>

#include "native/object/object_file.h"

// Link mode's runtime library: the support code and data a program's
// objects refer to without defining, generated on demand per link as one
// relocatable object whose public symbols are weak (a program's own
// definitions win). Members:
// - the process entry `_start` (always) and `__cppgm_exit`, the startup
//   and shutdown order of native/link/linker.h, `atexit`/`__cxa_atexit`;
// - the exception runtime: the handler-stack state and dispatcher
//   (native/runtime/runtime.h), the clause routine `eh_catch` uses,
//   `__cxa_allocate_exception`/`free_exception`/`throw`/`begin_catch`/
//   `end_catch`/`rethrow`, `std::terminate`, `__cxa_pure_virtual`;
// - allocation (`cppgm_builtin_operator_new`/`delete`, their array forms
//   and the Itanium operator new/delete names) and the string builtins;
// - type information: the vtables of the Itanium type_info classes,
//   fundamental types' type_info objects and pointers to them, and
//   `__dynamic_cast`.
// Machine-level members (entry, system calls, dispatch, builtins, data)
// are encoded directly; the others are LowIR built here and compiled by
// the object back end.
namespace native {

// Symbols the linker defines for the runtime: the bounds of the
// concatenated .init_array and .fini_array sections.
extern const char kInitArrayStart[];
extern const char kInitArrayEnd[];
extern const char kFiniArrayStart[];
extern const char kFiniArrayEnd[];
extern const char kEntrySymbol[];

// Whether the library defines `name`.
bool runtime_provides(const char* name);

// Builds the members defining `names` (each one runtime_provides accepts)
// and the process entry, with the members they depend on, as objects
// appended to `out`.
bool build_runtime(const std::vector<std::string>& names, std::vector<object::ObjectFile>& out, std::string& error);

} // namespace native
