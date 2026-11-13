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

Ranges intersect(const Ranges& a, const Ranges& b)
{
	Ranges out;
	for (const std::pair<int64_t, int64_t>& r : b)
		out = unite(out, clip(a, r.first, r.second));
	return out;
}

// The values of [lo, hi] with `v pred k`.
Ranges true_side(ComparePredicate pred, int64_t k, int64_t lo, int64_t hi)
{
	const Ranges all(1, std::make_pair(lo, hi));
	switch (pred)
	{
	case ComparePredicate::Eq:
		return clip(all, k, k);
	case ComparePredicate::Ne:
		return subtract(all, clip(all, k, k), lo, hi);
	case ComparePredicate::Lt:
	case ComparePredicate::Ult:
		return k == lo ? Ranges() : clip(all, lo, k - 1);
	case ComparePredicate::Le:
	case ComparePredicate::Ule:
		return clip(all, lo, k);
	case ComparePredicate::Gt:
	case ComparePredicate::Ugt:
		return k == hi ? Ranges() : clip(all, k + 1, hi);
	default:
		return clip(all, k, hi);
	}
}

// What a chain of integer extensions and constant additions computes of
// its root: for a root value in [lo, hi], that value plus `delta`.
struct Piece
{
	int64_t lo;
	int64_t hi;
	int64_t delta;
};

// The links one chain holds, and the pieces one may need.
const size_t kChainLinks = 4;
const size_t kChainPieces = 16;

// An integer type of at most 32 bits: a chain's sums stay within i64.
unsigned narrow_bits(Type type)
{
	switch (type.kind())
	{
	case TypeKind::I8:
	case TypeKind::U8:
		return 8;
	case TypeKind::I16:
	case TypeKind::U16:
		return 16;
	case TypeKind::I32:
	case TypeKind::U32:
		return 32;
	default:
		return 0;
	}
}

// The pieces' values reduced to `bits` width, read signed or unsigned: a
// piece crossing the width's bound becomes two. False past kChainPieces.
bool wrap(std::vector<Piece>& pieces, unsigned bits, bool sign)
{
	const int64_t m = static_cast<int64_t>(1) << bits, low = sign ? -(m / 2) : 0;
	auto turns = [&](int64_t v) { return v - low >= 0 ? (v - low) / m : -((low - v + m - 1) / m); };
	std::vector<Piece> out;
	for (const Piece& p : pieces)
	{
		const int64_t a = p.lo + p.delta, b = p.hi + p.delta;
		for (int64_t q = turns(a); q <= turns(b); ++q)
		{
			const int64_t first = low + q * m, last = first + m - 1;
			const int64_t from = a > first ? a : first, to = b < last ? b : last;
			out.push_back(Piece{ from - p.delta, to - p.delta, p.delta - q * m });
			if (out.size() > kChainPieces)
				return false;
		}
	}
	pieces.swap(out);
	return true;
}

// The root values whose derived value lies in `derived`.
Ranges roots_of(const std::vector<Piece>& pieces, const Ranges& derived)
{
	Ranges out;
	for (const Piece& p : pieces)
		for (const std::pair<int64_t, int64_t>& r : derived)
		{
			const int64_t a = r.first - p.delta > p.lo ? r.first - p.delta : p.lo;
			const int64_t b = r.second - p.delta < p.hi ? r.second - p.delta : p.hi;
			if (a <= b)
				out = unite(out, Ranges(1, std::make_pair(a, b)));
		}
	return out;
}

// A block's test of value `x` (of `type`; a pointer's equality tests as
// i64, `pointer` set): a branch on a compare with a literal, of it or of a
// value a chain of extensions and constant additions derives from it
// (`yes`: its values the branch takes), or a switch on either (`cases`:
// its value per case, in case order); `cost` the compares it stands for (a
// switch's runs of consecutive values per target). `hoist`: the chain's
// instructions in the test's own block.
struct Test
{
	TempId x;
	Type type;
	bool pointer;
	InstId term;
	bool is_switch;
	Ranges yes;
	std::vector<std::pair<int64_t, BlockId>> cases;
	std::vector<InstId> hoist;
	BlockId on_true;
	BlockId on_false;
	uint32_t cost;
};

