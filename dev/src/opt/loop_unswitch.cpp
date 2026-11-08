#include <algorithm>

#include "lowir/builder.h"
#include "opt/loops.h"
#include "opt/passes.h"

namespace opt {

using namespace lowir;

namespace {

// O3: a branch inside an innermost loop that tests a value the loop does not
// compute takes one path on every pass. The loop is copied, the preheader
// makes the test once, and each copy keeps its own path; what the test guarded
// then reads as straight code, for the passes that hoist and run in lanes. Only
// a branch with both sides in the loop: one that leaves the loop already runs
// at most once. At most kUnswitchBody instructions; each loop's copies are not
// taken up again. The values read past the loop go through slots, promoted
// again by the caller.
const uint32_t kUnswitchBody = 64;
const uint32_t kUnswitchRounds = 8;

class LoopUnswitch
{
public:
	explicit LoopUnswitch(FunctionContext& context) : c_(context), body_(context.body), cfg_(context.cfg) {}

	bool run();

private:
	struct Use
	{
		InstId inst;
		BlockId block; // the use's block, or a phi input's incoming block
		uint32_t position;
		TempId temp;
	};
	struct Candidate
	{
		uint32_t loop;
		InstId branch;
		bool ok;
		std::vector<Use> uses;
	};

	bool invariant(uint32_t loop, Value v) const;
	InstId test_of(uint32_t loop) const;
	bool wanted(uint32_t loop) const;
	void select_loop(uint32_t loop);
	void read_uses();
	void demote(const Candidate& candidate);
	void unswitch(const Candidate& candidate);
	Value mapped(Value v) const;

	FunctionContext& c_;
	FunctionBody& body_;
	Cfg& cfg_;
	LoopForest forest_;
	std::vector<Candidate> candidates_;
	std::vector<uint32_t> owner_;    // per block: the candidate whose loop holds it
	std::vector<uint8_t> touched_;   // per block: this round's rewrites may change it
	std::vector<uint8_t> done_;      // per block: a loop header already copied
	std::vector<TempId> temp_map_;   // per temp: its copy (kNoId: itself)
	std::vector<BlockId> block_map_; // per block: its copy (kNoId: itself)
};

bool LoopUnswitch::invariant(uint32_t loop, Value v) const
{
	if (!v.is(ValueKind::Temp) || v.index < body_.parameter_count)
		return true;
	const InstId def = body_.temps[v.index].def;
	return def != kNoId && def < c_.block_of.size() && c_.block_of[def] != kNoId &&
		!in_loop(forest_, loop, c_.block_of[def]);
}

// The first branch of the loop with both sides in it on a value the loop does
// not compute, or on a compare of two such values.
InstId LoopUnswitch::test_of(uint32_t loop) const
{
	const Loop& l = forest_.loops[loop];
	for (BlockId b : l.blocks)
	{
		const InstId term = body_.blocks[b].last;
		if (term == kNoId || body_.instructions[term].op != Opcode::Branch)
			continue;
		const Instruction& branch = body_.instructions[term];
		const BlockId t1 = branch.ops[1].index, t2 = branch.ops[2].index;
		if (t1 == t2 || !in_loop(forest_, loop, t1) || !in_loop(forest_, loop, t2) ||
			!branch.ops[0].is(ValueKind::Temp))
			continue;
		if (invariant(loop, branch.ops[0]))
			return term;
		const Instruction& test = body_.instructions[body_.temps[branch.ops[0].index].def];
		if (test.op == Opcode::Compare && invariant(loop, test.ops[0]) && invariant(loop, test.ops[1]))
			return term;
	}
	return kNoId;
}

// An innermost loop not copied before, small enough, with nothing the copy
// cannot repeat and a test to take out.
bool LoopUnswitch::wanted(uint32_t loop) const
{
	const Loop& l = forest_.loops[loop];
	if (forest_.innermost[l.header] != loop || done_[l.header] != 0)
		return false;
	uint32_t size = 0;
	for (BlockId b : l.blocks)
		for (InstId i = body_.blocks[b].first; i != kNoId; i = body_.instructions[i].next)
		{
			const Instruction& inst = body_.instructions[i];
			if (++size > kUnswitchBody || inst.op == Opcode::Switch || may_unwind(c_.program, inst) ||
				is_stack_instruction(inst.op) || (inst.op == Opcode::Load && inst.type.is_object()))
				return false;
		}
	return test_of(loop) != kNoId;
}

void LoopUnswitch::select_loop(uint32_t loop)
{
	const Loop& l = forest_.loops[loop];
	if (l.preheader == kNoId || (cfg_.flags[l.preheader] & BF_HANDLER) != 0 || !wanted(loop) ||
		!claim_loop(cfg_, l, touched_))
		return;
	const uint32_t candidate = static_cast<uint32_t>(candidates_.size());
	for (BlockId b : l.blocks)
		owner_[b] = candidate;
	candidates_.push_back(Candidate{ loop, test_of(loop), true, {} });
}

// Every read, outside its loop, of what a candidate's loop computes, in one
// scan of the body. A phi of an exit takes the copy's value on the copy's edge;
// every other read goes through a slot. A read inside another candidate's loop
// waits a round: that loop must not be copied with it unrenamed.
void LoopUnswitch::read_uses()
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
				if (def == kNoId || def >= c_.block_of.size() || c_.block_of[def] == kNoId)
					return;
				const uint32_t own = owner_[c_.block_of[def]];
				if (own == kNoId || owner_[b] == own)
					return;
				Candidate& c = candidates_[own];
				const bool phi_input = inst.op == Opcode::Phi && at >= 3;
				const BlockId from = phi_input ? body_.operands[inst.extra_first + (at - 3) - 1].index : b;
				// (An exit's phi reading the edge from the loop is the copy's.)
				if (phi_input && owner_[from] == own)
					return;
				// (Another's loop is copied with what it reads, phi inputs
				// on its edges too.)
				if (owner_[b] != kNoId || owner_[from] != kNoId || (cfg_.flags[from] & BF_HANDLER) != 0)
					c.ok = false;
				c.uses.push_back(Use{ i, from, at, v.index });
			});
		}
}

