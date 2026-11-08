#include "native/asm/operands.h"

#include "native/backend_error.h"

namespace native {
namespace asm_detail {
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
	if (address.symbol.symbol != asm_model::kNoId || address.rip)
	{
		if (!address.rip || address.symbol.symbol == asm_model::kNoId || address.has_base || address.has_index)
			fail(inst, "symbol memory requires an unindexed RIP-relative address");
		return RM::rip(address.symbol.symbol, address.displacement);
	}
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

unsigned integer_width(const assembly::Instruction& inst, uint16_t suffix, unsigned operands)
{
	unsigned bits = suffix;
	bool high = false, rex = suffix == 64;
	for (unsigned i = 0; i < inst.operands.size(); ++i)
	{
		const Operand& operand = inst.operands[i];
		if (operand.kind == OperandKind::Register && operand.reg.bank != RegisterBank::Gpr)
			fail(inst, "expected general register");
		if (i < operands && operand.kind != OperandKind::Immediate && operand.bits != 0)
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

} // namespace asm_detail
} // namespace native
