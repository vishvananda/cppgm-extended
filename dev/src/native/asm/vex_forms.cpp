#include "native/asm/vex_forms.h"

#include "native/asm/operands.h"
#include "native/asm/evex_forms.h"
#include "native/asm/vector_operands.h"
#include "native/backend_error.h"
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

}
void encode_vex_plan(Assembler& out, const x86_asm_model::VexForm& form, const VectorEncodingPlan& plan)
{
	emit(out, form, plan.bits, plan.reg, plan.source, plan.other, plan.opcode, plan.has_immediate, plan.control);
}
void encode_assembly_vector_transfer(Assembler& out, const asm_model::Register& reg, const RM& memory, bool store)
{
	if (reg.bits == 512)
	{
		encode_evex_vector_transfer(out, reg, memory, store);
		return;
	}
	if (reg.bank != RegisterBank::Vector || reg.number >= 16 || reg.bits != 256 || reg.offset != 0 || memory.is_reg)
		throw BackendError("invalid assembly vector transfer");
	const VexForm form = { "vmovdqu", L::Move, 0x6f, 1, 2, 0, 3, 0, 0, 0 };
	emit(out, form, reg.bits, reg.number, 0, memory, store ? 0x7f : 0x6f, false, 0);
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
				const auto plan = prepare_vector_encoding(inst, forms[i]);
				emit(out, forms[i], plan.bits, plan.reg, plan.source, plan.other, plan.opcode, plan.has_immediate,
					plan.control);
				return true;
			}
		forms = x86_asm_model::vex_fma_forms(count);
	}
	return false;
}
} // namespace native
