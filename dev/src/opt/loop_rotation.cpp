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
// is entered from it through a preheader. What the body does every pass
// then runs before every exit and once the preheader has run: a load
// has a block that runs once per entry beside the latch's value on the
// back edge (forward_memory's pre_load, hoist_loop_loads), and a store
// can leave the loop. The header's values go
// through slots, promoted again by the caller: its phis stored on its
// predecessors' edges (before a predecessor's terminator), the others
// right after their definition, each use outside the header and each
// read of a phi loading.
const uint32_t kRotateCopy = 4;
// Each round rotates the loops that do not touch one another, in one scan of
// the body: a nest takes a round per level, a chain of adjacent loops a round
// per halving.
const uint32_t kRotateRounds = 16;

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
	struct Use
	{
		InstId inst;
		BlockId block; // the use's block, or a phi input's incoming block
		uint32_t position;
		InstId def;
	};
	struct Candidate
	{
		uint32_t loop;
		BlockId inside, outside;
		std::vector<InstId> defined; // the header's phis, then its plain instructions
		std::vector<Use> uses;
		bool ok;
	};

	bool select_loop(uint32_t loop);
	void read_uses();
	void rotate(const Candidate& candidate);
	void place_preheaders();
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
	std::vector<Candidate> candidates_;
	std::vector<uint32_t> owner_;    // per instruction: the candidate whose header defines it
	std::vector<uint8_t> touched_;   // per block: this round's rotation may change it
	std::vector<BlockId> preheader_; // per block: the preheader to place after it
};

} // namespace

// Whether the loop can rotate, and is apart from those already chosen.
bool LoopRotation::select_loop(uint32_t loop)
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
	// The header's phis take none of its phis (each is stored on the edge: a
	// swap would read what another just wrote).
	for (InstId d : defined)
	{
		const Instruction& phi = body_.instructions[d];
		if (phi.op != Opcode::Phi)
			break;
		for (uint32_t k = 0; k + 1 < phi.extra_count; k += 2)
		{
			const InstId def = of_h(body_.operands[phi.extra_first + k + 1]);
			if (def != kNoId && body_.instructions[def].op == Opcode::Phi)
				return false;
		}
	}
	if (!claim_loop(cfg_, l, touched_))
		return false;
	const uint32_t candidate = static_cast<uint32_t>(candidates_.size());
	for (InstId d : defined)
		owner_[d] = candidate;
	owner_[term] = candidate;
	candidates_.push_back(Candidate{ loop, inside, outside, std::move(defined), {}, true });
	return true;
}

// Every use of the candidates' header values, in one scan of the body. A
// candidate whose values another's header reads waits a round: that header
// must not change before its copies are made.
void LoopRotation::read_uses()
{
	for (BlockId b : body_.layout)
		for (InstId i = body_.blocks[b].first; i != kNoId; i = body_.instructions[i].next)
		{
			const Instruction& inst = body_.instructions[i];
			uint32_t position = 0;
			for_each_operand(body_, inst, [&](const Value& v) {
				const uint32_t at = position++;
				if (!v.is(ValueKind::Temp) || v.index < body_.parameter_count)
					return;
				const InstId def = body_.temps[v.index].def;
				if (def == kNoId || owner_[def] == kNoId)
					return;
				Candidate& c = candidates_[owner_[def]];
				const BlockId h = forest_.loops[c.loop].header;
				const bool phi_input = inst.op == Opcode::Phi && at >= 3;
				const BlockId from = phi_input ? body_.operands[inst.extra_first + (at - 3) - 1].index : b;
				if ((cfg_.flags[from] & BF_HANDLER) != 0)
					c.ok = false;
				if (b != h && owner_[i] != kNoId && owner_[i] != owner_[def])
					c.ok = false;
				// (A phi input on another's header edge: that rotation copies it.)
				const InstId edge = phi_input ? body_.blocks[from].last : kNoId;
				if (edge != kNoId && owner_[edge] != kNoId && owner_[edge] != owner_[def])
					c.ok = false;
				// The header's own reads of its later values stay (the copies
				// rename them); of its phis, they load.
				if (b == h && !phi_input && body_.instructions[def].op != Opcode::Phi)
					return;
				c.uses.push_back(Use{ i, from, at, def });
			});
		}
}

void LoopRotation::rotate(const Candidate& candidate)
{
	const Loop& l = forest_.loops[candidate.loop];
	const BlockId h = l.header, inside = candidate.inside, outside = candidate.outside;
	const InstId term = body_.blocks[h].last;
	const std::vector<Use>& uses = candidate.uses;
	std::vector<InstId> defined = candidate.defined;
	// Demote the header's values, the phis last: what a phi takes from the
	// header is then a load the latch made, ahead of the phi's store.
	std::stable_partition(
		defined.begin(), defined.end(), [&](InstId d) { return body_.instructions[d].op != Opcode::Phi; });
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
		preheader_[h] = pre;
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
		for (const std::pair<BlockId, BlockId>& to : { std::make_pair(inside, pre), std::make_pair(outside, h) })
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
}

void LoopRotation::place_preheaders()
{
	std::vector<BlockId> layout;
	layout.reserve(body_.layout.size() + candidates_.size());
	for (BlockId b : body_.layout)
	{
		layout.push_back(b);
		if (preheader_[b] != kNoId)
			layout.push_back(preheader_[b]);
	}
	body_.layout.swap(layout);
}

