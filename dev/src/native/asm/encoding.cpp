#include "native/asm/encoding.h"

#include "native/asm/forms.h"
#include "native/asm/control_forms.h"
#include "native/asm/bit_forms.h"
#include "native/asm/extend_forms.h"
#include "native/asm/integer_forms.h"
#include "native/asm/operands.h"
#include "native/backend_error.h"

namespace native {
namespace {

using asm_detail::Encoding;
using asm_detail::Form;
using asm_model::RegisterBank;
using assembly::Operand;
using assembly::OperandKind;

using asm_detail::byte_rex;
using asm_detail::fail;
using asm_detail::integer_width;
using asm_detail::physical;
using asm_detail::rm;

void integer(Assembler& out, const assembly::Instruction& inst, const Form& form, uint16_t suffix)
{
	if (inst.operands.size() != 2)
		fail(inst, "expected two operands");
	const Operand& dst = inst.operands[0];
	const Operand& src = inst.operands[1];
	const unsigned bits = integer_width(inst, suffix);
	if (form.bits != 0 &&
		(bits != form.bits || dst.kind != OperandKind::Register ||
			(src.kind != OperandKind::Immediate && src.kind != OperandKind::Symbol)))
		fail(inst, "this absolute move form requires a 64-bit register and immediate");
	const unsigned bytes = bits / 8;
	if (src.kind == OperandKind::Symbol)
	{
		if (form.encoding != Encoding::Move || bits != 64 || dst.kind != OperandKind::Register ||
			src.address.relocation != assembly::Relocation::Address)
			fail(inst, "symbol immediate requires a 64-bit register move");
		out.op_plus(0, true, 0xb8, physical(dst.reg, inst));
		CodeFixup fixup;
		fixup.offset = out.size();
		fixup.target = src.address.symbol.symbol;
		fixup.addend = src.address.displacement;
		fixup.end = 8;
		fixup.absolute = true;
		fixup.branch = false;
		out.fixups().push_back(fixup);
		out.imm(0, 8);
		return;
	}
	const uint8_t prefix = bits == 16 ? 0x66 : 0;
	const bool byte_regs = bits == 8 && byte_rex(inst);
	const bool wide = bits == 64;
	if ((dst.kind != OperandKind::Register && dst.kind != OperandKind::Memory) ||
		(dst.kind == OperandKind::Memory && src.kind == OperandKind::Memory))
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
				if (wide && form.bits == 0 && fits_i32(value))
				{
					out.op(0, true, 0xc7, 1, 0, RM::r(reg), false, 4);
					out.imm(value, 4);
					return;
				}
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

void encode_instruction(Assembler& out, const assembly::Instruction& inst)
{
	uint16_t suffix = 0;
	const Form* form = asm_detail::assembly_form(inst.mnemonic, suffix, static_cast<uint32_t>(inst.operands.size()));
	if (form == nullptr)
		fail(inst, "instruction form is not implemented");
	if (form->encoding == Encoding::Control)
	{
		encode_control_form(out, inst, *form, suffix);
		return;
	}
	if (form->encoding == Encoding::ExtendMove)
	{
		encode_extend_form(out, inst, *form);
		return;
	}
	if (form->encoding >= Encoding::ConditionMove)
	{
		encode_bit_form(out, inst, *form, suffix);
		return;
	}
	if (form->encoding >= Encoding::Unary && form->encoding != Encoding::String)
	{
		encode_integer_form(out, inst, *form, integer_width(inst, suffix, form->encoding == Encoding::Shift ? 1 : 5));
		return;
	}
	if (form->encoding == Encoding::String &&
		(inst.prefix == 0xf0 ||
			(inst.prefix == 0xf2 && (form->opcode & 0xfe) != 0xa6 && (form->opcode & 0xfe) != 0xae)))
		fail(inst, "prefix is invalid for this string instruction");
	if (inst.prefix != 0 && form->encoding != Encoding::String && !(inst.prefix == 0xf3 && inst.mnemonic == "nop") &&
		!(inst.prefix == 0xf0 &&
			(form->encoding == Encoding::Exchange || (form->encoding == Encoding::Alu && form->opcode != 7)) &&
			!inst.operands.empty() && inst.operands[0].kind == OperandKind::Memory))
		fail(inst, "prefix is invalid for this instruction form");
	if (inst.prefix != 0)
		out.byte(inst.prefix);
	if (form->encoding == Encoding::Zero || form->encoding == Encoding::String)
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

void encode_assembly(Assembler& out, const std::vector<assembly::Instruction>& instructions, const RM& exit_frame)
{
	std::vector<uint32_t> labels(instructions.size(), kNoLabel);
	for (size_t i = 0; i < instructions.size(); ++i)
		if (instructions[i].label)
			labels[i] = out.new_label();
	for (const assembly::Instruction& inst : instructions)
	{
		if (inst.is_exit)
		{
			out.mov_store_imm(exit_frame, inst.exit_tag, 4);
			continue;
		}
		if (inst.label)
		{
			out.bind(labels[&inst - instructions.data()]);
			continue;
		}
		if (!encode_assembly_branch(out, inst, labels))
		{
			const size_t first = out.fixups().size();
			encode_instruction(out, inst);
			for (const assembly::Operand& operand : inst.operands)
				if (operand.kind == OperandKind::Memory && operand.address.relocation != assembly::Relocation::Address)
					for (size_t i = first; i < out.fixups().size(); ++i)
						out.fixups()[i].kind = operand.address.relocation == assembly::Relocation::Got ? FixupKind::Got
							: operand.address.relocation == assembly::Relocation::GotTpOff ? FixupKind::GotTpOff
																						   : FixupKind::Tlv;
		}
	}
}

} // namespace native
