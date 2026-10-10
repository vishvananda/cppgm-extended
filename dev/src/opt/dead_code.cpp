#include "opt/passes.h"

namespace opt {

using namespace lowir;

namespace {

// Whether an unused instruction can go: pure values, nonvolatile loads,
// stack allocations and calls whose boundary promises no effect.
bool removable(const Program& program, const Instruction& inst)
{
	if (inst.result == kNoId)
		return false;
	if (is_pure(inst))
		return true;
	switch (inst.op)
	{
	case Opcode::Load:
		return (inst.flags & IF_VOLATILE) == 0;
	case Opcode::StackAlloc:
		return true;
	case Opcode::Call:
	{
		const Signature* boundary = program.call_boundary(inst);
		return boundary != nullptr &&
			(boundary->boundary.effects == ir_model::CFXM_READNONE ||
				boundary->boundary.effects == ir_model::CFXM_READONLY) &&
			boundary->boundary.unwind == ir_model::CUM_NO && boundary->boundary.returns != ir_model::CRM_NORETURN;
	}
	default:
		return false;
	}
}

} // namespace

// Mark and sweep: instructions that must stay are roots, and the
// definitions of their operands are live; the rest go (cycles of dead
// phis included). The marking stops once every removable instruction of
// the layout is known live (nothing goes: the usual run).
bool remove_dead_code(FunctionContext& context)
{
	FunctionBody& body = context.body;
	const Program& program = context.program;
	// 1 live, 2 a removable instruction of the layout not yet known live.
	// (Kept across runs for their capacity: assigned or emptied here.)
	static std::vector<uint8_t> live;
	static std::vector<InstId> work;
	live.assign(body.instructions.size(), 0);
	work.clear();
	uint32_t unknown = 0;
	for (BlockId b : body.layout)
		for (InstId i = body.blocks[b].first; i != kNoId; i = body.instructions[i].next)
			if (!removable(program, body.instructions[i]))
			{
				live[i] = 1;
				work.push_back(i);
			}
			else
			{
				live[i] = 2;
				++unknown;
			}
	while (!work.empty() && unknown != 0)
	{
		const InstId i = work.back();
		work.pop_back();
		for_each_operand(body, body.instructions[i], [&](const Value& v) {
			if (!v.is(ValueKind::Temp) || v.index < body.parameter_count)
				return;
			const InstId def = body.temps[v.index].def;
			if (def != kNoId && live[def] != 1)
			{
				unknown -= live[def] == 2 ? 1 : 0;
				live[def] = 1;
				work.push_back(def);
			}
		});
	}
	if (unknown == 0)
		return false;
	bool changed = false;
	for (BlockId b : body.layout)
	{
		InstId prev = kNoId;
		for (InstId i = body.blocks[b].first; i != kNoId;)
		{
			const InstId next = body.instructions[i].next;
			if (live[i] != 1)
			{
				unlink_after(body, b, prev, i);
				++context.stats.dead;
				changed = true;
			}
			else
				prev = i;
			i = next;
		}
	}
	return changed;
}

} // namespace opt
