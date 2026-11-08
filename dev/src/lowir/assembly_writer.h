#pragma once

#include <string>

#include "lowir/program.h"

namespace lowir {

void append_assembly_reference(std::string& out, const Program& program, uint32_t id);
void append_assembly_definitions(std::string& out, const Program& program);

} // namespace lowir
