#include "native/select/function_selector.h"

#include "native/backend_error.h"

namespace native {
void FunctionSelector::assembly_transfer(Reg reg, const Operand& memory, bool store, unsigned bytes)
{
	const bool mask = assembly_mask_register(reg);
	if ((!assembly_vector_register(reg) && !mask) || memory.kind != OperandKind::Mem ||
		(mask ? (bytes != 1 && bytes != 2 && bytes != 4 && bytes != 8)
			  : (bytes != 4 && bytes != 8 && bytes != 16 && bytes != 32 && bytes != 64)))
		throw BackendError("invalid wide assembly transfer location");
	if (!out_->assembly)
		out_->assembly.reset(new AssemblySites());
	std::shared_ptr<AssemblyPlan> plan(new AssemblyPlan());
	plan->transfer_bytes = static_cast<uint16_t>(bytes);
	plan->transfer_store = store;
	plan->transfer_register = asm_model::Register{ mask ? asm_model::RegisterBank::Mask
														: asm_model::RegisterBank::Vector,
		static_cast<uint8_t>(mask ? reg - 48 : reg - XMM0),
		static_cast<uint16_t>(mask ? bytes * 8
				: bytes <= 16      ? 128
								   : bytes * 8),
		0 };
	const uint32_t site = static_cast<uint32_t>(out_->assembly->plans.size());
	out_->assembly->plans.push_back(plan);
	Inst& effect = emit2(Op::Asm, lowir::TypeKind::Void, Operand::make_imm(site), memory);
	effect.aux = store ? ordinary_assembly_mask(reg) : 0;
	effect.aux2 = store ? 0 : ordinary_assembly_mask(reg);
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
	const unsigned bytes = operand.type.bytes;
	const Operand saved = Operand::make_frame(frame_alloc(bytes, 16));
	// Capture the complete input before installing any assembly register.
	// A pointer and an object value share this transfer without scalar proxies.
	assembly_transfer(XMM15, source, false, bytes);
	assembly_transfer(XMM15, saved, true, bytes);
	return saved;
}
} // namespace native
