#pragma once

#include <string>
#include <vector>

#include "callsemantic.h"
#include "lowir_internal.h"

lowir_internal::Program build_lowir_program(
    const std::vector<CallSemNode> & translation_units,
    bool validate_closure = false,
    bool emit_runtime_support = false,
    bool enable_debug_value_names = false);