// What is read past the loop: a slot, written after each definition (the copy
// then copies the store), each read loading it.
void LoopUnswitch::demote(const Candidate& candidate)
{
	std::vector<TempId> temps;
	for (const Use& u : candidate.uses)
		temps.push_back(u.temp);
	std::sort(temps.begin(), temps.end());
	temps.erase(std::unique(temps.begin(), temps.end()), temps.end());
	for (TempId t : temps)
	{
		const Type type = body_.temps[t].type;
		const SlotId slot = static_cast<SlotId>(body_.slots.size());
		body_.slots.push_back(Slot{ 0, SlotKind::Demote, 0, 0, type });
		const InstId def = body_.temps[t].def;
		const BlockId block = c_.block_of[def];
		// (After the block's phis.)
		InstId after = def;
		while (body_.instructions[after].op == Opcode::Phi &&
			body_.instructions[body_.instructions[after].next].op == Opcode::Phi)
			after = body_.instructions[after].next;
		Instruction store = make_instruction(Opcode::Store, type);
		store.ops[0] = Value::temp(t);
		store.ops[1] = Value::slot(slot);
		store.location = body_.instructions[def].location;
		insert_after(body_, block, after, append_instruction(body_, store));
		for (const Use& u : candidate.uses)
		{
			if (u.temp != t)
				continue;
			Instruction load = make_instruction(Opcode::Load, type);
			load.ops[0] = Value::slot(slot);
			load.location = body_.instructions[def].location;
			const InstId id = append_instruction(body_, load);
			body_.instructions[id].result = append_temp(body_, type, id);
			const bool phi_input = body_.instructions[u.inst].op == Opcode::Phi && u.position >= 3;
			const InstId before = phi_input ? body_.blocks[u.block].last : u.inst;
			insert_after(body_, u.block, previous_in_block(body_, u.block, before), id);
			Instruction& reader = body_.instructions[u.inst];
			(u.position < 3 ? reader.ops[u.position] : body_.operands[reader.extra_first + u.position - 3]) =
				Value::temp(body_.instructions[id].result);
		}
	}
}

