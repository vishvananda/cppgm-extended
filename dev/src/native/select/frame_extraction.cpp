#include "native/select/frame_extraction.h"

#include <algorithm>

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
	std::stable_sort(extractions.begin(), extractions.end(),
		[](const std::pair<uint32_t, Inst>& a, const std::pair<uint32_t, Inst>& b) { return a.first < b.first; });
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

} // namespace native
