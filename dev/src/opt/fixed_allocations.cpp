#include "opt/passes.h"

#include "lowir/builder.h"

namespace opt {

using namespace lowir;

bool materialize_fixed_allocations(const Program& program, FunctionBody& body)
{
	bool candidate = false;
	for (BlockId block : body.layout)
		for (InstId i = body.blocks[block].first; i != kNoId; i = body.instructions[i].next)
		{
			const Instruction& inst = body.instructions[i];
			candidate = candidate || (inst.op == Opcode::StackAlloc && inst.ops[0].is(ValueKind::Integer));
		}
	if (!candidate || !body.code_labels.empty())
		return false;
	Cfg cfg;
	build_cfg_light(program, body, cfg);
	std::vector<uint32_t> degree(body.blocks.size(), 0);
	for (BlockId block : cfg.rpo)
	{
		for (uint32_t k = cfg.succ_first[block]; k < cfg.succ_first[block + 1]; ++k)
			++degree[cfg.succs[k]];
		for (uint32_t k = cfg.handler_first[block]; k < cfg.handler_first[block + 1]; ++k)
			++degree[cfg.handlers[k]];
	}
	// Peel from the entry side. Every peeled block is outside every cycle,
	// including irreducible goto and exception cycles. Unpeeled blocks are
	// conservatively left dynamic, even when merely downstream of a loop.
	std::vector<BlockId> work;
	for (BlockId block : cfg.rpo)
		if (degree[block] == 0)
			work.push_back(block);
	bool changed = false;
	for (size_t next = 0; next < work.size(); ++next)
	{
		const BlockId block = work[next];
		for (InstId i = body.blocks[block].first; i != kNoId; i = body.instructions[i].next)
		{
			Instruction& inst = body.instructions[i];
			if (inst.op != Opcode::StackAlloc || !inst.ops[0].is(ValueKind::Integer))
				continue;
			const Literal size = body.literals[inst.ops[0].index];
			// Bound unconditional frame growth; larger requests stay dynamic.
			if (size.high != 0 || size.low == 0 || size.low > 65536)
				continue;
			const SlotId slot = static_cast<SlotId>(body.slots.size());
			body.slots.push_back(Slot{ 0, SlotKind::Refarg, 0, 0, Type::object(size.low, 4) });
			const TempId result = inst.result;
			const InstId successor = inst.next;
			const uint32_t location = inst.location;
			inst = make_instruction(Opcode::Addr, Type::scalar(TypeKind::Ptr));
			inst.result = result;
			inst.next = successor;
			inst.location = location;
			inst.ops[0] = Value::slot(slot);
			changed = true;
		}
		auto remove = [&](BlockId successor) {
			if (--degree[successor] == 0)
				work.push_back(successor);
		};
		for (uint32_t k = cfg.succ_first[block]; k < cfg.succ_first[block + 1]; ++k)
			remove(cfg.succs[k]);
		for (uint32_t k = cfg.handler_first[block]; k < cfg.handler_first[block + 1]; ++k)
			remove(cfg.handlers[k]);
	}
	return changed;
}

// A substituted frame must release its dynamic allocations on every normal
// return. Unwinding restores the enclosing handler's saved stack independently.
void scope_dynamic_allocations(FunctionBody& body)
{
	bool dynamic = false;
	for (BlockId b : body.layout)
		for (InstId i = body.blocks[b].first; i != kNoId; i = body.instructions[i].next)
			dynamic = dynamic || body.instructions[i].op == Opcode::StackAlloc;
	if (!dynamic)
		return;
	Instruction save = make_instruction(Opcode::StackSave, Type::scalar(TypeKind::Ptr));
	const InstId saved = append_instruction(body, save);
	const TempId mark = append_temp(body, save.type, saved);
	body.instructions[saved].result = mark;
	const BlockId entry = body.layout.front();
	InstId before = kNoId;
	for (InstId i = body.blocks[entry].first; i != kNoId && body.instructions[i].op == Opcode::Phi;
		i = body.instructions[i].next)
		before = i;
	insert_after(body, entry, before, saved);
	for (BlockId b : body.layout)
	{
		InstId prev = kNoId;
		for (InstId i = body.blocks[b].first; i != kNoId; i = body.instructions[i].next)
		{
			if (body.instructions[i].op == Opcode::Return)
			{
				Instruction restore = make_instruction(Opcode::StackRestore, Type::scalar(TypeKind::Void));
				restore.ops[0] = Value::temp(mark);
				restore.location = body.instructions[i].location;
				insert_after(body, b, prev, append_instruction(body, restore));
			}
			prev = i;
		}
	}
}

} // namespace opt
