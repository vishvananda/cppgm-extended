#pragma once

#include <vector>

#include "lowir/assembly.h"
#include "native/mir/registers.h"

namespace native {

std::vector<mir_model::Reg> assign_assembly_registers(
	const lowir::AssemblyModule& module, const lowir::AssemblyTemplate& definition, uint32_t clobbers);

} // namespace native
