#include <algorithm>
#include <vector>

#include "lowir/builder.h"
#include "opt/passes.h"
#include "opt/scalar_fold.h"

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

struct BitChoice
{
	Value source;
	Type type;
	unsigned bit;
	bool when_set;
};

struct IndexedChoice
{
	Value base;
	BitChoice bit;
	int64_t bias;
	uint64_t delta;
	bool invert;
};

// A floating select the lanes' `min` and `max` give exactly: each is
// `first < second ? first : second` (or `>`), the second when unordered.
struct MinMax
{
	bool max;
	Value first, second;
};

class SelectFormation
{
public:
	explicit SelectFormation(FunctionContext& context) : c_(context), body_(context.body) {}
	bool run();

private:
	const Instruction* producer(Value v) const;
	Value pointer_source(Value v) const;
	bool bit_choice(Value cond, BitChoice& choice) const;
	bool match_indexed_choice(Value cond, Value a, Value b, IndexedChoice& choice) const;
	Value emit_indexed(BlockId e, InstId& at, const IndexedChoice& choice, uint32_t location);
	bool indexed_choice(BlockId e, InstId& at, Value cond, Value a, Value b, uint32_t location, Value& out);
	bool arm(BlockId a, BlockId from, BlockId& to) const;
	bool loads_through_merge(Value cond, BlockId join) const;
	bool form(BlockId e);
	bool form_store(BlockId e);
	InstId store_arm(BlockId arm, BlockId from) const;
	bool same_expression(Value a, Value b, unsigned depth = 0) const;
	bool unit_update(Type type, Value a, Value b) const;
	bool range_switch(BlockId block, InstId id);
	bool form_load(BlockId e);
	InstId load_arm(BlockId a, BlockId from) const;
	InstId hoist(BlockId a, BlockId e, InstId after);
	bool nonzero_literal(Value v) const;
	bool min_or_max(Value cond, Value a, Value b, MinMax& out) const;
	Value emit_min_max(BlockId e, InstId& at, const MinMax& m, Type type, uint32_t location);

	FunctionContext& c_;
	FunctionBody& body_;
	std::vector<uint8_t> touched_;
	const std::vector<uint32_t>* uses_ = nullptr;
};

const Instruction* SelectFormation::producer(Value v) const
{
	if (!v.is(ValueKind::Temp) || v.index < body_.parameter_count || body_.temps[v.index].def == kNoId)
		return nullptr;
	return &body_.instructions[body_.temps[v.index].def];
}

Value SelectFormation::pointer_source(Value v) const
{
	for (unsigned depth = 0; depth < 4; ++depth)
	{
		const Instruction* inst = producer(v);
		if (inst == nullptr || inst->op != Opcode::Copy || !inst->type.is_pointer())
			break;
		v = inst->ops[0];
	}
	return v;
}

// A mask of one bit, optionally compared with zero or that mask. A
// constant shift before the mask identifies the original input bit.
bool SelectFormation::bit_choice(Value cond, BitChoice& choice) const
{
	const Instruction* test = producer(cond);
	if (test == nullptr)
		return false;
	bool compared = test->op == Opcode::Compare;
	uint64_t compared_value = 0;
	ComparePredicate predicate = ComparePredicate::Ne;
	if (compared)
	{
		predicate = static_cast<ComparePredicate>(test->sub);
		if (predicate != ComparePredicate::Eq && predicate != ComparePredicate::Ne)
			return false;
		const unsigned k = test->ops[0].is(ValueKind::Integer) ? 0 : 1;
		if (!test->ops[k].is(ValueKind::Integer))
			return false;
		compared_value = body_.literals[test->ops[k].index].low;
		test = producer(test->ops[1 - k]);
	}
	if (test == nullptr || test->op != Opcode::Binary || static_cast<BinaryOp>(test->sub) != BinaryOp::And)
		return false;
	const unsigned width = lowir::integer_bits(test->type.kind());
	if (width == 0 || width > 64)
		return false;
	const unsigned k = test->ops[0].is(ValueKind::Integer) ? 0 : 1;
	if (!test->ops[k].is(ValueKind::Integer))
		return false;
	const uint64_t all = width == 64 ? ~0ull : (uint64_t(1) << width) - 1;
	const uint64_t mask = body_.literals[test->ops[k].index].low & all;
	if (mask == 0 || (mask & (mask - 1)) != 0)
		return false;
	compared_value &= all;
	if (compared && compared_value != 0 && compared_value != mask)
		return false;
	choice.source = test->ops[1 - k];
	choice.type = test->type;
	choice.bit = static_cast<unsigned>(__builtin_ctzll(mask));
	choice.when_set = !compared || (predicate == ComparePredicate::Eq ? compared_value == mask : compared_value == 0);
	const Instruction* shift = producer(choice.source);
	if (shift == nullptr || shift->op != Opcode::Binary || shift->type != choice.type ||
		!shift->ops[1].is(ValueKind::Integer))
		return true;
	const BinaryOp op = static_cast<BinaryOp>(shift->sub);
	if (op != BinaryOp::Shr && op != BinaryOp::UShr && op != BinaryOp::Shl)
		return true;
	const Literal count = body_.literals[shift->ops[1].index];
	if (count.high != 0 || count.low >= width)
		return false;
	if (op == BinaryOp::Shl)
	{
		if (choice.bit < count.low)
			return false;
		choice.bit -= static_cast<unsigned>(count.low);
	}
	else
	{
		if (choice.bit + count.low >= width)
			return false;
		choice.bit += static_cast<unsigned>(count.low);
	}
	choice.source = shift->ops[0];
	return true;
}

