#include <algorithm>
#include <queue>

#include "lowir/builder.h"
#include "opt/passes.h"
#include "opt/scalar_fold.h"

namespace opt {

using namespace lowir;

namespace {

const unsigned kCfgRounds = 8;
// A jump-only block is bypassed into a block with phis only when that
// block has at most this many predecessors (each bypass compares and
// copies its phis' input lists; past the bound the empty block stays,
// as the edge copies of a wide merge need one anyway).
const uint32_t kBypassPreds = 32;

class CfgCleanup
{
public:
	explicit CfgCleanup(FunctionContext& context) : c_(context), body_(context.body), cfg_(context.cfg) {}

	bool run();

private:
	bool truncate_noreturn();
	bool fold_terminators();
	bool remove_unreachable();
	bool preserve_definitions(const std::vector<uint8_t>& dead);
	bool drop_unreachable_edges();
	bool bypass_empty_blocks();
	bool merge_blocks();
	void make_jump(InstId term, BlockId target);
	bool only_jump(BlockId block, BlockId& target) const;
	bool only_unreachable(BlockId block) const;
	void remove_block_from_layout(const std::vector<uint8_t>& dead);
	void drop_blocks(const std::vector<uint8_t>& dead);

	// The CFG facts are rebuilt only after a step changed the graph.
	void ensure_cfg() { c_.ensure_cfg_light(); } // no step reads dominator facts