Value LoopUnswitch::mapped(Value v) const
{
	if (v.is(ValueKind::Temp) && v.index < temp_map_.size() && temp_map_[v.index] != kNoId)
		return Value::temp(temp_map_[v.index]);
	if (v.is(ValueKind::Block) && v.index < block_map_.size() && block_map_[v.index] != kNoId)
		return Value::block(block_map_[v.index]);
	return v;
}

void LoopUnswitch::unswitch(const Candidate& candidate)
{
	const Loop& l = forest_.loops[candidate.loop];
	const BlockId pre = l.preheader;
	const uint32_t blocks_before = static_cast<uint32_t>(body_.blocks.size());
	demote(candidate);
	// The copy: its blocks and results first, then its instructions.
	std::vector<BlockId> copies;
	block_map_.assign(blocks_before, kNoId);
	temp_map_.assign(body_.temps.size(), kNoId);
	for (BlockId b : l.blocks)
	{
		const BlockId copy = append_block(body_, BlockKind::LoopPreheader);
		block_map_[b] = copy;
		copies.push_back(copy);
		done_.resize(body_.blocks.size(), 0);
		for (InstId i = body_.blocks[b].first; i != kNoId; i = body_.instructions[i].next)
			if (body_.instructions[i].result != kNoId)
				temp_map_[body_.instructions[i].result] = append_temp(
					body_, body_.temps[body_.instructions[i].result].type, kNoId);
	}
	done_[l.header] = 1;
	done_[block_map_[l.header]] = 1;
	const InstId branch_copy_source = candidate.branch;
	InstId branch_copy = kNoId;
	for (size_t k = 0; k < l.blocks.size(); ++k)
	{
		InstId tail = kNoId;
		for (InstId i = body_.blocks[l.blocks[k]].first; i != kNoId; i = body_.instructions[i].next)
		{
			Instruction inst = body_.instructions[i];
			for (Value& v : inst.ops)
				v = mapped(v);
			std::vector<Value> extra;
			for (uint32_t e = 0; e < inst.extra_count; ++e)
				extra.push_back(mapped(body_.operands[inst.extra_first + e]));
			inst.extra_first = extra.empty() ? 0 : append_operands(body_, extra.data(), inst.extra_count);
			inst.next = kNoId;
			if (inst.result != kNoId)
				inst.result = temp_map_[inst.result];
			const InstId id = append_copy(body_, body_, i, inst);
			if (inst.result != kNoId)
				body_.temps[inst.result].def = id;
			insert_after(body_, copies[k], tail, id);
			tail = id;
			if (i == branch_copy_source)
				branch_copy = id;
		}
	}
	// Each exit takes the copy's edges.
	for (size_t k = 0; k < l.blocks.size(); ++k)
		for (uint32_t s = cfg_.succ_first[l.blocks[k]]; s < cfg_.succ_first[l.blocks[k] + 1]; ++s)
		{
			const BlockId exit = cfg_.succs[s];
			if (in_loop(forest_, candidate.loop, exit) ||
				(s > cfg_.succ_first[l.blocks[k]] && cfg_.succs[s - 1] == exit))
				continue;
			for (InstId i = body_.blocks[exit].first; i != kNoId && body_.instructions[i].op == Opcode::Phi;
				i = body_.instructions[i].next)
			{
				Instruction& phi = body_.instructions[i];
				std::vector<Value> pairs(body_.operands.begin() + phi.extra_first,
					body_.operands.begin() + phi.extra_first + phi.extra_count);
				pairs.push_back(Value::block(copies[k]));
				pairs.push_back(mapped(phi_incoming(body_, phi, l.blocks[k])));
				phi.extra_first = append_operands(body_, pairs.data(), static_cast<uint32_t>(pairs.size()));
				phi.extra_count = static_cast<uint32_t>(pairs.size());
			}
		}
	// The preheader tests once and enters one copy through its own block.
	const InstId jump = body_.blocks[pre].last;
	const Instruction test = body_.instructions[candidate.branch];
	Value tested = test.ops[0];
	InstId at = previous_in_block(body_, pre, jump);
	if (!invariant(candidate.loop, tested))
	{
		Instruction compare = body_.instructions[body_.temps[tested.index].def];
		compare.next = kNoId;
		const InstId id = append_instruction(body_, compare);
		const TempId result = append_temp(body_, compare.type, id);
		body_.instructions[id].result = result;
		insert_after(body_, pre, at, id);
		tested = Value::temp(result);
	}
	const BlockId enter[2] = { append_block(body_, BlockKind::LoopPreheader),
		append_block(body_, BlockKind::LoopPreheader) };
	done_.resize(body_.blocks.size(), 0);
	const BlockId headers[2] = { l.header, block_map_[l.header] };
	for (unsigned k = 0; k < 2; ++k)
	{
		Instruction into = make_instruction(Opcode::Jump, Type::scalar(TypeKind::Void));
		into.ops[0] = Value::block(headers[k]);
		insert_after(body_, enter[k], kNoId, append_instruction(body_, into));
		rename_phi_incoming(body_, headers[k], pre, enter[k]);
	}
	Instruction& entry = body_.instructions[jump];
	entry.op = Opcode::Branch;
	entry.ops[0] = tested;
	entry.ops[1] = Value::block(enter[0]);
	entry.ops[2] = Value::block(enter[1]);
	// Each copy keeps its path; the cleanup drops the other.
	Literal one = { 1, 0 }, zero = { 0, 0 };
	body_.instructions[candidate.branch].ops[0] = integer_value(body_, one);
	body_.instructions[branch_copy].ops[0] = integer_value(body_, zero);
	// In layout: the entries before their loops, the copy after the original.
	std::vector<BlockId> layout;
	layout.reserve(body_.layout.size() + copies.size() + 2);
	for (BlockId b : body_.layout)
	{
		if (b == l.header)
			layout.push_back(enter[0]);
		layout.push_back(b);
		if (b == l.blocks.back())
		{
			layout.push_back(enter[1]);
			layout.insert(layout.end(), copies.begin(), copies.end());
		}
	}
	body_.layout.swap(layout);
	++c_.stats.loops_unswitched;
}

