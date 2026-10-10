#pragma once

#include <string>
#include <vector>

#include "lowir/program.h"
#include "support/telemetry.h"

namespace lowir {

// Work of one read (telemetry, `lowir --stats`).
struct ReadStatistics
{
	telemetry::Counter source_bytes = 0;
	telemetry::Milliseconds parse_ms = 0;
	telemetry::Milliseconds validate_ms = 0;
};

// Reads LowIR files, in order, as one unit into `program`, applies the
// legacy entry/init/fini roles (lowir.md "Reserved Runtime Hooks") and
// validates the result (lowir/validator.h). Names are shared across the
// files, so a symbol may be defined in a later file than its first use.
// Returns false with a diagnostic naming the source position on the first
// syntax, structural or validity error.
bool read_program_files(
	Program& program, const std::vector<std::string>& paths, std::string& error, ReadStatistics* statistics = nullptr);

// The same for one in-memory text.
bool read_program_text(Program& program, const std::string& text, const std::string& source_name, std::string& error);

} // namespace lowir
