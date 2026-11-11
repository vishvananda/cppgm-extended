#include "opt/passes.h"

#include <algorithm>

#include "lowir/builder.h"
#include "opt/loops.h"
#include "opt/scalar_fold.h"

namespace opt {

using namespace lowir;

namespace {

struct Counter
{
	TempId value, next;
	Value init = Value::none();
};

// A single-latch phi fed by an exact add-one step of its own type.
// Other recurrences and merged pointers do not receive this range proof.
bool counter(const FunctionBody& body, const Loop& loop, InstId id, Counter& out)
{
	const Instruction& phi = body.instructions[id];
	if (phi.extra_count != 4 || loop.latches.size() != 1)
		return false;
	Value next = phi_incoming(body, phi, loop.latches[0]);
	if (!next.is(ValueKind::Temp) || body.temps[next.index].def == kNoId)
		return false;
	out.value = phi.result;
	out.next = next.index;
	for (uint32_t k = 0; k < phi.extra_count; k += 2)
		if (body.operands[phi.extra_first + k].index != loop.latches[0])
			out.init = body.operands[phi.extra_first + k + 1];
	const Instruction& step = body.instructions[body.temps[next.index].def];
	return step.op == Opcode::Binary && step.type == phi.type && static_cast<BinaryOp>(step.sub) == BinaryOp::Add &&
		step.ops[0] == Value::temp(phi.result) && step.ops[1].is(ValueKind::Integer) &&
		normalize_integer(step.type, body.literals[step.ops[1].index]).low == 1;
}

// A bounded inductive proof for coupled integer phis. Literal entry seeds
// propose one interval; every continuing update must preserve it. Queries
// touch at most 64 values and 128 operands, using stack scratch only.
struct Interval
{
	__int128 lo, hi;
};

class BoundedPhis
{
public:
	BoundedPhis(FunctionContext& c, const LoopForest& forest, uint32_t loop, Type type)
		: c_(c), body_(c.body), forest_(forest), loop_(loop), type_(type)
	{
		const __int128 limit = static_cast<__int128>(1) << (integer_bits(type.kind()) - 1);
		minimum_ = -limit;
		maximum_ = limit - 1;
	}
	void mark(std::vector<uint8_t>& nonnegative);

private:
	bool seeds();
	bool value(Value v, Interval& result, BlockId use);
	void header_range(Value v, BlockId use, Interval& result);
	bool instruction(const Instruction& inst, Interval& result, BlockId block);
	bool arithmetic(const Instruction& inst, Interval& result, BlockId block);
	bool update(Value v, Interval& result);

