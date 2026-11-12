#pragma once

#include <string>

#include "wasm/bytes.h"
#include "wasm/mir.h"

namespace wasm {

void encode_function(const Function& function, Bytes& output, std::vector<Relocation>* relocations = nullptr);

} // namespace wasm
