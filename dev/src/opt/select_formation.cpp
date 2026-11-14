#include <algorithm>
#include <vector>

#include "lowir/builder.h"
#include "opt/loops.h"
#include "opt/passes.h"
#include "opt/scalar_fold.h"

namespace opt {

using namespace lowir;

namespace {

// Speculated instructions per arm and selects per merge: past them the
// branch is cheaper than computing both sides.
const unsigned kArmCap = 2;
const unsigned kSelectCap = 2;
// Copies an arm may hold besides (a copy renames a value: no instruction).
const unsigned kArmCopies = 2;
// The instructions of a block that a test only skips where they compute
// what the other path gives: the block runs on both paths, the test goes.
const unsigned kSkippedCap = 4;
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
	if (inst.result == kNoId)
		return false;
	// A floating negation flips a sign bit and raises nothing: the arm of a
	// conditional magnitude (min_or_max).
	if (inst.op == Opcode::Unary && static_cast<UnaryOp>(inst.sub) == UnaryOp::Neg &&
		(inst.type.kind() == TypeKind::F32 || inst.type.kind() == TypeKind::F64))
		return true;
	if (!selectable(body.temps[inst.result].type))
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
	bool returning(BlockId a, BlockId from, bool& computes) const;
	bool truth_value(BlockId e, InstId& at, Value cond, Type type, Value a, Value b, uint32_t location, Value& out);
	bool form_return(BlockId e);
	typedef std::vector<std::pair<TempId, Constant>> Known;
	bool known_value(Value v, const Known& known, Constant& out) const;
	bool evaluate(const Instruction& inst, const Known& known, Constant& out) const;
	bool form_skipped(BlockId e);
	bool same_target(BlockId a, BlockId b) const;
	bool form_store(BlockId e);
	InstId store_arm(BlockId arm, BlockId from) const;
	bool same_expression(Value a, Value b, unsigned depth = 0) const;
	bool unit_update(Type type, Value a, Value b) const;
	bool invariant_condition(BlockId block, Value condition);
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
	LoopForest loops_;
	bool loops_ready_ = false;
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
	int64_t bias = choice.bias;
	uint64_t delta = choice.delta;
	const bool invert = choice.invert;
	unsigned output_bit = static_cast<unsigned>(__builtin_ctzll(delta));
	// Let the address's element scale perform a left shift when both field
	// offsets allow it. The selected bit then stays closer to its source.
	unsigned scale = output_bit > bit.bit ? std::min(3u, output_bit - bit.bit) : 0;
	while (scale != 0 && bias % (int64_t(1) << scale) != 0)
		--scale;
	delta >>= scale;
	bias /= int64_t(1) << scale;
	output_bit -= scale;
	const TypeKind elements[] = { TypeKind::I8, TypeKind::I16, TypeKind::I32, TypeKind::I64 };
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
	Instruction index = make_instruction(Opcode::Index, Type::scalar(elements[scale]));
	index.ops[0] = base;
	index.ops[1] = source;
	return emit(index, body_.pointer_type());
}

// A block reached only from `from` that computes at most kArmCap
// speculatable values (and kArmCopies copies) and jumps on (to `to`).
bool SelectFormation::arm(BlockId a, BlockId from, BlockId& to) const
{
	const Cfg& cfg = c_.cfg;
	if (a == from || touched_[a] || (cfg.flags[a] & (BF_HANDLER | BF_EH)) || cfg.pred_count(a) != 1 ||
		cfg.preds[cfg.pred_first[a]] != from)
		return false;
	const InstId last = body_.blocks[a].last;
	if (last == kNoId || body_.instructions[last].op != Opcode::Jump)
		return false;
	unsigned count = 0, copies = 0;
	for (InstId i = body_.blocks[a].first; i != last; i = body_.instructions[i].next)
		if ((body_.instructions[i].op == Opcode::Copy ? ++copies > kArmCopies : ++count > kArmCap) ||
			!speculatable(body_, body_.instructions[i]))
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
		if (body_.instructions[i].op == Opcode::Index)
			body_.instructions[i].set_index_bounds(IndexBounds::None);
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
	Value x = test.ops[0], y = test.ops[1];
	// A strict test of v against zero orders v and -v alike (v < 0 exactly
	// when v < -v, for the zeros and an unordered v too): a choice between
	// the two is their min or max (`v < 0 ? -v : v` is max(-v, v)).
	const auto zero = [&](Value v) { return v.is(ValueKind::Float) && !nonzero_literal(v); };
	const auto negated = [&](Value v, Value of) {
		if (!v.is(ValueKind::Temp) || v.index < body_.parameter_count)
			return false;
		const Instruction& def = body_.instructions[body_.temps[v.index].def];
		return def.op == Opcode::Unary && static_cast<UnaryOp>(def.sub) == UnaryOp::Neg && def.type == test.type &&
			def.ops[0] == of;
	};
	if (strict && zero(y) && (negated(a, x) || negated(b, x)))
		y = negated(a, x) ? a : b;
	else if (strict && zero(x) && (negated(a, y) || negated(b, y)))
		x = negated(a, y) ? a : b;
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
	const auto negates = [&](Value v, Value of) {
		const Instruction* def = v.is(ValueKind::Temp) && v.index >= body_.parameter_count
			? &body_.instructions[body_.temps[v.index].def]
			: nullptr;
		return def != nullptr && def->op == Opcode::Unary && static_cast<UnaryOp>(def->sub) == UnaryOp::Neg &&
			def->ops[0] == of;
	};
	if (m.max && (negates(m.first, m.second) || negates(m.second, m.first)))
		c_.magnitude_formed = true;
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
	// (The Wasm target takes the select there, as its reference toolchain does.)
	if (selects != 0 && !target_model::is_wasm(c_.program.target) && loads_through_merge(cond, join))
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
	unsigned count = 0, copies = 0;
	for (InstId i = body_.blocks[a].first; i != last; i = body_.instructions[i].next)
	{
		const Instruction& inst = body_.instructions[i];
		if (inst.op == Opcode::Load && inst.next == last && !(inst.flags & IF_VOLATILE) && inst.result != kNoId &&
			selectable(inst.type) && inst.ops[0].is(ValueKind::Temp) &&
			body_.temps[inst.ops[0].index].type.is_pointer())
			return i;
		// (A copy renames the address, as a smart pointer's accessor does.)
		if ((inst.op == Opcode::Copy ? ++copies > kArmCopies : ++count > kArmCap) || !speculatable(body_, inst))
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

// A block that returns a selectable value: only that, or, reached only from
// `from`, after an arm's instructions (`computes`).
bool SelectFormation::returning(BlockId a, BlockId from, bool& computes) const
{
	const Cfg& cfg = c_.cfg;
	const InstId last = body_.blocks[a].last;
	if (a == from || touched_[a] || last == kNoId || body_.instructions[last].op != Opcode::Return ||
		(cfg.flags[a] & (BF_HANDLER | BF_EH)))
		return false;
	const Instruction& ret = body_.instructions[last];
	if (!selectable(ret.type) || ret.extra_count != 0 || ret.ops[0].is(ValueKind::None) ||
		ret.ops[0].is(ValueKind::Slot))
		return false;
	computes = body_.blocks[a].first != last;
	if (!computes)
		return true;
	if (cfg.pred_count(a) != 1 || cfg.preds[cfg.pred_first[a]] != from)
		return false;
	unsigned count = 0, copies = 0;
	for (InstId i = body_.blocks[a].first; i != last; i = body_.instructions[i].next)
		if ((body_.instructions[i].op == Opcode::Copy ? ++copies > kArmCopies : ++count > kArmCap) ||
			!speculatable(body_, body_.instructions[i]))
			return false;
	return true;
}

// The choice of the literals 1 and 0 (or 0 and 1) by a compare's result is
// that result (or its opposite) at the chosen type's width: no select.
bool SelectFormation::truth_value(
	BlockId e, InstId& at, Value cond, Type type, Value a, Value b, uint32_t location, Value& out)
{
	const Instruction* test = producer(cond);
	Constant ca, cb;
	if (test == nullptr || test->op != Opcode::Compare || !type.is_integer() || !literal_constant(body_, a, ca) ||
		!literal_constant(body_, b, cb))
		return false;
	const bool inverted = is_integer(type, ca, 0) && is_integer(type, cb, 1);
	if (!inverted && !(is_integer(type, ca, 1) && is_integer(type, cb, 0)))
		return false;
	const Type truth = Type::scalar(TypeKind::I64);
	auto emit = [&](Instruction inst, Type result) {
		inst.location = location;
		const InstId n = append_instruction(body_, inst);
		body_.instructions[n].result = append_temp(body_, result, n);
		insert_after(body_, e, at, n);
		at = n;
		return Value::temp(body_.instructions[n].result);
	};
	out = cond;
	if (inverted)
	{
		Instruction opposite = make_instruction(Opcode::Compare, truth);
		opposite.sub = static_cast<uint8_t>(ComparePredicate::Eq);
		opposite.ops[0] = cond;
		opposite.ops[1] = integer_value(body_, Literal{ 0, 0 });
		out = emit(opposite, truth);
	}
	if (type == truth)
		return true;
	Instruction narrow = make_instruction(storage_bytes(type) == 8 ? Opcode::Copy : Opcode::Convert, type);
	if (narrow.op == Opcode::Convert)
	{
		narrow.sub = static_cast<uint8_t>(ConvertOp::Trunc);
		set_convert_source_type(narrow, truth);
	}
	narrow.ops[0] = out;
	out = emit(narrow, type);
	return true;
}

// `e` branches to two blocks that return, at most one of them after an
// arm's instructions: the branch only chooses the value. `e` computes the
// arm's and returns the one its condition selects (LLVM folds such a branch
// to a select as well). A value a block that only returns names is defined
// above every block that reaches it, so above `e`.
bool SelectFormation::form_return(BlockId e)
{
	const InstId term = body_.blocks[e].last;
	if (term == kNoId || body_.instructions[term].op != Opcode::Branch)
		return false;
	const Instruction br = body_.instructions[term];
	const Value cond = br.ops[0];
	const BlockId t = br.ops[1].index, f = br.ops[2].index;
	bool t_computes = false, f_computes = false;
	if (!cond.is(ValueKind::Temp) || !selectable(body_.temps[cond.index].type) || t == f ||
		!returning(t, e, t_computes) || !returning(f, e, f_computes) || (t_computes && f_computes))
		return false;
	// A stated expectation seldom runs the other side's instructions.
	if ((t_computes || f_computes) && br.sub != static_cast<uint8_t>(BranchPrediction::None))
		return false;
	const Instruction rt = body_.instructions[body_.blocks[t].last], rf = body_.instructions[body_.blocks[f].last];
	if (!(rt.type == rf.type))
		return false;
	InstId at = previous_in_block(body_, e, term);
	if (t_computes)
		at = hoist(t, e, at);
	if (f_computes)
		at = hoist(f, e, at);
	Value merged = rt.ops[0];
	if (!(rt.ops[0] == rf.ops[0]) && !same_literal(body_, rt.type, rt.ops[0], rf.ops[0]) &&
		!truth_value(e, at, cond, rt.type, rt.ops[0], rf.ops[0], br.location, merged))
	{
		Instruction sel = make_instruction(Opcode::Select, rt.type);
		sel.ops[0] = cond;
		sel.ops[1] = rt.ops[0];
		sel.ops[2] = rf.ops[0];
		sel.location = br.location;
		const InstId s = append_instruction(body_, sel);
		body_.instructions[s].result = append_temp(body_, rt.type, s);
		insert_after(body_, e, at, s);
		merged = Value::temp(body_.instructions[s].result);
	}
	// The emptied arm ends unreachable (the cleanup drops it); a block that
	// only returns stays for its other predecessors.
	const BlockId arms[2] = { t_computes ? t : kNoId, f_computes ? f : kNoId };
	for (BlockId a : arms)
	{
		if (a == kNoId)
			continue;
		Instruction& end = body_.instructions[body_.blocks[a].last];
		end.op = Opcode::Unreachable;
		end.type = Type::scalar(TypeKind::Void);
		end.ops[0] = Value::none();
	}
	Instruction ret = make_instruction(Opcode::Return, rt.type);
	ret.ops[0] = merged;
	ret.location = br.location;
	body_.instructions[term] = ret;
	touched_[e] = touched_[t] = touched_[f] = 1;
	++c_.stats.branches_folded;
	return true;
}

bool SelectFormation::known_value(Value v, const Known& known, Constant& out) const
{
	if (literal_constant(body_, v, out))
		return true;
	if (!v.is(ValueKind::Temp))
		return false;
	for (const std::pair<TempId, Constant>& k : known)
		if (k.first == v.index)
		{
			out = k.second;
			return true;
		}
	return false;
}

// What `inst` computes where the temporaries of `known` have their values,
// when those decide it.
bool SelectFormation::evaluate(const Instruction& inst, const Known& known, Constant& out) const
{
	Constant a, b;
	const bool ka = known_value(inst.ops[0], known, a), kb = known_value(inst.ops[1], known, b);
	const auto zero = [](const Constant& c) {
		return c.kind == ValueKind::Integer && c.bits.low == 0 && c.bits.high == 0;
	};
	switch (inst.op)
	{
	case Opcode::Copy:
		return ka && fold_copy(inst.type, a, out);
	case Opcode::Unary:
		return ka && fold_unary(static_cast<UnaryOp>(inst.sub), inst.type, a, out);
	case Opcode::Binary:
	{
		const BinaryOp op = static_cast<BinaryOp>(inst.sub);
		if (ka && kb)
			return fold_binary(op, inst.type, a, b, out);
		// Zero under `and` and `mul`, whatever the other operand.
		if ((op != BinaryOp::And && op != BinaryOp::Mul) || !inst.type.is_integer() ||
			!((ka && zero(a)) || (kb && zero(b))))
			return false;
		out = integer_constant(inst.type, Literal{ 0, 0 });
		return true;
	}
	case Opcode::Compare:
		return ka && kb && !inst.type.is_float() &&
			fold_compare(static_cast<ComparePredicate>(inst.sub), inst.type, a, b, out);
	case Opcode::Convert:
		return ka && fold_convert(static_cast<ConvertOp>(inst.sub), inst.type, convert_source_type(inst), a, out);
	case Opcode::Select:
		return ka && a.kind == ValueKind::Integer && known_value(inst.ops[zero(a) ? 2 : 1], known, out);
	default:
		return false;
	}
}

// `e` tests a value against a literal and where they are equal goes straight
// to a merge, else through a block of its own that only computes (at most
// kSkippedCap speculatable instructions). Where each phi's value from that
// block, computed from the literal, is the constant the phi takes from `e`,
// the block's values serve both paths: `e` runs into it and the test goes.
// (`p ? p & ~7 : 0` is `p & ~7`; a pointer union's `dyn_cast` after its null
// test is such a block.) Likewise where the block returns its value and the
// equal side only returns the literal the block computes from the tested one.
bool SelectFormation::form_skipped(BlockId e)
{
	const InstId term = body_.blocks[e].last;
	if (term == kNoId || body_.instructions[term].op != Opcode::Branch)
		return false;
	const Instruction br = body_.instructions[term];
	const Value cond = br.ops[0];
	if (!cond.is(ValueKind::Temp) || br.ops[1].index == br.ops[2].index)
		return false;
	// The side taken where the tested value is the literal: `x == K`'s true
	// side, the false side of `x != K` and of a branch on `x` itself (K 0).
	Known known;
	unsigned equal_side = 2;
	const Instruction* test = producer(cond);
	const ComparePredicate predicate = test != nullptr && test->op == Opcode::Compare
		? static_cast<ComparePredicate>(test->sub)
		: ComparePredicate::Lt;
	if (predicate == ComparePredicate::Eq || predicate == ComparePredicate::Ne)
	{
		Constant k;
		const unsigned literal = literal_constant(body_, test->ops[1], k) ? 1
			: literal_constant(body_, test->ops[0], k)                    ? 0
																		  : 2;
		Literal bits;
		if (literal == 2 || !test->ops[1 - literal].is(ValueKind::Temp) || !test->type.is_integer() ||
			!integer_of(test->type, k, bits))
			return false;
		known.push_back(std::make_pair(test->ops[1 - literal].index, integer_constant(test->type, bits)));
		known.push_back(std::make_pair(cond.index, truth_constant(predicate == ComparePredicate::Eq)));
		equal_side = predicate == ComparePredicate::Eq ? 1 : 2;
	}
	else if (body_.temps[cond.index].type.is_integer() && selectable(body_.temps[cond.index].type))
		known.push_back(std::make_pair(cond.index, integer_constant(body_.temps[cond.index].type, Literal{ 0, 0 })));
	else
		return false;
	const BlockId join = br.ops[equal_side].index, a = br.ops[3 - equal_side].index;
	const Cfg& cfg = c_.cfg;
	if (a == e || join == e || touched_[a] || touched_[join] || (cfg.flags[a] & (BF_HANDLER | BF_EH)) ||
		(cfg.flags[join] & BF_HANDLER) || cfg.pred_count(a) != 1 || cfg.preds[cfg.pred_first[a]] != e)
		return false;
	const InstId last = body_.blocks[a].last;
	if (last == kNoId || body_.blocks[a].first == last)
		return false;
	const Instruction end_a = body_.instructions[last];
	const bool returns = end_a.op == Opcode::Return;
	if (!returns && (end_a.op != Opcode::Jump || end_a.ops[0].index != join))
		return false;
	// (The equal side of a returning block: only the return of a literal.)
	const InstId only = body_.blocks[join].last;
	if (returns &&
		(end_a.extra_count != 0 || !selectable(end_a.type) || only == kNoId || body_.blocks[join].first != only ||
			body_.instructions[only].op != Opcode::Return || body_.instructions[only].extra_count != 0 ||
			!(body_.instructions[only].type == end_a.type)))
		return false;
	unsigned count = 0;
	for (InstId i = body_.blocks[a].first; i != last; i = body_.instructions[i].next)
	{
		const Instruction& inst = body_.instructions[i];
		if (++count > kSkippedCap || !speculatable(body_, inst))
			return false;
		Constant r;
		if (evaluate(inst, known, r))
			known.push_back(std::make_pair(inst.result, r));
	}
	if (returns)
	{
		Constant c, r;
		if (!literal_constant(body_, body_.instructions[only].ops[0], c) || !known_value(end_a.ops[0], known, r) ||
			c.kind != r.kind || !same_constant(end_a.type, c, r))
			return false;
		hoist(a, e, previous_in_block(body_, e, term));
		Instruction ret = make_instruction(Opcode::Return, end_a.type);
		ret.ops[0] = end_a.ops[0];
		ret.location = end_a.location;
		body_.instructions[term] = ret;
		Instruction& end = body_.instructions[body_.blocks[a].last];
		end.op = Opcode::Unreachable;
		end.type = Type::scalar(TypeKind::Void);
		end.ops[0] = Value::none();
		touched_[e] = touched_[a] = touched_[join] = 1;
		++c_.stats.branches_folded;
		return true;
	}
	for (InstId i = body_.blocks[join].first; i != kNoId && body_.instructions[i].op == Opcode::Phi;
		i = body_.instructions[i].next)
	{
		const Instruction& phi = body_.instructions[i];
		const Value direct = phi_incoming(body_, phi, e), computed = phi_incoming(body_, phi, a);
		Constant c, r;
		if (!(direct == computed) &&
			(!literal_constant(body_, direct, c) || !known_value(computed, known, r) || !same_constant(phi.type, c, r)))
			return false;
	}
	hoist(a, e, previous_in_block(body_, e, term));
	for (InstId i = body_.blocks[join].first; i != kNoId && body_.instructions[i].op == Opcode::Phi;
		i = body_.instructions[i].next)
	{
		const Instruction& phi = body_.instructions[i];
		const Value computed = phi_incoming(body_, phi, a);
		for (uint32_t k = 0; k < phi.extra_count; k += 2)
			if (body_.operands[phi.extra_first + k].index == e)
				body_.operands[phi.extra_first + k + 1] = computed;
	}
	remove_phi_incoming(body_, join, a);
	Instruction& end = body_.instructions[body_.blocks[a].last];
	end.op = Opcode::Unreachable;
	end.ops[0] = Value::none();
	Instruction& jump = body_.instructions[term];
	jump.op = Opcode::Jump;
	jump.sub = 0;
	jump.flags = 0;
	jump.type = Type::scalar(TypeKind::Void);
	jump.ops[0] = Value::block(join);
	jump.ops[1] = jump.ops[2] = Value::none();
	jump.extra_count = 0;
	touched_[e] = touched_[a] = touched_[join] = 1;
	++c_.stats.branches_folded;
	return true;
}

// Whether a switch reaches the same code at `a` as at `b`: one block, or two
// that only return one literal (`case A: return true; case B: return true;`).
bool SelectFormation::same_target(BlockId a, BlockId b) const
{
	if (a == b)
		return true;
	const InstId la = body_.blocks[a].last, lb = body_.blocks[b].last;
	if (la == kNoId || lb == kNoId || body_.blocks[a].first != la || body_.blocks[b].first != lb)
		return false;
	const Instruction& ra = body_.instructions[la];
	const Instruction& rb = body_.instructions[lb];
	return ra.op == Opcode::Return && rb.op == Opcode::Return && ra.type == rb.type && ra.extra_count == 0 &&
		rb.extra_count == 0 && same_literal(body_, ra.type, ra.ops[0], rb.ops[0]);
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
	if (same_target(target, term.ops[1].index))
		return false;
	for (uint32_t k = 1; k < term.extra_count; k += 2)
		if (!same_target(body_.operands[term.extra_first + k].index, target))
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
	// A run that ends at the type's greatest unsigned value: `x >= low`.
	const bool top = low != 0 && values.back() == mask;
	if (low != 0 && !top)
	{
		Instruction sub = make_instruction(Opcode::Binary, type);
		sub.sub = static_cast<uint8_t>(BinaryOp::Sub);
		sub.ops[0] = selector;
		sub.ops[1] = builder.integer(static_cast<int64_t>(low));
		selector = emit(sub, type);
	}
	Instruction cmp = make_instruction(Opcode::Compare, type);
	cmp.sub = static_cast<uint8_t>(top ? ComparePredicate::Uge
			: width < mask             ? ComparePredicate::Ult
									   : ComparePredicate::Ule);
	cmp.ops[0] = selector;
	cmp.ops[1] = top ? integer_value(body_, normalize_integer(type, Literal{ low, 0 }))
					 : builder.integer(static_cast<int64_t>(width < mask ? width + 1 : width));
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

// Leave invariant loop choices available for unswitching and vectorization.
// Build the loop forest only after a store diamond has passed admission.
bool SelectFormation::invariant_condition(BlockId block, Value condition)
{
	if (!loops_ready_)
	{
		find_loops(body_, c_.cfg, loops_);
		c_.entry_block_of(form_selects);
		loops_ready_ = true;
	}
	if (loops_.partial || loops_.dropped[block])
		return true;
	const uint32_t loop = loops_.innermost[block];
	if (loop == kNoId)
		return false;
	const auto outside = [&](Value value) {
		if (!value.is(ValueKind::Temp) || value.index < body_.parameter_count)
			return true;
		const InstId def = body_.temps[value.index].def;
		return def != kNoId && def < c_.block_of.size() && c_.block_of[def] != kNoId &&
			!in_loop(loops_, loop, c_.block_of[def]);
	};
	if (outside(condition))
		return true;
	const Instruction* test = producer(condition);
	return test != nullptr && test->op == Opcode::Compare && outside(test->ops[0]) && outside(test->ops[1]);
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
	if (invariant_condition(e, branch.ops[0]))
		return false;
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
		// Range normalization keeps the successors, or leaves blocks that only
		// return a literal without their one predecessor (no form reads those
		// blocks' predecessors): reuse the CFG and this walk, rather than
		// rebuilding it after a separate scan.
		const InstId id = body_.blocks[b].last;
		if (id != kNoId && body_.instructions[id].op == Opcode::Switch)
			changed = range_switch(b, id) || changed;
		if (c_.cfg.reachable(b) && !touched_[b] && !(c_.cfg.flags[b] & BF_EH))
			changed = form_skipped(b) || form(b) || form_return(b) || form_load(b) ||
				(c_.level >= 2 && form_store(b)) || changed;
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
