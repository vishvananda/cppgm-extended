#include "native/asm/control_forms.h"

#include "native/asm/operands.h"

namespace native {
namespace {
using asm_detail::fail;
using asm_detail::rm;
using assembly::OperandKind;
}

void encode_control_form(
	Assembler& out, const assembly::Instruction& inst, const asm_detail::Form& form, unsigned suffix)
{
	if (inst.prefix != 0 || (suffix != 0 && suffix != 16 && suffix != 64) || (form.opcode == 0xc3 && suffix == 16))
		fail(inst, "invalid stack-control prefix or width");
	if (form.opcode == 0xc3)
	{
		if (inst.operands.empty())
			out.ret();
		else if (inst.operands.size() == 1 && inst.operands[0].kind == OperandKind::Immediate &&
			inst.operands[0].immediate <= UINT16_MAX)
		{
			out.byte(0xc2);
			out.imm(inst.operands[0].immediate, 2);
		}
		else
			fail(inst, "ret expects no operand or an unsigned 16-bit immediate");
		return;
	}
	if (inst.operands.size() != 1)
		fail(inst, "push/pop expects one operand");
	const assembly::Operand& operand = inst.operands[0];
	const unsigned bits = suffix != 0 ? suffix : operand.bits != 0 ? operand.bits : 64;
	if ((bits != 16 && bits != 64) || (operand.bits != 0 && operand.bits != bits))
		fail(inst, "push/pop requires a 16-bit or 64-bit operand");
	const uint8_t prefix = bits == 16 ? 0x66 : 0;
	if (operand.kind == OperandKind::Register)
	{
		if (operand.reg.bank != asm_model::RegisterBank::Gpr)
			fail(inst, "push/pop requires a general-purpose register");
		out.op_plus(prefix, false, form.opcode, asm_detail::physical(operand.reg, inst));
	}
	else if (operand.kind == OperandKind::Memory)
		out.op(prefix, false, form.opcode == 0x50 ? 0xff : 0x8f, 1, form.opcode == 0x50 ? 6 : 0, rm(operand, inst));
	else if (operand.kind == OperandKind::Immediate && form.opcode == 0x50)
	{
		const int64_t value = static_cast<int64_t>(operand.immediate);
		if (!fits_operation(value, bits / 8))
			fail(inst, "push immediate exceeds its signed encoding");
		if (prefix != 0)
			out.byte(prefix);
		const bool small = fits_i8(value);
		out.byte(small ? 0x6a : 0x68);
		out.imm(value, small ? 1 : bits == 16 ? 2 : 4);
	}
	else
		fail(inst, "invalid push/pop operand");
}

bool encode_assembly_branch(Assembler& out, const assembly::Instruction& inst, const std::vector<uint32_t>& labels)
{
	const int condition = asm_detail::branch_condition(inst.mnemonic);
	const bool call = inst.mnemonic == "call" || inst.mnemonic == "callq";
	const bool jump = inst.mnemonic == "jmp" || inst.mnemonic == "jmpq";
	if (!call && !jump && condition < 0)
		return false;
	if (inst.prefix != 0 || inst.operands.size() != 1)
		fail(inst, "expected one branch target");
	const assembly::Operand& target = inst.operands[0];
	if (target.kind == OperandKind::Label)
	{
		const uint32_t label = labels[target.label];
		if (call)
			out.call_label(label);
		else if (condition >= 0)
			out.jcc(static_cast<Cond>(condition), label);
		else
			out.jmp(label);
	}
	else if (condition >= 0)
		fail(inst, "conditional branch requires an assembly-local label");
	else if (target.kind == OperandKind::Symbol && target.address.relocation == assembly::Relocation::Address)
	{
		if (call)
			out.call_symbol(target.address.symbol.symbol);
		else
			out.jmp_symbol(target.address.symbol.symbol);
		out.fixups().back().addend += target.address.displacement;
	}
	else if ((target.kind == OperandKind::Register || target.kind == OperandKind::Memory) &&
		(target.bits == 0 || target.bits == 64))
	{
		if (call)
			out.call_rm(rm(target, inst));
		else
			out.jmp_rm(rm(target, inst));
	}
	else
		fail(inst, "invalid direct or indirect branch target");
	return true;
}
} // namespace native