// A bit chooses two constant offsets from the same base. Their byte
// distance is a power of two: compute that one scaled bit instead of
// computing both full addresses. No load is introduced or speculated.
bool SelectFormation::match_indexed_choice(Value cond, Value a, Value b, IndexedChoice& choice) const
{
	a = pointer_source(a);
	b = pointer_source(b);
	const Instruction* left = producer(a);
	const Instruction* right = producer(b);
	if (left == nullptr || right == nullptr || left->op != Opcode::Index || right->op != Opcode::Index ||
		left->type != right->type || !(left->ops[0] == right->ops[0]) || !left->ops[1].is(ValueKind::Integer) ||
		!right->ops[1].is(ValueKind::Integer))
		return false;
	const int64_t ta = static_cast<int64_t>(body_.literals[left->ops[1].index].low);
	const int64_t fb = static_cast<int64_t>(body_.literals[right->ops[1].index].low);
	const uint64_t distance = ta > fb ? static_cast<uint64_t>(ta) - static_cast<uint64_t>(fb)
									  : static_cast<uint64_t>(fb) - static_cast<uint64_t>(ta);
	const uint64_t element = storage_bytes(left->type);
	if (distance == 0 || element == 0 || element > INT32_MAX || distance > INT32_MAX / element)
		return false;
	const uint64_t delta = distance * element;
	if ((delta & (delta - 1)) != 0)
		return false;
	BitChoice bit;
	if (!bit_choice(cond, bit))
		return false;
	const Value base = left->ops[0];
	const int64_t offset = ta < fb ? ta : fb;
	const int64_t bias = static_cast<int64_t>(static_cast<uint64_t>(offset) * element);
	if (bias < INT32_MIN || bias > INT32_MAX)
		return false;
	const bool invert = (ta > fb) != bit.when_set;
	choice = IndexedChoice{ base, bit, bias, delta, invert };
	return true;
}

bool SelectFormation::indexed_choice(BlockId e, InstId& at, Value cond, Value a, Value b, uint32_t location, Value& out)
{
	IndexedChoice choice;
	if (!match_indexed_choice(cond, a, b, choice))
		return false;
	out = emit_indexed(e, at, choice, location);
	return true;
}

