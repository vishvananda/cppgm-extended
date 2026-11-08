#include "native/asm/mask_forms.h"

#include "native/asm/operands.h"
#include "native/backend_error.h"
#include "native/asm/vex_forms.h"

namespace native {
namespace {
using asm_detail::fail;
using asm_model::RegisterBank;
using assembly::Instruction;
using assembly::Operand;
using assembly::OperandKind;
unsigned mask(const Operand& operand, const Instruction& inst)
{
	if (operand.kind != assembly::OperandKind::Register || operand.reg.bank != RegisterBank::Mask ||
		operand.reg.number >= 8)
		fail(inst, "mask move requires k0..k7");
	return operand.reg.number;
}
bool gpr(const Operand& operand)
{
	return operand.kind == assembly::OperandKind::Register && operand.reg.bank == RegisterBank::Gpr;
}
unsigned general(const Operand& operand, unsigned bits, const Instruction& inst)
{
	if (!gpr(operand) || operand.reg.number >= 16 || operand.bits != bits || operand.reg.offset)
		fail(inst, "mask move requires a matching 32/64-bit general register");
	return operand.reg.number;
}
}
void encode_assembly_mask_transfer(
	Assembler& out, const asm_model::Register& reg, const RM& memory, bool store, unsigned bytes)
{
	if (reg.bank != RegisterBank::Mask || reg.number >= 8 || memory.is_reg || reg.offset || reg.bits != bytes * 8)
		throw BackendError("invalid assembly mask transfer");
	size_t count = 0;
	const auto* forms = x86_asm_model::mask_move_forms(count);
	for (size_t i = 0; i < count; ++i)
		if (forms[i].bits == bytes * 8)
		{
			const auto& f = forms[i];
			const x86_asm_model::VexForm form = { "transfer", x86_asm_model::VexLayout::Move, 0x90, 1, f.pp, f.w, 1,
				f.bits, 0, 0 };
			const VectorEncodingPlan plan{ 128, f.bits, reg.number, 0, memory,
				static_cast<uint8_t>(store ? 0x91 : 0x90), 0, false };
			encode_vex_plan(out, form, plan);
			return;
		}
	throw BackendError("unsupported assembly mask transfer width");
}

bool encode_mask_form(Assembler& out, const assembly::Instruction& inst)
{
	size_t count = 0;
	const auto* forms = x86_asm_model::mask_move_forms(count);
	for (size_t i = 0; i < count; ++i)
		if (inst.mnemonic == forms[i].mnemonic)
		{
			if (inst.prefix || inst.operands.size() != 2)
				fail(inst, "mask move requires two operands and no prefix");
			const auto& dst = inst.operands[0];
			const auto& src = inst.operands[1];
			VectorEncodingPlan plan{ 128, forms[i].bits, 0, 0, RM(), 0x90, 0, false };
			x86_asm_model::VexForm form = { inst.mnemonic.c_str(), x86_asm_model::VexLayout::Move, 0x90, 1, forms[i].pp,
				forms[i].w, 1, forms[i].bits, 0, 0 };
			if (gpr(dst) || gpr(src))
			{
				form.pp = forms[i].gpr_pp;
				form.w = forms[i].gpr_w;
				if (gpr(dst))
				{
					plan.reg = general(dst, forms[i].bits == 64 ? 64 : 32, inst);
					plan.other = RM::r(static_cast<Reg>(mask(src, inst)));
					plan.opcode = 0x93;
				}
				else
				{
					plan.reg = mask(dst, inst);
					plan.other = RM::r(static_cast<Reg>(general(src, forms[i].bits == 64 ? 64 : 32, inst)));
					plan.opcode = 0x92;
				}
			}
			else if (dst.kind == assembly::OperandKind::Memory || src.kind == assembly::OperandKind::Memory)
			{
				const bool store = dst.kind == assembly::OperandKind::Memory;
				const auto& memory = store ? dst : src;
				if (memory.bits && memory.bits != forms[i].bits)
					fail(inst, "mask memory width disagrees with mnemonic");
				plan.reg = mask(store ? src : dst, inst);
				plan.other = asm_detail::rm(memory, inst);
				plan.opcode = store ? 0x91 : 0x90;
			}
			else
			{
				plan.reg = mask(dst, inst);
				plan.other = RM::r(static_cast<Reg>(mask(src, inst)));
			}
			encode_vex_plan(out, form, plan);
			return true;
		}
	return false;
}
} // namespace native
