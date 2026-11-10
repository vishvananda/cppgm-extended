#pragma once

#include <memory>
#include <vector>

#include "asm/parser.h"
#include "lowir/assembly.h"
#include "native/mir/registers.h"
#include "native/target.h"
#include "native/asm/constraints.h"

namespace native {

class AssemblyEnvironment;

inline bool wide_assembly_operand(lowir::Type type)
{
	return type.is_object() && (type.bytes == 32 || type.bytes == 64);
}

inline bool assembly_transfer_required(lowir::Type type, mir_model::Reg reg)
{
	return wide_assembly_operand(type) || (assembly_vector_register(reg) && reg >= 32) || assembly_mask_register(reg);
}

struct AssemblyPlan : AssemblyBinding
{
	uint32_t definition = 0;
	asm_model::Register transfer_register;
	uint16_t transfer_bytes = 0;
	bool transfer_store = false;
	uint32_t exit_operand = lowir::kNoId;
	uint32_t reads = 0;
	uint32_t writes = 0; // declared clobbers and operand setup destinations
	std::vector<assembly::Instruction> instructions;
};

struct AssemblySites
{
	std::vector<std::shared_ptr<const AssemblyPlan>> plans;
};

std::shared_ptr<AssemblyPlan> prepare_assembly_source(const lowir::AssemblyModule& module, uint32_t definition,
	const lowir::FunctionBody& body, const std::vector<lowir::Value>& arguments, std::string& source,
	std::vector<assembly::SymbolUse>& symbols, std::vector<assembly::LabelUse>& labels);

void append_assembly_exits(AssemblyPlan& plan, const lowir::AssemblyTemplate& definition);

std::shared_ptr<const AssemblyPlan> bind_assembly(const lowir::AssemblyModule& module, uint32_t definition,
	const lowir::FunctionBody& body, const std::vector<lowir::Value>& arguments, AssemblyEnvironment& environment,
	Target target);

} // namespace native
