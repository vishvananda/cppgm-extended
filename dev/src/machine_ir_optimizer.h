#pragma once

#include "mir_model.h"

mir_model::MirProgram optimize_machine_ir_program(mir_model::MirProgram program,
                                                  int optimization_level);