Value SelectFormation::emit_indexed(BlockId e, InstId& at, const IndexedChoice& choice, uint32_t location)
{
	const Value base = choice.base;
	const BitChoice bit = choice.bit;
	const int64_t bias = choice.bias;
	const uint64_t delta = choice.delta;
	const bool invert = choice.invert;
	const unsigned output_bit = static_cast<unsigned>(__builtin_ctzll(delta));
	const Type word = Type::scalar(TypeKind::I64);
	auto emit = [&](Instruction inst, Type result) {
		inst.location = location;
		const InstId id = append_instruction(body_, inst);
		body_.instructions[id].result = append_temp(body_, result, id);
		insert_after(body_, e, at, id);
		at = id;
		return Value::temp(body_.instructions[id].result);
	};
	auto binary = [&](BinaryOp op, Value x, int64_t constant) {
		Instruction inst = make_instruction(Opcode::Binary, word);
		inst.sub = static_cast<uint8_t>(op);
		inst.ops[0] = x;
		inst.ops[1] = integer_value(body_, Literal{ static_cast<uint64_t>(constant), constant < 0 ? ~0ull : 0 });
		return emit(inst, word);
	};
	Value source = bit.source;
	if (bit.type != word)
	{
		Instruction extend = make_instruction(Opcode::Convert, word);
		extend.sub = static_cast<uint8_t>(
			lowir::is_unsigned(bit.type.kind()) || bit.type.kind() == TypeKind::I1 ? ConvertOp::ZExt : ConvertOp::SExt);
		set_convert_source_type(extend, bit.type);
		extend.ops[0] = source;
		source = emit(extend, word);
	}
	if (bit.bit > output_bit)
		source = binary(BinaryOp::UShr, source, bit.bit - output_bit);
	else if (bit.bit < output_bit)
		source = binary(BinaryOp::Shl, source, output_bit - bit.bit);
	source = binary(BinaryOp::And, source, delta);
	if (invert)
		source = binary(BinaryOp::Xor, source, delta);
	if (bias != 0)
		source = binary(BinaryOp::Add, source, bias);
	Instruction index = make_instruction(Opcode::Index, Type::scalar(TypeKind::I8));
	index.ops[0] = base;
	index.ops[1] = source;
	return emit(index, Type::scalar(TypeKind::Ptr));
}

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
		body_.instructions[i].flags &= static_cast<uint8_t>(~(IF_NO_WRAP | IF_EXACT | IF_NONNULL));
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

bool SelectFormation::nonzero_literal(Value v) const
{
	if (!v.is(ValueKind::Float))
		return false;
	const Literal& bits = body_.literals[v.index];
	return bits.low != 0 && bits.low != (1ull << 63) && bits.low != (1ull << 31);
}

// Whether `cond ? a : b` is a lane `min` or `max`. `<=` and `>=` differ from
// them when the two compare equal, as +0 and -0 do: a literal other than zero
// rules that out.
bool SelectFormation::min_or_max(Value cond, Value a, Value b, MinMax& out) const
{
	if (!cond.is(ValueKind::Temp) || cond.index < body_.parameter_count)
		return false;
	const Instruction& test = body_.instructions[body_.temps[cond.index].def];
	if (test.op != Opcode::Compare || !test.type.is_float() || test.type.kind() == TypeKind::F80)
		return false;
	const ComparePredicate p = static_cast<ComparePredicate>(test.sub);
	const bool strict = p == ComparePredicate::Lt || p == ComparePredicate::Gt;
	const bool less = p == ComparePredicate::Lt || p == ComparePredicate::Le;
	if (!strict && !less && p != ComparePredicate::Ge)
		return false;
	if (!strict && !nonzero_literal(test.ops[0]) && !nonzero_literal(test.ops[1]))
		return false;
	const Value x = test.ops[0], y = test.ops[1];
	// (Each mention of a literal has its own index.)
	const auto same = [&](Value p, Value q) {
		return p == q ||
			(p.is(ValueKind::Float) && q.is(ValueKind::Float) && p.float_kind == q.float_kind &&
				body_.literals[p.index].low == body_.literals[q.index].low &&
				body_.literals[p.index].high == body_.literals[q.index].high);
	};
	if (same(a, x) && same(b, y))
		out = MinMax{ !less, x, y };
	else if (same(a, y) && same(b, x))
		out = MinMax{ less, y, x };
	else
		return false;
	return true;
}

