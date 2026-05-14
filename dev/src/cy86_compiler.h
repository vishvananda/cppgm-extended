#pragma once

#include <string>
#include <vector>

#include "posttokenizer.h"

std::vector<PostToken> tokenize_cy86_translation_unit(
    const std::string & srcfile);

void build_cy86_program(const std::vector<std::string> & srcfiles,
                        const std::string & outfile,
                        const std::string & output_target);
