#pragma once

#include <string>
#include <vector>

#include "lowir_model.h"
#include "machine_object.h"
#include "mir_model.h"

machine_object::ObjectFile build_machine_object(lowir_model::LowirProgram program,
                                                const std::string & output_target,
                                                bool enable_host_eh = false,
                                                bool use_macos_static_init_sections = false,
                                                int debug_info_level = 0,
                                                int optimization_level = 0,
                                                bool use_direct_native_tls_abi = false);
machine_object::ObjectFile build_machine_object(const mir_model::MirProgram & program,
                                                int debug_info_level = 0,
                                                bool use_direct_native_tls_abi = false);
machine_object::ObjectFile build_machine_object(const std::vector<std::string> & srcfiles,
                                                const std::string & output_target,
                                                bool enable_host_eh = false,
                                                bool use_macos_static_init_sections = false,
                                                int debug_info_level = 0,
                                                int optimization_level = 0,
                                                bool use_direct_native_tls_abi = false);
void write_lowir_object_file(const std::vector<std::string> & srcfiles,
                             const std::string & outfile,
                             const std::string & output_target);
