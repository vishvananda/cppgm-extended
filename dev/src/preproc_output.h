#pragma once

#include <iosfwd>
#include <string>
#include <vector>

#include "cpp_preprocess_options.h"

void write_preprocessed_posttoken_output(
    std::ostream & out,
    const std::vector<std::string> & srcfiles,
    const CppPreprocessOptions & options = CppPreprocessOptions());
void write_preprocessed_posttoken_output_file(
    const std::string & outfile,
    const std::vector<std::string> & srcfiles,
    const CppPreprocessOptions & options = CppPreprocessOptions());
