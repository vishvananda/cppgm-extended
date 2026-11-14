#pragma once

#include "backend/options.h"

namespace driver {

bool parse_wasm_link_option(const std::vector<std::string>& args, size_t& index, backend::WasmLinkOptions& options);

} // namespace driver
