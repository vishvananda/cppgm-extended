#include "native/asm/binding.h"
#include "native/backend_error.h"

namespace native {
namespace {
void tag(std::vector<assembly::Instruction>& instructions, uint32_t value)
{
	assembly::Instruction inst;
	inst.is_exit = true;
	inst.exit_tag = value;
	instructions.push_back(inst);
}
void jump(std::vector<assembly::Instruction>& instructions)
{
	assembly::Instruction inst;
	inst.mnemonic = "jmp";
	assembly::Operand target;
	target.kind = assembly::OperandKind::Label;
	inst.operands.push_back(target);
	instructions.push_back(inst);
}
void label(std::vector<assembly::Instruction>& instructions)
{
	assembly::Instruction inst;
	inst.label = true;
	instructions.push_back(inst);
}
} // namespace

void append_assembly_exits(AssemblyPlan& plan, const lowir::AssemblyTemplate& definition)
{
	if (plan.exit_operand == lowir::kNoId)
		return;
	std::vector<assembly::Instruction>& instructions = plan.instructions;
	std::vector<uint32_t> targets(definition.operand_count, lowir::kNoId), joins;
	tag(instructions, 0);
	joins.push_back(static_cast<uint32_t>(instructions.size()));
	jump(instructions);
	for (uint32_t i = 0; i < definition.operand_count; ++i)
		if (plan.locations[i] == asm_model::AL_LABEL)
		{
			targets[i] = static_cast<uint32_t>(instructions.size());
			label(instructions);
			tag(instructions, static_cast<uint32_t>(plan.constants[i].low));
			joins.push_back(static_cast<uint32_t>(instructions.size()));
			jump(instructions);
		}
	const uint32_t end = static_cast<uint32_t>(instructions.size());
	label(instructions);
	for (uint32_t join : joins)
		instructions[join].operands[0].label = end;
	for (assembly::Instruction& inst : instructions)
		for (assembly::Operand& operand : inst.operands)
			if (operand.external_label)
			{
				if (operand.label >= targets.size() || targets[operand.label] == lowir::kNoId)
					throw BackendError("invalid outgoing assembly label reference");
				operand.label = targets[operand.label];
				operand.external_label = false;
			}
}
} // namespace native
