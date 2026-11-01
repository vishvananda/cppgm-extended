#include <vector>

#include "lowir/builder.h"
#include "opt/passes.h"

namespace opt {

using namespace lowir;

namespace {

// Speculated instructions per arm and selects per merge: past them the
// branch is cheaper than computing both sides.
const unsigned kArmCap = 2;
const unsigned kSelectCap = 2;
// The values the walk from a condition back to its merge visits.
const unsigned kCycleWalkCap = 48;

bool selectable(Type type)
{
	return (type.is_integer() || type.is_pointer()) && type.kind() != TypeKind::I128;
}

// Integer computation that cannot trap, safe where its block's branch did
// not lead (its result unused there).
bool speculatable(const FunctionBody& body, const Instruction& inst)
{
	if (inst.result == kNoId || !selectable(body.temps[inst.result].type))
		return false;
	switch (inst.op)
	{
	case Opcode::Copy:
	case Opcode::Index:
	case Opcode::Addr:
		return true;
	case Opcode::Unary:
	case Opcode::Compare:
	case Opcode::Select:
		return selectable(inst.type);
	case Opcode::Binary:
	{
		const BinaryOp op = static_cast<BinaryOp>(inst.sub);
		return selectable(inst.type) && op != BinaryOp::Div && op != BinaryOp::Mod && op != BinaryOp::UDiv &&
			op != BinaryOp::UMod;
	}
	case Opcode::Convert:
	{
		const ConvertOp op = static_cast<ConvertOp>(inst.sub);
		return (op == ConvertOp::SExt || op == ConvertOp::ZExt || op == ConvertOp::Trunc) &&
			selectable(convert_source_type(inst));
	}
	default:
		return false;
	}
}

class SelectFormation
{
public:
	explicit SelectFormation(FunctionContext& context) : c_(context), body_(context.body) {}
	bool run();

private:
	bool arm(BlockId a, BlockId from, BlockId& to) const;
	bool loads_through_merge(Value cond, BlockId join) const;
	bool form(BlockId e);
	InstId hoist(BlockId a, BlockId e, InstId after);

