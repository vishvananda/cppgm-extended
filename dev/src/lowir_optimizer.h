#pragma once

#include <string>
#include <vector>

#include "lowir_internal.h"

lowir_internal::Program optimize_lowir_program(const lowir_internal::Program & program,
                                               int optimization_level);
std::string optimize_lowir_text(const std::vector<std::string> & srcfiles,
                                int optimization_level);
