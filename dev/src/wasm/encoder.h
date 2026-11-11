#pragma once

#include <string>

#include "wasm/bytes.h"
#include "wasm/mir.h"

namespace wasm {

void encode_function(const Function& function, Bytes& output, std::vector<Relocation>* relocations = nullptr);
void dump_function(const Function& function, const char* name, std::string& output);

} // namespace wasm