	FunctionContext& c_;
	FunctionBody& body_;
	std::vector<uint8_t> touched_;
};

// A block reached only from `from` that computes at most kArmCap
// speculatable values and jumps on (to `to`).
bool SelectFormation::arm(BlockId a, BlockId from, BlockId& to) const
{
	const Cfg& cfg = c_.cfg;
	if (a == from || touched_[a] || (cfg.flags[a] & (BF_HANDLER | BF_EH)) || cfg.pred_count(a) != 1 ||
		cfg.preds[cfg.pred_first[a]] != from)
		return false;
	const InstId last = body_.blocks[a].last;
	if (last == kNoId || body_.instructions[last].op != Opcode::Jump)
		return false;
	unsigned count = 0;
	for (InstId i = body_.blocks[a].first; i != last; i = body_.instructions[i].next)
		if (++count > kArmCap || !speculatable(body_, body_.instructions[i]))
			return false;
	to = body_.instructions[last].ops[0].index;
	return true;
}

// Moves the arm's computations to the end of e, after `after`.
InstId SelectFormation::hoist(BlockId a, BlockId e, InstId after)
{
	const InstId last = body_.blocks[a].last;
	while (body_.blocks[a].first != last)
	{
		const InstId i = body_.blocks[a].first;
		unlink_after(body_, a, kNoId, i);
		insert_after(body_, e, after, i);
		// Speculated: its facts may not hold where its arm did not run.
		body_.instructions[i].flags &= static_cast<uint8_t>(~(IF_NO_WRAP | IF_EXACT));
		after = i;
	}
	return after;
}

// Whether `cond` reads a load whose address comes from a phi of `join`, as
// `j = a[j] < a[j + 1] ? j + 1 : j` picks a heap's next child (only a back
// edge leads from a merge to the condition before it). Under a cmov the next
// load waits for this one; a branch starts it on the predicted value. Past
// the cache a heap takes twice as long on the cmov, for a sixth saved inside.
bool SelectFormation::loads_through_merge(Value cond, BlockId join) const
{
	struct Step
	{
		TempId temp;
		bool loaded;
	};
	std::vector<Step> work;
	const auto follow = [&](Value v, bool loaded) {
		if (!v.is(ValueKind::Temp) || v.index < body_.parameter_count)
			return;
		for (const Step& s : work)
			if (s.temp == v.index && s.loaded == loaded)
				return;
		work.push_back(Step{ v.index, loaded });
	};
	follow(cond, false);
	for (size_t next = 0; next < work.size() && next < kCycleWalkCap; ++next)
	{
		const Step step = work[next];
		const Instruction& inst = body_.instructions[body_.temps[step.temp].def];
		switch (inst.op)
		{
		case Opcode::Phi:
			for (InstId i = body_.blocks[join].first;
				step.loaded && i != kNoId && body_.instructions[i].op == Opcode::Phi; i = body_.instructions[i].next)
				if (body_.instructions[i].result == step.temp)
					return true;
			for (uint32_t k = 1; k < inst.extra_count; k += 2)
				follow(body_.operands[inst.extra_first + k], step.loaded);
			break;
		case Opcode::Load:
			follow(inst.ops[0], true);
			break;
		case Opcode::Binary:
		case Opcode::Compare:
		case Opcode::Select:
		case Opcode::Index:
		case Opcode::Unary:
		case Opcode::Convert:
		case Opcode::Copy:
			for (unsigned k = 0; k < 3; ++k)
				follow(inst.ops[k], step.loaded);
			break;
		default:
			break;
		}
	}
	return false;
}

// e ends `branch c, T, F`: a diamond (T and F each jump to J) or a
// triangle (one of them is J) whose arms speculate, and whose merge's
// phis differ in at most kSelectCap values between the two paths.
bool SelectFormation::form(BlockId e)
{
	const InstId term = body_.blocks[e].last;
	if (term == kNoId || body_.instructions[term].op != Opcode::Branch)
		return false;
	const Instruction& br = body_.instructions[term];
	const Value cond = br.ops[0];
	const BlockId t = br.ops[1].index, f = br.ops[2].index;
	if (!cond.is(ValueKind::Temp) || !selectable(body_.temps[cond.index].type) || t == f)
		return false;
	BlockId jt = kNoId, jf = kNoId, join = kNoId;
	const bool t_arm = arm(t, e, jt), f_arm = arm(f, e, jf);
	// The blocks the two paths reach the merge from.
	BlockId from_t = kNoId, from_f = kNoId;
	if (t_arm && f_arm && jt == jf)
	{
		join = jt;
		from_t = t;
		from_f = f;
	}
	else if (t_arm && jt == f)
	{
		join = f;
		from_t = t;
		from_f = e;
	}
	else if (f_arm && jf == t)
	{
		join = t;
		from_t = e;
		from_f = f;
	}
	if (join == kNoId || join == e || touched_[join] || (c_.cfg.flags[join] & BF_HANDLER))
		return false;
	unsigned selects = 0;
	for (InstId i = body_.blocks[join].first; i != kNoId && body_.instructions[i].op == Opcode::Phi;
		i = body_.instructions[i].next)
	{
		const Instruction& phi = body_.instructions[i];
		const Value a = phi_incoming(body_, phi, from_t), b = phi_incoming(body_, phi, from_f);
		if (a == b)
			continue;
		if (++selects > kSelectCap || !selectable(phi.type) || a.is(ValueKind::Slot) || b.is(ValueKind::Slot) ||
			a.is(ValueKind::None) || b.is(ValueKind::None))
			return false;
	}
	if (selects != 0 && loads_through_merge(cond, join))
		return false;
	InstId at = previous_in_block(body_, e, term);
	if (from_t == t)
		at = hoist(t, e, at);
	if (from_f == f)
		at = hoist(f, e, at);
	for (InstId i = body_.blocks[join].first; i != kNoId && body_.instructions[i].op == Opcode::Phi;
		i = body_.instructions[i].next)
	{
		const Value a = phi_incoming(body_, body_.instructions[i], from_t);
		const Value b = phi_incoming(body_, body_.instructions[i], from_f);
		Value merged = a;
		if (!(a == b))
		{
			Instruction sel = make_instruction(Opcode::Select, body_.instructions[i].type);
			sel.ops[0] = cond;
			sel.ops[1] = a;
			sel.ops[2] = b;
			sel.location = body_.instructions[term].location;
			const InstId s = append_instruction(body_, sel);
			body_.instructions[s].result = append_temp(body_, body_.instructions[i].type, s);
			insert_after(body_, e, at, s);
			at = s;
			merged = Value::temp(body_.instructions[s].result);
		}
		// e's input to the merge.
		Instruction& phi = body_.instructions[i];
		if (from_t == e || from_f == e)
		{
			for (uint32_t k = 0; k < phi.extra_count; k += 2)
				if (body_.operands[phi.extra_first + k].index == e)
					body_.operands[phi.extra_first + k + 1] = merged;
			continue;
		}
		std::vector<Value> pairs(
			body_.operands.begin() + phi.extra_first, body_.operands.begin() + phi.extra_first + phi.extra_count);
		pairs.push_back(Value::block(e));
		pairs.push_back(merged);
		phi.extra_first = append_operands(body_, pairs.data(), static_cast<uint32_t>(pairs.size()));
		phi.extra_count = static_cast<uint32_t>(pairs.size());
	}
	// The arms end unreachable (the cleanup drops them), their inputs gone.
	const BlockId arms[2] = { from_t == t ? t : kNoId, from_f == f ? f : kNoId };
	for (BlockId a : arms)
	{
		if (a == kNoId)
			continue;
		remove_phi_incoming(body_, join, a);
		Instruction& end = body_.instructions[body_.blocks[a].last];
		end.op = Opcode::Unreachable;
		end.ops[0] = Value::none();
	}
	Instruction& jump = body_.instructions[term];
	jump.op = Opcode::Jump;
	jump.sub = 0;
	jump.flags = 0;
	jump.type = Type::scalar(TypeKind::Void);
	jump.ops[0] = Value::block(join);
	jump.ops[1] = jump.ops[2] = Value::none();
	jump.extra_count = 0;
	touched_[e] = touched_[t] = touched_[f] = touched_[join] = 1;
	++c_.stats.branches_folded;
	return true;
}

bool SelectFormation::run()
{
	c_.ensure_cfg();
	touched_.assign(body_.blocks.size(), 0);
	bool changed = false;
	for (BlockId b : body_.layout)
		if (c_.cfg.reachable(b) && !touched_[b] && !(c_.cfg.flags[b] & BF_EH))
			changed = form(b) || changed;
	if (changed)
		c_.invalidate_cfg();
	return changed;
}

} // namespace

bool form_selects(FunctionContext& context)
{
	SelectFormation formation(context);
	return formation.run();
}

} // namespace opt
