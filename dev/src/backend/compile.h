#pragma once

#include "backend/options.h"
#include "lowir/program.h"

namespace backend {

bool compile_program(lowir::Program& program, const Options& options, std::string& error, std::string* stats_line);

} // namespace backend
