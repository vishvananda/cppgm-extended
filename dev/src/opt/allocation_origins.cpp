#include "opt/allocation_origins.h"

#include <algorithm>

#include "opt/function_ir.h"

namespace opt {

using namespace lowir;

namespace {

bool pointer_step(const Instruction& inst)
{
	return inst.op == Opcode::Index || (inst.op == Opcode::Copy && inst.type.is_pointer());
}

} // namespace

AllocationOrigins::AllocationOrigins(const Program& program, const FunctionBody& body)
{
	bool any = false;
	for (BlockId b : body.layout)
		for (InstId i = body.blocks[b].first; i != kNoId; i = body.instructions[i].next)
			if (fresh_result_call(program, body.instructions[i]))
			{
				if (!any)
					roots_.assign(body.temps.size(), kNoId);
				any = true;
				roots_[body.instructions[i].result] = body.instructions[i].result;
			}
	if (!any)
		return;
	const TempId unknown = kNoId - 1;
	for (TempId& r : roots_)
		if (r == kNoId)
			r = unknown;
	std::vector<TempId> path;
	for (TempId t = 0; t < body.temps.size(); ++t)
	{
		Value value = Value::temp(t);
		path.clear();
		TempId found = kNoId;
		while (value.is(ValueKind::Temp) && body.temps[value.index].type.is_pointer())
		{
			if (roots_[value.index] != unknown)
			{
				found = roots_[value.index];
				break;
			}
			path.push_back(value.index);
			const InstId def = body.temps[value.index].def;
			if (def == kNoId || !pointer_step(body.instructions[def]))
				break;
			value = body.instructions[def].ops[0];
		}
		for (TempId p : path)
			roots_[p] = found;
		if (roots_[t] == unknown)
			roots_[t] = kNoId;
	}
	captured_.assign(roots_.size(), 0);
	for (BlockId b : body.layout)
		for (InstId i = body.blocks[b].first; i != kNoId; i = body.instructions[i].next)
		{
			const Instruction& inst = body.instructions[i];
			if (inst.op == Opcode::FrameAddress)
			{
				std::fill(captured_.begin(), captured_.end(), 1);
				return;
			}
			const bool release = releasing_call(program, inst);
			unsigned operand = 0;
			for_each_operand(body, inst, [&](Value value) {
				const unsigned at = operand++;
				const TempId r = root(value);
				if (r == kNoId)
					return;
				const bool local = (pointer_step(inst) && at == 0) || (inst.op == Opcode::Load && at == 0) ||
					(inst.op == Opcode::Store && at == 1) || inst.op == Opcode::CopyObject ||
					(inst.op == Opcode::ZeroInit && at == 0) || inst.op == Opcode::Compare || (release && at == 3);
				if (!local)
					captured_[r] = 1;
			});
		}
}

TempId AllocationOrigins::root(Value value) const
{
	return value.is(ValueKind::Temp) && value.index < roots_.size() ? roots_[value.index] : kNoId;
}

bool AllocationOrigins::apart(Value a, Value b) const
{
	if ((a.is(ValueKind::Temp) && a.index >= roots_.size()) || (b.is(ValueKind::Temp) && b.index >= roots_.size()))
		return false;
	const TempId x = root(a), y = root(b);
	return x != y && ((x != kNoId && !captured_[x]) || (y != kNoId && !captured_[y]));
}

} // namespace opt
