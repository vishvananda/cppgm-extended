#include "native/asm/bit_forms.h"
#include "native/asm/operands.h"

namespace native {
namespace {
using namespace asm_detail;
using assembly::Operand;
using assembly::OperandKind;
using x86_asm_model::Encoding;

void wide_atomic(Assembler& out, const assembly::Instruction& inst, const Form& form, unsigned suffix)
{
	if (suffix != 0 || inst.operands.size() != 1 || inst.operands[0].kind != OperandKind::Memory ||
		(inst.operands[0].bits != 0 && inst.operands[0].bits != form.bits))
		fail(inst, "wide compare/exchange requires its fixed-width memory operand");
	out.op(0, form.bits == 128, form.opcode, 2, 1, rm(inst.operands[0], inst));
}

void bit_test(Assembler& out, const assembly::Instruction& inst, const Form& form, unsigned bits)
{
	if (inst.operands.size() != 2)
		fail(inst, "bit test/update requires a target and bit index");
	const Operand& target = inst.operands[0];
	const Operand& index = inst.operands[1];
	if (index.kind == OperandKind::Immediate)
	{
		const int64_t value = static_cast<int64_t>(index.immediate);
		if (!fits_operation(value, 1))
			fail(inst, "bit index immediate exceeds eight bits");
		out.op(bits == 16 ? 0x66 : 0, bits == 64, 0x0fba, 2, form.opcode, rm(target, inst), false, 1);
		out.imm(value, 1);
	}
	else if (index.kind == OperandKind::Register)
		out.op(bits == 16 ? 0x66 : 0, bits == 64, 0x0fa3 + (form.opcode - 4) * 8, 2, physical(index.reg, inst),
			rm(target, inst));
	else
		fail(inst, "bit index requires a register or immediate");
}

void double_shift(Assembler& out, const assembly::Instruction& inst, const Form& form, unsigned bits)
{
	if (inst.operands.size() != 3 || inst.operands[1].kind != OperandKind::Register)
		fail(inst, "double shift requires target, source register and count");
	const Operand& count = inst.operands[2];
	const bool immediate = count.kind == OperandKind::Immediate;
	if (immediate ? !fits_operation(static_cast<int64_t>(count.immediate), 1)
				  : (count.kind != OperandKind::Register || count.reg.number != 1 || count.bits != 8 ||
						count.reg.offset != 0))
		fail(inst, "double shift count requires an eight-bit immediate or CL");
	out.op(bits == 16 ? 0x66 : 0, bits == 64, form.opcode + (immediate ? 0 : 1), 2,
		physical(inst.operands[1].reg, inst), rm(inst.operands[0], inst), false, immediate ? 1 : 0);
	if (immediate)
		out.imm(static_cast<int64_t>(count.immediate), 1);
}

} // namespace

void encode_bit_form(Assembler& out, const assembly::Instruction& inst, const Form& form, unsigned suffix)
{
	const bool lockable = form.encoding == Encoding::WideAtomic ||
		(form.encoding == Encoding::BitTest && form.opcode != 4);
	if (inst.prefix != 0)
	{
		if (inst.prefix != 0xf0 || !lockable || inst.operands.empty() || inst.operands[0].kind != OperandKind::Memory)
			fail(inst, "invalid prefix for conditional/bit instruction");
		out.byte(inst.prefix);
	}
	if (form.encoding == Encoding::WideAtomic)
		return wide_atomic(out, inst, form, suffix);
	const unsigned bits = integer_width(
		inst, static_cast<uint16_t>(suffix != 0 ? suffix : form.bits), form.encoding == Encoding::DoubleShift ? 2 : 5);
	if (form.encoding == Encoding::ConditionSet)
	{
		if (bits != 8 || inst.operands.size() != 1)
			fail(inst, "setcc requires one byte register or memory operand");
		out.op(0, false, form.opcode, 2, 0, rm(inst.operands[0], inst), byte_rex(inst));
		return;
	}
	if (bits == 8)
		fail(inst, "instruction requires a 16, 32 or 64-bit operand");
	if (form.encoding == Encoding::BitTest)
		return bit_test(out, inst, form, bits);
	if (form.encoding == Encoding::DoubleShift)
		return double_shift(out, inst, form, bits);
	if (form.encoding == Encoding::BitSwap)
	{
		if (bits == 16 || inst.operands.size() != 1 || inst.operands[0].kind != OperandKind::Register)
			fail(inst, "bswap requires one 32 or 64-bit register");
		out.bswap(physical(inst.operands[0].reg, inst), bits == 64);
		return;
	}
	if (inst.operands.size() != 2 || inst.operands[0].kind != OperandKind::Register ||
		(inst.operands[1].kind != OperandKind::Register && inst.operands[1].kind != OperandKind::Memory))
		fail(inst, "instruction requires a destination register and register/memory source");
	if (bits == 16 && form.prefix != 0)
		out.byte(0x66);
	out.op(form.prefix != 0 ? form.prefix
			: bits == 16    ? 0x66
							: 0,
		bits == 64, form.opcode, 2, physical(inst.operands[0].reg, inst), rm(inst.operands[1], inst));
}
} // namespace native