// The most blocks one switch replaces, and the most case values.
const uint32_t kRegionBlocks = 24;
const uint64_t kMaxCases = 256;
// The fewest runs that take a jump table whatever their targets (the
// selector's kJumpTableClusters).
const uint64_t kTableRuns = 6;

class SwitchFormer
{
public:
	explicit SwitchFormer(FunctionContext& context) : c_(context), body_(context.body), cfg_(context.cfg) {}

	bool run();

private:
	bool test_of(BlockId block, bool head, Test& test) const;
	bool switch_of(BlockId block, bool head, Test& test) const;
	bool root_of(TempId x, TempId& root, std::vector<Piece>& pieces, std::vector<InstId>& links) const;
	bool only_links(
		BlockId block, bool head, InstId test, const std::vector<InstId>& links, std::vector<InstId>& hoist) const;
	void sides(const Test& test, const Ranges& entry, int64_t tmin, int64_t tmax,
		std::vector<std::pair<BlockId, Ranges>>& out) const;
	bool form(BlockId head);
	bool form_region(const std::vector<BlockId>& region, const std::vector<Test>& tests);
	bool reroot(const Test& test);

	FunctionContext& c_;
	FunctionBody& body_;
	Cfg& cfg_;
	std::vector<uint8_t> consumed_; // per block: inside a formed switch
};

// x's root: x without the extensions (from no wider a type) and additions
// of a constant that define it, types of at most 32 bits each (`links`,
// the outermost first), and what they compute of the root (`pieces`).
// False when the pieces are too many.
bool SwitchFormer::root_of(TempId x, TempId& root, std::vector<Piece>& pieces, std::vector<InstId>& links) const
{
	links.clear();
	root = x;
	while (links.size() < kChainLinks && root >= body_.parameter_count)
	{
		const InstId d = body_.temps[root].def;
		if (d == kNoId)
			break;
		const Instruction& inst = body_.instructions[d];
		if (narrow_bits(inst.type) == 0 || !inst.ops[0].is(ValueKind::Temp))
			break;
		const BinaryOp sum = static_cast<BinaryOp>(inst.sub);
		const ConvertOp ext = static_cast<ConvertOp>(inst.sub);
		const bool adds = inst.op == Opcode::Binary && (sum == BinaryOp::Add || sum == BinaryOp::Sub) &&
			inst.ops[1].is(ValueKind::Integer) && body_.temps[inst.ops[0].index].type == inst.type;
		const bool extends = inst.op == Opcode::Convert && (ext == ConvertOp::ZExt || ext == ConvertOp::SExt) &&
			narrow_bits(convert_source_type(inst)) != 0 &&
			narrow_bits(convert_source_type(inst)) <= narrow_bits(inst.type) &&
			body_.temps[inst.ops[0].index].type == convert_source_type(inst);
		if (!adds && !extends)
			break;
		links.push_back(d);
		root = inst.ops[0].index;
	}
	int64_t lo = 0, hi = 0;
	const Type type = body_.temps[root].type;
	if (!bounds_of(type.is_pointer() ? TypeKind::I64 : type.kind(), lo, hi))
		return false;
	pieces.assign(1, Piece{ lo, hi, 0 });
	for (size_t k = links.size(); k-- > 0;)
	{
		const Instruction& inst = body_.instructions[links[k]];
		bounds_of(inst.type.kind(), lo, hi);
		if (inst.op == Opcode::Binary)
		{
			Constant literal;
			Literal bits;
			if (!literal_constant(body_, inst.ops[1], literal) || !integer_of(inst.type, literal, bits))
				return false;
			const int64_t k1 = static_cast<int64_t>(bits.low);
			for (Piece& p : pieces)
				p.delta += static_cast<BinaryOp>(inst.sub) == BinaryOp::Add ? k1 : -k1;
		}
		// An extension reads the source's bits: unsigned to add zeros,
		// signed to copy the sign.
		else if (!wrap(pieces, narrow_bits(convert_source_type(inst)),
					 static_cast<ConvertOp>(inst.sub) == ConvertOp::SExt))
			return false;
		if (!wrap(pieces, narrow_bits(inst.type), lo < 0))
			return false;
	}
	return true;
}

