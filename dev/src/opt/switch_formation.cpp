#include <algorithm>

#include "lowir/builder.h"
#include "opt/passes.h"
#include "opt/scalar_fold.h"

namespace opt {

using namespace lowir;

namespace {

// Disjoint inclusive ranges of an integer type's values (its canonical
// form: signed types sign-extended, unsigned ones zero-extended), sorted.
typedef std::vector<std::pair<int64_t, int64_t>> Ranges;

// The ranges a predicate's true side keeps of [lo, hi].
Ranges clip(const Ranges& s, int64_t lo, int64_t hi)
{
	Ranges out;
	for (const std::pair<int64_t, int64_t>& r : s)
	{
		const int64_t a = r.first > lo ? r.first : lo, b = r.second < hi ? r.second : hi;
		if (a <= b)
			out.push_back(std::make_pair(a, b));
	}
	return out;
}

Ranges unite(const Ranges& a, const Ranges& b)
{
	Ranges all(a);
	all.insert(all.end(), b.begin(), b.end());
	std::sort(all.begin(), all.end());
	Ranges out;
	for (const std::pair<int64_t, int64_t>& r : all)
	{
		if (!out.empty() && (out.back().second == INT64_MAX || r.first <= out.back().second + 1))
			out.back().second = r.second > out.back().second ? r.second : out.back().second;
		else
			out.push_back(r);
	}
	return out;
}

// The values of `all` outside `part` (part within all).
Ranges subtract(const Ranges& all, const Ranges& part, int64_t tmin, int64_t tmax)
{
	Ranges out;
	int64_t from = tmin;
	bool open = true;
	for (const std::pair<int64_t, int64_t>& p : part)
	{
		if (p.first > from)
			out = unite(out, clip(all, from, p.first - 1));
		open = p.second != tmax;
		if (!open)
			break;
		from = p.second + 1;
	}
	if (open)
		out = unite(out, clip(all, from, tmax));
	return out;
}

// The number of values (saturating at 2^62).
uint64_t count(const Ranges& s)
{
	uint64_t n = 0;
	for (const std::pair<int64_t, int64_t>& r : s)
	{
		const uint64_t width = static_cast<uint64_t>(r.second) - static_cast<uint64_t>(r.first);
		n += width >= (1ull << 62) ? (1ull << 62) : width + 1;
		if (n >= (1ull << 62))
			return 1ull << 62;
	}
	return n;
}

bool bounds_of(TypeKind kind, int64_t& lo, int64_t& hi)
{
	switch (kind)
	{
	case TypeKind::I8:
		lo = INT8_MIN;
		hi = INT8_MAX;
		return true;
	case TypeKind::U8:
		lo = 0;
		hi = UINT8_MAX;
		return true;
	case TypeKind::I16:
		lo = INT16_MIN;
		hi = INT16_MAX;
		return true;
	case TypeKind::U16:
		lo = 0;
		hi = UINT16_MAX;
		return true;
	case TypeKind::I32:
		lo = INT32_MIN;
		hi = INT32_MAX;
		return true;
	case TypeKind::U32:
		lo = 0;
		hi = UINT32_MAX;
		return true;
	case TypeKind::I64:
		lo = INT64_MIN;
		hi = INT64_MAX;
		return true;
	default:
		return false;
	}
}

// A block's test of value `x` (of `type`): a branch on its compare with
// the literal `k`, or a switch on it; `cost` the compares it stands for
// (a switch's runs of consecutive values per target).
struct Test
{
	TempId x;
	Type type;
	InstId term;
	bool is_switch;
	ComparePredicate pred;
	int64_t k;
	BlockId on_true;
	BlockId on_false;
	uint32_t cost;
};

// The most blocks one switch replaces, and the most case values.
const uint32_t kRegionBlocks = 24;
const uint64_t kMaxCases = 256;

class SwitchFormer
{
public:
	explicit SwitchFormer(FunctionContext& context) : c_(context), body_(context.body), cfg_(context.cfg) {}

	bool run();

private:
	bool test_of(BlockId block, bool head, Test& test) const;
	bool switch_of(BlockId block, bool head, Test& test) const;
	void sides(const Test& test, const Ranges& entry, int64_t tmin, int64_t tmax,
		std::vector<std::pair<BlockId, Ranges>>& out) const;
	bool form(BlockId head);