	struct Node
	{
		TempId temp;
		Interval range;
		bool ready;
	};
	FunctionContext& c_;
	FunctionBody& body_;
	const LoopForest& forest_;
	uint32_t loop_;
	Type type_;
	__int128 minimum_, maximum_;
	Interval hypothesis_;
	TempId phis_[8];
	Node nodes_[64];
	uint32_t phi_count_ = 0, node_count_ = 0, remaining_ = 128;
	bool useful_ = false;
};

bool BoundedPhis::seeds()
{
	const Loop& loop = forest_.loops[loop_];
	hypothesis_ = Interval{ maximum_, minimum_ };
	for (InstId id = body_.blocks[loop.header].first; id != kNoId && body_.instructions[id].op == Opcode::Phi;
		id = body_.instructions[id].next)
	{
		const Instruction& phi = body_.instructions[id];
		if (phi.type != type_)
			continue;
		if (phi_count_ == 8)
			return false;
		phis_[phi_count_++] = phi.result;
		bool entered = false, carried = false;
		for (uint32_t k = 0; k < phi.extra_count; k += 2)
		{
			const BlockId pred = body_.operands[phi.extra_first + k].index;
			const Value v = body_.operands[phi.extra_first + k + 1];
			if (pred == loop.latches[0])
			{
				carried = true;
				continue;
			}
			if (in_loop(forest_, loop_, pred) || !v.is(ValueKind::Integer))
				return false;
			const int64_t seed = static_cast<int64_t>(normalize_integer(type_, body_.literals[v.index]).low);
			if (seed < 0)
				return false;
			hypothesis_.lo = std::min(hypothesis_.lo, static_cast<__int128>(seed));
			hypothesis_.hi = std::max(hypothesis_.hi, static_cast<__int128>(seed));
			entered = true;
		}
		if (!entered || !carried)
			return false;
	}
	return phi_count_ != 0 && hypothesis_.lo < hypothesis_.hi;
}

// With one in-loop header edge, body definitions execute after its guard.
// Header computations themselves keep the unrefined hypotheses.
void BoundedPhis::header_range(Value v, BlockId use, Interval& result)
{
	const Loop& loop = forest_.loops[loop_];
	if (use == loop.header || !in_loop(forest_, loop_, use))
		return;
	const Instruction& term = body_.instructions[body_.blocks[loop.header].last];
	if (term.op != Opcode::Branch || !term.ops[0].is(ValueKind::Temp))
		return;
	const bool on_true = in_loop(forest_, loop_, term.ops[1].index);
	if (on_true == in_loop(forest_, loop_, term.ops[2].index))
		return;
	const InstId def = body_.temps[term.ops[0].index].def;
	if (def == kNoId)
		return;
	const Instruction& cmp = body_.instructions[def];
	if (cmp.op != Opcode::Compare || cmp.type != type_)
		return;
	for (unsigned side = 0; side < 2; ++side)
	{
		if (!(cmp.ops[side] == v))
			continue;
		const Value other = cmp.ops[1 - side];
		Interval range;
		bool known = other.is(ValueKind::Integer);
		if (known)
		{
			const int64_t n = static_cast<int64_t>(normalize_integer(type_, body_.literals[other.index]).low);
			range = Interval{ n, n };
		}
		else
			for (uint32_t k = 0; k < phi_count_; ++k)
				if (other.is(ValueKind::Temp) && other.index == phis_[k])
				{
					range = hypothesis_;
					known = true;
				}
		if (!known)
			continue;
		ComparePredicate pred = static_cast<ComparePredicate>(cmp.sub);
		if (side != 0)
			pred = swapped(pred);
		if (!on_true)
			pred = inverted(pred);
		if (pred == ComparePredicate::Lt || pred == ComparePredicate::Le || pred == ComparePredicate::Eq)
			result.hi = std::min(result.hi, range.hi - (pred == ComparePredicate::Lt ? 1 : 0));
		if (pred == ComparePredicate::Gt || pred == ComparePredicate::Ge || pred == ComparePredicate::Eq)
			result.lo = std::max(result.lo, range.lo + (pred == ComparePredicate::Gt ? 1 : 0));
	}
}

bool BoundedPhis::value(Value v, Interval& result, BlockId use)
{
	if (remaining_ == 0)
		return false;
	--remaining_;
	if (v.is(ValueKind::Integer))
	{
		const int64_t x = static_cast<int64_t>(normalize_integer(type_, body_.literals[v.index]).low);
		result = Interval{ x, x };
		return true;
	}
	if (!v.is(ValueKind::Temp) || body_.temps[v.index].type != type_)
		return false;
	for (uint32_t k = 0; k < phi_count_; ++k)
		if (v.index == phis_[k])
		{
			result = hypothesis_;
			header_range(v, use, result);
			return result.lo <= result.hi;
		}
	for (uint32_t k = 0; k < node_count_; ++k)
		if (nodes_[k].temp == v.index)
		{
			result = nodes_[k].range;
			return nodes_[k].ready;
		}
	const InstId def = body_.temps[v.index].def;
	if (node_count_ == 64 || def == kNoId || c_.block_of[def] == kNoId || !in_loop(forest_, loop_, c_.block_of[def]))
		return false;
	const uint32_t k = node_count_++;
	nodes_[k] = Node{ v.index, Interval{ 0, 0 }, false };
	if (!instruction(body_.instructions[def], result, c_.block_of[def]) || result.lo < minimum_ || result.hi > maximum_)
		return false;
	nodes_[k].range = result;
	nodes_[k].ready = true;
	return true;
}

bool BoundedPhis::instruction(const Instruction& inst, Interval& result, BlockId block)
{
	if (inst.type != type_)
		return false;
	if (inst.op == Opcode::Copy || inst.op == Opcode::Const)
		return value(inst.ops[0], result, block);
	if (inst.op == Opcode::Binary)
		return arithmetic(inst, result, block);
	if (inst.op != Opcode::Phi && inst.op != Opcode::Select)
		return false;
	result = Interval{ maximum_, minimum_ };
	const uint32_t count = inst.op == Opcode::Phi ? inst.extra_count / 2 : 2;
	for (uint32_t k = 0; k < count; ++k)
	{
		Interval part;
		const Value v = inst.op == Opcode::Phi ? body_.operands[inst.extra_first + 2 * k + 1] : inst.ops[k + 1];
		if (!value(v, part, inst.op == Opcode::Phi ? body_.operands[inst.extra_first + 2 * k].index : block))
			return false;
		result.lo = std::min(result.lo, part.lo);
		result.hi = std::max(result.hi, part.hi);
	}
	return count != 0;
}

bool BoundedPhis::arithmetic(const Instruction& inst, Interval& result, BlockId block)
{
	const BinaryOp op = static_cast<BinaryOp>(inst.sub);
	if (op != BinaryOp::Add && op != BinaryOp::Sub && op != BinaryOp::Div && op != BinaryOp::Mod)
		return false;
	Interval a, b;
	if (!value(inst.ops[0], a, block) || !value(inst.ops[1], b, block))
		return false;
	if (op == BinaryOp::Add)
		result = Interval{ a.lo + b.lo, a.hi + b.hi };
	else if (op == BinaryOp::Sub)
		result = Interval{ a.lo - b.hi, a.hi - b.lo };
	else
	{
		if (b.lo != b.hi || b.lo <= 0)
			return false;
		result = op == BinaryOp::Div ? Interval{ a.lo / b.lo, a.hi / b.lo }
									 : Interval{ a.lo < 0 ? 1 - b.lo : 0, a.hi > 0 ? b.lo - 1 : 0 };
		const uint64_t divisor = static_cast<uint64_t>(b.lo);
		useful_ = useful_ || (a.lo >= 0 && (divisor & (divisor - 1)) == 0);
	}
	return true;
}

// Only the latch-to-header edge supplies this restriction. Its facts never
// replace an instruction's ordinary range or leak to an earlier use.
bool BoundedPhis::update(Value v, Interval& result)
{
	if (!value(v, result, forest_.loops[loop_].latches[0]))
		return false;
	const Loop& loop = forest_.loops[loop_];
	const Instruction& term = body_.instructions[body_.blocks[loop.latches[0]].last];
	if (term.op != Opcode::Branch || !term.ops[0].is(ValueKind::Temp))
		return true;
	const InstId def = body_.temps[term.ops[0].index].def;
	if (def == kNoId)
		return true;
	const Instruction& cmp = body_.instructions[def];
	if (cmp.op != Opcode::Compare || cmp.type != type_ || term.ops[1].index == term.ops[2].index)
		return true;
	for (unsigned side = 0; side < 2; ++side)
	{
		Interval other;
		if (!(cmp.ops[side] == v) || !value(cmp.ops[1 - side], other, loop.latches[0]))
			continue;
		ComparePredicate pred = static_cast<ComparePredicate>(cmp.sub);
		if (side != 0)
			pred = swapped(pred);
		if (term.ops[1].index != loop.header)
			pred = inverted(pred);
		if (pred == ComparePredicate::Lt || pred == ComparePredicate::Le || pred == ComparePredicate::Eq)
			result.hi = std::min(result.hi, other.hi - (pred == ComparePredicate::Lt ? 1 : 0));
		if (pred == ComparePredicate::Gt || pred == ComparePredicate::Ge || pred == ComparePredicate::Eq)
			result.lo = std::max(result.lo, other.lo + (pred == ComparePredicate::Gt ? 1 : 0));
	}
	return result.lo <= result.hi;
}

void BoundedPhis::mark(std::vector<uint8_t>& nonnegative)
{
	if (!seeds())
		return;
	const Loop& loop = forest_.loops[loop_];
	for (uint32_t k = 0; k < phi_count_; ++k)
	{
		Interval range;
		const Instruction& phi = body_.instructions[body_.temps[phis_[k]].def];
		if (!update(phi_incoming(body_, phi, loop.latches[0]), range) || range.lo < hypothesis_.lo ||
			range.hi > hypothesis_.hi)
			return;
	}
	if (!useful_)
		return;
	if (nonnegative.empty())
		nonnegative.assign(body_.temps.size(), 0);
	for (uint32_t k = 0; k < phi_count_; ++k)
		nonnegative[phis_[k]] = 1;
	for (uint32_t k = 0; k < node_count_; ++k)
		if (nodes_[k].ready && nodes_[k].range.lo >= 0)
			nonnegative[nodes_[k].temp] = 1;
}

} // namespace

bool mark_no_wrap_counters(FunctionContext& context)
{
	FunctionBody& body = context.body;
	Cfg& cfg = context.ensure_cfg();
	LoopForest forest;
	context.entry_block_of(mark_no_wrap_counters);
	const auto invariant = [&](uint32_t loop, Value value) {
		if (!value.is(ValueKind::Temp) || value.index < body.parameter_count)
			return true;
		const InstId def = body.temps[value.index].def;
		return def != kNoId && context.block_of[def] != kNoId && !in_loop(forest, loop, context.block_of[def]);
	};
	find_loops(body, cfg, forest);
	bool any = false;
	std::vector<uint8_t> nonnegative;
	for (uint32_t loop = 0; loop < forest.loops.size(); ++loop)
	{
		const Loop& l = forest.loops[loop];
		if (l.latches.size() != 1)
			continue;
		for (TypeKind kind : { TypeKind::I8, TypeKind::I16, TypeKind::I32, TypeKind::I64 })
			BoundedPhis(context, forest, loop, Type::scalar(kind)).mark(nonnegative);
		for (InstId i = body.blocks[l.header].first; i != kNoId && body.instructions[i].op == Opcode::Phi;
			i = body.instructions[i].next)
		{
			const Type type = body.instructions[i].type;
			const TypeKind kind = type.kind();
			if (!(kind == TypeKind::I8 || kind == TypeKind::U8 || kind == TypeKind::I16 || kind == TypeKind::U16 ||
					kind == TypeKind::I32 || kind == TypeKind::I64))
				continue;
			Counter iv;
			if (!counter(body, forest.loops[loop], i, iv) || !invariant(loop, iv.init))
				continue;
			const InstId step = body.temps[iv.next].def;
			if (body.instructions[step].op != Opcode::Binary ||
				static_cast<BinaryOp>(body.instructions[step].sub) != BinaryOp::Add)
				continue;
			const bool is_signed = kind == TypeKind::I8 || kind == TypeKind::I16 || kind == TypeKind::I32 ||
				kind == TypeKind::I64;
			// (A latch test sees the start's increment first.)
			bool start_below_max = false;
			if (iv.init.is(ValueKind::Integer))
			{
				const unsigned bits = static_cast<unsigned>(8 * storage_bytes(type)) - (is_signed ? 1 : 0);
				const Literal start = normalize_integer(type, body.literals[iv.init.index]);
				start_below_max = static_cast<int64_t>(start.low) < static_cast<int64_t>((1ull << bits) - 1);
			}
			// The guard: the header's branch on the counter (the header's
			// other successor leaving), or the latch's on the next value.
			// The header's guards only a step past it: one in the header
			// runs before its test, on the leaving pass too.
			bool step_in_header = false;
			for (InstId k = body.blocks[l.header].first; k != kNoId && !step_in_header; k = body.instructions[k].next)
				step_in_header = k == step;
			bool guarded = false;
			for (unsigned which = 0; which < 2 && !guarded; ++which)
			{
				if ((which == 0 && step_in_header) || (which == 1 && !start_below_max))
					continue;
				const BlockId b = which == 0 ? l.header : l.latches[0];
				const Instruction& term = body.instructions[body.blocks[b].last];
				if (term.op != Opcode::Branch || !term.ops[0].is(ValueKind::Temp) ||
					body.temps[term.ops[0].index].def == kNoId)
					continue;
				const Instruction& cmp = body.instructions[body.temps[term.ops[0].index].def];
				if (cmp.op != Opcode::Compare || cmp.type != type)
					continue;
				const bool in1 = in_loop(forest, loop, term.ops[1].index),
						   in2 = in_loop(forest, loop, term.ops[2].index);
				if (in1 == in2)
					continue;
				// Continuing on true (the in-loop target first) or on false.
				const bool continue_on_true = in1;
				const TempId tested = which == 0 ? iv.value : iv.next;
				for (unsigned side = 0; side < 2 && !guarded; ++side)
				{
					if (!(cmp.ops[side].is(ValueKind::Temp) && cmp.ops[side].index == tested) ||
						!invariant(loop, cmp.ops[1 - side]))
						continue;
					ComparePredicate pred = side == 0 ? static_cast<ComparePredicate>(cmp.sub)
													  : swapped(static_cast<ComparePredicate>(cmp.sub));
					if (!continue_on_true)
						pred = inverted(pred);
					guarded = pred == (is_signed ? ComparePredicate::Lt : ComparePredicate::Ult);
				}
			}
			if (!guarded)
				continue;
			if ((body.instructions[step].flags & IF_NO_WRAP) == 0)
			{
				body.instructions[step].flags |= IF_NO_WRAP;
				any = true;
			}
			// A positive step that cannot wrap preserves a nonnegative
			// start, including the last tested next value. Keep this proof
			// local to this unchanged graph; no new serialized fact is needed.
			if (is_signed && iv.init.is(ValueKind::Integer) &&
				static_cast<int64_t>(normalize_integer(type, body.literals[iv.init.index]).low) >= 0)
			{
				if (nonnegative.empty())
					nonnegative.assign(body.temps.size(), 0);
				nonnegative[iv.value] = nonnegative[iv.next] = 1;
			}
		}
	}
	if (nonnegative.empty())
		return any;
	for (BlockId b : body.layout)
		for (InstId i = body.blocks[b].first; i != kNoId; i = body.instructions[i].next)
		{
			Instruction& inst = body.instructions[i];
			if (inst.op != Opcode::Binary || !inst.ops[0].is(ValueKind::Temp) || !nonnegative[inst.ops[0].index] ||
				body.temps[inst.ops[0].index].type != inst.type || !inst.ops[1].is(ValueKind::Integer))
				continue;
			const BinaryOp op = static_cast<BinaryOp>(inst.sub);
			if (op != BinaryOp::Mod && op != BinaryOp::Div)
				continue;
			const int64_t divisor = static_cast<int64_t>(
				normalize_integer(inst.type, body.literals[inst.ops[1].index]).low);
			if (divisor <= 0 || (divisor & (divisor - 1)) != 0)
				continue;
			uint64_t operand = static_cast<uint64_t>(divisor - 1);
			if (op == BinaryOp::Div)
			{
				operand = 0;
				for (uint64_t value = static_cast<uint64_t>(divisor); value > 1; value >>= 1)
					++operand;
			}
			inst.sub = static_cast<uint8_t>(op == BinaryOp::Mod ? BinaryOp::And : BinaryOp::UShr);
			inst.ops[1] = integer_value(body, integer_literal(static_cast<int64_t>(operand)));
			inst.flags &= ~(IF_NO_WRAP | IF_EXACT);
			any = true;
		}
	return any;
}

} // namespace opt