// Whether a block after the head holds only its test (`test`, then the
// terminator) and before it links of the test's chain, which `hoist` then
// lists in order: the head runs them when the block goes.
bool SwitchFormer::only_links(
	BlockId block, bool head, InstId test, const std::vector<InstId>& links, std::vector<InstId>& hoist) const
{
	hoist.clear();
	if (head)
		return true;
	const InstId last = body_.blocks[block].last;
	for (InstId i = body_.blocks[block].first; i != test; i = body_.instructions[i].next)
	{
		if (i == last || std::find(links.begin(), links.end(), i) == links.end())
			return false;
		hoist.push_back(i);
	}
	return test == last || body_.instructions[test].next == last;
}

// The block's branch on a single-use compare of a temporary with an
// integer literal, at a predicate of the type's signedness (a pointer's:
// equality alone), read as a test of the temporary's root. A block after
// the head holds only the root's chain, the compare and the branch.
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
	ComparePredicate pred = static_cast<ComparePredicate>(cmp.sub);
	Value x = cmp.ops[0], k = cmp.ops[1];
	if (!x.is(ValueKind::Temp))
	{
		std::swap(x, k);
		pred = swapped(pred);
	}
	Constant literal;
	if (!x.is(ValueKind::Temp) || !literal_constant(body_, k, literal) || body_.temps[x.index].type != cmp.type)
		return false;
	const bool pointer = cmp.type.is_pointer();
	const Type type = pointer ? Type::scalar(TypeKind::I64) : cmp.type;
	int64_t lo = 0, hi = 0;
	if (!bounds_of(type.kind(), lo, hi) || (pointer && pred != ComparePredicate::Eq && pred != ComparePredicate::Ne))
		return false;
	const bool sign = lo < 0;
	const bool ordered_signed = pred == ComparePredicate::Lt || pred == ComparePredicate::Le ||
		pred == ComparePredicate::Gt || pred == ComparePredicate::Ge;
	const bool ordered_unsigned = pred == ComparePredicate::Ult || pred == ComparePredicate::Ule ||
		pred == ComparePredicate::Ugt || pred == ComparePredicate::Uge;
	if ((ordered_signed && !sign) || (ordered_unsigned && sign))
		return false;
	Literal bits;
	std::vector<Piece> pieces;
	std::vector<InstId> links;
	if (!integer_of(type, literal, bits) || !root_of(x.index, test.x, pieces, links) ||
		!only_links(block, head, def, links, test.hoist))
		return false;
	test.pointer = pointer;
	test.type = links.empty() ? type : body_.temps[test.x].type;
	test.term = last;
	test.is_switch = false;
	test.yes = roots_of(pieces, true_side(pred, static_cast<int64_t>(bits.low), lo, hi));
	test.cases.clear();
	test.on_true = branch.ops[1].index;
	test.on_false = branch.ops[2].index;
	test.cost = 1;
	return true;
}