bool LoopUnswitch::run()
{
	bool any = false;
	done_.assign(body_.blocks.size(), 0);
	for (unsigned round = 0; round < kUnswitchRounds; ++round)
	{
		c_.ensure_cfg();
		find_loops(body_, cfg_, forest_);
		if (forest_.loops.empty())
			break;
		bool placed = false;
		for (uint32_t loop = 0; loop < forest_.loops.size(); ++loop)
			placed = (forest_.loops[loop].preheader == kNoId && wanted(loop) &&
						 make_loop_preheader(body_, cfg_, forest_, loop)) ||
				placed;
		if (placed)
		{
			any = true;
			c_.invalidate_cfg();
			c_.ensure_cfg();
			find_loops(body_, cfg_, forest_);
			done_.resize(body_.blocks.size(), 0);
		}
		c_.entry_block_of(unswitch_loops);
		map_instruction_blocks(body_, c_.block_of);
		c_.invalidate_block_of();
		candidates_.clear();
		owner_.assign(body_.blocks.size(), kNoId);
		touched_.assign(body_.blocks.size(), 0);
		for (uint32_t loop = static_cast<uint32_t>(forest_.loops.size()); loop-- > 0;)
			select_loop(loop);
		if (candidates_.empty())
			break;
		read_uses();
		bool one = false;
		for (const Candidate& candidate : candidates_)
			if (candidate.ok)
			{
				unswitch(candidate);
				one = true;
			}
		if (!one)
			break;
		any = true;
		c_.invalidate_cfg();
	}
	return any;
}

} // namespace

bool unswitch_loops(FunctionContext& context)
{
	LoopUnswitch pass(context);
	return pass.run();
}

} // namespace opt
