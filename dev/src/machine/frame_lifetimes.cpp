#include "machine/frame_lifetimes.h"

#include "opt/loops.h"

namespace machine {
namespace {

using namespace lowir;

struct CopyLife
{
	uint32_t slot, block = kNoId, first = kNoId, last = 0, writes = 0, calls = 0;
	bool pinned = false;
	explicit CopyLife(uint32_t id) : slot(id) {}
};

void touch(const Program& program, const FunctionBody& body, const Instruction& inst, Value value, uint32_t operand,
	BlockId block, uint32_t position, const std::vector<uint32_t>& index, std::vector<CopyLife>& lives)
{
	if (!value.is(ValueKind::Slot) && !value.is(ValueKind::Temp))
		return;
	const auto address = opt::locate(body, value);
	if (!address.root.is(ValueKind::Slot) || index[address.root.index] == kNoId)
		return;
	auto& life = lives[index[address.root.index]];
	if (life.block != kNoId && life.block != block)
		life.pinned = true;
	life.block = block;
	if (life.first == kNoId)
		life.first = position;
	life.last = position;
	if (!address.known || address.offset != 0)
		life.pinned = true;
	else if (inst.op == Opcode::Addr && operand == 0)
		return;
	else if (inst.op == Opcode::CopyObject && operand == 1 && inst.immediate >= 0 &&
		static_cast<uint64_t>(inst.immediate) == body.slots[life.slot].type.bytes)
		++life.writes;
	else if (inst.op == Opcode::Call && operand >= 3)
	{
		const auto* signature = program.call_boundary(inst);
		const uint32_t argument = operand - 3;
		if (signature != nullptr && argument < signature->parameter_count &&
			program.parameters_of(*signature)[argument].type == body.slots[life.slot].type)
			++life.calls;
		else
			life.pinned = true;
	}
	else
		life.pinned = true;
}

} // namespace

std::vector<uint32_t> sequential_copy_slots(const lowir::Program& program, const lowir::FunctionBody& body)
{
	std::vector<CopyLife> lives;
	for (uint32_t s = 0; s < body.slots.size(); ++s)
		if (body.slots[s].type.is_object())
			lives.emplace_back(s);
	if (lives.size() < 2 || lives.size() > 64)
		return {};
	std::vector<uint32_t> index(body.slots.size(), kNoId);
	for (uint32_t i = 0; i < lives.size(); ++i)
		index[lives[i].slot] = i;
	uint32_t position = 0;
	for (BlockId block : body.layout)
		for (InstId i = body.blocks[block].first; i != kNoId; i = body.instructions[i].next, ++position)
		{
			const auto& inst = body.instructions[i];
			if (opt::is_eh_instruction(inst.op) || inst.op == Opcode::StackAlloc || inst.op == Opcode::Asm)
				return {};
			for (uint32_t k = 0; k < 3; ++k)
				touch(program, body, inst, inst.ops[k], k, block, position, index, lives);
			for (uint32_t k = 0; k < inst.extra_count; ++k)
				touch(program, body, inst, body.operands[inst.extra_first + k], k + 3, block, position, index, lives);
		}
	std::vector<uint32_t> owner(body.slots.size(), kNoId);
	for (uint32_t i = 0; i < lives.size(); ++i)
	{
		const auto& life = lives[i];
		if (life.pinned || life.writes != 1 || life.calls != 1)
			continue;
		owner[life.slot] = life.slot;
		for (uint32_t j = 0; j < i; ++j)
		{
			const uint32_t candidate = lives[j].slot;
			if (owner[candidate] != candidate || body.slots[candidate].type != body.slots[life.slot].type)
				continue;
			bool disjoint = true;
			for (uint32_t k = 0; k < i && disjoint; ++k)
				if (owner[lives[k].slot] == candidate)
					disjoint = lives[k].block == life.block &&
						(lives[k].last < life.first || life.last < lives[k].first);
			if (disjoint)
			{
				owner[life.slot] = candidate;
				break;
			}
		}
	}
	return owner;
}

} // namespace machine
