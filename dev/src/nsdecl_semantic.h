#pragma once

#include <string>
#include <vector>

#include "recog_parser.h"

std::string describe_nsdecl_translation_unit(IRecogTokenSequence & tokens);
std::string describe_nsdecl_translation_unit(const std::vector<RecogToken> & tokens);
