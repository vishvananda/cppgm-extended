#pragma once

#include <string>

#include "target_model.h"

namespace target {

using namespace target_model;

// Parses supported target identities, preserving the native CLI aliases.
bool parse(const std::string& text, Target& result);

} // namespace target
