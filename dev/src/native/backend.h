#pragma once

#include <string>
#include <vector>

#include "lowir/program.h"

// LowIR program -> MIR -> native executable (the lowir2native pipeline).
namespace native {

struct BackendOptions
{
	std::string output;     // executable path, or empty
	std::string machine_ir; // MIR dump path, or empty
	int optimization_level = 0;
	bool stats = false;
	// Set (when given) if the executable failed for a symbol the standalone
	// runtime does not define (link mode's runtime library may: catch
	// clauses, the C++ ABI runtime, RTTI, i128 helpers).
	bool* undefined_symbol = nullptr;
};

// Lowers a read and validated program. Returns false with `error` set
// for LowIR outside the supported native subset or an I/O failure; an
// InternalError (a broken invariant of the back end) propagates.
bool compile_program(
	const lowir::Program& program, const BackendOptions& options, std::string& error, std::string* stats_line);

} // namespace native
