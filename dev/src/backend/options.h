#pragma once

#include <string>
#include <vector>

#include "target_model.h"

namespace opt {
struct OptimizerOptions;
}

namespace backend {

struct Options
{
	target_model::Target target = target_model::Target::Linux;
	std::string output;
	std::string machine_ir;
	int optimization_level = 0;
	bool stats = false;
	bool* undefined_symbol = nullptr;
	std::vector<std::string> exports;
	std::vector<std::string> link_inputs;
	std::string sysroot;
	std::string resource_directory;
	bool no_standard_libraries = false;
	bool no_startfiles = false;
	const opt::OptimizerOptions* optimizer = nullptr;
};

} // namespace backend
