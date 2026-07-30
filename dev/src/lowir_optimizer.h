#pragma once

#include <string>
#include <vector>

#include "lowir_model.h"

lowir_model::LowirProgram optimize_lowir_program(
    lowir_model::LowirProgram program,
    int optimization_level);
lowir_model::LowirProgram inline_required_lowir_calls(
    lowir_model::LowirProgram program);
std::string optimize_lowir_text(const std::vector<std::string> & srcfiles,
                                int optimization_level);
