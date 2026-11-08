#include "native/asm/encoding.h"

#include "native/asm/forms.h"
#include "native/backend_error.h"

namespace native {
namespace {

using asm_detail::Encoding;
using asm_detail::Form;
using asm_model::RegisterBank;
using assembly::Operand;
using assembly::OperandKind;

[[noreturn]] void fail(const assembly::Instruction& inst, const char* why)
{
	throw BackendError("assembly line " + std::to_string(inst.line) + ": " + inst.mnemonic + ": " + why);
}

Reg physical(const asm_model::Register& reg, const assembly::Instruction& inst)
{
	if (reg.number >= 16 || (reg.bank != RegisterBank::Gpr && reg.bank != RegisterBank::Vector) ||
		(reg.bank == RegisterBank::Vector && reg.bits != 128))
		fail(inst, "register requires an instruction encoding capability not implemented yet");
	if (reg.offset != 0)
		return static_cast<Reg>(reg.number + 4); // validated high-byte alias
	return static_cast<Reg>(reg.number + (reg.bank == RegisterBank::Vector ? 16 : 0));
}

RM rm(const Operand& op, const assembly::Instruction& inst)
{
	if (op.kind == OperandKind::Register)
		return RM::r(physical(op.reg, inst));
	if (op.kind != OperandKind::Memory)
		fail(inst, "expected register or memory operand");
	const assembly::Address& address = op.address;
	return RM::m(address.has_base ? physical(address.base, inst) : kNoReg, address.displacement,
		address.has_index ? physical(address.index, inst) : kNoReg, address.scale);
}

bool rex_required(const Operand& op)
{
	if (op.kind == OperandKind::Register)
		return op.reg.number >= 8 ||
			(op.reg.bank == RegisterBank::Gpr && op.reg.offset == 0 && op.bits == 8 && op.reg.number >= 4);
	return op.kind == OperandKind::Memory &&
		((op.address.has_base && op.address.base.number >= 8) ||
			(op.address.has_index && op.address.index.number >= 8));
}

unsigned integer_width(const assembly::Instruction& inst, uint16_t suffix)
{
	unsigned bits = suffix;
	bool high = false, rex = suffix == 64;
	for (const Operand& operand : inst.operands)
	{
		if (operand.kind == OperandKind::Register && operand.reg.bank != RegisterBank::Gpr)
			fail(inst, "expected general register");
		if (operand.kind != OperandKind::Immediate && operand.bits != 0)
		{
			if (bits != 0 && bits != operand.bits)
				fail(inst, "operand widths disagree");
			bits = operand.bits;
		}
		high |= operand.kind == OperandKind::Register && operand.reg.offset != 0;
		rex |= rex_required(operand);
	}
	if (bits != 8 && bits != 16 && bits != 32 && bits != 64)
		fail(inst, "integer operand width is absent or unsupported");
	if (high && (rex || bits == 64))
		fail(inst, "high-byte registers cannot be encoded with a REX prefix");
	return bits;
}

bool byte_rex(const assembly::Instruction& inst)
{
	for (const Operand& operand : inst.operands)
		if (operand.kind == OperandKind::Register && operand.reg.offset != 0)
			return false;
	return true;
}

void integer(Assembler& out, const assembly::Instruction& inst, const Form& form, uint16_t suffix)
{
	if (inst.operands.size() != 2)
		fail(inst, "expected two operands");
	const Operand& dst = inst.operands[0];
	const Operand& src = inst.operands[1];
	const unsigned bits = integer_width(inst, suffix);
	if (form.bits != 0 &&
		(bits != form.bits || dst.kind != OperandKind::Register || src.kind != OperandKind::Immediate))
		fail(inst, "this absolute move form requires a 64-bit register and immediate");
	const unsigned bytes = bits / 8;
	const uint8_t prefix = bits == 16 ? 0x66 : 0;
	const bool byte_regs = bits == 8 && byte_rex(inst);
	const bool wide = bits == 64;
	if (dst.kind == OperandKind::Immediate || (dst.kind == OperandKind::Memory && src.kind == OperandKind::Memory))
		fail(inst, "invalid integer destination or two memory operands");
	if (form.encoding == Encoding::Move)
	{
		if (src.kind == OperandKind::Immediate)
		{
			const int64_t value = static_cast<int64_t>(src.immediate);
			if (dst.kind == OperandKind::Register)
			{
				if (bits != 64 && !fits_operation(value, bytes))
					fail(inst, "immediate exceeds the destination width");
				const Reg reg = physical(dst.reg, inst);
				// Byte B0+r needs the same low-byte REX rule as ModRM.
				if (byte_regs && reg >= 4 && reg < 8)
					out.byte(0x40);
				out.op_plus(prefix, wide, bits == 8 ? 0xb0 : 0xb8, reg);
				out.imm(value, bytes);
			}
			else
			{
				if (!fits_operation(value, bytes))
					fail(inst, "immediate exceeds the memory form's encoding");
				const unsigned immediate = bits == 64 ? 4 : bytes;
				out.op(prefix, wide, bits == 8 ? 0xc6 : 0xc7, 1, 0, rm(dst, inst), false, immediate);
				out.imm(value, immediate);
			}
		}
		else if (dst.kind == OperandKind::Register)
			out.op(prefix, wide, bits == 8 ? 0x8a : 0x8b, 1, physical(dst.reg, inst), rm(src, inst), byte_regs);
		else
			out.op(prefix, wide, bits == 8 ? 0x88 : 0x89, 1, physical(src.reg, inst), rm(dst, inst), byte_regs);
	}
	else if (form.encoding == Encoding::Lea)
	{
		if (bits == 8 || dst.kind != OperandKind::Register || src.kind != OperandKind::Memory)
			fail(inst, "lea requires a non-byte register and memory address");
		out.op(prefix, wide, 0x8d, 1, physical(dst.reg, inst), rm(src, inst));
	}
	else if (form.encoding == Encoding::Exchange)
	{
		if (src.kind == OperandKind::Immediate ||
			(dst.kind != OperandKind::Register && src.kind != OperandKind::Register))
			fail(inst, "xchg requires a register operand");
		const bool dst_reg = dst.kind == OperandKind::Register;
		out.op(prefix, wide, bits == 8 ? 0x86 : 0x87, 1, physical(dst_reg ? dst.reg : src.reg, inst),
			rm(dst_reg ? src : dst, inst), byte_regs);
	}
	else
	{
		const unsigned digit = form.opcode;
		if (src.kind == OperandKind::Immediate)
		{
			const int64_t value = static_cast<int64_t>(src.immediate);
			if (!fits_operation(value, bytes))
				fail(inst, "ALU immediate exceeds the instruction encoding");
			const bool small = bits != 8 && fits_i8(value);
			const unsigned immediate = small || bits == 8 ? 1 : bits == 64 ? 4 : bytes;
			out.op(prefix, wide, bits == 8 ? 0x80 : small ? 0x83 : 0x81, 1, digit, rm(dst, inst), byte_regs, immediate);
			out.imm(value, immediate);
		}
		else if (dst.kind == OperandKind::Register)
			out.op(prefix, wide, digit * 8 + (bits == 8 ? 2 : 3), 1, physical(dst.reg, inst), rm(src, inst), byte_regs);
		else
			out.op(prefix, wide, digit * 8 + (bits == 8 ? 0 : 1), 1, physical(src.reg, inst), rm(dst, inst), byte_regs);
	}
}

void vector(Assembler& out, const assembly::Instruction& inst, const Form& form)
{
	if (inst.operands.size() != 2)
		fail(inst, "expected two vector operands");
	const Operand& dst = inst.operands[0];
	const Operand& src = inst.operands[1];
	bool store = form.encoding == Encoding::VectorMR;
	uint32_t opcode = form.opcode;
	if (!store && dst.kind == OperandKind::Memory && (opcode == 0x0f28 || opcode == 0x0f10 || opcode == 0x0f6f))
	{
		store = true;
		opcode += opcode == 0x0f6f ? 0x10 : 1;
	}
	const Operand& reg = store ? src : dst;
	const Operand& other = store ? dst : src;
	if (reg.kind != OperandKind::Register || reg.reg.bank != RegisterBank::Vector || reg.bits != 128 ||
		other.kind == OperandKind::Immediate || (form.memory_only && other.kind != OperandKind::Memory) ||
		(other.kind == OperandKind::Register && (other.reg.bank != RegisterBank::Vector || other.bits != 128)) ||
		(other.kind == OperandKind::Memory && other.bits != 0 && other.bits != form.bits))
		fail(inst, "invalid vector register or memory form");
	out.op(form.prefix, false, opcode, 2, physical(reg.reg, inst), rm(other, inst));
}

} // namespace

void encode_assembly(Assembler& out, const assembly::Instruction& inst)
{
	uint16_t suffix = 0;
	const Form* form = asm_detail::assembly_form(inst.mnemonic, suffix);
	if (form == nullptr)
		fail(inst, "instruction form is not implemented");
	if (form->encoding == Encoding::Zero)
	{
		if (!inst.operands.empty())
			fail(inst, "instruction takes no operands");
		const unsigned bytes = form->opcode > 0xffff ? 3 : form->opcode > 0xff ? 2 : 1;
		for (unsigned i = bytes; i != 0; --i)
			out.byte(static_cast<uint8_t>(form->opcode >> ((i - 1) * 8)));
	}
	else if (form->encoding == Encoding::VectorRM || form->encoding == Encoding::VectorMR)
		vector(out, inst, *form);
	else
		integer(out, inst, *form, suffix);
}

} // namespace native
