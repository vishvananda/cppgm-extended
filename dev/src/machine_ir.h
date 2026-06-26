#pragma once

#include "mir_model.h"

namespace machine_ir {

using namespace mir_model;

const char * register_text(X64Register reg);
std::string dump_program(const Program & program);

}  // namespace machine_ir
