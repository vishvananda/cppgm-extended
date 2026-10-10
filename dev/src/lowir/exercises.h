#pragma once

#include <string>

#include "lowir/program.h"

namespace lowir {

// Builds one PA8 construction exercise into an empty program through the
// model's builder: "sum" (@sum_to), "swap" (@swap_values) or "call"
// (@call_twice). Returns false for an unknown exercise name.
bool build_exercise(Program& program, const std::string& exercise);

} // namespace lowir
