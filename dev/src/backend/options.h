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
	const opt::OptimizerOptions* optimizer = nullptr;
};

} // namespace backend