Value SelectFormation::emit_min_max(BlockId e, InstId& at, const MinMax& m, Type type, uint32_t location)
{
	const bool wide = type.kind() == TypeKind::F64;
	const Type v128 = Type::scalar(TypeKind::V128);
	auto vector = [&](VectorOp op, Type result, Value p, Value q) {
		Instruction inst = make_instruction(Opcode::Vector, result);
		inst.sub = static_cast<uint8_t>(op);
		inst.ops[0] = p;
		inst.ops[1] = q;
		inst.location = location;
		const InstId id = append_instruction(body_, inst);
		body_.instructions[id].result = append_temp(body_, result, id);
		insert_after(body_, e, at, id);
		at = id;
		return Value::temp(body_.instructions[id].result);
	};
	const VectorOp from = wide ? VectorOp::Fromf64 : VectorOp::Fromf32;
	const Value first = vector(from, v128, m.first, Value::none());
	const Value second = vector(from, v128, m.second, Value::none());
	const VectorOp op = wide ? (m.max ? VectorOp::Maxpd : VectorOp::Minpd)
							 : (m.max ? VectorOp::Maxps : VectorOp::Minps);
	const Value lanes = vector(op, v128, first, second);
	return vector(wide ? VectorOp::Tof64 : VectorOp::Tof32, type, lanes, Value::none());
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
		MinMax unused;
		if (++selects > kSelectCap || (!selectable(phi.type) && !min_or_max(cond, a, b, unused)) ||
			a.is(ValueKind::Slot) || b.is(ValueKind::Slot) || a.is(ValueKind::None) || b.is(ValueKind::None))
			return false;
	}
	if (selects != 0 && loads_through_merge(cond, join))
		return false;
	// Two values with work on both arms: selects run both arms on every path.
	if (selects == kSelectCap && from_t == t && from_f == f && body_.blocks[t].first != body_.blocks[t].last &&
		body_.blocks[f].first != body_.blocks[f].last)
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
		MinMax lanes;
		if (!(a == b) && !selectable(body_.instructions[i].type) && min_or_max(cond, a, b, lanes))
			merged = emit_min_max(e, at, lanes, body_.instructions[i].type, body_.instructions[term].location);
		else if (!(a == b) &&
			(!body_.instructions[i].type.is_pointer() ||
				!indexed_choice(e, at, cond, a, b, body_.instructions[term].location, merged)))
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

// A terminal ordinary load after at most the existing arm budget's pure
// address computations. Nothing else is allowed to observe an arm's result.
InstId SelectFormation::load_arm(BlockId a, BlockId from) const
{
	if (a == from || touched_[a] || (c_.cfg.flags[a] & (BF_HANDLER | BF_EH)) || c_.cfg.pred_count(a) != 1 ||
		c_.cfg.preds[c_.cfg.pred_first[a]] != from)
		return kNoId;
	const InstId last = body_.blocks[a].last;
	if (last == kNoId || body_.instructions[last].op != Opcode::Jump)
		return kNoId;
	unsigned count = 0;
	for (InstId i = body_.blocks[a].first; i != last; i = body_.instructions[i].next)
	{
		const Instruction& inst = body_.instructions[i];
		if (++count > kArmCap)
			return kNoId;
		if (inst.op == Opcode::Load && inst.next == last && !(inst.flags & IF_VOLATILE) && inst.result != kNoId &&
			selectable(inst.type) && inst.ops[0].is(ValueKind::Temp) &&
			body_.temps[inst.ops[0].index].type.is_pointer())
			return i;
		if (!speculatable(body_, inst))
			return kNoId;
	}
	return kNoId;
}

