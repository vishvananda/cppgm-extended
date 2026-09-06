#pragma once

#include "native/mir/model.h"

#include <string>

namespace mir_model {

// Render the machine-IR program as the text --dump-machine-ir writes;
// write_mir_program_file writes it to a path.
std::string serialize_mir_program(const MirProgram & program);
void write_mir_program_file(const std::string & path,
                            const MirProgram & program);

}  // namespace mir_model
