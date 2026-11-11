#pragma once

#include "wasm/module.h"

namespace wasm {

void validate_function(const Function& function, const Signature& signature, const Module& module);

} // namespace wasm
