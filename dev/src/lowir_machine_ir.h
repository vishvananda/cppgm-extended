#pragma once

#include <string>
#include <vector>

#include "lowir_model.h"
#include "mir_model.h"

mir_model::MirProgram build_lowir_machine_ir(const std::vector<std::string> & srcfiles,
                                             const std::string & output_target);
mir_model::MirProgram build_lowir_machine_ir(const lowir_model::LowirProgram & program,
                                             const std::string & output_target);
mir_model::MirProgram build_lowir_machine_ir_object(const std::vector<std::string> & srcfiles,
                                                    const std::string & output_target,
                                                    bool enable_host_eh = false);
mir_model::MirProgram build_lowir_machine_ir_object(
    const lowir_model::LowirProgram & program,
    const std::string & output_target,
    bool enable_host_eh = false);
