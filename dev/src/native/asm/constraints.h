#pragma once

#include <vector>

#include "lowir/assembly.h"
#include "lowir/program.h"
#include "native/mir/registers.h"

namespace native {

struct AssemblyBinding
{
	std::vector<mir_model::Reg> registers;
	std::vector<uint8_t> locations;
	std::vector<lowir::Literal> constants;
};

AssemblyBinding assign_assembly_operands(const lowir::AssemblyModule& module, const lowir::AssemblyTemplate& definition,
	uint32_t clobbers, const lowir::FunctionBody& body, const std::vector<lowir::Value>& arguments);

} // namespace native
