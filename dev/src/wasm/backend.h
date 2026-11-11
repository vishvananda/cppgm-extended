#pragma once

#include "backend/options.h"
#include "lowir/program.h"
#include "wasm/object.h"

namespace wasm {

bool compile_object(
	lowir::Program& program, const backend::Options& options, Object& object, std::string& dump, std::string& error);
bool output_objects(
	const std::vector<Object>& objects, const backend::Options& options, bool relocatable, std::string& error);
bool compile_program(lowir::Program& program, const backend::Options& options, std::string& error);

} // namespace wasm
