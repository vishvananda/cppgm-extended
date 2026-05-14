#pragma once

#include <string>
#include <vector>

#include "lowir_internal.h"
#include "machine_ir.h"

machine_ir::Program build_lowir_machine_ir(const std::vector<std::string> & srcfiles,
                                           const std::string & output_target);
machine_ir::Program build_lowir_machine_ir(const lowir_internal::Program & program,
                                           const std::string & output_target);
machine_ir::Program build_lowir_machine_ir_object(const std::vector<std::string> & srcfiles,
                                                  const std::string & output_target,
                                                  bool enable_host_eh = false);
machine_ir::Program build_lowir_machine_ir_object(const lowir_internal::Program & program,
                                                  const std::string & output_target,
                                                  bool enable_host_eh = false);
