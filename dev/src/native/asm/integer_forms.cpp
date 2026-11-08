#include "native/asm/integer_forms.h"
#include "native/asm/operands.h"

namespace native {
namespace {
using namespace asm_detail;
using assembly::Operand;
using assembly::OperandKind;
using x86_asm_model::Encoding;

void multiply(Assembler& out, const assembly::Instruction& inst, unsigned bits)
{
	const unsigned count = static_cast<unsigned>(inst.operands.size());
	if (count == 0 || count > 3)
		fail(inst, "expected one to three multiply operands");
	const Operand& dst = inst.operands[0];
	if (count == 1)
	{
		out.op(bits == 16 ? 0x66 : 0, bits == 64, bits == 8 ? 0xf6 : 0xf7, 1, 5, rm(dst, inst),
			bits == 8 && byte_rex(inst));
		return;
	}
	if (bits == 8 || dst.kind != OperandKind::Register)
		fail(inst, "multiply requires a non-byte destination register");
	const bool immediate = inst.operands.back().kind == OperandKind::Immediate;
	const Operand& src = count == 2 && immediate ? dst : inst.operands[1];
	if (count == 3 && !immediate)
		fail(inst, "third multiply operand must be an immediate");
	if (immediate)
	{
		const int64_t value = static_cast<int64_t>(inst.operands.back().immediate);
		if (!fits_operation(value, bits / 8))
			fail(inst, "multiply immediate exceeds the encoding");
		const unsigned bytes = fits_i8(value) ? 1 : bits == 16 ? 2 : 4;
		out.op(bits == 16 ? 0x66 : 0, bits == 64, bytes == 1 ? 0x6b : 0x69, 1, physical(dst.reg, inst), rm(src, inst),
			false, bytes);
		out.imm(value, bytes);
	}
	else
		out.op(bits == 16 ? 0x66 : 0, bits == 64, 0x0faf, 2, physical(dst.reg, inst), rm(src, inst));
}

void shift(Assembler& out, const assembly::Instruction& inst, unsigned bits, unsigned digit)
{
	if (inst.operands.empty() || inst.operands.size() > 2)
		fail(inst, "expected a shift target and optional count");
	const Operand& dst = inst.operands[0];
	const Operand* count = inst.operands.size() == 2 ? &inst.operands[1] : nullptr;
	const bool immediate = count == nullptr || count->kind == OperandKind::Immediate;
	const uint64_t amount = count == nullptr ? 1 : count->immediate;
	if (immediate && amount > 255)
		fail(inst, "shift immediate exceeds 8 bits");
	if (!immediate &&
		(count->kind != OperandKind::Register || count->reg.bank != asm_model::RegisterBank::Gpr ||
			count->reg.number != 1 || count->reg.bits != 8 || count->reg.offset != 0))
		fail(inst, "variable shift count requires CL");
	const bool one = immediate && amount == 1;
	const uint8_t opcode = immediate ? (one ? 0xd0 : 0xc0) : 0xd2;
	out.op(bits == 16 ? 0x66 : 0, bits == 64, opcode + (bits != 8), 1, digit, rm(dst, inst),
		bits == 8 && byte_rex(inst), immediate && !one ? 1 : 0);
	if (immediate && !one)
		out.imm(static_cast<int64_t>(amount), 1);
}

void test(Assembler& out, const assembly::Instruction& inst, unsigned bits)
{
	if (inst.operands.size() != 2)
		fail(inst, "test requires two operands");
	const Operand& dst = inst.operands[0];
	const Operand& src = inst.operands[1];
	if (src.kind == OperandKind::Immediate)
	{
		const int64_t value = static_cast<int64_t>(src.immediate);
		if (!fits_operation(value, bits / 8))
			fail(inst, "test immediate exceeds the encoding");
		const unsigned bytes = bits == 64 ? 4 : bits / 8;
		if (dst.kind == OperandKind::Register && dst.reg.number == 0 && dst.reg.offset == 0)
			out.op_plus(bits == 16 ? 0x66 : 0, bits == 64, bits == 8 ? 0xa8 : 0xa9, RAX);
		else
			out.op(bits == 16 ? 0x66 : 0, bits == 64, bits == 8 ? 0xf6 : 0xf7, 1, 0, rm(dst, inst),
				bits == 8 && byte_rex(inst), bytes);
		out.imm(value, bytes);
		return;
	}
	const bool source_register = src.kind == OperandKind::Register;
	const Operand& reg = source_register ? src : dst;
	if (reg.kind != OperandKind::Register)
		fail(inst, "test requires a register or immediate operand");
	out.op(bits == 16 ? 0x66 : 0, bits == 64, bits == 8 ? 0x84 : 0x85, 1, physical(reg.reg, inst),
		rm(source_register ? dst : src, inst), bits == 8 && byte_rex(inst));
}

} // namespace

void encode_integer_form(
	Assembler& out, const assembly::Instruction& inst, const x86_asm_model::Form& form, unsigned bits)
{
	using namespace asm_detail;
	using x86_asm_model::Encoding;
	if (inst.prefix != 0)
	{
		const bool lockable = form.encoding == Encoding::Atomic ||
			(form.encoding == Encoding::Unary && form.opcode < 4);
		if (inst.prefix != 0xf0 || !lockable || inst.operands.empty() ||
			inst.operands[0].kind != assembly::OperandKind::Memory)
			fail(inst, "invalid prefix for integer instruction");
		out.byte(inst.prefix);
	}
	if (form.encoding == Encoding::Multiply)
		multiply(out, inst, bits);
	else if (form.encoding == Encoding::Shift)
		shift(out, inst, bits, form.opcode);
	else if (form.encoding == Encoding::Test)
		test(out, inst, bits);
	else if (form.encoding == Encoding::Unary)
	{
		if (inst.operands.size() != 1)
			fail(inst, "expected one integer operand");
		out.op(bits == 16 ? 0x66 : 0, bits == 64,
			form.opcode < 2 ? (bits == 8 ? 0xfe : 0xff) : (bits == 8 ? 0xf6 : 0xf7), 1, form.opcode,
			rm(inst.operands[0], inst), bits == 8 && byte_rex(inst));
	}
	else
	{
		if (inst.operands.size() != 2 || inst.operands[1].kind != assembly::OperandKind::Register)
			fail(inst, "expected integer target and source register");
		out.op(bits == 16 ? 0x66 : 0, bits == 64, form.opcode + (bits != 8), 2, physical(inst.operands[1].reg, inst),
			rm(inst.operands[0], inst), bits == 8 && byte_rex(inst));
	}
}

} // namespace native
