#include "native/asm/evex_forms.h"

#include "native/asm/operands.h"
#include "native/asm/vector_operands.h"

namespace native {
namespace {
using asm_detail::fail;
using assembly::Instruction;
using x86_asm_model::EvexForm;
using x86_asm_model::VexForm;
using L = x86_asm_model::VexLayout;

bool required(const Instruction& inst)
{
	for (const auto& operand : inst.operands)
		if (operand.mask || operand.zero || operand.broadcast ||
			(operand.kind == assembly::OperandKind::Register && operand.reg.bank == asm_model::RegisterBank::Vector &&
				(operand.bits == 512 || operand.reg.number >= 16)))
			return true;
	return false;
}
bool find(const std::string& name, EvexForm& result)
{
	size_t count = 0;
	const EvexForm* extra = x86_asm_model::evex_forms(count);
	for (size_t i = 0; i < count; ++i)
		if (name == extra[i].form.mnemonic)
		{
			result = extra[i];
			return true;
		}
	const VexForm* fma = x86_asm_model::vex_fma_forms(count);
	for (size_t i = 0; i < count; ++i)
		if (name == fma[i].mnemonic)
		{
			result.form = fma[i];
			result.form.lengths = fma[i].memory_bits ? 1 : 7;
			result.element_bits = fma[i].w ? 64 : 32;
			result.flags = fma[i].memory_bits ? 3 : 7;
			return true;
		}
	size_t variants = 0;
	const auto* metadata = x86_asm_model::evex_variants(variants);
	const VexForm* base = x86_asm_model::vex_forms(count);
	for (size_t v = 0; v < variants; ++v)
		if (name == metadata[v].mnemonic)
			for (size_t i = 0; i < count; ++i)
				if (name == base[i].mnemonic)
				{
					result.form = base[i];
					result.form.w = metadata[v].w;
					result.form.lengths = metadata[v].lengths;
					result.element_bits = metadata[v].element_bits;
					result.flags = metadata[v].flags;
					return true;
				}
	return false;
}
void address(Assembler& out, unsigned reg, const RM& rm, unsigned scale)
{
	const unsigned field = (reg & 7) << 3;
	if (rm.base == kNoReg)
	{
		out.byte(static_cast<uint8_t>(field | 4));
		const unsigned index = rm.index == kNoReg ? 4 : rm.index & 7;
		const unsigned shift = rm.scale == 8 ? 3 : rm.scale == 4 ? 2 : rm.scale == 2 ? 1 : 0;
		out.byte(static_cast<uint8_t>((shift << 6) | (index << 3) | 5));
		out.imm(rm.disp, 4);
		return;
	}
	const bool compressed = rm.disp % static_cast<int32_t>(scale) == 0 &&
		fits_i8(rm.disp / static_cast<int32_t>(scale));
	const unsigned base = rm.base & 7;
	const unsigned mod = rm.disp == 0 && base != 5 ? 0 : compressed ? 1 : 2;
	const bool sib = rm.index != kNoReg || base == 4;
	out.byte(static_cast<uint8_t>((mod << 6) | field | (sib ? 4 : base)));
	if (sib)
	{
		const unsigned index = rm.index == kNoReg ? 4 : rm.index & 7;
		const unsigned shift = rm.scale == 8 ? 3 : rm.scale == 4 ? 2 : rm.scale == 2 ? 1 : 0;
		out.byte(static_cast<uint8_t>((shift << 6) | (index << 3) | base));
	}
	if (mod == 1)
		out.imm(rm.disp / static_cast<int32_t>(scale), 1);
	else if (mod == 2)
		out.imm(rm.disp, 4);
}
void emit(Assembler& out, const EvexForm& form, const VectorEncodingPlan& plan, uint8_t mask, bool zero, bool broadcast)
{
	const RM& other = plan.other;
	const bool r = (plan.reg & 8) != 0, r2 = (plan.reg & 16) != 0;
	const bool x = other.is_reg ? (other.reg & 16) != 0 : other.index != kNoReg && (other.index & 8) != 0;
	const bool b = other.is_reg ? (other.reg & 8) != 0 : other.base != kNoReg && (other.base & 8) != 0;
	out.byte(0x62);
	out.byte(static_cast<uint8_t>((r ? 0 : 0x80) | (x ? 0 : 0x40) | (b ? 0 : 0x20) | (r2 ? 0 : 0x10) | form.form.map));
	out.byte(static_cast<uint8_t>((form.form.w << 7) | ((~plan.source & 15) << 3) | 4 | form.form.pp));
	const unsigned length = plan.bits == 512 ? 2 : plan.bits == 256 ? 1 : 0;
	out.byte(static_cast<uint8_t>(
		(zero ? 0x80 : 0) | (length << 5) | (broadcast ? 0x10 : 0) | ((plan.source & 16) ? 0 : 8) | mask));
	if (other.is_reg || other.symbol != asm_model::kNoId || other.label != kNoLabel)
	{
		RM low = other;
		if (low.is_reg)
			low.reg &= 7;
		out.op(0, false, plan.opcode, 1, plan.reg & 7, low, false, plan.has_immediate ? 1 : 0);
	}
	else
	{
		out.byte(plan.opcode);
		address(out, plan.reg, other, plan.memory_bits / 8);
	}
	if (plan.has_immediate)
		out.byte(plan.control);
}
}
bool encode_evex_form(Assembler& out, const assembly::Instruction& inst)
{
	EvexForm form;
	const bool force = required(inst);
	// EVEX-only names also select it without decorators or extended views.
	if (!find(inst.mnemonic, form))
	{
		if (force)
			fail(inst, "EVEX instruction form is not implemented");
		return false;
	}
	if (!force)
	{
		size_t count = 0;
		const auto* only = x86_asm_model::evex_forms(count);
		bool selected = false;
		for (size_t i = 0; i < count; ++i)
			selected = selected || inst.mnemonic == only[i].form.mnemonic;
		if (!selected)
			return false;
	}
	if (inst.prefix)
		fail(inst, "legacy prefix is invalid before EVEX");
	if (inst.operands.empty())
		fail(inst, "EVEX instruction requires operands");
	const auto& destination = inst.operands[0];
	if ((destination.mask && !(form.flags & 1)) ||
		(destination.zero &&
			(!(form.flags & 2) || !destination.mask || destination.kind != assembly::OperandKind::Register)))
		fail(inst, "invalid EVEX destination mask");
	bool broadcast = false;
	for (size_t i = 0; i < inst.operands.size(); ++i)
	{
		const auto& operand = inst.operands[i];
		if (i != 0 && (operand.mask || operand.zero))
			fail(inst, "write mask is only valid on the destination");
		if (operand.broadcast)
		{
			if (broadcast || !(form.flags & 4) || operand.kind != assembly::OperandKind::Memory ||
				i + 1 <
					inst.operands.size() - (form.form.layout == L::NdsImm || form.form.layout == L::UnaryImm ? 1 : 0))
				fail(inst, "invalid EVEX memory broadcast");
			const unsigned bits = destination.bits;
			if (bits / form.element_bits != operand.broadcast)
				fail(inst, "EVEX broadcast count disagrees with vector width");
			broadcast = true;
			form.form.memory_bits = form.element_bits;
		}
	}
	auto plan = prepare_vector_encoding(inst, form.form, 32);
	if (form.form.layout == L::Shift && !plan.has_immediate)
		plan.memory_bits = 128;
	emit(out, form, plan, destination.mask, destination.zero, broadcast);
	return true;
}
} // namespace native
