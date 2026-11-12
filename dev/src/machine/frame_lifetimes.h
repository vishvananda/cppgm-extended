#pragma once

#include "lowir/program.h"

namespace machine {

// Complete object copies consumed by one by-value call in one block.
// Address escape, partial writes, EH and dynamic-stack operations pin storage.
// A nonempty result maps each eligible slot to an earlier owner of its type.
std::vector<uint32_t> sequential_copy_slots(const lowir::Program& program, const lowir::FunctionBody& body);

} // namespace machine
