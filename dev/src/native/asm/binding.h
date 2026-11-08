#pragma once

#include <memory>
#include <vector>

#include "asm/parser.h"
#include "lowir/assembly.h"
#include "native/mir/registers.h"

namespace native {

struct AssemblyPlan
{
	uint32_t definition = 0;
	uint32_t reads = 0;
	uint32_t writes = 0; // declared clobbers and operand setup destinations
	std::vector<mir_model::Reg> registers;
	std::vector<assembly::Instruction> instructions;
};

struct AssemblySites
{
	std::vector<std::shared_ptr<const AssemblyPlan>> plans;
};

std::shared_ptr<const AssemblyPlan> bind_assembly(const lowir::AssemblyModule& module, uint32_t definition);

} // namespace native
