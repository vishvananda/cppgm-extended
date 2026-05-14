#pragma once

#include <string>
#include <vector>

#include "recog_token_buffer.h"
#include "recog_token.h"

std::string describe_types_translation_unit(IRecogTokenSequence & tokens);
std::string describe_types_translation_unit(const std::vector<RecogToken> & tokens);
