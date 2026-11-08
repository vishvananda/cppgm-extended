#pragma once

#include <memory>
#include <vector>

#include "asm/parser.h"
#include "lowir/assembly.h"
#include "native/mir/registers.h"
#include "native/asm/constraints.h"

namespace native {

struct AssemblyPlan : AssemblyBinding
{
	uint32_t definition = 0;
	uint32_t reads = 0;
	uint32_t writes = 0; // declared clobbers and operand setup destinations
	std::vector<assembly::Instruction> instructions;
};

struct AssemblySites
{
	std::vector<std::shared_ptr<const AssemblyPlan>> plans;
};

std::shared_ptr<const AssemblyPlan> bind_assembly(const lowir::AssemblyModule& module, uint32_t definition,
	const lowir::FunctionBody& body, const std::vector<lowir::Value>& arguments);

} // namespace native