// A switch on a temporary of an integer type with integer cases (alone in
// its block after the head, but for its root's chain), read as one on the
// temporary's root.
bool SwitchFormer::switch_of(BlockId block, bool head, Test& test) const
{
	const InstId last = body_.blocks[block].last;
	const Instruction& term = body_.instructions[last];
	int64_t lo = 0, hi = 0;
	if (!term.ops[0].is(ValueKind::Temp))
		return false;
	const Type type = body_.temps[term.ops[0].index].type;
	std::vector<Piece> pieces;
	std::vector<InstId> links;
	if (!bounds_of(type.kind(), lo, hi) || (!term.type.is_void() && term.type != type) ||
		!root_of(term.ops[0].index, test.x, pieces, links) || !only_links(block, head, last, links, test.hoist))
		return false;
	test.cases.clear();
	std::vector<std::pair<int64_t, BlockId>> cases;
	for (uint32_t k = 0; k + 1 < term.extra_count; k += 2)
	{
		Constant literal;
		Literal bits;
		if (!literal_constant(body_, body_.operands[term.extra_first + k], literal) || !integer_of(type, literal, bits))
			return false;
		const int64_t v = static_cast<int64_t>(bits.low);
		const BlockId to = body_.operands[term.extra_first + k + 1].index;
		cases.push_back(std::make_pair(v, to));
		// (An extension or a sum of a constant: one root value at most.)
		const Ranges roots = roots_of(pieces, Ranges(1, std::make_pair(v, v)));
		if (!roots.empty())
			test.cases.push_back(std::make_pair(roots[0].first, to));
	}
	std::stable_sort(cases.begin(), cases.end(),
		[](const std::pair<int64_t, BlockId>& a, const std::pair<int64_t, BlockId>& b) { return a.first < b.first; });
	test.type = body_.temps[test.x].type;
	test.pointer = false;
	test.term = last;
	test.is_switch = true;
	test.yes.clear();
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
		Ranges taken;
		for (const std::pair<int64_t, BlockId>& c : t.cases)
		{
			// A repeated value: its first case decides.
			const Ranges one = subtract(clip(entry, c.first, c.first), taken, tmin, tmax);
			taken = unite(taken, one);
			out.push_back(std::make_pair(c.second, one));
		}
		out.push_back(std::make_pair(t.on_false, subtract(entry, taken, tmin, tmax)));
		return;
	}
	const Ranges yes = intersect(entry, t.yes);
	out.push_back(std::make_pair(t.on_true, yes));
	out.push_back(std::make_pair(t.on_false, subtract(entry, yes, tmin, tmax)));
}

// The blocks after `head` that test its value, reached only from among
// them, become one switch of the head on that value: each target outside
// the region takes the values its edges carry, the largest set the
// default (the others at most kMaxCases values together, as fewer runs
// than the region's tests, or as many from kTableRuns), when every
// target's phis agree across the region's edges into it. A region that
// does not qualify gives up its last block (each was reached only from
// those before it) and tries again.
bool SwitchFormer::form(BlockId head)
{
	Test first;
	if (!test_of(head, true, first))
		return false;
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
	for (; region.size() >= 2; region.pop_back(), tests.pop_back())
		if (form_region(region, tests))
			return true;
	return first.is_switch && reroot(first);
}

// A switch by itself on a value derived from its root (an opcode less its
// first enumerator) becomes the switch on the root, each case at the root's
// value of it: the table's index is one difference from what was loaded.
bool SwitchFormer::reroot(const Test& test)
{
	Instruction& term = body_.instructions[test.term];
	const Type type = body_.temps[test.x].type;
	if (!term.ops[0].is(ValueKind::Temp) || term.ops[0].index == test.x || !type.is_integer() ||
		test.cases.size() * 2 != term.extra_count)
		return false;
	// (Every case has its root value: they stand in the cases' order.)
	for (size_t k = 0; k < test.cases.size(); ++k)
		body_.operands[term.extra_first + 2 * k] = integer_value(body_, integer_literal(test.cases[k].first));
	--c_.uses[term.ops[0].index];
	++c_.uses[test.x];
	term.ops[0] = Value::temp(test.x);
	term.type = type;
	return true;
}

