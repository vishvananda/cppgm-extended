#pragma once

#include <string>
#include <vector>

#include "cy86_internal.h"
#include "lowir_internal.h"

cy86_internal::Program build_lowir_cy86_program(const std::vector<std::string> & srcfiles);
cy86_internal::Program build_lowir_cy86_program(const lowir_internal::Program & program);
