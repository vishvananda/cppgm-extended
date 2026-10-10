#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "native/object/object_file.h"
#include "support/telemetry.h"

// Link mode's static linker: relocatable objects (this compiler's and
// host-compiled helpers, in command-line order) and the runtime library
// members they need -> one static x86-64 Linux executable.
//
// - COMDAT groups: the first group with a signature is kept, later ones
//   are discarded whole (their symbols become references).
// - Symbols: a global definition (strong) takes precedence over weak ones
//   and over common symbols; two strong definitions of a name are an
//   error; a strong reference no input or runtime member defines is an
//   error; a weak reference left undefined is address 0.
// - The runtime library (native/link/runtime_library.h) defines, as weak
//   symbols, what the inputs still need: the process entry, which runs
//   every .init_array entry in link order, calls `main` (which must be
//   defined), then the exit handlers and the .fini_array entries last to
//   first; the exception runtime; allocation; type information.
// - Layout: one segment per protection class (code; constants and the
//   startup arrays; data and zero-filled storage), sections in input
//   order; GOT-relative references get a slot each in the data segment.
// - Relocations: the x86-64 psABI absolute, PC-relative, PLT and GOT
//   forms, range checked.
namespace native {

struct LinkStats
{
	telemetry::Counter objects = 0;
	telemetry::Counter sections = 0;
	telemetry::Counter discarded_groups = 0;
	telemetry::Counter symbols = 0;
	telemetry::Counter relocations = 0;
	telemetry::Counter got_slots = 0;
	telemetry::Counter runtime_members = 0;
	telemetry::Counter text_bytes = 0;
	telemetry::Counter rodata_bytes = 0;
	telemetry::Counter data_bytes = 0;
	telemetry::Counter bss_bytes = 0;
	telemetry::Milliseconds runtime_ms = 0;
	telemetry::Milliseconds link_ms = 0;
};

// Links `objects` into the executable `output`. Returns false with
// `error` set for duplicate or unresolved symbols, a missing `main`,
// unsupported relocations or an I/O failure.
bool link_executable(
	std::vector<object::ObjectFile>& objects, const std::string& output, std::string& error, LinkStats* stats);

} // namespace native