// Factor one ordinary read through the selected indexed address. The
// incoming loads have no other uses; their result phi becomes the one load.
// Original arm computations disappear with their now-unreachable blocks.
bool SelectFormation::form_load(BlockId e)
{
	const InstId term = body_.blocks[e].last;
	if (term == kNoId || body_.instructions[term].op != Opcode::Branch)
		return false;
	const Value cond = body_.instructions[term].ops[0];
	const uint32_t location = body_.instructions[term].location;
	const BlockId t = body_.instructions[term].ops[1].index, f = body_.instructions[term].ops[2].index;
	if (t == f)
		return false;
	const InstId ti = load_arm(t, e), fi = load_arm(f, e);
	if (ti == kNoId || fi == kNoId)
		return false;
	const BlockId join = body_.instructions[body_.blocks[t].last].ops[0].index;
	if (join != body_.instructions[body_.blocks[f].last].ops[0].index || join == e || touched_[join] ||
		(c_.cfg.flags[join] & (BF_HANDLER | BF_EH)) || c_.cfg.pred_count(join) != 2 || c_.cfg.dominates(join, e))
		return false;
	const InstId phi = body_.blocks[join].first;
	const Instruction left = body_.instructions[ti], right = body_.instructions[fi];
	if (phi == kNoId || body_.instructions[phi].op != Opcode::Phi ||
		(body_.instructions[phi].next != kNoId && body_.instructions[body_.instructions[phi].next].op == Opcode::Phi) ||
		left.type != right.type || body_.instructions[phi].type != left.type ||
		!(phi_incoming(body_, body_.instructions[phi], t) == Value::temp(left.result)) ||
		!(phi_incoming(body_, body_.instructions[phi], f) == Value::temp(right.result)))
		return false;
	IndexedChoice choice;
	if (!match_indexed_choice(cond, left.ops[0], right.ops[0], choice) || loads_through_merge(cond, join))
		return false;
	if (uses_ == nullptr)
		uses_ = &c_.entry_uses(form_selects);
	if (left.result >= uses_->size() || right.result >= uses_->size() || (*uses_)[left.result] != 1 ||
		(*uses_)[right.result] != 1)
		return false;
	InstId at = previous_in_block(body_, e, term);
	const Value address = emit_indexed(e, at, choice, location);
	Instruction load = left;
	load.result = body_.instructions[phi].result;
	load.next = body_.instructions[phi].next;
	load.ops[0] = address;
	load.flags &= right.flags;
	body_.instructions[phi] = load;
	for (BlockId arm : { t, f })
	{
		Instruction& jump = body_.instructions[body_.blocks[arm].last];
		jump.op = Opcode::Unreachable;
		jump.ops[0] = Value::none();
	}
	Instruction jump = make_instruction(Opcode::Jump, Type::scalar(TypeKind::Void));
	jump.ops[0] = Value::block(join);
	jump.location = location;
	body_.instructions[term] = jump;
	touched_[e] = touched_[t] = touched_[f] = touched_[join] = 1;
	++c_.stats.branches_folded;
	return true;
}

bool SelectFormation::range_switch(BlockId block, InstId id)
{
	const Instruction term = body_.instructions[id];
	if (term.op != Opcode::Switch || term.extra_count < 4)
		return false;
	Type type = term.type;
	if (type.is_void())
	{
		const Value v = term.ops[0];
		if (v.is(ValueKind::Temp))
			type = body_.temps[v.index].type;
		else if (v.is(ValueKind::Slot))
			type = body_.slots[v.index].type;
		else
			return false;
	}
	if (!type.is_integer() || type.kind() == TypeKind::I128)
		return false;
	const BlockId target = body_.operands[term.extra_first + 1].index;
	if (target == term.ops[1].index)
		return false;
	for (uint32_t k = 1; k < term.extra_count; k += 2)
		if (body_.operands[term.extra_first + k].index != target)
			return false;
	const unsigned bits = static_cast<unsigned>(storage_bytes(type) * 8);
	const uint64_t mask = bits == 64 ? UINT64_MAX : (uint64_t(1) << bits) - 1;
	std::vector<uint64_t> values;
	values.reserve(term.extra_count / 2);
	for (uint32_t k = 0; k < term.extra_count; k += 2)
	{
		Constant c;
		if (!literal_constant(body_, body_.operands[term.extra_first + k], c) || c.kind != ValueKind::Integer)
			return false;
		values.push_back(c.bits.low & mask);
	}
	std::sort(values.begin(), values.end());
	values.erase(std::unique(values.begin(), values.end()), values.end());
	// A singleton already lowers to one equality test.
	if (values.size() < 2)
		return false;
	const uint64_t low = values.front(), width = values.back() - low;
	if (width != values.size() - 1)
		return false;
	FunctionBuilder builder(c_.program, c_.symbol);
	InstId at = previous_in_block(body_, block, id);
	auto emit = [&](Instruction inst, Type result) -> Value {
		inst.location = term.location;
		const InstId n = append_instruction(body_, inst);
		const TempId t = append_temp(body_, result, n);
		body_.instructions[n].result = t;
		insert_after(body_, block, at, n);
		at = n;
		return Value::temp(t);
	};
	Value selector = term.ops[0];
	if (low != 0)
	{
		Instruction sub = make_instruction(Opcode::Binary, type);
		sub.sub = static_cast<uint8_t>(BinaryOp::Sub);
		sub.ops[0] = selector;
		sub.ops[1] = builder.integer(static_cast<int64_t>(low));
		selector = emit(sub, type);
	}
	Instruction cmp = make_instruction(Opcode::Compare, type);
	cmp.sub = static_cast<uint8_t>(width < mask ? ComparePredicate::Ult : ComparePredicate::Ule);
	cmp.ops[0] = selector;
	cmp.ops[1] = builder.integer(static_cast<int64_t>(width < mask ? width + 1 : width));
	const Value condition = emit(cmp, Type::scalar(TypeKind::I64));
	Instruction branch = make_instruction(Opcode::Branch, Type::scalar(TypeKind::Void));
	branch.location = term.location;
	branch.ops[0] = condition;
	branch.ops[1] = Value::block(target);
	branch.ops[2] = term.ops[1];
	body_.instructions[id] = branch;
	++c_.stats.branches_folded;
	return true;
}

