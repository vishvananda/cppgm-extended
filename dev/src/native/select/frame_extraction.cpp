#include "native/select/frame_extraction.h"

#include "native/mir/type_kinds.h"

namespace native {

// Without a direct frame read, this pass has no rewrite to perform.
bool has_direct_frame_read(const Function& function)
{
	for (const Inst& inst : function.insts)
	{
		if ((inst.flags & MF_VOLATILE) || (inst.op != Op::Load && inst.op != Op::Cmp && inst.op != Op::Test))
			continue;
		for (unsigned k = 0; k < inst.nops; ++k)
		{
			const Operand& operand = inst.ops[k];
			if (operand.kind == OperandKind::Mem && operand.reg == kFrame && operand.index == kNoReg &&
				operand.symbol == lowir::kNoId)
				return true;
		}
	}
	return false;
}

// The caller proves byte coverage, memory validity and dead flags.
bool extract_integer_carrier(
	Inst& inst, uint32_t i, Reg holder, unsigned shift, std::vector<std::pair<uint32_t, Inst>>& extractions)
{
	const lowir::TypeKind type = inst.type;
	if (type != lowir::TypeKind::I1 && type != lowir::TypeKind::I8 && type != lowir::TypeKind::U8 &&
		type != lowir::TypeKind::I16 && type != lowir::TypeKind::U16 && type != lowir::TypeKind::I32 &&
		type != lowir::TypeKind::U32)
		return false;
	const unsigned bytes = register_bytes(type);
	const Reg dst = inst.ops[0].reg;
	const bool sign = type == lowir::TypeKind::I8 || type == lowir::TypeKind::I16 || type == lowir::TypeKind::I32;
	if (shift != 0)
	{
		Inst copy = inst;
		copy.op = Op::Mov;
		copy.type = lowir::TypeKind::Void;
		copy.ops[1] = Operand::make_reg(holder);
		if (dst != holder)
			extractions.push_back(std::make_pair(i, copy));
		inst.op = sign && shift + bytes * 8 == 64 ? Op::Sar : Op::Shr;
		inst.type = lowir::TypeKind::I64;
		inst.ops[1] = Operand::make_imm(shift);
		if (shift + bytes * 8 == 64)
			return true;
		extractions.push_back(std::make_pair(i, inst));
		holder = dst;
	}
	inst.op = sign ? Op::Sext : Op::Zext;
	inst.type = type == lowir::TypeKind::I1 ? lowir::TypeKind::U8 : type;
	inst.ops[1] = Operand::make_reg(holder);
	return true;
}

void insert_frame_extractions(Function& function, std::vector<std::pair<uint32_t, Inst>>& extractions)
{
	if (extractions.empty())
		return;
	std::vector<Inst> insts;
	insts.reserve(function.insts.size() + extractions.size());
	std::vector<uint32_t> offsets(function.insts.size() + 1);
	size_t next = 0;
	for (uint32_t i = 0; i < function.insts.size(); ++i)
	{
		offsets[i] = static_cast<uint32_t>(insts.size());
		while (next < extractions.size() && extractions[next].first == i)
			insts.push_back(extractions[next++].second);
		insts.push_back(function.insts[i]);
	}
	offsets.back() = static_cast<uint32_t>(insts.size());
	for (Block& block : function.blocks)
	{
		const uint32_t past = offsets[block.first + block.count];
		block.first = offsets[block.first];
		block.count = past - block.first;
	}
	function.insts.swap(insts);
}

// The same frame/ABI proof is used before committing field extractions and
// after dead-store elimination. Ignored stores must be plain private frame
// writes: any frame read or exposed address keeps materialization necessary.
bool frame_storage_elision_allowed(const Function& function)
{
	return function.preserve_mask == 0 && function.stack_size != 0 && function.scratch_bytes == 0 &&
		!function.variadic && !function.eh_records_pushed && function.eh.landings.empty() &&
		function.debug_slots.empty() && function.aligned_base == 0;
}

bool removable_frame_storage(const Function& function, bool dead_stores)
{
	if (!frame_storage_elision_allowed(function))
		return false;
	for (const Inst& inst : function.insts)
	{
		switch (inst.op)
		{
		case Op::Mov:
		case Op::Load:
		case Op::Store:
		case Op::Lea:
		case Op::Add:
		case Op::Sub:
		case Op::And:
		case Op::Or:
		case Op::Xor:
		case Op::Imul:
		case Op::Neg:
		case Op::Not:
		case Op::Bswap:
		case Op::Shl:
		case Op::Shr:
		case Op::Sar:
		case Op::Rol:
		case Op::Ror:
		case Op::Cmp:
		case Op::Test:
		case Op::Bt:
		case Op::Setcc:
		case Op::Movzx:
		case Op::Sext:
		case Op::Zext:
		case Op::Cqo:
		case Op::Idiv:
		case Op::Div:
		case Op::Jcc:
		case Op::Jmp:
		case Op::CopyBytes:
		case Op::ZeroBytes:
		case Op::Ret:
		case Op::FMov:
		case Op::FAdd:
		case Op::FSub:
		case Op::FMul:
		case Op::FDiv:
		case Op::FNeg:
		case Op::FAbs:
		case Op::FCmp:
		case Op::FSetcc:
		case Op::Convert:
		case Op::JumpTable:
		case Op::MulWide:
		case Op::ImulWide:
		case Op::Trap:
			break;
		case Op::Nop:
			continue;
		default:
			return false;
		}
		const Operand& dst = inst.ops[0];
		if (dead_stores && inst.op == Op::Store && !(inst.flags & MF_VOLATILE) && dst.kind == OperandKind::Mem &&
			dst.reg == kFrame && dst.index == kNoReg && dst.symbol == lowir::kNoId && dst.disp < 0 &&
			-static_cast<int64_t>(dst.disp) <= function.stack_size &&
			static_cast<int64_t>(dst.disp) + register_bytes(inst.type) <= 0)
			continue;
		for (unsigned k = 0; k < inst.nops; ++k)
			if ((inst.ops[k].kind == OperandKind::Mem &&
					(inst.ops[k].reg == kFrame || inst.ops[k].index == kFrame || inst.ops[k].reg == RSP)) ||
				inst.ops[k].uses_reg(RSP) || inst.ops[k].uses_reg(RBP))
				return false;
	}
	return true;
}

} // namespace native
