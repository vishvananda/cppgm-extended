#pragma once

#include "lowir/model/program.h"

namespace lowir_intro {

// Construct the introductory programs as typed IR, independently of parsing.
lowir_model::Program BuildExercise(const std::string & name);

}  // namespace lowir_intro
