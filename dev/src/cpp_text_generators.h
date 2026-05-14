#pragma once

#include <string>
#include <vector>

#include "callsemantic.h"

std::string generate_cppast_translation_units(
    const std::vector<std::string> & srcfiles);
std::string generate_types_translation_units(
    const std::vector<std::string> & srcfiles);
std::string generate_calls_translation_units(
    const std::vector<std::string> & srcfiles);
