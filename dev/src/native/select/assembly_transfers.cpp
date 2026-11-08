#include "native/select/function_selector.h"

#include "native/backend_error.h"

namespace native {
void FunctionSelector::assembly_transfer(Reg reg, const Operand& memory, bool store)
{
	if (!is_xmm(reg) || memory.kind != OperandKind::Mem)
		throw BackendError("invalid wide assembly transfer location");
	if (!out_->assembly)
		out_->assembly.reset(new AssemblySites());
	std::shared_ptr<AssemblyPlan> plan(new AssemblyPlan());
	plan->transfer_bytes = 32;
	plan->transfer_store = store;
	plan->transfer_register = asm_model::Register{ asm_model::RegisterBank::Vector, static_cast<uint8_t>(reg - XMM0),
		256, 0 };
	const uint32_t site = static_cast<uint32_t>(out_->assembly->plans.size());
	out_->assembly->plans.push_back(plan);
	Inst& effect = emit2(Op::Asm, lowir::TypeKind::Void, Operand::make_imm(site), memory);
	effect.aux = store ? reg_bit(reg) : 0;
	effect.aux2 = store ? 0 : reg_bit(reg);
	if (is_gpr(memory.reg))
		effect.aux |= reg_bit(memory.reg);
	if (is_gpr(memory.index))
		effect.aux |= reg_bit(memory.index);
}
Operand FunctionSelector::wide_assembly_input(const lowir::AssemblyOperand& operand, lowir::Value value)
{
	using lowir::TypeKind;
	Operand source = memory_value_operand(
		value, operand.input_storage ? lowir::Type::scalar(TypeKind::Ptr) : operand.type);
	if (operand.input_storage)
	{
		load_into(R10, source, TypeKind::Ptr);
		source = Operand::make_mem(R10, 0);
	}
	if (source.kind != OperandKind::Mem)
		throw BackendError("wide assembly input requires stored bytes");
	const Operand saved = Operand::make_frame(frame_alloc(32, 16));
	// Capture the complete input before installing any assembly register.
	// A pointer and an object value share this transfer without scalar proxies.
	assembly_transfer(XMM15, source, false);
	assembly_transfer(XMM15, saved, true);
	return saved;
}
} // namespace native
