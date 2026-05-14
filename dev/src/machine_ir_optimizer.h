#pragma once

#include "machine_ir.h"

machine_ir::Program optimize_machine_ir_program(const machine_ir::Program & program,
                                                int optimization_level);
