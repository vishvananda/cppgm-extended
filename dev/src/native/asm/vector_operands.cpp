#include "native/asm/vector_operands.h"

#include "native/asm/operands.h"

namespace native {
namespace {
using asm_detail::fail;
using asm_model::RegisterBank;
using assembly::Instruction;
using assembly::Operand;
using assembly::OperandKind;
using x86_asm_model::VexForm;
using L = x86_asm_model::VexLayout;
unsigned vector(const assembly::Operand& operand, unsigned bits, const Instruction& inst, unsigned register_count)
{
	if (operand.kind != assembly::OperandKind::Register || operand.reg.bank != RegisterBank::Vector ||
		operand.reg.number >= register_count || operand.bits != bits)
		fail(inst, "vector register view is incompatible with this instruction form");
	return operand.reg.number;
}
RM location(const assembly::Operand& operand, unsigned register_bits, unsigned memory_bits, const Instruction& inst,
	unsigned register_count)
{
	if (operand.kind == assembly::OperandKind::Register)
		return RM::r(static_cast<Reg>(vector(operand, register_bits, inst, register_count)));
	if (operand.kind != assembly::OperandKind::Memory || (operand.bits != 0 && operand.bits != memory_bits))
		fail(inst, "invalid VEX memory operand or explicit width");
	return asm_detail::rm(operand, inst);
}
uint8_t immediate(const assembly::Operand& operand, const Instruction& inst)
{
	const int64_t value = static_cast<int64_t>(operand.immediate);
	if (operand.kind != assembly::OperandKind::Immediate || value < -128 || value > 255)
		fail(inst, "VEX control immediate must fit one byte");
	return static_cast<uint8_t>(value);
}
unsigned width(const Instruction& inst, const VexForm& form)
{
	if (inst.operands.size() < 2)
		fail(inst, "VEX instruction requires operands");
	const assembly::Operand& operand = form.layout == L::Extract || form.layout == L::Mask ||
			inst.operands[0].kind == assembly::OperandKind::Memory
		? inst.operands[1]
		: inst.operands[0];
	const unsigned bits = operand.bits;
	if ((bits != 128 && bits != 256 && bits != 512) || (form.lengths & (bits == 128 ? 1 : bits == 256 ? 2 : 4)) == 0)
		fail(inst, "unsupported vector width for this VEX instruction");
	return bits;
}
}
VectorEncodingPlan prepare_vector_encoding(const Instruction& inst, const VexForm& form, unsigned register_count)
{
	const unsigned bits = width(inst, form);
	const unsigned memory_bits = form.memory_bits != 0 ? form.memory_bits : bits;
	const auto& operands = inst.operands;
	const assembly::Operand& dst = operands[0];
	const assembly::Operand& src = operands[1];
	unsigned reg = 0, source = 0;
	RM other;
	uint8_t opcode = form.opcode, control = 0;
	bool has_immediate = false;
	const bool scalar_merge = form.layout == L::ScalarMove && operands.size() == 3;
	if (form.layout == L::Nds || form.layout == L::NdsImm || form.layout == L::Insert || scalar_merge)
	{
		has_immediate = form.layout == L::NdsImm || form.layout == L::Insert;
		if (operands.size() != (has_immediate ? 4u : 3u))
			fail(inst, "incorrect VEX operand count");
		reg = vector(dst, bits, inst, register_count);
		source = vector(src, bits, inst, register_count);
		const unsigned other_bits = form.layout == L::Insert ? 128 : bits;
		other = location(operands[2], other_bits, memory_bits, inst, register_count);
		if (scalar_merge && operands[2].kind != assembly::OperandKind::Register)
			fail(inst, "three-operand scalar move requires registers");
		if (has_immediate)
			control = immediate(operands[3], inst);
	}
	else if (form.layout == L::Shift)
	{
		if (operands.size() != 3)
			fail(inst, "packed VEX shift requires three operands");
		vector(dst, bits, inst, register_count);
		if (operands[2].kind == assembly::OperandKind::Immediate)
		{
			if (register_count == 16 && src.kind != assembly::OperandKind::Register)
				fail(inst, "VEX immediate shift requires a register source");
			reg = form.group;
			source = dst.reg.number;
			other = location(src, bits, bits, inst, register_count);
			opcode = form.immediate_opcode;
			has_immediate = true;
			control = immediate(operands[2], inst);
		}
		else
		{
			if (opcode == 0)
				fail(inst, "this packed shift requires an immediate");
			vector(src, bits, inst, register_count);
			reg = dst.reg.number;
			source = src.reg.number;
			other = location(operands[2], 128, 128, inst, register_count);
		}
	}
	else if (form.layout == L::Extract)
	{
		if (operands.size() != 3)
			fail(inst, "vector extraction requires three operands");
		reg = vector(src, bits, inst, register_count);
		other = location(dst, 128, 128, inst, register_count);
		has_immediate = true;
		control = immediate(operands[2], inst);
	}
	else if (form.layout == L::Move || form.layout == L::ScalarMove || form.layout == L::Store)
	{
		if (operands.size() != 2)
			fail(inst, "VEX move requires two operands");
		const bool store = dst.kind == assembly::OperandKind::Memory;
		if (form.layout == L::Store && !store)
			fail(inst, "non-temporal VEX store requires memory");
		reg = vector(store ? src : dst, bits, inst, register_count);
		other = location(store ? dst : src, bits, memory_bits, inst, register_count);
		if (form.layout == L::ScalarMove && other.is_reg)
			fail(inst, "two-operand scalar VEX move requires memory");
		if (store && form.layout != L::Store)
			opcode = static_cast<uint8_t>(opcode + (opcode == 0x6f ? 0x10 : 1));
	}
	else
	{
		has_immediate = form.layout == L::UnaryImm;
		if (operands.size() != (has_immediate ? 3u : 2u))
			fail(inst, "incorrect VEX operand count");
		if (form.layout == L::Mask)
		{
			if (dst.kind != assembly::OperandKind::Register || dst.reg.bank != RegisterBank::Gpr || dst.bits != 32 ||
				dst.reg.number >= 16)
				fail(inst, "VEX mask extraction requires a 32-bit general register");
			reg = dst.reg.number;
			vector(src, bits, inst, register_count);
		}
		else
			reg = vector(dst, bits, inst, register_count);
		other = location(src, form.layout == L::Broadcast ? 128 : bits, memory_bits, inst, register_count);
		if (form.layout == L::Broadcast && memory_bits == 128 && src.kind != assembly::OperandKind::Memory)
			fail(inst, "128-bit lane broadcast requires memory");
		if (has_immediate)
			control = immediate(operands[2], inst);
	}
	// Bidirectional moves can put an extended source in ModRM.reg, keeping
	// the equivalent two-byte VEX prefix when the destination is low.
	if (register_count == 16 && (form.layout == L::Move || form.layout == L::ScalarMove) && other.is_reg && reg < 8 &&
		other.reg >= 8)
	{
		const unsigned previous = reg;
		reg = other.reg;
		other.reg = static_cast<Reg>(previous);
		opcode = static_cast<uint8_t>(opcode + (opcode == 0x6f ? 0x10 : 1));
	}
	return VectorEncodingPlan{ bits, memory_bits, reg, source, other, opcode, control, has_immediate };
}
} // namespace native
