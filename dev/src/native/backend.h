#pragma once

#include <string>
#include <vector>

#include "lowir/program.h"
#include "native/target.h"
#include "backend/options.h"

// LowIR program -> MIR -> native executable (the lowir2native pipeline).
namespace native {

using BackendOptions = backend::Options;

// Lowers a read and validated program. Returns false with `error` set
// for LowIR outside the supported native subset or an I/O failure; an
// InternalError (a broken invariant of the back end) propagates.
bool compile_program(
	lowir::Program& program, const BackendOptions& options, std::string& error, std::string* stats_line);

} // namespace native