	FunctionContext& c_;
	FunctionBody& body_;
	Cfg& cfg_;
	std::vector<uint8_t> consumed_; // per block: inside a formed switch
};

// The block's branch on a single-use compare of a temporary with an
// integer literal, at a predicate of the type's signedness. A block after
// the head holds only the compare and the branch.
bool SwitchFormer::test_of(BlockId block, bool head, Test& test) const
{
	const InstId last = body_.blocks[block].last;
	if (last != kNoId && body_.instructions[last].op == Opcode::Switch)
		return switch_of(block, head, test);
	if (last == kNoId || body_.instructions[last].op != Opcode::Branch)
		return false;
	const Instruction& branch = body_.instructions[last];
	if (!branch.ops[0].is(ValueKind::Temp) || branch.ops[1].index == branch.ops[2].index)
		return false;
	const InstId def = body_.temps[branch.ops[0].index].def;
	if (def == kNoId || c_.uses[branch.ops[0].index] != 1)
		return false;
	const Instruction& cmp = body_.instructions[def];
	if (cmp.op != Opcode::Compare || c_.block_of[def] != block)
		return false;
	if (!head && (body_.blocks[block].first != def || cmp.next != last))
		return false;
	ComparePredicate pred = static_cast<ComparePredicate>(cmp.sub);
	Value x = cmp.ops[0], k = cmp.ops[1];
	if (!x.is(ValueKind::Temp))
	{
		std::swap(x, k);
		pred = swapped_predicate(pred);
	}
	Constant literal;
	if (!x.is(ValueKind::Temp) || !literal_constant(body_, k, literal) || body_.temps[x.index].type != cmp.type)
		return false;
	int64_t lo = 0, hi = 0;
	if (!bounds_of(cmp.type.kind(), lo, hi))
		return false;
	const bool sign = lo < 0;
	const bool ordered_signed = pred == ComparePredicate::Lt || pred == ComparePredicate::Le ||
		pred == ComparePredicate::Gt || pred == ComparePredicate::Ge;
	const bool ordered_unsigned = pred == ComparePredicate::Ult || pred == ComparePredicate::Ule ||
		pred == ComparePredicate::Ugt || pred == ComparePredicate::Uge;
	if ((ordered_signed && !sign) || (ordered_unsigned && sign))
		return false;
	Literal bits;
	if (!integer_of(cmp.type, literal, bits))
		return false;
	test.x = x.index;
	test.type = cmp.type;
	test.term = last;
	test.is_switch = false;
	test.pred = pred;
	test.k = static_cast<int64_t>(bits.low);
	test.on_true = branch.ops[1].index;
	test.on_false = branch.ops[2].index;
	test.cost = 1;
	return true;
}

// A switch on a temporary of an integer type with integer cases (alone in
// its block after the head).
bool SwitchFormer::switch_of(BlockId block, bool head, Test& test) const
{
	const InstId last = body_.blocks[block].last;
	const Instruction& term = body_.instructions[last];
	int64_t lo = 0, hi = 0;
	if (!term.ops[0].is(ValueKind::Temp) || (!head && body_.blocks[block].first != last))
		return false;
	const Type type = body_.temps[term.ops[0].index].type;
	if (!bounds_of(type.kind(), lo, hi) || (!term.type.is_void() && term.type != type))
		return false;
	std::vector<std::pair<int64_t, BlockId>> cases;
	for (uint32_t k = 0; k + 1 < term.extra_count; k += 2)
	{
		Constant literal;
		Literal bits;
		if (!literal_constant(body_, body_.operands[term.extra_first + k], literal) || !integer_of(type, literal, bits))
			return false;
		cases.push_back(std::make_pair(static_cast<int64_t>(bits.low), body_.operands[term.extra_first + k + 1].index));
	}
	std::stable_sort(cases.begin(), cases.end(),
		[](const std::pair<int64_t, BlockId>& a, const std::pair<int64_t, BlockId>& b) { return a.first < b.first; });
	test.x = term.ops[0].index;
	test.type = type;
	test.term = last;
	test.is_switch = true;
	test.on_true = test.on_false = term.ops[1].index;
	test.cost = 0;
	for (size_t k = 0; k < cases.size(); ++k)
		if (k == 0 || cases[k].second != cases[k - 1].second || cases[k].first != cases[k - 1].first + 1)
			++test.cost;
	return true;
}

// The values each successor of a test receives of `entry`.
void SwitchFormer::sides(
	const Test& t, const Ranges& entry, int64_t tmin, int64_t tmax, std::vector<std::pair<BlockId, Ranges>>& out) const
{
	out.clear();
	if (t.is_switch)
	{
		const Instruction& term = body_.instructions[t.term];
		Ranges taken;
		for (uint32_t k = 0; k + 1 < term.extra_count; k += 2)
		{
			Constant literal;
			Literal bits;
			literal_constant(body_, body_.operands[term.extra_first + k], literal);
			integer_of(t.type, literal, bits);
			const int64_t v = static_cast<int64_t>(bits.low);
			// A repeated value: its first case decides.
			const Ranges one = subtract(clip(entry, v, v), taken, tmin, tmax);
			taken = unite(taken, one);
			out.push_back(std::make_pair(body_.operands[term.extra_first + k + 1].index, one));
		}
		out.push_back(std::make_pair(t.on_false, subtract(entry, taken, tmin, tmax)));
		return;
	}
	const Ranges at_k = clip(entry, t.k, t.k);
	Ranges yes;
	switch (t.pred)
	{
	case ComparePredicate::Eq:
		yes = at_k;
		break;
	case ComparePredicate::Ne:
		yes = subtract(entry, at_k, tmin, tmax);
		break;
	case ComparePredicate::Lt:
	case ComparePredicate::Ult:
		yes = t.k == tmin ? Ranges() : clip(entry, tmin, t.k - 1);
		break;
	case ComparePredicate::Le:
	case ComparePredicate::Ule:
		yes = clip(entry, tmin, t.k);
		break;
	case ComparePredicate::Gt:
	case ComparePredicate::Ugt:
		yes = t.k == tmax ? Ranges() : clip(entry, t.k + 1, tmax);
		break;
	default:
		yes = clip(entry, t.k, tmax);
		break;
	}
	out.push_back(std::make_pair(t.on_true, yes));
	out.push_back(std::make_pair(t.on_false, subtract(entry, yes, tmin, tmax)));
}

// The blocks after `head` that test its value, reached only from among
// them, become one switch of the head on that value: each target outside
// the region takes the values its edges carry, the largest set the
// default (the others at most kMaxCases values together, as fewer runs
// than the region's tests), when every target's phis agree across the
// region's edges into it.
bool SwitchFormer::form(BlockId head)
{
	Test first;
	if (!test_of(head, true, first))
		return false;
	int64_t tmin = 0, tmax = 0;
	bounds_of(first.type.kind(), tmin, tmax);
	std::vector<BlockId> region(1, head);
	std::vector<Test> tests(1, first);
	auto inside = [&](BlockId b) { return std::find(region.begin(), region.end(), b) != region.end(); };
	for (bool grew = true; grew && region.size() < kRegionBlocks;)
	{
		grew = false;
		for (size_t r = 0; r < region.size() && region.size() < kRegionBlocks; ++r)
		{
			std::vector<BlockId> outs(1, tests[r].on_true);
			outs.push_back(tests[r].on_false);
			if (tests[r].is_switch)
			{
				const Instruction& term = body_.instructions[tests[r].term];
				for (uint32_t k = 1; k < term.extra_count; k += 2)
					outs.push_back(body_.operands[term.extra_first + k].index);
			}
			for (BlockId s : outs)
			{
				Test t;
				if (inside(s) || consumed_[s] || (cfg_.flags[s] & BF_HANDLER) || s == cfg_.rpo[0] ||
					!test_of(s, false, t) || t.x != first.x)
					continue;
				bool all = cfg_.pred_count(s) > 0;
				for (uint32_t p = cfg_.pred_first[s]; p < cfg_.pred_first[s + 1] && all; ++p)
					all = inside(cfg_.preds[p]);
				if (!all || region.size() >= kRegionBlocks)
					continue;
				region.push_back(s);
				tests.push_back(t);
				grew = true;
			}
		}
	}
	if (region.size() < 2)
		return false;
	// Value sets in reverse postorder (an edge back into the region: none).
	std::vector<size_t> order(region.size());
	for (size_t r = 0; r < region.size(); ++r)
		order[r] = r;
	std::sort(order.begin(), order.end(),
		[&](size_t a, size_t b) { return cfg_.rpo_index[region[a]] < cfg_.rpo_index[region[b]]; });
	std::vector<Ranges> entry(region.size());
	entry[0] = Ranges(1, std::make_pair(tmin, tmax));
	std::vector<std::pair<BlockId, Ranges>> exits;
	std::vector<std::pair<BlockId, Ranges>> out;
	uint32_t replaced = 0;
	for (size_t o = 0; o < order.size(); ++o)
	{
		const size_t r = order[o];
		replaced += tests[r].cost;
		sides(tests[r], entry[r], tmin, tmax, out);
		for (const std::pair<BlockId, Ranges>& side : out)
		{
			if (side.first == head)
				return false;
			const std::vector<BlockId>::iterator at = std::find(region.begin(), region.end(), side.first);
			if (at != region.end())
			{
				const size_t to = static_cast<size_t>(at - region.begin());
				if (cfg_.rpo_index[side.first] <= cfg_.rpo_index[region[r]])
					return false;
				entry[to] = unite(entry[to], side.second);
				continue;
			}
			size_t e = 0;
			while (e < exits.size() && exits[e].first != side.first)
				++e;
			if (e == exits.size())
				exits.push_back(std::make_pair(side.first, Ranges()));
			exits[e].second = unite(exits[e].second, side.second);
		}
	}
	// The default: the most values; ties to the target whose own runs are
	// the most.
	size_t def = exits.size();
	for (size_t e = 0; e < exits.size(); ++e)
		if (def == exits.size() || count(exits[e].second) > count(exits[def].second) ||
			(count(exits[e].second) == count(exits[def].second) && exits[e].second.size() > exits[def].second.size()))
			def = e;
	if (exits.size() < 2)
		return false;
	uint64_t cases = 0, runs = 0;
	for (size_t e = 0; e < exits.size(); ++e)
		if (e != def)
		{
			cases += count(exits[e].second);
			runs += exits[e].second.size();
		}
	if (cases > kMaxCases || runs >= replaced)
		return false;
	// Each target's phis: one value across the region's edges into it.
	for (const std::pair<BlockId, Ranges>& exit : exits)
		for (InstId i = body_.blocks[exit.first].first; i != kNoId && body_.instructions[i].op == Opcode::Phi;
			i = body_.instructions[i].next)
		{
			Value seen = Value::none();
			const Instruction& phi = body_.instructions[i];
			for (uint32_t k = 0; k < phi.extra_count; k += 2)
			{
				if (!inside(body_.operands[phi.extra_first + k].index))
					continue;
				const Value v = body_.operands[phi.extra_first + k + 1];
				if (!seen.is(ValueKind::None) && !(seen.kind == v.kind && seen.index == v.index))
					return false;
				seen = v;
			}
		}
	// The rewrite: the head switches; the region's other blocks lose their
	// predecessors (cleanup removes them).
	// A target keeps one phi input from the region, renamed to the head's;
	// a successor the switch never reaches keeps none.
	for (size_t e = 0; e < exits.size(); ++e)
	{
		const BlockId to = exits[e].first;
		const bool reached = e == def || !exits[e].second.empty();
		bool from_head = false;
		BlockId keep = kNoId;
		std::vector<BlockId> preds;
		for (uint32_t p = cfg_.pred_first[to]; p < cfg_.pred_first[to + 1]; ++p)
		{
			const BlockId pred = cfg_.preds[p];
			if (!inside(pred) || std::find(preds.begin(), preds.end(), pred) != preds.end())
				continue;
			preds.push_back(pred);
			from_head = from_head || pred == head;
			if (pred != head && keep == kNoId)
				keep = pred;
		}
		if (!reached)
			keep = kNoId;
		for (BlockId pred : preds)
			if ((pred == head && !reached) || (pred != head && (from_head || pred != keep)))
				remove_phi_incoming(body_, to, pred);
		if (reached && !from_head && keep != kNoId)
			rename_phi_incoming(body_, to, keep, head);
	}
	std::vector<Value> pairs;
	for (size_t e = 0; e < exits.size(); ++e)
	{
		if (e == def)
			continue;
		for (const std::pair<int64_t, int64_t>& r : exits[e].second)
			for (int64_t v = r.first;; ++v)
			{
				pairs.push_back(integer_value(body_, integer_literal(v)));
				pairs.push_back(Value::block(exits[e].first));
				if (v == r.second)
					break;
			}
	}
	Instruction& term = body_.instructions[body_.blocks[head].last];
	term.op = Opcode::Switch;
	term.type = first.type;
	term.ops[0] = Value::temp(first.x);
	term.ops[1] = Value::block(exits[def].first);
	term.ops[2] = Value::none();
	term.extra_first = append_operands(body_, pairs.data(), static_cast<uint32_t>(pairs.size()));
	term.extra_count = static_cast<uint32_t>(pairs.size());
	for (BlockId b : region)
		consumed_[b] = 1;
	return true;
}

bool SwitchFormer::run()
{
	c_.ensure_cfg();
	c_.entry_uses(form_switches);
	c_.entry_block_of(form_switches);
	consumed_.assign(body_.blocks.size(), 0);
	bool changed = false;
	const std::vector<BlockId> rpo = cfg_.rpo;
	for (BlockId b : rpo)
	{
		if (consumed_[b] || !form(b))
			continue;
		// The graph changed: fresh predecessors for the next head.
		changed = true;
		c_.invalidate_cfg();
		c_.ensure_cfg();
	}
	return changed;
}

} // namespace

bool form_switches(FunctionContext& context)
{
	SwitchFormer pass(context);
	return pass.run();
}

} // namespace opt
