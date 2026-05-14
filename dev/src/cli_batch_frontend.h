#pragma once

#include <functional>
#include <iosfwd>
#include <string>
#include <vector>

using CliFrontendRunner = std::function<int(const std::vector<std::string> &)>;

int run_cli_frontend(int argc,
                     char ** argv,
                     const CliFrontendRunner & runner);