bool SwitchFormer::form_region(const std::vector<BlockId>& region, const std::vector<Test>& tests)
{
	const BlockId head = region[0];
	const Test& first = tests[0];
	int64_t tmin = 0, tmax = 0;
	bounds_of(first.type.kind(), tmin, tmax);
	auto inside = [&](BlockId b) { return std::find(region.begin(), region.end(), b) != region.end(); };
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
			const std::vector<BlockId>::const_iterator at = std::find(region.begin(), region.end(), side.first);
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
	// (As many runs as tests replaced: a switch only from the count that
	// takes a jump table, kTableRuns.)
	if (cases > kMaxCases || runs > replaced || (runs == replaced && runs < kTableRuns))
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
	// A pointer is switched on as the integer it is.
	Value selector = Value::temp(first.x);
	if (first.pointer)
	{
		InstId prev = kNoId;
		for (InstId i = body_.blocks[head].first; i != body_.blocks[head].last; i = body_.instructions[i].next)
			prev = i;
		Instruction copy = make_instruction(Opcode::Copy, first.type);
		copy.ops[0] = selector;
		copy.location = body_.instructions[body_.blocks[head].last].location;
		const InstId id = append_instruction(body_, copy);
		body_.instructions[id].result = append_temp(body_, first.type, id);
		insert_after(body_, head, prev, id);
		selector = Value::temp(body_.instructions[id].result);
		c_.uses.resize(body_.temps.size(), 1);
		c_.block_of.resize(body_.instructions.size(), head);
	}
	// A chain a block that goes computed for a reader outside it: the head
	// computes it (blocks in reverse postorder: a chain's own links first).
	auto read_outside = [&](BlockId block, const std::vector<InstId>& chain) {
		for (InstId link : chain)
		{
			const Value v = Value::temp(body_.instructions[link].result);
			uint32_t inside_uses = 0;
			for (InstId i = body_.blocks[block].first; i != kNoId; i = body_.instructions[i].next)
				for_each_operand(body_, body_.instructions[i], [&](const Value& u) { inside_uses += u == v ? 1 : 0; });
			if (c_.uses[v.index] != inside_uses)
				return true;
		}
		return false;
	};
	for (size_t o = 0; o < order.size(); ++o)
	{
		const size_t r = order[o];
		if (r == 0 || !read_outside(region[r], tests[r].hoist))
			continue;
		for (InstId link : tests[r].hoist)
		{
			unlink_after(body_, region[r], previous_in_block(body_, region[r], link), link);
			insert_after(body_, head, previous_in_block(body_, head, body_.blocks[head].last), link);
			c_.block_of[link] = head;
		}
	}
	// Two targets, one of them for one value: a branch on the equality.
	const size_t lone = exits.size() == 2 ? 1 - def : def;
	if (lone != def && !first.pointer && count(exits[lone].second) == 1)
	{
		Instruction cmp = make_instruction(Opcode::Compare, first.type);
		cmp.sub = static_cast<uint8_t>(ComparePredicate::Eq);
		cmp.ops[0] = selector;
		cmp.ops[1] = integer_value(body_, integer_literal(exits[lone].second[0].first));
		cmp.location = body_.instructions[body_.blocks[head].last].location;
		const InstId id = append_instruction(body_, cmp);
		body_.instructions[id].result = append_temp(body_, Type::scalar(TypeKind::I64), id);
		insert_after(body_, head, previous_in_block(body_, head, body_.blocks[head].last), id);
		c_.uses.resize(body_.temps.size(), 1);
		c_.block_of.resize(body_.instructions.size(), head);
		Instruction& branch = body_.instructions[body_.blocks[head].last];
		branch.op = Opcode::Branch;
		branch.sub = 0;
		branch.flags = 0;
		branch.type = Type::scalar(TypeKind::Void);
		branch.ops[0] = Value::temp(body_.instructions[id].result);
		branch.ops[1] = Value::block(exits[lone].first);
		branch.ops[2] = Value::block(exits[def].first);
		branch.extra_first = 0;
		branch.extra_count = 0;
	}
	else
	{
		Instruction& term = body_.instructions[body_.blocks[head].last];
		term.op = Opcode::Switch;
		term.sub = 0;
		term.type = first.type;
		term.ops[0] = selector;
		term.ops[1] = Value::block(exits[def].first);
		term.ops[2] = Value::none();
		term.extra_first = append_operands(body_, pairs.data(), static_cast<uint32_t>(pairs.size()));
		term.extra_count = static_cast<uint32_t>(pairs.size());
	}
	// The region's other blocks end unreachable (the cleanup drops them), as
	// their inputs are gone: no phi names a block that still jumps to it.
	for (size_t r = 1; r < region.size(); ++r)
	{
		Instruction& end = body_.instructions[body_.blocks[region[r]].last];
		if (end.ops[0].is(ValueKind::Temp))
			--c_.uses[end.ops[0].index];
		end.op = Opcode::Unreachable;
		end.sub = 0;
		end.flags = 0;
		end.type = Type::scalar(TypeKind::Void);
		end.ops[0] = end.ops[1] = end.ops[2] = Value::none();
		end.extra_first = 0;
		end.extra_count = 0;
	}
	for (BlockId b : region)
		consumed_[b] = 1;
	return true;
}

bool SwitchFormer::run()
{
	c_.ensure_cfg_light();
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
		c_.ensure_cfg_light();
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
