#pragma once

#include <string>

bool parse_optimization_level_arg(const std::string & arg, int & level);
int normalize_optimization_level(int level);