// One ordinary store and a small amount of pure work on an arm. Exclude
// implicit slot reads and operations whose unselected evaluation may trap.
InstId SelectFormation::store_arm(BlockId arm, BlockId from) const
{
	const Cfg& cfg = c_.cfg;
	if (arm == from || touched_[arm] || (cfg.flags[arm] & (BF_HANDLER | BF_EH)) || cfg.pred_count(arm) != 1 ||
		cfg.preds[cfg.pred_first[arm]] != from)
		return kNoId;
	const InstId last = body_.blocks[arm].last;
	if (last == kNoId || body_.instructions[last].op != Opcode::Jump)
		return kNoId;
	InstId store = kNoId;
	unsigned count = 0;
	for (InstId i = body_.blocks[arm].first; i != last; i = body_.instructions[i].next)
	{
		const Instruction& inst = body_.instructions[i];
		if (++count > 2 * kArmCap)
			return kNoId;
		for (const Value op : inst.ops)
			if (op.is(ValueKind::Slot) && inst.op != Opcode::Addr)
				return kNoId;
		if (inst.op == Opcode::Store)
		{
			if (store != kNoId || (inst.flags & IF_VOLATILE) || !selectable(inst.type))
				return kNoId;
			store = i;
		}
		else if (!speculatable(body_, inst))
			return kNoId;
		else if (inst.op == Opcode::Binary)
		{
			const BinaryOp op = static_cast<BinaryOp>(inst.sub);
			if (op == BinaryOp::Shl || op == BinaryOp::Shr || op == BinaryOp::UShr)
				return kNoId;
		}
	}
	return store;
}

// Bounded structural equality of pure expressions. Shared loaded values
// are allowed, but two independent loads are never treated as one value.
bool SelectFormation::same_expression(Value a, Value b, unsigned depth) const
{
	if (a == b)
		return true;
	if (depth == 4)
		return false;
	const Instruction* left = producer(a);
	const Instruction* right = producer(b);
	if (left == nullptr || right == nullptr || !speculatable(body_, *left) || !speculatable(body_, *right) ||
		left->op != right->op || left->sub != right->sub || left->type != right->type ||
		left->immediate != right->immediate || left->extra_count != 0 || right->extra_count != 0)
		return false;
	for (unsigned k = 0; k < 3; ++k)
		if (!same_expression(left->ops[k], right->ops[k], depth + 1))
			return false;
	return true;
}

bool SelectFormation::unit_update(Type type, Value a, Value b) const
{
	if (!type.is_integer())
		return false;
	for (unsigned side = 0; side < 2; ++side)
	{
		const Instruction* step = producer(side == 0 ? a : b);
		Constant by;
		if (step != nullptr && step->op == Opcode::Binary && step->type == type &&
			(static_cast<BinaryOp>(step->sub) == BinaryOp::Add || static_cast<BinaryOp>(step->sub) == BinaryOp::Sub) &&
			step->ops[0] == (side == 0 ? b : a) && literal_constant(body_, step->ops[1], by) &&
			(is_integer(type, by, 1) || is_integer(type, by, -1)))
			return true;
	}
	return false;
}

