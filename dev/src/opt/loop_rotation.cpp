#include <algorithm>

#include "lowir/builder.h"
#include "opt/loops.h"
#include "opt/passes.h"

namespace opt {

using namespace lowir;

namespace {

// O3, last (finish_function): a loop whose header only tests -- its
// phis, at most kRotateCopy plain instructions and a branch with one side
// in the loop -- and whose latches jump to it is rotated: each latch ends
// in its own copy of the header's instructions and branch, the header
// stays as the entry's guard, and the side in the loop (the new header)
// is entered from it through a preheader. A load the body makes every
// pass then has a block that runs once per entry beside the latch's value
// on the back edge (forward_memory's pre_load, hoist_loop_loads), where
// before the header's test stood between them. The header's values go
// through slots, promoted again by the caller: its phis stored on its
// predecessors' edges (before a predecessor's terminator), the others
// right after their definition, each use outside the header and each
// read of a phi loading.
const uint32_t kRotateCopy = 4;
const uint32_t kRotateLoops = 16; // per body

bool rotation_copyable(const Instruction& inst)
{
	if ((inst.flags & IF_VOLATILE) != 0 || inst.result == kNoId)
		return false;
	switch (inst.op)
	{
	case Opcode::Compare:
	case Opcode::Unary:
	case Opcode::Convert:
	case Opcode::Copy:
	case Opcode::Index:
	case Opcode::Select:
		return !inst.type.is_object();
	case Opcode::Binary:
	{
		const BinaryOp op = static_cast<BinaryOp>(inst.sub);
		return !inst.type.is_object() && op != BinaryOp::Div && op != BinaryOp::Mod && op != BinaryOp::UDiv &&
			op != BinaryOp::UMod;
	}
	case Opcode::Load:
		return !inst.type.is_object() && inst.type.kind() != TypeKind::I128;
	default:
		return false;
	}
}

class LoopRotation
{
public:
	explicit LoopRotation(FunctionContext& context) : c_(context), body_(context.body), cfg_(context.cfg) {}

	bool run();

private:
	bool rotate_loop(uint32_t loop);
	// Whether v is defined outside `loop` (a parameter, or before it).
	bool invariant(uint32_t loop, Value v) const
	{
		if (!v.is(ValueKind::Temp) || v.index < body_.parameter_count)
			return true;
		const InstId def = body_.temps[v.index].def;
		return def != kNoId && def < c_.block_of.size() && c_.block_of[def] != kNoId &&
			!in_loop(forest_, loop, c_.block_of[def]);
	}

