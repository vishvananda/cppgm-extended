#include "opt/loops.h"
#include "opt/passes.h"
#include "opt/scalar_fold.h"

#include <algorithm>

namespace opt {

using namespace lowir;

namespace {

struct Range
{
	uint64_t low, high;
};

const uint64_t kUnsignedMax = 0xffffffffu;

// A one-block loop's incoming value is restricted by that very back edge's
// test. The seed may lie outside the continuing interval; include it too.
bool phi_range(const FunctionBody& body, BlockId block, const Instruction& phi, Range& range)
{
	if (phi.type.kind() != TypeKind::U32 || phi.extra_count != 4)
		return false;
	const Value next = phi_incoming(body, phi, block);
	if (next.is(ValueKind::None))
		return false;
	const unsigned entry = body.operands[phi.extra_first].index == block ? 3 : 1;
	const Value seed = body.operands[phi.extra_first + entry];
	if (!seed.is(ValueKind::Integer))
		return false;
	const Instruction& branch = body.instructions[body.blocks[block].last];
	if (branch.op != Opcode::Branch || !branch.ops[0].is(ValueKind::Temp) ||
		(branch.ops[1].index == block) == (branch.ops[2].index == block))
		return false;
	const InstId def = body.temps[branch.ops[0].index].def;
	if (def == kNoId)
		return false;
	const Instruction& cmp = body.instructions[def];
	if (cmp.op != Opcode::Compare || cmp.type != phi.type)
		return false;
	const unsigned side = cmp.ops[0] == next ? 0 : 1;
	if (!(cmp.ops[side] == next) || !cmp.ops[1 - side].is(ValueKind::Integer))
		return false;
	ComparePredicate pred = static_cast<ComparePredicate>(cmp.sub);
	if (side != 0)
		pred = swapped(pred);
	if (branch.ops[1].index != block)
		pred = inverted(pred);
	if (pred != ComparePredicate::Ult && pred != ComparePredicate::Ule && pred != ComparePredicate::Eq)
		return false;
	const uint64_t first = normalize_integer(phi.type, body.literals[seed.index]).low;
	const uint64_t bound = normalize_integer(phi.type, body.literals[cmp.ops[1 - side].index]).low;
	const uint64_t high = pred == ComparePredicate::Ult && bound != 0 ? bound - 1 : bound;
	range = Range{ pred == ComparePredicate::Eq ? std::min(first, bound) : 0, std::max(first, high) };
	return true;
}

bool arithmetic(const FunctionBody& body, const Instruction& inst, const std::vector<Range>& ranges, Range& out)
{
	auto range = [&](Value v) {
		if (v.is(ValueKind::Integer))
		{
			const uint64_t n = normalize_integer(inst.type, body.literals[v.index]).low;
			return Range{ n, n };
		}
		return v.is(ValueKind::Temp) ? ranges[v.index] : Range{ 0, kUnsignedMax };
	};
	const Range a = range(inst.ops[0]), b = range(inst.ops[1]);
	switch (static_cast<BinaryOp>(inst.sub))
	{
	case BinaryOp::Add:
		out = Range{ a.low + b.low, a.high + b.high };
		break;
	case BinaryOp::Sub:
		if (a.low < b.high)
			return false;
		out = Range{ a.low - b.high, a.high - b.low };
		break;
	case BinaryOp::Mul:
		out = Range{ a.low * b.low, a.high * b.high };
		break;
	case BinaryOp::Shl:
		if (b.low != b.high || b.high >= 32)
			return false;
		out = Range{ a.low << b.low, a.high << b.high };
		break;
	default:
		return false;
	}
	return out.high <= kUnsignedMax;
}

} // namespace

bool mark_unsigned_ranges(FunctionContext& context)
{
	FunctionBody& body = context.body;
	bool any = false;
	for (const Instruction& inst : body.instructions)
		any = any || (inst.op == Opcode::Phi && inst.type.kind() == TypeKind::U32);
	if (!any)
		return false;
	LoopForest forest;
	find_loops(body, context.ensure_cfg(), forest);
	std::vector<std::pair<TempId, Range>> seeds;
	for (const Loop& loop : forest.loops)
	{
		if (loop.blocks.size() != 1)
			continue;
		for (InstId id = body.blocks[loop.header].first; id != kNoId && body.instructions[id].op == Opcode::Phi;
			id = body.instructions[id].next)
		{
			Range range;
			if (phi_range(body, loop.header, body.instructions[id], range) && range.high < kUnsignedMax)
				seeds.push_back({ body.instructions[id].result, range });
		}
	}
	if (seeds.empty())
		return false;
	std::vector<Range> ranges(body.temps.size(), Range{ 0, kUnsignedMax });
	for (const auto& seed : seeds)
		ranges[seed.first] = seed.second;
	bool changed = false;
	for (BlockId block : context.cfg.rpo)
		for (InstId id = body.blocks[block].first; id != kNoId; id = body.instructions[id].next)
		{
			Instruction& inst = body.instructions[id];
			Range range;
			if (inst.op != Opcode::Binary || inst.type.kind() != TypeKind::U32 ||
				!arithmetic(body, inst, ranges, range))
				continue;
			ranges[inst.result] = range;
			changed = changed || (inst.flags & IF_NO_WRAP) == 0;
			inst.flags |= IF_NO_WRAP;
		}
	return changed;
}

} // namespace opt
