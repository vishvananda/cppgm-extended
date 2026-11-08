#include "native/asm/vex_forms.h"

#include "native/asm/operands.h"
#include "x86_asm_model.h"

namespace native {
namespace {
using asm_detail::fail;
using asm_model::RegisterBank;
using assembly::Instruction;
using assembly::Operand;
using assembly::OperandKind;
using x86_asm_model::VexForm;
using L = x86_asm_model::VexLayout;

unsigned vector(const Operand& operand, unsigned bits, const Instruction& inst)
{
	if (operand.kind != OperandKind::Register || operand.reg.bank != RegisterBank::Vector || operand.reg.number >= 16 ||
		operand.bits != bits)
		fail(inst, "VEX register requires a matching XMM/YMM view in registers 0..15");
	return operand.reg.number;
}
RM location(const Operand& operand, unsigned register_bits, unsigned memory_bits, const Instruction& inst)
{
	if (operand.kind == OperandKind::Register)
		return RM::r(static_cast<Reg>(vector(operand, register_bits, inst)));
	if (operand.kind != OperandKind::Memory || (operand.bits != 0 && operand.bits != memory_bits))
		fail(inst, "invalid VEX memory operand or explicit width");
	return asm_detail::rm(operand, inst);
}
uint8_t immediate(const Operand& operand, const Instruction& inst)
{
	const int64_t value = static_cast<int64_t>(operand.immediate);
	if (operand.kind != OperandKind::Immediate || value < -128 || value > 255)
		fail(inst, "VEX control immediate must fit one byte");
	return static_cast<uint8_t>(value);
}
void emit(Assembler& out, const VexForm& form, unsigned bits, unsigned reg, unsigned source, const RM& operand,
	uint8_t opcode, bool has_immediate, uint8_t control)
{
	const bool r = (reg & 8) != 0;
	const bool x = !operand.is_reg && operand.index != kNoReg && (operand.index & 8) != 0;
	const bool b = operand.is_reg ? (operand.reg & 8) != 0 : operand.base != kNoReg && (operand.base & 8) != 0;
	const uint8_t payload = static_cast<uint8_t>(((~source & 15) << 3) | (bits == 256 ? 4 : 0) | form.pp);
	if (form.map == 1 && form.w == 0 && !x && !b)
	{
		out.byte(0xc5);
		out.byte(static_cast<uint8_t>((r ? 0 : 0x80) | payload));
	}
	else
	{
		out.byte(0xc4);
		out.byte(static_cast<uint8_t>((r ? 0 : 0x80) | (x ? 0 : 0x40) | (b ? 0 : 0x20) | form.map));
		out.byte(static_cast<uint8_t>((form.w << 7) | payload));
	}
	// VEX already supplies the extension bits. Reuse the existing ModRM/SIB,
	// displacement and fixup encoder with their low fields, emitting no REX.
	RM low = operand;
	if (low.is_reg)
		low.reg &= 7;
	else
	{
		if (low.base != kNoReg)
			low.base &= 7;
		if (low.index != kNoReg)
			low.index &= 7;
	}
	out.op(0, false, opcode, 1, reg & 7, low, false, has_immediate ? 1 : 0);
	if (has_immediate)
		out.byte(control);
}
unsigned width(const Instruction& inst, const VexForm& form)
{
	if (inst.operands.size() < 2)
		fail(inst, "VEX instruction requires operands");
	const Operand& operand = form.layout == L::Extract || form.layout == L::Mask ||
			inst.operands[0].kind == OperandKind::Memory
		? inst.operands[1]
		: inst.operands[0];
	const unsigned bits = operand.bits;
	if ((bits != 128 && bits != 256) || (form.lengths & (bits == 128 ? 1 : 2)) == 0)
		fail(inst, "unsupported vector width for this VEX instruction");
	return bits;
}
void encode(Assembler& out, const Instruction& inst, const VexForm& form)
{
	const unsigned bits = width(inst, form);
	const unsigned memory_bits = form.memory_bits != 0 ? form.memory_bits : bits;
	const auto& operands = inst.operands;
	const Operand& dst = operands[0];
	const Operand& src = operands[1];
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
		reg = vector(dst, bits, inst);
		source = vector(src, bits, inst);
		const unsigned other_bits = form.layout == L::Insert ? 128 : bits;
		other = location(operands[2], other_bits, memory_bits, inst);
		if (scalar_merge && operands[2].kind != OperandKind::Register)
			fail(inst, "three-operand scalar move requires registers");
		if (has_immediate)
			control = immediate(operands[3], inst);
	}
	else if (form.layout == L::Shift)
	{
		if (operands.size() != 3)
			fail(inst, "packed VEX shift requires three operands");
		vector(dst, bits, inst);
		vector(src, bits, inst);
		if (operands[2].kind == OperandKind::Immediate)
		{
			reg = form.group;
			source = dst.reg.number;
			other = location(src, bits, bits, inst);
			opcode = form.immediate_opcode;
			has_immediate = true;
			control = immediate(operands[2], inst);
		}
		else
		{
			if (opcode == 0)
				fail(inst, "this packed shift requires an immediate");
			reg = dst.reg.number;
			source = src.reg.number;
			other = location(operands[2], 128, 128, inst);
		}
	}
	else if (form.layout == L::Extract)
	{
		if (operands.size() != 3)
			fail(inst, "vector extraction requires three operands");
		reg = vector(src, bits, inst);
		other = location(dst, 128, 128, inst);
		has_immediate = true;
		control = immediate(operands[2], inst);
	}
	else if (form.layout == L::Move || form.layout == L::ScalarMove || form.layout == L::Store)
	{
		if (operands.size() != 2)
			fail(inst, "VEX move requires two operands");
		const bool store = dst.kind == OperandKind::Memory;
		if (form.layout == L::Store && !store)
			fail(inst, "non-temporal VEX store requires memory");
		reg = vector(store ? src : dst, bits, inst);
		other = location(store ? dst : src, bits, memory_bits, inst);
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
			if (dst.kind != OperandKind::Register || dst.reg.bank != RegisterBank::Gpr || dst.bits != 32 ||
				dst.reg.number >= 16)
				fail(inst, "VEX mask extraction requires a 32-bit general register");
			reg = dst.reg.number;
			vector(src, bits, inst);
		}
		else
			reg = vector(dst, bits, inst);
		other = location(src, form.layout == L::Broadcast ? 128 : bits, memory_bits, inst);
		if (form.layout == L::Broadcast && memory_bits == 128 && src.kind != OperandKind::Memory)
			fail(inst, "128-bit lane broadcast requires memory");
		if (has_immediate)
			control = immediate(operands[2], inst);
	}
	// Bidirectional moves can put an extended source in ModRM.reg, keeping
	// the equivalent two-byte VEX prefix when the destination is low.
	if ((form.layout == L::Move || form.layout == L::ScalarMove) && other.is_reg && reg < 8 && other.reg >= 8)
	{
		const unsigned previous = reg;
		reg = other.reg;
		other.reg = static_cast<Reg>(previous);
		opcode = static_cast<uint8_t>(opcode + (opcode == 0x6f ? 0x10 : 1));
	}
	emit(out, form, bits, reg, source, other, opcode, has_immediate, control);
}
}
bool encode_vex_form(Assembler& out, const assembly::Instruction& inst)
{
	if (inst.mnemonic == "vzeroupper" || inst.mnemonic == "vzeroall")
	{
		if (!inst.operands.empty() || inst.prefix != 0)
			fail(inst, "VEX zero operation takes no operands or prefixes");
		out.byte(0xc5);
		out.byte(inst.mnemonic == "vzeroupper" ? 0xf8 : 0xfc);
		out.byte(0x77);
		return true;
	}
	size_t count = 0;
	const VexForm* forms = x86_asm_model::vex_forms(count);
	for (unsigned family = 0; family < 2; ++family)
	{
		for (size_t i = 0; i < count; ++i)
			if (inst.mnemonic == forms[i].mnemonic)
			{
				if (inst.prefix != 0)
					fail(inst, "legacy instruction prefix is invalid before VEX");
				encode(out, inst, forms[i]);
				return true;
			}
		forms = x86_asm_model::vex_fma_forms(count);
	}
	return false;
}
} // namespace native
