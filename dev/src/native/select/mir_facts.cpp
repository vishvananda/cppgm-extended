#include "native/select/mir_facts.h"

#include "native/mir/type_kinds.h"

// See mir_facts.h.
namespace native {

bool flags_dead_after(const std::vector<Inst>& insts, uint32_t i, uint32_t end)
{
	for (uint32_t k = i + 1; k < end; ++k)
	{
		const Inst& inst = insts[k];
		switch (inst.op)
		{
		case Op::Mov:
		case Op::Load:
		case Op::Store:
		case Op::Lea:
		case Op::Movzx:
		case Op::Sext:
		case Op::Zext:
		case Op::Nop:
			break;
		case Op::Cmp:
		case Op::Test:
		case Op::Bt:
		case Op::Add:
		case Op::Sub:
		case Op::And:
		case Op::Or:
		case Op::Xor:
		case Op::Neg:
		case Op::Imul:
		case Op::Bsf:
		case Op::Bsr:
		case Op::Popcnt:
		case Op::Idiv:
		case Op::Div:
		case Op::MulWide:
		case Op::ImulWide:
		case Op::FCmp:
		case Op::Call:
		case Op::Ret:
		case Op::Jmp:
		case Op::JumpTable:
			return true;
		case Op::Shl:
		case Op::Shr:
		case Op::Sar:
			// A shift by a nonzero immediate sets them; one by cl may leave
			// them (a zero count), reading none: the scan goes on.
			if (inst.ops[1].kind == OperandKind::Imm &&
				(inst.ops[1].imm & (inst.type == lowir::TypeKind::U32 ? 31 : 63)) != 0)
				return true;
			if (inst.ops[1].kind == OperandKind::Imm)
				return false;
			break;
		case Op::Rol:
		case Op::Ror:
			// A rotation writes CF and OF only, reading none: the scan
			// goes on.
			break;
		default:
			return false;
		}
	}
	return false;
}

bool plain_gpr(const Operand& op)
{
	return op.kind == OperandKind::Reg && op.reg < 16;
}

bool narrow_kind(lowir::TypeKind type)
{
	return type == lowir::TypeKind::I32 || type == lowir::TypeKind::U32;
}

bool wide_kind(lowir::TypeKind type)
{
	return type == lowir::TypeKind::Void || type == lowir::TypeKind::Ptr || type == lowir::TypeKind::I64;
}

bool simple_inst(const Inst& inst)
{
	switch (inst.op)
	{
	case Op::Mov:
	case Op::Load:
	case Op::Lea:
	case Op::Add:
	case Op::Sub:
	case Op::And:
	case Op::Or:
	case Op::Xor:
	case Op::Imul:
	case Op::Neg:
	case Op::Not:
	case Op::Shl:
	case Op::Shr:
	case Op::Sar:
	case Op::Rol:
	case Op::Ror:
	case Op::Setcc:
	case Op::Cmov:
	case Op::Movzx:
	case Op::Sext:
	case Op::Zext:
	case Op::Cmp:
	case Op::Test:
	case Op::Bt:
	case Op::Store:
	case Op::Nop:
		break;
	default:
		return false;
	}
	for (unsigned k = 0; k < inst.nops; ++k)
	{
		const Operand& op = inst.ops[k];
		if ((op.kind == OperandKind::Mem && op.symbol != lowir::kNoId) || op.kind == OperandKind::Symbol ||
			(op.kind == OperandKind::Imm && static_cast<int64_t>(static_cast<int32_t>(op.imm)) != op.imm) ||
			(op.kind == OperandKind::Reg && op.reg >= 16) ||
			(op.kind == OperandKind::Mem && op.index != kNoReg && op.index >= 16))
			return false;
	}
	return true;
}

Reg written_reg(const Inst& inst)
{
	if (inst.op == Op::Store || inst.op == Op::Cmp || inst.op == Op::Test || inst.op == Op::Bt || inst.op == Op::Nop ||
		inst.ops[0].kind != OperandKind::Reg)
		return kNoReg;
	return inst.ops[0].reg;
}

} // namespace native
