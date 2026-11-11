#pragma once

#include "preprocess/preprocessor.h"

namespace driver {

void configure_wasm_environment(PreprocessorOptions& options);
void add_wasm_headers(PreprocessorOptions& options, const std::string& sysroot, const std::string& resource_directory);

} // namespace driver