	FunctionContext& c_;
	FunctionBody& body_;
	Cfg& cfg_;
	LoopForest forest_;
};

} // namespace

bool LoopRotation::rotate_loop(uint32_t loop)
{
	const Loop& l = forest_.loops[loop];
	const BlockId h = l.header;
	if ((cfg_.flags[h] & BF_HANDLER) != 0 || l.latches.empty() || h == cfg_.rpo[0])
		return false;
	const InstId term = body_.blocks[h].last;
	if (term == kNoId || body_.instructions[term].op != Opcode::Branch)
		return false;
	const BlockId t1 = body_.instructions[term].ops[1].index, t2 = body_.instructions[term].ops[2].index;
	if (t1 == t2 || in_loop(forest_, loop, t1) == in_loop(forest_, loop, t2))
		return false;
	const BlockId inside = in_loop(forest_, loop, t1) ? t1 : t2, outside = inside == t1 ? t2 : t1;
	if (inside == h || ((cfg_.flags[inside] | cfg_.flags[outside]) & BF_HANDLER) != 0)
		return false;
	for (BlockId lt : l.latches)
	{
		const InstId last = body_.blocks[lt].last;
		if (lt == h || last == kNoId || body_.instructions[last].op != Opcode::Jump)
			return false;
	}
	for (uint32_t k = cfg_.pred_first[h]; k < cfg_.pred_first[h + 1]; ++k)
		if ((cfg_.flags[cfg_.preds[k]] & BF_HANDLER) != 0 || body_.blocks[cfg_.preds[k]].last == kNoId)
			return false;
	// The header: phis, then a few plain instructions.
	std::vector<InstId> defined;
	uint32_t plain = 0;
	for (InstId i = body_.blocks[h].first; i != kNoId && i != term; i = body_.instructions[i].next)
	{
		const Instruction& inst = body_.instructions[i];
		if (inst.op == Opcode::Phi)
		{
			if (plain != 0 || inst.type.is_object())
				return false;
		}
		else if (++plain > kRotateCopy || !rotation_copyable(inst))
			return false;
		defined.push_back(i);
	}
	auto of_h = [&](const Value& v) -> InstId {
		if (!v.is(ValueKind::Temp) || v.index < body_.parameter_count)
			return kNoId;
		const InstId def = body_.temps[v.index].def;
		return def != kNoId && std::find(defined.begin(), defined.end(), def) != defined.end() ? def : kNoId;
	};
	// Only where a reload goes: the in-loop side starts (before anything
	// there may write memory) with a load the latches read too -- at the
	// same offset from their input of one of the header's phis, or from the
	// same root from outside the loop -- or with a load of an address from
	// outside a loop that writes no memory (then hoisted).
	bool writes = false;
	for (BlockId b : l.blocks)
		for (InstId i = body_.blocks[b].first; i != kNoId && !writes; i = body_.instructions[i].next)
		{
			const Instruction& inst = body_.instructions[i];
			writes = inst.op == Opcode::Store || inst.op == Opcode::CopyObject || inst.op == Opcode::ZeroInit ||
				may_write_memory(c_.program, inst);
		}
	bool reload = false;
	for (InstId i = body_.blocks[inside].first; i != kNoId && !reload; i = body_.instructions[i].next)
	{
		const Instruction& inst = body_.instructions[i];
		if (inst.op == Opcode::Store || inst.op == Opcode::CopyObject || inst.op == Opcode::ZeroInit ||
			may_write_memory(c_.program, inst))
			break;
		if (inst.op != Opcode::Load || (inst.flags & IF_VOLATILE) != 0 || inst.type.is_object())
			continue;
		const Located at = locate(body_, inst.ops[0]);
		if (!at.known)
			continue;
		const bool outside = invariant(loop, at.root);
		if (outside && !writes)
		{
			reload = true;
			break;
		}
		const InstId phi = of_h(at.root);
		if (!outside && (phi == kNoId || body_.instructions[phi].op != Opcode::Phi))
			continue;
		bool every = true;
		for (BlockId lt : l.latches)
		{
			const Value in = outside ? at.root : phi_incoming(body_, body_.instructions[phi], lt);
			bool found = false;
			for (InstId j = body_.blocks[lt].first; j != kNoId && !found; j = body_.instructions[j].next)
			{
				const Instruction& prior = body_.instructions[j];
				if (prior.op != Opcode::Load || prior.type != inst.type)
					continue;
				const Located p = locate(body_, prior.ops[0]);
				found = p.known && p.root == in && p.offset == at.offset;
			}
			every = every && found;
		}
		reload = every;
	}
	if (!reload)
		return false;
	// The header's phis take nothing the header defines.
	for (InstId d : defined)
	{
		const Instruction& phi = body_.instructions[d];
		if (phi.op != Opcode::Phi)
			break;
		for (uint32_t k = 0; k + 1 < phi.extra_count; k += 2)
			if (of_h(body_.operands[phi.extra_first + k + 1]) != kNoId)
				return false;
	}
	struct Use
	{
		InstId inst;
		BlockId block; // the use's block, or a phi input's incoming block
		uint32_t position;
		InstId def;
	};
	std::vector<Use> uses;
	bool ok = true;
	for (BlockId b : body_.layout)
		for (InstId i = body_.blocks[b].first; i != kNoId && ok; i = body_.instructions[i].next)
		{
			const Instruction& inst = body_.instructions[i];
			uint32_t position = 0;
			for_each_operand(body_, inst, [&](const Value& v) {
				const uint32_t at = position++;
				const InstId def = of_h(v);
				if (def == kNoId)
					return;
				const bool phi_input = inst.op == Opcode::Phi && at >= 3;
				const BlockId from = phi_input ? body_.operands[inst.extra_first + (at - 3) - 1].index : b;
				if ((cfg_.flags[from] & BF_HANDLER) != 0)
					ok = false;
				// The header's own reads of its later values stay (the copies
				// rename them); of its phis, they load.
				if (b == h && !phi_input && body_.instructions[def].op != Opcode::Phi)
					return;
				uses.push_back(Use{ i, from, at, def });
			});
		}
	if (!ok)
		return false;
	// Demote the header's values.
	for (InstId d : defined)
	{
		const Instruction value = body_.instructions[d];
		const Type type = body_.temps[value.result].type;
		bool used = value.op == Opcode::Phi;
		for (const Use& u : uses)
			used = used || u.def == d;
		if (!used)
			continue;
		const SlotId slot = static_cast<SlotId>(body_.slots.size());
		body_.slots.push_back(Slot{ 0, SlotKind::Demote, 0, 0, type });
		if (value.op == Opcode::Phi)
		{
			for (uint32_t k = 0; k + 1 < value.extra_count; k += 2)
			{
				const BlockId from = body_.operands[value.extra_first + k].index;
				Instruction store = make_instruction(Opcode::Store, type);
				store.ops[0] = body_.operands[value.extra_first + k + 1];
				store.ops[1] = Value::slot(slot);
				store.location = value.location;
				insert_after(body_, from, previous_in_block(body_, from, body_.blocks[from].last),
					append_instruction(body_, store));
			}
		}
		else
		{
			Instruction store = make_instruction(Opcode::Store, type);
			store.ops[0] = Value::temp(value.result);
			store.ops[1] = Value::slot(slot);
			store.location = value.location;
			insert_after(body_, h, d, append_instruction(body_, store));
		}
		for (const Use& u : uses)
		{
			if (u.def != d)
				continue;
			Instruction load = make_instruction(Opcode::Load, type);
			load.ops[0] = Value::slot(slot);
			load.location = value.location;
			const InstId id = append_instruction(body_, load);
			body_.instructions[id].result = append_temp(body_, type, id);
			const bool phi_input = body_.instructions[u.inst].op == Opcode::Phi && u.position >= 3;
			const InstId before = phi_input ? body_.blocks[u.block].last : u.inst;
			insert_after(body_, u.block, previous_in_block(body_, u.block, before), id);
			Instruction& again = body_.instructions[u.inst];
			(u.position < 3 ? again.ops[u.position] : body_.operands[again.extra_first + u.position - 3]) = Value::temp(
				body_.instructions[id].result);
		}
		if (value.op == Opcode::Phi)
			unlink_after(body_, h, previous_in_block(body_, h, d), d);
	}
	// The guard enters the loop through a preheader.
	const BlockId pre = append_block(body_, BlockKind::LoopPreheader);
	{
		Instruction jump = make_instruction(Opcode::Jump, Type::scalar(TypeKind::Void));
		jump.ops[0] = Value::block(inside);
		insert_after(body_, pre, kNoId, append_instruction(body_, jump));
		std::vector<BlockId> layout;
		for (BlockId b : body_.layout)
		{
			layout.push_back(b);
			if (b == h)
				layout.push_back(pre);
		}
		body_.layout.swap(layout);
		Instruction& branch = body_.instructions[term];
		(branch.ops[1].index == inside ? branch.ops[1] : branch.ops[2]) = Value::block(pre);
		rename_phi_incoming(body_, inside, h, pre);
	}
	// Each latch tests again with a copy of the header.
	for (BlockId lt : l.latches)
	{
		std::vector<std::pair<TempId, TempId>> renamed;
		auto mapped = [&](Value v) {
			for (const std::pair<TempId, TempId>& n : renamed)
				if (v.is(ValueKind::Temp) && v.index == n.first)
					return Value::temp(n.second);
			return v;
		};
		const InstId jump = body_.blocks[lt].last;
		InstId at = previous_in_block(body_, lt, jump);
		for (InstId i = body_.blocks[h].first; i != kNoId && i != term; i = body_.instructions[i].next)
		{
			Instruction inst = body_.instructions[i];
			for (Value& v : inst.ops)
				v = mapped(v);
			std::vector<Value> extra;
			for (uint32_t k = 0; k < inst.extra_count; ++k)
				extra.push_back(mapped(body_.operands[inst.extra_first + k]));
			inst.extra_first = extra.empty() ? 0 : append_operands(body_, extra.data(), inst.extra_count);
			inst.next = kNoId;
			const TempId old = inst.result;
			const InstId id = append_copy(body_, body_, i, inst);
			if (old != kNoId)
			{
				const TempId fresh = append_temp(body_, body_.temps[old].type, id);
				body_.instructions[id].result = fresh;
				renamed.push_back(std::make_pair(old, fresh));
			}
			insert_after(body_, lt, at, id);
			at = id;
		}
		Instruction& branch = body_.instructions[jump];
		branch.op = Opcode::Branch;
		branch.ops[0] = mapped(body_.instructions[term].ops[0]);
		branch.ops[1] = Value::block(body_.instructions[term].ops[1].index == pre ? inside : outside);
		branch.ops[2] = Value::block(body_.instructions[term].ops[2].index == pre ? inside : outside);
		for (const std::pair<BlockId, BlockId> to : { std::make_pair(inside, pre), std::make_pair(outside, h) })
			for (InstId i = body_.blocks[to.first].first; i != kNoId && body_.instructions[i].op == Opcode::Phi;
				i = body_.instructions[i].next)
			{
				Instruction& phi = body_.instructions[i];
				std::vector<Value> pairs(body_.operands.begin() + phi.extra_first,
					body_.operands.begin() + phi.extra_first + phi.extra_count);
				pairs.push_back(Value::block(lt));
				pairs.push_back(mapped(phi_incoming(body_, phi, to.second)));
				phi.extra_first = append_operands(body_, pairs.data(), static_cast<uint32_t>(pairs.size()));
				phi.extra_count = static_cast<uint32_t>(pairs.size());
			}
	}
	return true;
}

bool LoopRotation::run()
{
	bool any = false;
	for (uint32_t n = 0; n < kRotateLoops; ++n)
	{
		c_.ensure_cfg();
		find_loops(body_, cfg_, forest_);
		map_instruction_blocks(body_, c_.block_of);
		c_.invalidate_block_of();
		bool one = false;
		for (uint32_t loop = static_cast<uint32_t>(forest_.loops.size()); loop-- > 0 && !one;)
			one = rotate_loop(loop);
		if (!one)
			break;
		any = true;
		c_.invalidate_cfg();
	}
	return any;
}

bool rotate_loops(FunctionContext& context)
{
	LoopRotation pass(context);
	return pass.run();
}

} // namespace opt