	FunctionContext& c_;
	FunctionBody& body_;
	Cfg& cfg_;
};

void CfgCleanup::make_jump(InstId term, BlockId target)
{
	Instruction& inst = body_.instructions[term];
	inst.op = Opcode::Jump;
	inst.sub = 0;
	inst.flags = 0;
	inst.type = Type::scalar(TypeKind::Void);
	inst.ops[0] = Value::block(target);
	inst.ops[1] = inst.ops[2] = Value::none();
	inst.extra_count = 0;
}

// Branches and switches whose outcome is known (a literal selector or a
// single distinct target) become jumps; the dropped edges leave their
// targets' phis.
bool CfgCleanup::fold_terminators()
{
	bool changed = false;
	for (BlockId b : body_.layout)
	{
		const InstId id = body_.blocks[b].last;
		if (id == kNoId)
			continue;
		Instruction& term = body_.instructions[id];
		Constant c;
		BlockId target = kNoId;
		if (term.op == Opcode::Branch)
		{
			if (term.ops[1].index == term.ops[2].index)
				target = term.ops[1].index;
			else if (literal_constant(body_, term.ops[0], c) && c.kind == ValueKind::Integer)
				target = (c.bits.low | c.bits.high) != 0 ? term.ops[1].index : term.ops[2].index;
		}
		else if (term.op == Opcode::Switch)
		{
			bool single = true;
			for (uint32_t k = 1; k < term.extra_count; k += 2)
				single = single && body_.operands[term.extra_first + k].index == term.ops[1].index;
			if (single)
				target = term.ops[1].index;
			else if (literal_constant(body_, term.ops[0], c) && c.kind == ValueKind::Integer)
			{
				// The selector's recorded type (record_switch_types).
				const Type type = term.type.is_void() ? Type::scalar(TypeKind::I64) : term.type;
				target = term.ops[1].index;
				for (uint32_t k = 0; k < term.extra_count; k += 2)
				{
					Constant label;
					if (literal_constant(body_, body_.operands[term.extra_first + k], label) &&
						same_constant(type, label, c))
					{
						target = body_.operands[term.extra_first + k + 1].index;
						break;
					}
				}
			}
		}
		if (target == kNoId)
			continue;
		// Every other distinct successor loses this predecessor.
		std::vector<BlockId> dropped;
		for_each_successor(body_, term, [&](BlockId s) {
			if (s == target)
				return;
			for (BlockId d : dropped)
				if (d == s)
					return;
			dropped.push_back(s);
		});
		for (BlockId s : dropped)
			remove_phi_incoming(body_, s, b);
		make_jump(id, target);
		++c_.stats.branches_folded;
		changed = true;
	}
	return changed;
}

void CfgCleanup::remove_block_from_layout(const std::vector<uint8_t>& dead)
{
	std::vector<BlockId> layout;
	for (BlockId b : body_.layout)
		if (!dead[b])
			layout.push_back(b);
	body_.layout.swap(layout);
}

// A value an unreachable block defines that live code still uses (input
// whose textual order, not dominance, places the definition first): one
// that depends only on literals, slots, symbols and such values moves to
// the start of the entry block; any other scalar is undefined on every
// path that reads it and becomes zero. Returns false when an object value
// would be lost (the block then stays).
bool CfgCleanup::preserve_definitions(const std::vector<uint8_t>& dead)
{
	c_.invalidate_block_of();
	// No live use can name a value no unreachable block defines (the common
	// case): the unreachable blocks' instructions are stamped (the stamps
	// restore_definition_order takes a fresh one of) and the temporaries'
	// definitions looked up.
	std::vector<uint32_t>& stamped = c_.passed;
	if (stamped.size() < body_.instructions.size())
		stamped.resize(body_.instructions.size(), 0);
	if (++c_.passed_stamp == 0)
	{
		std::fill(stamped.begin(), stamped.end(), 0);
		c_.passed_stamp = 1;
	}
	const uint32_t stamp = c_.passed_stamp;
	for (BlockId b : body_.layout)
		if (dead[b])
			for (InstId i = body_.blocks[b].first; i != kNoId; i = body_.instructions[i].next)
				stamped[i] = stamp;
	bool defined = false;
	for (TempId t = body_.parameter_count; t < body_.temps.size() && !defined; ++t)
		defined = body_.temps[t].def != kNoId && stamped[body_.temps[t].def] == stamp;
	if (!defined)
		return true;
	map_instruction_blocks(body_, c_.block_of);
	std::vector<uint8_t> state(body_.temps.size(), 0); // 1 moved, 2 zeroed
	const BlockId entry = body_.layout[0];
	InstId at = kNoId;
	// Moves the definition of `t` (and first those of its operands): whether
	// `t` is available at the start of the entry block once its definition
	// (and its operands') moved there. Parameters are; values defined in
	// live blocks are not (they may follow the entry's start). Iterative
	// (a dead chain may be long); a failure is remembered.
	std::vector<uint8_t> failed(body_.temps.size(), 0);
	auto check = [&](TempId t) -> int { // 1 available, 0 not, -1 its operands decide
		if (t < body_.parameter_count || state[t] == 1)
			return 1;
		const InstId d = body_.temps[t].def;
		if (d == kNoId || failed[t])
			return 0;
		const BlockId b = c_.block_of[d];
		if (b == kNoId || !dead[b] || !is_pure(body_.instructions[d]) || body_.instructions[d].op == Opcode::Phi)
		{
			failed[t] = 1;
			return 0;
		}
		return -1;
	};
	struct Frame
	{
		TempId temp;
		uint32_t operand;
	};
	std::vector<Frame> stack;
	auto movable = [&](TempId root) -> bool {
		const int first = check(root);
		if (first >= 0)
			return first == 1;
		stack.assign(1, Frame{ root, 0 });
		bool result = true; // the frame that just finished
		while (!stack.empty())
		{
			if (!result)
			{
				failed[stack.back().temp] = 1;
				stack.pop_back();
				continue;
			}
			const TempId t = stack.back().temp;
			const InstId d = body_.temps[t].def;
			const Instruction& inst = body_.instructions[d];
			const uint32_t total = 3 + inst.extra_count;
			bool descended = false;
			while (stack.back().operand < total)
			{
				const uint32_t k = stack.back().operand++;
				const Value v = k < 3 ? inst.ops[k] : body_.operands[inst.extra_first + k - 3];
				if (!v.is(ValueKind::Temp))
					continue;
				const int r = check(v.index);
				if (r == 1)
					continue;
				if (r == 0)
				{
					result = false;
					break;
				}
				stack.push_back(Frame{ v.index, 0 });
				descended = true;
				break;
			}
			if (descended || !result)
				continue;
			const BlockId b = c_.block_of[d];
			unlink_after(body_, b, previous_in_block(body_, b, d), d);
			insert_after(body_, entry, at, d);
			// It now runs where it did not: its facts may not hold there
			// (lowir.md "Instruction Facts").
			body_.instructions[d].flags &= static_cast<uint8_t>(~(IF_NO_WRAP | IF_EXACT));
			c_.block_of[d] = entry;
			at = d;
			state[t] = 1;
			stack.pop_back();
		}
		return result;
	};
	bool kept = true;
	for (BlockId b : body_.layout)
	{
		if (dead[b])
			continue;
		for (InstId i = body_.blocks[b].first; i != kNoId; i = body_.instructions[i].next)
			for_each_operand(body_, body_.instructions[i], [&](Value& v) {
				if (!v.is(ValueKind::Temp) || v.index < body_.parameter_count)
					return;
				const InstId d = body_.temps[v.index].def;
				if (d == kNoId || c_.block_of[d] == kNoId || !dead[c_.block_of[d]] || state[v.index] == 1 ||
					movable(v.index))
					return;
				const Type type = body_.temps[v.index].type;
				if (type.is_object())
				{
					kept = false;
					return;
				}
				Literal zero = { 0, 0 };
				v = type.is_float() ? literal_value(body_, ValueKind::Float, type.kind(), zero)
									: integer_value(body_, zero);
			});
	}
	return kept;
}

bool CfgCleanup::remove_unreachable()
{
	ensure_cfg();
	std::vector<uint8_t> dead(cfg_.block_count, 0);
	bool any = false;
	for (BlockId b : body_.layout)
		if (!cfg_.reachable(b))
		{
			dead[b] = 1;
			any = true;
		}
	if (!any || !preserve_definitions(dead))
		return false;
	for (BlockId b : body_.layout)
	{
		if (!dead[b])
			continue;
		for (uint32_t k = cfg_.succ_first[b]; k < cfg_.succ_first[b + 1]; ++k)
			if (!dead[cfg_.succs[k]])
				remove_phi_incoming(body_, cfg_.succs[k], b);
		++c_.stats.blocks_removed;
	}
	remove_block_from_layout(dead);
	drop_blocks(dead);
	return true;
}

// The CFG once the unreachable blocks `dead` are gone, as a fresh build of
// the body gives it: the other blocks keep their terminators and EH
// instructions (definitions moved into the entry are pure), so they keep
// their successors and handler targets; the reverse postorder and the
// dominators never held a dead block. Their edges go, the handler flags
// and the predecessors are derived again.
void CfgCleanup::drop_blocks(const std::vector<uint8_t>& dead)
{
	Cfg& cfg = cfg_;
	const uint32_t n = cfg.block_count;
	uint32_t s = 0, h = 0;
	for (BlockId b = 0; b < n; ++b)
	{
		const uint32_t s0 = cfg.succ_first[b], s1 = cfg.succ_first[b + 1];
		const uint32_t h0 = cfg.handler_first[b], h1 = cfg.handler_first[b + 1];
		cfg.succ_first[b] = s;
		cfg.handler_first[b] = h;
		if (dead[b])
		{
			cfg.flags[b] = 0;
			continue;
		}
		cfg.flags[b] &= static_cast<uint8_t>(~BF_HANDLER);
		for (uint32_t k = s0; k < s1; ++k)
			cfg.succs[s++] = cfg.succs[k];
		for (uint32_t k = h0; k < h1; ++k)
			cfg.handlers[h++] = cfg.handlers[k];
	}
	cfg.succ_first[n] = s;
	cfg.handler_first[n] = h;
	cfg.succs.resize(s);
	cfg.handlers.resize(h);
	for (uint32_t k = 0; k < h; ++k)
		cfg.flags[cfg.handlers[k]] |= BF_HANDLER;
	cfg.pred_first.assign(n + 1, 0);
	for (uint32_t k = 0; k < s; ++k)
		++cfg.pred_first[cfg.succs[k] + 1];
	for (uint32_t b = 0; b < n; ++b)
		cfg.pred_first[b + 1] += cfg.pred_first[b];
	std::vector<uint32_t>& fill = cfg.scratch;
	fill.assign(cfg.pred_first.begin(), cfg.pred_first.end() - 1);
	cfg.preds.assign(s, 0);
	for (BlockId b = 0; b < n; ++b)
		for (uint32_t k = cfg.succ_first[b]; k < cfg.succ_first[b + 1]; ++k)
			cfg.preds[fill[cfg.succs[k]]++] = b;
}

bool CfgCleanup::only_jump(BlockId block, BlockId& target) const
{
	const Block& b = body_.blocks[block];
	if (b.first == kNoId || b.first != b.last || body_.instructions[b.first].op != Opcode::Jump)
		return false;
	target = body_.instructions[b.first].ops[0].index;
	return true;
}

bool CfgCleanup::only_unreachable(BlockId block) const
{
	const Block& b = body_.blocks[block];
	return b.first != kNoId && b.first == b.last && body_.instructions[b.first].op == Opcode::Unreachable;
}

// A conditional edge into a block that is only `unreachable` cannot be
// taken: the branch becomes a jump to its other target.
bool CfgCleanup::drop_unreachable_edges()
{
	bool changed = false;
	for (BlockId b : body_.layout)
	{
		const InstId id = body_.blocks[b].last;
		if (id == kNoId || body_.instructions[id].op != Opcode::Branch)
			continue;
		Instruction& term = body_.instructions[id];
		const BlockId t = term.ops[1].index, f = term.ops[2].index;
		if (t == f || (cfg_.flags[t] & BF_HANDLER) != 0 || (cfg_.flags[f] & BF_HANDLER) != 0)
			continue;
		const bool dead_t = only_unreachable(t), dead_f = only_unreachable(f);
		if (dead_t == dead_f)
			continue;
		remove_phi_incoming(body_, dead_t ? t : f, b);
		make_jump(id, dead_t ? f : t);
		changed = true;
	}
	return changed;
}

// A jump-only block B (not the entry or a handler) is bypassed: each
// predecessor P jumps to B's target T directly, and T's phis take B's
// input for P. When P already reaches T, the inputs must agree.
bool CfgCleanup::bypass_empty_blocks()
{
	ensure_cfg();
	bool changed = false;
	const BlockId entry = body_.layout.empty() ? kNoId : body_.layout[0];
	// One CFG per round: a block whose neighborhood a bypass changed waits
	// for the next round.
	std::vector<uint8_t> touched(cfg_.block_count, 0);
	for (BlockId b : body_.layout)
	{
		BlockId target;
		if (b == entry || (cfg_.flags[b] & BF_HANDLER) != 0 || !only_jump(b, target) || target == b ||
			!cfg_.reachable(b) || touched[b] || touched[target])
			continue;
		if (cfg_.pred_count(target) > kBypassPreds && body_.blocks[target].first != kNoId &&
			body_.instructions[body_.blocks[target].first].op == Opcode::Phi)
			continue;
		bool safe = cfg_.pred_count(b) > 0;
		for (uint32_t k = cfg_.pred_first[b]; k < cfg_.pred_first[b + 1] && safe; ++k)
		{
			const BlockId p = cfg_.preds[k];
			if (p == b || touched[p])
			{
				safe = false;
				break;
			}
			// P already a predecessor of T: its phi inputs must equal B's.
			for (InstId i = body_.blocks[target].first; i != kNoId && body_.instructions[i].op == Opcode::Phi;
				i = body_.instructions[i].next)
			{
				const Value direct = phi_incoming(body_, body_.instructions[i], p);
				const Value through = phi_incoming(body_, body_.instructions[i], b);
				Constant x, y;
				if (!direct.is(ValueKind::None) && !(direct == through) &&
					!(literal_constant(body_, direct, x) && literal_constant(body_, through, y) &&
						same_constant(body_.instructions[i].type, x, y)))
					safe = false;
			}
		}
		if (!safe)
			continue;
		// Rewrite each predecessor's terminator and T's phis.
		std::vector<BlockId> preds(
			cfg_.preds.begin() + cfg_.pred_first[b], cfg_.preds.begin() + cfg_.pred_first[b + 1]);
		for (BlockId p : preds)
		{
			for (InstId i = body_.blocks[target].first; i != kNoId && body_.instructions[i].op == Opcode::Phi;
				i = body_.instructions[i].next)
			{
				Instruction& phi = body_.instructions[i];
				if (!phi_incoming(body_, phi, p).is(ValueKind::None))
					continue;
				const Value through = phi_incoming(body_, phi, b);
				const Value pair[2] = { Value::block(p), through };
				// Grow the phi's pair list at the end of the operand pool.
				std::vector<Value> pairs(body_.operands.begin() + phi.extra_first,
					body_.operands.begin() + phi.extra_first + phi.extra_count);
				pairs.push_back(pair[0]);
				pairs.push_back(pair[1]);
				phi.extra_first = append_operands(body_, pairs.data(), static_cast<uint32_t>(pairs.size()));
				phi.extra_count = static_cast<uint32_t>(pairs.size());
			}
			for_each_successor(body_, body_.instructions[body_.blocks[p].last], [&](BlockId& s) {
				if (s == b)
					s = target;
			});
		}
		remove_phi_incoming(body_, target, b);
		// B is unreachable now; the next removal drops it.
		touched[b] = touched[target] = 1;
		for (BlockId p : preds)
			touched[p] = 1;
		changed = true;
		++c_.stats.blocks_removed;
	}
	return changed;
}

// A block whose only predecessor jumps to it joins that predecessor.
bool CfgCleanup::merge_blocks()
{
	ensure_cfg();
	bool changed = false;
	std::vector<uint8_t> dead(cfg_.block_count, 0);
	std::vector<Value> repl(body_.temps.size(), Value::none());
	bool replaced = false;
	for (BlockId a : body_.layout)
	{
		if (dead[a])
			continue;
		// The instruction before A's last once a merge has found it (a
		// chain of merges walks each absorbed block once).
		InstId before_last = kNoId;
		bool before_known = false;
		for (;;)
		{
			const InstId last = body_.blocks[a].last;
			if (last == kNoId || body_.instructions[last].op != Opcode::Jump)
				break;
			const BlockId b = body_.instructions[last].ops[0].index;
			if (b == a || dead[b] || b == body_.layout[0] || (cfg_.flags[b] & BF_HANDLER) != 0 ||
				cfg_.pred_count(b) != 1)
				break;
			// B's phis have one input: A's.
			InstId first = body_.blocks[b].first;
			while (first != kNoId && body_.instructions[first].op == Opcode::Phi)
			{
				const Instruction& phi = body_.instructions[first];
				repl[phi.result] = phi.extra_count >= 2 ? body_.operands[phi.extra_first + 1] : Value::none();
				replaced = true;
				first = phi.next;
			}
			// A's jump is replaced by B's instructions.
			const InstId prev = before_known ? before_last : previous_in_block(body_, a, last);
			unlink_after(body_, a, prev, last);
			before_known = false;
			if (first != kNoId)
			{
				before_last = prev;
				for (InstId i = first; i != body_.blocks[b].last; i = body_.instructions[i].next)
					before_last = i;
				before_known = true;
				if (body_.blocks[a].last == kNoId)
					body_.blocks[a].first = first;
				else
					body_.instructions[body_.blocks[a].last].next = first;
				body_.blocks[a].last = body_.blocks[b].last;
			}
			body_.blocks[b].first = body_.blocks[b].last = kNoId;
			// B's successors now come from A.
			if (body_.blocks[a].last != kNoId)
				for_each_successor(body_, body_.instructions[body_.blocks[a].last],
					[&](BlockId s) { rename_phi_incoming(body_, s, b, a); });
			dead[b] = 1;
			++c_.stats.blocks_merged;
			changed = true;
			// The merged CFG facts for a and b's successors stay valid
			// for this walk: b's successors have the same predecessor
			// counts with a in b's place.
		}
	}
	if (!changed)
		return false;
	remove_block_from_layout(dead);
	if (replaced)
	{
		for (TempId t = 0; t < repl.size(); ++t)
			if (!repl[t].is(ValueKind::None))
			{
				Value r = repl[t];
				while (r.is(ValueKind::Temp) && !repl[r.index].is(ValueKind::None))
					r = repl[r.index];
				repl[t] = r;
			}
		for (BlockId blk : body_.layout)
			for (InstId i = body_.blocks[blk].first; i != kNoId; i = body_.instructions[i].next)
				for_each_operand(body_, body_.instructions[i], [&](Value& v) {
					if (v.is(ValueKind::Temp) && !repl[v.index].is(ValueKind::None))
						v = repl[v.index];
				});
		// The merged phis themselves are gone: unlink them.
		for (BlockId blk : body_.layout)
		{
			InstId prev = kNoId;
			for (InstId i = body_.blocks[blk].first; i != kNoId;)
			{
				const InstId next = body_.instructions[i].next;
				const Instruction& inst = body_.instructions[i];
				if (inst.op == Opcode::Phi && inst.result != kNoId && !repl[inst.result].is(ValueKind::None))
					unlink_after(body_, blk, prev, i);
				else
					prev = i;
				i = next;
			}
		}
	}
	return true;
}

// Nothing after a call that does not return runs: `unreachable` follows
// the call (every block keeps a terminator), and the block's former
// successors lose it as a predecessor.
bool CfgCleanup::truncate_noreturn()
{
	bool changed = false;
	for (BlockId b : body_.layout)
	{
		for (InstId i = body_.blocks[b].first; i != kNoId; i = body_.instructions[i].next)
		{
			const Instruction& inst = body_.instructions[i];
			if (inst.op != Opcode::Call || !c_.program.may_end_block(inst) || inst.next == kNoId)
				continue;
			const InstId last = body_.blocks[b].last;
			if (body_.instructions[last].op == Opcode::Unreachable && inst.next == last)
				break;
			std::vector<BlockId> dropped;
			for_each_successor(body_, body_.instructions[last], [&](BlockId s) {
				for (BlockId d : dropped)
					if (d == s)
						return;
				dropped.push_back(s);
			});
			for (BlockId s : dropped)
				remove_phi_incoming(body_, s, b);
			const InstId end = append_instruction(
				body_, make_instruction(Opcode::Unreachable, Type::scalar(TypeKind::Void)));
			body_.instructions[i].next = end;
			body_.blocks[b].last = end;
			changed = true;
			break;
		}
	}
	return changed;
}

bool CfgCleanup::run()
{
	bool any = false;
	auto step = [&](bool changed) {
		if (changed)
			c_.invalidate_cfg();
		return changed;
	};
	for (unsigned round = 0; round < kCfgRounds; ++round)
	{
		// (After the first round every noreturn call ends its block: the
		// other steps only rewrite terminators, add to the entry's start
		// and merge a block ending in a jump with its successor.)
		bool changed = round == 0 && step(truncate_noreturn());
		changed = step(fold_terminators()) || changed;
		changed = remove_unreachable() || changed; // (keeps the CFG current: drop_blocks)
		ensure_cfg();
		changed = step(drop_unreachable_edges()) || changed;
		changed = step(bypass_empty_blocks()) || changed;
		changed = remove_unreachable() || changed;
		changed = step(merge_blocks()) || changed;
		if (!changed)
			break;
		any = true;
	}
	return any;
}

// Whether `layout` places each definition before its ordinary uses.
bool defines_before_uses(const FunctionBody& body, const std::vector<BlockId>& layout)
{
	std::vector<uint32_t> position(body.instructions.size(), kNoId);
	uint32_t at = 0;
	for (BlockId b : layout)
		for (InstId i = body.blocks[b].first; i != kNoId; i = body.instructions[i].next)
			position[i] = at++;
	for (BlockId b : layout)
		for (InstId i = body.blocks[b].first; i != kNoId; i = body.instructions[i].next)
		{
			const Instruction& inst = body.instructions[i];
			if (inst.op == Opcode::Phi)
				continue;
			bool ok = true;
			for_each_operand(body, inst, [&](const Value& v) {
				if (v.is(ValueKind::Temp) && v.index >= body.parameter_count)
				{
					const InstId def = body.temps[v.index].def;
					ok = ok && def != kNoId && position[def] != kNoId && position[def] < position[i];
				}
			});
			if (!ok)
				return false;
		}
	return true;
}

// The blocks of body.layout in a topological order of "a definition's
// block precedes each block using it", ties in layout order; `fallback`
// when the constraints form a cycle.
std::vector<BlockId> topological_layout(const FunctionBody& body, const std::vector<BlockId>& fallback)
{
	const uint32_t n = static_cast<uint32_t>(body.blocks.size());
	std::vector<uint32_t> rank(n, kNoId);
	for (uint32_t k = 0; k < body.layout.size(); ++k)
		rank[body.layout[k]] = k;
	std::vector<BlockId> block_of(body.instructions.size(), kNoId);
	for (BlockId b : body.layout)
		for (InstId i = body.blocks[b].first; i != kNoId; i = body.instructions[i].next)
			block_of[i] = b;
	std::vector<std::pair<uint32_t, uint32_t>> edges; // (definition block, using block)
	for (BlockId b : body.layout)
		for (InstId i = body.blocks[b].first; i != kNoId; i = body.instructions[i].next)
		{
			const Instruction& inst = body.instructions[i];
			if (inst.op == Opcode::Phi)
				continue;
			for_each_operand(body, inst, [&](const Value& v) {
				if (!v.is(ValueKind::Temp) || v.index < body.parameter_count)
					return;
				const InstId def = body.temps[v.index].def;
				const BlockId d = def != kNoId ? block_of[def] : kNoId;
				if (d != kNoId && d != b)
					edges.push_back(std::make_pair(d, b));
			});
		}
	std::vector<uint32_t> first(n + 1, 0), indegree(n, 0);
	for (const std::pair<uint32_t, uint32_t>& e : edges)
	{
		++first[e.first + 1];
		++indegree[e.second];
	}
	for (uint32_t b = 0; b < n; ++b)
		first[b + 1] += first[b];
	std::vector<uint32_t> fill(first.begin(), first.end() - 1), next(edges.size());
	for (const std::pair<uint32_t, uint32_t>& e : edges)
		next[fill[e.first]++] = e.second;
	std::priority_queue<uint32_t, std::vector<uint32_t>, std::greater<uint32_t>> ready;
	for (BlockId b : body.layout)
		if (indegree[b] == 0)
			ready.push(rank[b]);
	std::vector<BlockId> layout;
	layout.reserve(body.layout.size());
	while (!ready.empty())
	{
		const BlockId b = body.layout[ready.top()];
		ready.pop();
		layout.push_back(b);
		for (uint32_t k = first[b]; k < first[b + 1]; ++k)
			if (--indegree[next[k]] == 0)
				ready.push(rank[next[k]]);
	}
	if (layout.size() != body.layout.size() || layout[0] != body.layout[0])
		return fallback;
	return layout;
}

} // namespace

bool cleanup_cfg(FunctionContext& context)
{
	CfgCleanup cleanup(context);
	const bool changed = cleanup.run();
	// A merge may have placed a definition after a use in layout.
	if (changed)
		restore_definition_order(context);
	return changed;
}

bool restore_definition_order(FunctionContext& context)
{
	FunctionBody& body = context.body;
	// One walk in layout order: each instruction is stamped once passed, so
	// an ordinary use is in order when its definition carries this walk's
	// stamp.
	std::vector<uint32_t>& passed = context.passed;
	if (passed.size() < body.instructions.size())
		passed.resize(body.instructions.size(), 0);
	if (++context.passed_stamp == 0)
	{
		std::fill(passed.begin(), passed.end(), 0);
		context.passed_stamp = 1;
	}
	const uint32_t stamp = context.passed_stamp;
	bool ordered = true;
	for (size_t k = 0; k < body.layout.size() && ordered; ++k)
		for (InstId i = body.blocks[body.layout[k]].first; i != kNoId && ordered; i = body.instructions[i].next)
		{
			const Instruction& inst = body.instructions[i];
			if (inst.op != Opcode::Phi)
				for_each_operand(body, inst, [&](const Value& v) {
					if (v.is(ValueKind::Temp) && v.index >= body.parameter_count)
					{
						const InstId def = body.temps[v.index].def;
						if (def == kNoId || passed[def] != stamp)
							ordered = false;
					}
				});
			passed[i] = stamp;
		}
	context.left_ordered = ordered;
	if (ordered)
		return false;
	// Reverse postorder (reachable blocks, then the others) puts every
	// definition a use is dominated by first. Input may also order a use
	// after a definition it is not dominated by (a flag-guarded shared
	// cleanup); when reverse postorder breaks such a pair, a stable
	// topological order under "a definition's block precedes each block
	// using it" keeps the input's order instead. O((blocks + uses) log
	// blocks).
	context.ensure_cfg_light();
	std::vector<BlockId> layout(context.cfg.rpo);
	for (BlockId b : body.layout)
		if (!context.cfg.reachable(b))
			layout.push_back(b);
	if (defines_before_uses(body, layout))
		context.left_ordered = true;
	else
		layout = topological_layout(body, layout);
	const bool moved = layout != body.layout;
	body.layout.swap(layout);
	return moved;
}

} // namespace opt
