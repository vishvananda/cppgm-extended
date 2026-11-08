#include "native/asm/extend_forms.h"

#include "native/asm/operands.h"

namespace native {

void encode_extend_form(Assembler& out, const assembly::Instruction& inst, const asm_detail::Form& form)
{
	using assembly::OperandKind;
	using namespace asm_detail;
	if (inst.prefix != 0 || inst.operands.size() != 2 || inst.operands[0].kind != OperandKind::Register ||
		inst.operands[0].reg.bank != asm_model::RegisterBank::Gpr ||
		(inst.operands[1].kind != OperandKind::Register && inst.operands[1].kind != OperandKind::Memory))
		fail(inst, "extension move requires a destination GPR and register/memory source");
	const assembly::Operand& destination = inst.operands[0];
	const assembly::Operand& source = inst.operands[1];
	const unsigned bits = destination.bits;
	const unsigned source_bits = form.source_bits != 0 ? form.source_bits : source.bits;
	if ((bits != 16 && bits != 32 && bits != 64) || (form.bits != 0 && form.bits != bits) ||
		(source.bits != 0 && source.bits != source_bits) ||
		(source.kind == OperandKind::Register && source.reg.bank != asm_model::RegisterBank::Gpr))
		fail(inst, "invalid extension move register widths");
	uint32_t opcode = form.opcode;
	if (opcode == 0x63)
	{
		if (bits != 64 || source_bits != 32)
			fail(inst, "movsxd source width differs from its encoding");
	}
	else
	{
		if ((source_bits != 8 && source_bits != 16 && source_bits != 32) || source_bits >= bits)
			fail(inst, "extension move source must have a smaller encodable width");
		if (source_bits == 32)
		{
			if (opcode != 0x0fbe || bits != 64)
				fail(inst, "zero-extension from 32 bits has no movzx encoding");
			opcode = 0x63;
		}
		else
			opcode = (opcode & ~1u) + (source_bits == 16);
	}
	const bool high = source.kind == OperandKind::Register && source.reg.offset != 0;
	if (high && (bits == 64 || destination.reg.number >= 8))
		fail(inst, "high-byte source cannot be encoded with a REX destination");
	const bool byte_rex = source.kind == OperandKind::Register && source_bits == 8 && !high && source.reg.number >= 4 &&
		source.reg.number < 8;
	out.op(bits == 16 ? 0x66 : 0, bits == 64, opcode, opcode > 0xff ? 2 : 1, physical(destination.reg, inst),
		rm(source, inst), byte_rex);
}

} // namespace native
