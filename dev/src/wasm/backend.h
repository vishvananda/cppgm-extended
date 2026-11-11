#pragma once

#include "backend/options.h"
#include "lowir/program.h"

namespace wasm {

bool compile_program(lowir::Program& program, const backend::Options& options, std::string& error);

} // namespace wasm
