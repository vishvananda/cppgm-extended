#pragma once

#include <functional>
#include <iosfwd>
#include <string>
#include <vector>

using CppTextGenerator = std::function<std::string(const std::vector<std::string> &)>;

int run_cpp_text_frontend(int argc,
                          char ** argv,
                          const CppTextGenerator & generator);