// Factor an always-performed store through a small diamond. Sequential
// cursor updates can be predicated even when the condition reads through
// those cursors; keep the pointer-chasing admission rule for other cycles.
bool SelectFormation::form_store(BlockId e)
{
	const InstId term = body_.blocks[e].last;
	if (term == kNoId || body_.instructions[term].op != Opcode::Branch ||
		body_.instructions[term].sub != static_cast<uint8_t>(BranchPrediction::None))
		return false;
	const Instruction branch = body_.instructions[term];
	const BlockId t = branch.ops[1].index, f = branch.ops[2].index;
	if (t == f || !branch.ops[0].is(ValueKind::Temp))
		return false;
	const InstId ti = store_arm(t, e), fi = store_arm(f, e);
	if (ti == kNoId || fi == kNoId)
		return false;
	const BlockId join = body_.instructions[body_.blocks[t].last].ops[0].index;
	if (join != body_.instructions[body_.blocks[f].last].ops[0].index || join == e || touched_[join] ||
		(c_.cfg.flags[join] & (BF_HANDLER | BF_EH)) || c_.cfg.pred_count(join) != 2 || c_.cfg.dominates(join, e))
		return false;
	const Instruction left = body_.instructions[ti], right = body_.instructions[fi];
	if (left.type != right.type || !same_expression(left.ops[1], right.ops[1]))
		return false;
	const bool dependent = loads_through_merge(branch.ops[0], join);
	unsigned selects = 0;
	std::vector<InstId> phis;
	for (InstId i = body_.blocks[join].first; i != kNoId && body_.instructions[i].op == Opcode::Phi;
		i = body_.instructions[i].next)
	{
		const Instruction& phi = body_.instructions[i];
		const Value a = phi_incoming(body_, phi, t), b = phi_incoming(body_, phi, f);
		if (!selectable(phi.type) || a.is(ValueKind::Slot) || b.is(ValueKind::Slot))
			return false;
		if (!(a == b) && (++selects > kSelectCap || (dependent && !unit_update(phi.type, a, b))))
			return false;
		phis.push_back(i);
	}
	unlink_after(body_, t, previous_in_block(body_, t, ti), ti);
	unlink_after(body_, f, previous_in_block(body_, f, fi), fi);
	InstId at = previous_in_block(body_, e, term);
	at = hoist(t, e, at);
	at = hoist(f, e, at);
	const auto choose = [&](Type type, Value a, Value b) -> Value {
		if (a == b)
			return a;
		Instruction select = make_instruction(Opcode::Select, type);
		select.ops[0] = branch.ops[0];
		select.ops[1] = a;
		select.ops[2] = b;
		select.location = branch.location;
		const InstId id = append_instruction(body_, select);
		body_.instructions[id].result = append_temp(body_, type, id);
		insert_after(body_, e, at, id);
		at = id;
		return Value::temp(body_.instructions[id].result);
	};
	const Value stored = choose(left.type, left.ops[0], right.ops[0]);
	for (InstId id : phis)
	{
		const Instruction phi = body_.instructions[id];
		const Value value = choose(phi.type, phi_incoming(body_, phi, t), phi_incoming(body_, phi, f));
		Instruction copy = make_instruction(Opcode::Copy, phi.type);
		copy.result = phi.result;
		copy.ops[0] = value;
		copy.location = phi.location;
		copy.next = phi.next;
		body_.instructions[id] = copy;
	}
	body_.instructions[ti].ops[0] = stored;
	body_.instructions[ti].flags &= right.flags;
	body_.instructions[ti].location = branch.location;
	insert_after(body_, e, at, ti);
	for (BlockId arm : { t, f })
	{
		Instruction end = make_instruction(Opcode::Unreachable, Type::scalar(TypeKind::Void));
		end.location = branch.location;
		body_.instructions[body_.blocks[arm].last] = end;
	}
	Instruction jump = make_instruction(Opcode::Jump, Type::scalar(TypeKind::Void));
	jump.ops[0] = Value::block(join);
	jump.location = branch.location;
	body_.instructions[term] = jump;
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
	{
		// Range normalization preserves the unique successors: reuse the CFG
		// and this walk, rather than rebuilding it after a separate scan.
		const InstId id = body_.blocks[b].last;
		if (id != kNoId && body_.instructions[id].op == Opcode::Switch)
			changed = range_switch(b, id) || changed;
		if (c_.cfg.reachable(b) && !touched_[b] && !(c_.cfg.flags[b] & BF_EH))
			changed = form(b) || form_load(b) || (c_.level >= 2 && form_store(b)) || changed;
	}
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