bool LoopRotation::run()
{
	bool any = false;
	for (uint32_t round = 0; round < kRotateRounds; ++round)
	{
		c_.ensure_cfg();
		find_loops(body_, cfg_, forest_);
		map_instruction_blocks(body_, c_.block_of);
		c_.invalidate_block_of();
		candidates_.clear();
		owner_.assign(body_.instructions.size(), kNoId);
		touched_.assign(body_.blocks.size(), 0);
		for (uint32_t loop = static_cast<uint32_t>(forest_.loops.size()); loop-- > 0;)
			select_loop(loop);
		if (candidates_.empty())
			break;
		read_uses();
		preheader_.assign(body_.blocks.size(), kNoId);
		bool one = false;
		for (const Candidate& candidate : candidates_)
			if (candidate.ok)
			{
				rotate(candidate);
				one = true;
			}
		if (!one)
			break;
		place_preheaders();
		any = true;
		c_.invalidate_cfg();
	}
	return any;
}

bool merge_loop_entries(FunctionContext& context)
{
	context.ensure_cfg();
	LoopForest forest;
	find_loops(context.body, context.cfg, forest);
	bool any = false;
	for (uint32_t loop = 0; loop < forest.loops.size(); ++loop)
	{
		const Loop& l = forest.loops[loop];
		BlockId entry = kNoId;
		bool several = false;
		for (uint32_t k = context.cfg.pred_first[l.header]; k < context.cfg.pred_first[l.header + 1]; ++k)
		{
			const BlockId p = context.cfg.preds[k];
			if (in_loop(forest, loop, p))
				continue;
			several = several || (entry != kNoId && entry != p);
			entry = p;
		}
		any = (several && make_loop_preheader(context.body, context.cfg, forest, loop)) || any;
	}
	if (any)
		context.invalidate_cfg();
	return any;
}

// Small branchy loops often have a back edge on both the ordinary and boundary
// paths. Join those edges so induction analysis and rotation see one latch.
bool merge_loop_latches(FunctionContext& context)
{
	context.ensure_cfg();
	FunctionBody& body = context.body;
	LoopForest forest;
	find_loops(body, context.cfg, forest);
	std::vector<uint8_t> touched(body.blocks.size(), 0);
	unsigned joined = 0;
	for (uint32_t loop = static_cast<uint32_t>(forest.loops.size()); loop-- > 0 && joined < kRotateRounds;)
	{
		const Loop& l = forest.loops[loop];
		if (l.latches.size() < 2 || l.latches.size() > 4 || l.blocks.size() > 4)
			continue;
		const InstId term = body.blocks[l.header].last;
		if (term == kNoId || body.instructions[term].op != Opcode::Branch)
			continue;
		bool plain = true;
		unsigned count = 0;
		for (InstId i = body.blocks[l.header].first; i != term && plain; i = body.instructions[i].next)
			if (body.instructions[i].op != Opcode::Phi)
				plain = ++count <= kRotateCopy && rotation_copyable(body.instructions[i]);
		if (!plain || !claim_loop(context.cfg, l, touched))
			continue;
		const BlockId latch = append_block(body, BlockKind::LoopPreheader);
		body.layout.push_back(latch);
		InstId tail = kNoId;
		for (InstId i = body.blocks[l.header].first; i != term && body.instructions[i].op == Opcode::Phi;
			i = body.instructions[i].next)
		{
			const Instruction phi = body.instructions[i];
			std::vector<Value> outside, incoming;
			Value common = Value::none();
			bool same = true;
			for (uint32_t k = 0; k < phi.extra_count; k += 2)
			{
				const Value pred = body.operands[phi.extra_first + k], value = body.operands[phi.extra_first + k + 1];
				std::vector<Value>& pairs = in_loop(forest, loop, pred.index) ? incoming : outside;
				if (in_loop(forest, loop, pred.index))
				{
					if (common.is(ValueKind::None))
						common = value;
					else
						same = same && common == value;
				}
				pairs.push_back(pred);
				pairs.push_back(value);
			}
			if (!same)
			{
				Instruction merge = make_instruction(Opcode::Phi, phi.type);
				merge.location = phi.location;
				merge.extra_first = append_operands(body, incoming.data(), static_cast<uint32_t>(incoming.size()));
				merge.extra_count = static_cast<uint32_t>(incoming.size());
				const InstId id = append_instruction(body, merge);
				body.instructions[id].result = append_temp(body, phi.type, id);
				insert_after(body, latch, tail, id);
				tail = id;
				common = Value::temp(body.instructions[id].result);
			}
			outside.push_back(Value::block(latch));
			outside.push_back(common);
			const uint32_t first = append_operands(body, outside.data(), static_cast<uint32_t>(outside.size()));
			body.instructions[i].extra_first = first;
			body.instructions[i].extra_count = static_cast<uint32_t>(outside.size());
		}
		Instruction jump = make_instruction(Opcode::Jump, Type::scalar(TypeKind::Void));
		jump.ops[0] = Value::block(l.header);
		insert_after(body, latch, tail, append_instruction(body, jump));
		for (BlockId from : l.latches)
			for_each_successor(body, body.instructions[body.blocks[from].last], [&](BlockId& target) {
				if (target == l.header)
					target = latch;
			});
		++joined;
	}
	if (joined != 0)
	{
		context.invalidate_cfg();
		context.invalidate_block_of();
		context.invalidate_uses();
	}
	return joined != 0;
}

bool rotate_loops(FunctionContext& context)
{
	LoopRotation pass(context);
	return pass.run();
}

} // namespace opt
