#include <initializer_list>
#include <vector>

#include "lowir/builder.h"
#include "opt/passes.h"
#include "opt/scalar_fold.h"

namespace opt {

using namespace lowir;

namespace {

// The instructions a second test's block holds before its branch.
const unsigned kTestInstructions = 3;
// The instructions a select of one bit may become.
const unsigned kBitInstructions = 3;
// The terms of an `or` tree read for one mask, and the masks and
// zero-extensions read over a term's value.
const unsigned kMaskTerms = 8, kMaskSteps = 4;

// A test of one integer value against literals. The values that pass are
// one run of consecutive values modulo the type's range (`value - low <=
// span`), or those whose masked bits equal a literal (or differ from it).
struct Test
{
	enum Kind
	{
		Run,
		Masked
	};
	Kind kind;
	Value value;
	Value masked; // Masked: `value & mask` as the test read it
	Type type;
	uint64_t all;  // the type's bits, all set
	uint64_t low;  // Run
	uint64_t span; // Run: less than `all` (some value fails)
	uint64_t mask; // Masked: (value & mask) == bits, != when `differs`
	uint64_t bits;
	bool differs;
};

bool test_type(Type type)
{
	switch (type.kind())
	{
	case TypeKind::I8:
	case TypeKind::U8:
	case TypeKind::I16:
	case TypeKind::U16:
	case TypeKind::I32:
	case TypeKind::U32:
	case TypeKind::I64:
		return true;
	default:
		return false;
	}
}

// The run a predicate against `literal` keeps; false when it keeps every
// value or none.
bool run_of(ComparePredicate predicate, uint64_t literal, uint64_t all, uint64_t& low, uint64_t& span)
{
	const uint64_t least = (all >> 1) + 1, most = all >> 1; // the signed bounds' bits
	switch (predicate)
	{
	case ComparePredicate::Eq:
		low = literal;
		span = 0;
		return true;
	case ComparePredicate::Ne:
		low = (literal + 1) & all;
		span = all - 1;
		return true;
	case ComparePredicate::Ult:
		low = 0;
		span = literal - 1;
		return literal != 0;
	case ComparePredicate::Ule:
		low = 0;
		span = literal;
		return literal != all;
	case ComparePredicate::Ugt:
		low = literal + 1;
		span = all - literal - 1;
		return literal != all;
	case ComparePredicate::Uge:
		low = literal;
		span = all - literal;
		return literal != 0;
	case ComparePredicate::Lt:
		low = least;
		span = (literal - least - 1) & all;
		return literal != least;
	case ComparePredicate::Le:
		low = least;
		span = (literal - least) & all;
		return literal != most;
	case ComparePredicate::Gt:
		low = (literal + 1) & all;
		span = (most - literal - 1) & all;
		return literal != most;
	case ComparePredicate::Ge:
		low = literal;
		span = (most - literal) & all;
		return literal != least;
	}
	return false;
}

// The test the values failing `t` pass.
Test complement(Test t)
{
	if (t.kind == Test::Masked)
	{
		t.differs = !t.differs;
		return t;
	}
	t.low = (t.low + t.span + 1) & t.all;
	t.span = t.all - t.span - 1;
	return t;
}

// Makes a run of more than half the values the run of the others: the
// literals of its test are the smaller ones. True when it did.
bool shorter(Test& t)
{
	if (t.kind != Test::Run || t.span <= t.all / 2)
		return false;
	t = complement(t);
	return true;
}

// The test passing where `a` or `b` does, when one test says that.
bool unite(const Test& a, const Test& b, Test& out)
{
	if (a.kind != b.kind || !(a.value == b.value) || !(a.type == b.type))
		return false;
	out = a;
	if (a.kind == Test::Masked)
	{
		// Any masked bit set, or any masked bit clear: of either mask. (One
		// bit equal to a literal differs from the other literal.)
		Test x = a, y = b;
		for (Test* t : { &x, &y })
			if (!t->differs && (t->mask & (t->mask - 1)) == 0)
			{
				t->bits ^= t->mask;
				t->differs = true;
			}
		const bool none = x.bits == 0 && y.bits == 0, full = x.bits == x.mask && y.bits == y.mask;
		if (!x.differs || !y.differs || (!none && !full))
			return false;
		out.mask = x.mask | y.mask;
		out.bits = none ? 0 : out.mask;
		out.differs = true;
		return true;
	}
	// One run starts in the other, or right after it.
	const Test* first = &a;
	const Test* second = &b;
	uint64_t offset = (b.low - a.low) & a.all;
	if (offset > a.span + 1)
	{
		first = &b;
		second = &a;
		offset = (a.low - b.low) & a.all;
	}
	if (offset <= first->span + 1)
	{
		// (Around to the first's start: every value passes.)
		if (second->span >= a.all - offset)
			return false;
		out.low = first->low;
		out.span = first->span > offset + second->span ? first->span : offset + second->span;
		return true;
	}
	// Two values one bit apart: the value less that bit.
	const uint64_t bit = a.low ^ b.low;
	if (a.span != 0 || b.span != 0 || (bit & (bit - 1)) != 0)
		return false;
	out.kind = Test::Masked;
	out.mask = a.all & ~bit;
	out.bits = a.low & ~bit;
	out.differs = false;
	return true;
}

class TestMerging
{
public:
	explicit TestMerging(FunctionContext& context) : c_(context), body_(context.body) {}
	bool run();

private:
	const Instruction* producer(Value v) const;
	bool literal_bits(Value v, uint64_t all, uint64_t& out) const;
	bool test_of(Value v, bool truth, Test& out) const;
	Value add(BlockId block, InstId& at, Instruction inst, Type result);
	Instruction emit(BlockId block, InstId& at, const Test& test, uint32_t location);
	bool merge_select(BlockId block, InstId before, InstId id);
	bool merge_nested(BlockId block, InstId before, InstId id);
	bool move_bit(BlockId block, InstId before, InstId id);
	// A term of an `or` tree: `root`, of type `from`, under literal masks
	// and zero-extensions. `dying` counts the term's own instructions over
	// the root: each read once, by the tree, and in its block.
	struct Term
	{
		Value value, root;
		Type from;
		uint64_t mask;
		unsigned dying, lead, members;
	};
	bool own(const Instruction& def, BlockId block) const
	{
		const InstId at = body_.temps[def.result].def;
		return used_once(Value::temp(def.result)) && at < block_of_.size() && block_of_[at] == block;
	}
	Term term(Value v, Type type, BlockId block) const;
	bool unite_masks(BlockId block, InstId before, InstId id);
	bool merge_branches(BlockId e);
	bool second_block(BlockId b, BlockId from) const;
	bool used_once(Value v) const { return v.is(ValueKind::Temp) && v.index < uses_.size() && uses_[v.index] == 1; }
	bool dies(Value condition) const;

	FunctionContext& c_;
	FunctionBody& body_;
	std::vector<uint32_t> uses_;
	std::vector<uint8_t> touched_;
	std::vector<BlockId> block_of_; // per instruction this run has passed: its block
};

const Instruction* TestMerging::producer(Value v) const
{
	if (!v.is(ValueKind::Temp) || v.index < body_.parameter_count || body_.temps[v.index].def == kNoId)
		return nullptr;
	return &body_.instructions[body_.temps[v.index].def];
}

// A condition's own instruction (a compare, or the value's step or mask)
// has no reader left once the condition is replaced.
bool TestMerging::dies(Value condition) const
{
	const Instruction* def = producer(condition);
	return def == nullptr || (def->op != Opcode::Compare && def->op != Opcode::Binary) || used_once(condition);
}

bool TestMerging::literal_bits(Value v, uint64_t all, uint64_t& out) const
{
	Constant c;
	if (!literal_constant(body_, v, c) || c.kind != ValueKind::Integer)
		return false;
	out = c.bits.low & all;
	return true;
}

// The test `v` computes: a compare of a value against a literal, or as a
// condition (`truth`) any other value against zero. The value compared may
// be the tested one less a literal step, or its masked bits (against zero
// or the mask).
bool TestMerging::test_of(Value v, bool truth, Test& out) const
{
	if (!v.is(ValueKind::Temp))
		return false;
	const Instruction* def = producer(v);
	const bool compare = def != nullptr && def->op == Opcode::Compare;
	if (!compare && !truth)
		return false;
	out.type = compare ? def->type : body_.temps[v.index].type;
	if (!test_type(out.type))
		return false;
	out.all = ~uint64_t(0) >> (64 - 8 * storage_bytes(out.type));
	out.low = out.span = out.mask = out.bits = 0;
	out.differs = false;
	out.value = compare ? def->ops[0] : v;
	out.masked = out.value;
	uint64_t literal = 0;
	if (compare && !literal_bits(def->ops[1], out.all, literal))
		return false;
	const ComparePredicate predicate = compare ? static_cast<ComparePredicate>(def->sub) : ComparePredicate::Ne;
	const Instruction* inner = producer(out.value);
	uint64_t step = 0;
	const bool stepped = inner != nullptr && inner->op == Opcode::Binary && inner->type == out.type &&
		literal_bits(inner->ops[1], out.all, step);
	const BinaryOp op = stepped ? static_cast<BinaryOp>(inner->sub) : BinaryOp::Mul;
	if (op == BinaryOp::And && step != 0 && (predicate == ComparePredicate::Eq || predicate == ComparePredicate::Ne) &&
		(literal == 0 || literal == step))
	{
		out.kind = Test::Masked;
		out.value = inner->ops[0];
		out.mask = step;
		out.bits = literal;
		out.differs = predicate == ComparePredicate::Ne;
		return true;
	}
	out.kind = Test::Run;
	if (!run_of(predicate, literal, out.all, out.low, out.span))
		return false;
	if (op == BinaryOp::Sub || op == BinaryOp::Add)
	{
		out.value = inner->ops[0];
		out.low = (op == BinaryOp::Sub ? out.low + step : out.low - step) & out.all;
	}
	return true;
}

// `inst` after `at` in `block`, with a new result used once.
Value TestMerging::add(BlockId block, InstId& at, Instruction inst, Type result)
{
	const InstId n = append_instruction(body_, inst);
	const TempId t = append_temp(body_, result, n);
	body_.instructions[n].result = t;
	insert_after(body_, block, at, n);
	at = n;
	uses_.resize(body_.temps.size(), 1);
	return Value::temp(t);
}

// The compare that computes the test (0 or 1), for the caller to place;
// the value it compares goes after `at` in `block` when it is not the
// tested one.
Instruction TestMerging::emit(BlockId block, InstId& at, const Test& test, uint32_t location)
{
	auto literal = [&](uint64_t bits) {
		return integer_value(body_, normalize_integer(test.type, Literal{ bits, 0 }));
	};
	auto compare = [&](ComparePredicate predicate, Value value, uint64_t bits) {
		Instruction cmp = make_instruction(Opcode::Compare, test.type);
		cmp.sub = static_cast<uint8_t>(predicate);
		cmp.location = location;
		cmp.ops[0] = value;
		cmp.ops[1] = literal(bits);
		return cmp;
	};
	auto binary = [&](BinaryOp op, uint64_t bits) {
		Instruction inst = make_instruction(Opcode::Binary, test.type);
		inst.sub = static_cast<uint8_t>(op);
		inst.location = location;
		inst.ops[0] = test.value;
		inst.ops[1] = literal(bits);
		return add(block, at, inst, test.type);
	};
	if (test.kind == Test::Masked)
		return compare(
			test.differs ? ComparePredicate::Ne : ComparePredicate::Eq, binary(BinaryOp::And, test.mask), test.bits);
	const uint64_t all = test.all, low = test.low, span = test.span, high = (low + span) & all;
	const uint64_t least = (all >> 1) + 1, most = all >> 1;
	if (span == 0)
		return compare(ComparePredicate::Eq, test.value, low);
	if (span == all - 1)
		return compare(ComparePredicate::Ne, test.value, (low - 1) & all);
	if (low == 0)
		return compare(ComparePredicate::Ule, test.value, span);
	if (high == all)
		return compare(ComparePredicate::Uge, test.value, low);
	// (The signed predicates for signed types: an unsigned one's values are
	// not kept sign-extended.)
	const TypeKind kind = test.type.kind();
	const bool is_signed = kind == TypeKind::I8 || kind == TypeKind::I16 || kind == TypeKind::I32 ||
		kind == TypeKind::I64;
	if (is_signed && low == least)
		return compare(ComparePredicate::Le, test.value, high);
	if (is_signed && high == most)
		return compare(ComparePredicate::Ge, test.value, low);
	return compare(ComparePredicate::Ule, binary(BinaryOp::Sub, low), span);
}

// `select c, a, 0` (both), `select c, 1, b` (either) and the two with the
// condition's other side, of tests of one value that one test replaces.
// The tests have no other use. The select, after `before`, becomes the
// compare.
bool TestMerging::merge_select(BlockId block, InstId before, InstId id)
{
	const Instruction select = body_.instructions[id];
	if (select.result == kNoId || !dies(select.ops[0]))
		return false;
	uint64_t constant = 0;
	const bool second_literal = literal_bits(select.ops[2], ~uint64_t(0), constant);
	if (!second_literal && !literal_bits(select.ops[1], ~uint64_t(0), constant))
		return false;
	const Value other = select.ops[second_literal ? 1 : 2];
	if (constant > 1 || !used_once(other))
		return false;
	Test condition, value, merged;
	const Instruction* compare = producer(other);
	if (compare == nullptr || compare->op != Opcode::Compare || !test_of(select.ops[0], true, condition) ||
		!test_of(other, false, value))
		return false;
	// The select is `other` on one side of the condition and the constant
	// on the other: with constant 1 it is 1 where either holds.
	if (second_literal == (constant == 1))
		condition = complement(condition);
	if (constant == 1)
	{
		if (!unite(condition, value, merged))
			return false;
	}
	else
	{
		if (!unite(complement(condition), complement(value), merged))
			return false;
		merged = complement(merged);
	}
	Instruction test = emit(block, before, merged, select.location);
	test.result = select.result;
	test.next = select.next;
	body_.instructions[id] = test;
	++c_.stats.branches_folded;
	return true;
}

// `select c, K, (select d, K, L)` and the three forms with a condition's
// other side, the inner select read here only: K where one test of the
// conditions' value passes, L elsewhere.
bool TestMerging::merge_nested(BlockId block, InstId before, InstId id)
{
	const Instruction select = body_.instructions[id];
	if (select.result == kNoId || !dies(select.ops[0]))
		return false;
	for (unsigned arm = 1; arm <= 2; ++arm)
	{
		const Instruction* inner = producer(select.ops[arm]);
		const Value same = select.ops[3 - arm];
		if (inner == nullptr || inner->op != Opcode::Select || !(inner->type == select.type) ||
			!used_once(select.ops[arm]) || !dies(inner->ops[0]) || (inner->ops[1] == same) == (inner->ops[2] == same))
			continue;
		const bool on_true = inner->ops[1] == same;
		const Value rest = inner->ops[on_true ? 2 : 1];
		Test first, second, merged;
		if (!test_of(select.ops[0], true, first) || !test_of(inner->ops[0], true, second))
			continue;
		// The outer select is `same` on the side its inner one is not.
		if (arm == 1)
			first = complement(first);
		if (!on_true)
			second = complement(second);
		if (!unite(first, second, merged))
			continue;
		const bool other = shorter(merged);
		const Instruction test = emit(block, before, merged, select.location);
		const Value condition = add(block, before, test, Type::scalar(TypeKind::I64));
		Instruction& out = body_.instructions[id];
		out.ops[0] = condition;
		out.ops[1] = other ? rest : same;
		out.ops[2] = other ? same : rest;
		++c_.stats.branches_folded;
		return true;
	}
	return false;
}

// `select c, k, 0` and `select c, 0, k` on one bit of a value, `k` one bit:
// the bit moved to `k`'s place (the second form's inverted), in at most
// kBitInstructions instructions. The moved bits of one value then merge
// under one mask (simplify_values).
bool TestMerging::move_bit(BlockId block, InstId before, InstId id)
{
	const Instruction select = body_.instructions[id];
	Test test;
	if (select.result == kNoId || !test_type(select.type) || !test_of(select.ops[0], true, test) ||
		test.kind != Test::Masked || (test.mask & (test.mask - 1)) != 0 || (test.bits != 0 && test.bits != test.mask))
		return false;
	// The condition holds with the tested bit set (or clear). An arm may
	// name the masked value itself: the bit on that side, zero on the other.
	const bool set = test.differs == (test.bits == 0);
	const Type to_type = select.type;
	const unsigned to_width = 8 * storage_bytes(to_type);
	uint64_t arms[2] = { 0, 0 };
	for (unsigned arm = 0; arm < 2; ++arm)
		if (select.ops[arm + 1] == test.masked)
			arms[arm] = (arm == 0) == set ? test.mask : 0;
		else if (!literal_bits(select.ops[arm + 1], ~uint64_t(0) >> (64 - to_width), arms[arm]))
			return false;
	const uint64_t bit = arms[0] | arms[1];
	if ((arms[0] != 0) == (arms[1] != 0) || (bit & (bit - 1)) != 0)
		return false;
	// The select is `bit` where the tested bit is clear when `inverted`.
	const bool inverted = (arms[0] != 0) != set;
	const Type from_type = test.type;
	const unsigned from_width = 8 * storage_bytes(from_type);
	const unsigned from = static_cast<unsigned>(__builtin_ctzll(test.mask)),
				   to = static_cast<unsigned>(__builtin_ctzll(bit));
	// Shifted and masked in the tested value's type where the bit fits it.
	// A test that stays for another reader leaves room for one step less.
	const bool early = to < from_width;
	const bool converts = !(from_type == to_type);
	if ((from != to) + 1u + inverted + converts + !dies(select.ops[0]) > kBitInstructions)
		return false;
	std::vector<Instruction> steps;
	auto binary = [&](BinaryOp op, Type type, uint64_t bits) {
		Instruction inst = make_instruction(Opcode::Binary, type);
		inst.sub = static_cast<uint8_t>(op);
		inst.location = select.location;
		inst.ops[1] = integer_value(body_, normalize_integer(type, Literal{ bits, 0 }));
		steps.push_back(inst);
	};
	auto convert = [&]() {
		Instruction inst = make_instruction(Opcode::Convert, to_type);
		inst.sub = static_cast<uint8_t>(to_width < from_width ? ConvertOp::Trunc : ConvertOp::ZExt);
		inst.location = select.location;
		set_convert_source_type(inst, from_type);
		if (to_width == from_width)
		{
			inst.op = Opcode::Copy;
			inst.sub = 0;
		}
		steps.push_back(inst);
	};
	const Type type = early ? from_type : to_type;
	if (converts && !early)
		convert();
	if (from != to)
		binary(to > from ? BinaryOp::Shl : BinaryOp::UShr, type, to > from ? to - from : from - to);
	binary(BinaryOp::And, type, bit);
	if (inverted)
		binary(BinaryOp::Xor, type, bit);
	if (converts && early)
		convert();
	Value value = test.value;
	for (size_t k = 0; k < steps.size(); ++k)
	{
		steps[k].ops[0] = value;
		if (k + 1 == steps.size())
			break;
		const Type result = steps[k].op == Opcode::Binary ? steps[k].type : to_type;
		value = add(block, before, steps[k], result);
	}
	Instruction last = steps.back();
	last.result = select.result;
	last.next = select.next;
	body_.instructions[id] = last;
	++c_.stats.branches_folded;
	return true;
}

// A block reached only from `from` that computes a test and branches on it:
// at most kTestInstructions values, each used once and in the block.
bool TestMerging::second_block(BlockId b, BlockId from) const
{
	const Cfg& cfg = c_.cfg;
	if (b == from || touched_[b] || (cfg.flags[b] & (BF_HANDLER | BF_EH)) || cfg.pred_count(b) != 1 ||
		cfg.preds[cfg.pred_first[b]] != from)
		return false;
	const InstId last = body_.blocks[b].last;
	if (last == kNoId || body_.instructions[last].op != Opcode::Branch)
		return false;
	TempId values[kTestInstructions];
	unsigned count = 0, read = 0;
	for (InstId i = body_.blocks[b].first;; i = body_.instructions[i].next)
	{
		const Instruction& inst = body_.instructions[i];
		for_each_operand(body_, inst, [&](const Value& v) {
			for (unsigned k = 0; k < count && v.is(ValueKind::Temp); ++k)
				read += v.index == values[k];
		});
		if (i == last)
			return read == count;
		if (count == kTestInstructions || (inst.op != Opcode::Binary && inst.op != Opcode::Compare) ||
			inst.result == kNoId || !used_once(Value::temp(inst.result)) || !test_type(inst.type))
			return false;
		values[count++] = inst.result;
	}
}

// `e` branches on a test of a value to S or to a block that only tests the
// same value and branches to S or to R: where one test says which of the
// two is reached, `e` branches on it. S's phis agree for the two edges.
bool TestMerging::merge_branches(BlockId e)
{
	const InstId term = body_.blocks[e].last;
	if (term == kNoId || body_.instructions[term].op != Opcode::Branch)
		return false;
	const Instruction branch = body_.instructions[term];
	if (!dies(branch.ops[0]) || branch.ops[1].index == branch.ops[2].index)
		return false;
	for (unsigned side = 0; side < 2; ++side)
	{
		const BlockId next = branch.ops[1 + side].index, shared = branch.ops[2 - side].index;
		if (shared == e || touched_[shared] || !second_block(next, e))
			continue;
		const Instruction& inner = body_.instructions[body_.blocks[next].last];
		const BlockId inner_true = inner.ops[1].index, inner_false = inner.ops[2].index;
		if ((inner_true != shared) == (inner_false != shared) || inner_true == next || inner_false == next)
			continue;
		const BlockId rest = inner_true == shared ? inner_false : inner_true;
		if (rest == e || touched_[rest] || (c_.cfg.flags[shared] & BF_HANDLER) || (c_.cfg.flags[rest] & BF_HANDLER))
			continue;
		Test first, second, merged;
		if (!test_of(branch.ops[0], true, first) || !test_of(inner.ops[0], true, second))
			continue;
		// `e` reaches S on its condition's other side than `next`'s.
		if (side == 0)
			first = complement(first);
		if (inner_true != shared)
			second = complement(second);
		if (!unite(first, second, merged))
			continue;
		bool agree = true;
		for (InstId i = body_.blocks[shared].first; i != kNoId && body_.instructions[i].op == Opcode::Phi && agree;
			i = body_.instructions[i].next)
			agree = phi_incoming(body_, body_.instructions[i], e) == phi_incoming(body_, body_.instructions[i], next);
		if (!agree)
			continue;
		const bool other = shorter(merged);
		InstId at = previous_in_block(body_, e, term);
		const Instruction test = emit(e, at, merged, branch.location);
		const Value condition = add(e, at, test, Type::scalar(TypeKind::I64));
		Instruction& out = body_.instructions[term];
		out.ops[0] = condition;
		out.ops[1] = Value::block(other ? rest : shared);
		out.ops[2] = Value::block(other ? shared : rest);
		remove_phi_incoming(body_, shared, next);
		rename_phi_incoming(body_, rest, next, e);
		// The second test's block ends unreachable (the cleanup drops it).
		Instruction& end = body_.instructions[body_.blocks[next].last];
		end.op = Opcode::Unreachable;
		end.ops[0] = end.ops[1] = end.ops[2] = Value::none();
		touched_[e] = touched_[next] = touched_[shared] = touched_[rest] = 1;
		++c_.stats.branches_folded;
		return true;
	}
	return false;
}

// The widths the masks cover.
uint64_t all_bits(Type type)
{
	return ~uint64_t(0) >> (64 - 8 * storage_bytes(type));
}

TestMerging::Term TestMerging::term(Value v, Type type, BlockId block) const
{
	Term t = { v, v, type, all_bits(type), 0, 0, 1 };
	bool owned = true;
	for (unsigned step = 0; step < kMaskSteps; ++step)
	{
		const Instruction* def = producer(t.root);
		if (def == nullptr || !(def->type == t.from))
			break;
		Type next = t.from;
		uint64_t kept = 0;
		if (def->op == Opcode::Convert && static_cast<ConvertOp>(def->sub) == ConvertOp::ZExt &&
			test_type(convert_source_type(*def)))
		{
			next = convert_source_type(*def);
			kept = all_bits(next);
		}
		else if (def->op != Opcode::Binary || static_cast<BinaryOp>(def->sub) != BinaryOp::And ||
			!literal_bits(def->ops[1], all_bits(t.from), kept))
			break;
		owned = owned && own(*def, block);
		t.dying += owned ? 1 : 0;
		t.mask &= kept;
		t.root = def->ops[0];
		t.from = next;
	}
	return t;
}

// The terms of the `or` tree at `id` that keep bits of one value, each
// under its own masks, keep them under one: `(x & a) | (x & b)` is `x & (a |
// b)`, through zero-extensions too. Only where fewer instructions stay.
bool TestMerging::unite_masks(BlockId block, InstId before, InstId id)
{
	const Instruction seen = body_.instructions[id];
	const Type type = seen.type;
	if (!test_type(type))
		return false;
	Term terms[kMaskTerms];
	Value pending[kMaskTerms] = { seen.ops[1], seen.ops[0] };
	unsigned pending_count = 2, count = 0;
	while (pending_count != 0)
	{
		const Value v = pending[--pending_count];
		const Instruction* def = producer(v);
		if (def != nullptr && def->op == Opcode::Binary && static_cast<BinaryOp>(def->sub) == BinaryOp::Or &&
			def->type == type && own(*def, block) && count + pending_count + 2 <= kMaskTerms)
		{
			pending[pending_count++] = def->ops[1];
			pending[pending_count++] = def->ops[0];
			continue;
		}
		terms[count++] = term(v, type, block);
	}
	// A term joins the first one of its root, whose mask grows. A group of
	// several is new: the root, masked unless the mask is all of it,
	// extended unless it has the tree's type.
	unsigned groups = 0, was = count - 1, now = 0;
	for (unsigned i = 0; i < count; ++i)
	{
		Term& t = terms[i];
		t.lead = i;
		for (unsigned j = 0; j < i && t.lead == i; ++j)
			if (terms[j].lead == j && terms[j].root == t.root && terms[j].from == t.from)
				t.lead = j;
		if (t.lead == i)
		{
			++groups;
			continue;
		}
		Term& lead = terms[t.lead];
		lead.mask |= t.mask;
		lead.dying += t.dying;
		++lead.members;
	}
	for (unsigned i = 0; i < count; ++i)
		if (terms[i].lead == i && terms[i].members > 1)
		{
			was += terms[i].dying;
			now += (terms[i].mask != all_bits(terms[i].from) ? 1 : 0) + (terms[i].from == type ? 0 : 1);
		}
	if (groups == count || now + groups - 1 >= was)
		return false;
	Value values[kMaskTerms];
	groups = 0;
	for (unsigned i = 0; i < count; ++i)
	{
		if (terms[i].lead != i)
			continue;
		const bool merged = terms[i].members > 1;
		const Type from = terms[i].from;
		Value value = merged ? terms[i].root : terms[i].value;
		if (merged && terms[i].mask != all_bits(from))
		{
			Instruction inst = make_instruction(Opcode::Binary, from);
			inst.sub = static_cast<uint8_t>(BinaryOp::And);
			inst.location = seen.location;
			inst.ops[0] = value;
			inst.ops[1] = integer_value(body_, normalize_integer(from, Literal{ terms[i].mask, 0 }));
			value = add(block, before, inst, from);
		}
		if (merged && !(from == type))
		{
			Instruction inst = make_instruction(Opcode::Convert, type);
			inst.sub = static_cast<uint8_t>(ConvertOp::ZExt);
			set_convert_source_type(inst, from);
			inst.location = seen.location;
			inst.ops[0] = value;
			value = add(block, before, inst, type);
		}
		values[groups++] = value;
	}
	Value chain = values[0];
	for (unsigned k = 1; k + 1 < groups; ++k)
	{
		Instruction inst = make_instruction(Opcode::Binary, type);
		inst.sub = static_cast<uint8_t>(BinaryOp::Or);
		inst.location = seen.location;
		inst.ops[0] = chain;
		inst.ops[1] = values[k];
		chain = add(block, before, inst, type);
	}
	Instruction& changed = body_.instructions[id];
	changed.flags = 0;
	changed.ops[0] = chain;
	if (groups == 1)
	{
		changed.op = Opcode::Copy;
		changed.sub = 0;
		changed.ops[1] = Value::none();
	}
	else
		changed.ops[1] = values[groups - 1];
	++c_.stats.folded;
	return true;
}

bool TestMerging::run()
{
	c_.ensure_cfg();
	count_uses(body_, uses_);
	touched_.assign(body_.blocks.size(), 0);
	block_of_.assign(body_.instructions.size(), kNoId);
	bool changed = false;
	for (BlockId b : body_.layout)
	{
		if (!c_.cfg.reachable(b) || (c_.cfg.flags[b] & BF_EH))
			continue;
		for (InstId i = body_.blocks[b].first, before = kNoId; i != kNoId; before = i, i = body_.instructions[i].next)
		{
			const Instruction& inst = body_.instructions[i];
			if (i < block_of_.size())
				block_of_[i] = b;
			if (inst.op == Opcode::Select)
				changed = (inst.type.kind() == TypeKind::I64 && merge_select(b, before, i)) ||
					merge_nested(b, before, i) || move_bit(b, before, i) || changed;
			else if (inst.op == Opcode::Binary && static_cast<BinaryOp>(inst.sub) == BinaryOp::Or)
				changed = unite_masks(b, before, i) || changed;
		}
		if (!touched_[b])
			changed = merge_branches(b) || changed;
	}
	if (changed)
		c_.invalidate_cfg();
	return changed;
}

} // namespace

bool merge_value_tests(FunctionContext& context)
{
	TestMerging merging(context);
	return merging.run();
}

} // namespace opt
