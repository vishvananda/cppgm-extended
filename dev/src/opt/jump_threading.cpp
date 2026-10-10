#include <algorithm>

#include "lowir/builder.h"
#include "opt/passes.h"
#include "opt/scalar_fold.h"

namespace opt {

using namespace lowir;

namespace {

// A block threaded through holds at most this many instructions besides
// its phis (each predecessor re-evaluates them).
const uint32_t kThreadWork = 8;
// A block whose branch a jumping predecessor takes over (its values not
// decided there) holds at most this many instructions besides its phis,
// and one block gives at most kCloneBudget copies in all.
const uint32_t kCopyWork = 2;
const uint32_t kCloneBudget = 8;

// Values of an integer of some width as unsigned bit patterns: disjoint
// intervals (at most four; `lost` once more were needed, which decides
// nothing). The values a branch on a compare with a constant admits on
// one of its edges, carried through integer extensions.
struct ValueSet
{
	uint64_t lo[4];
	uint64_t hi[4];
	unsigned n = 0;
	bool lost = false;

	void add(uint64_t a, uint64_t b)
	{
		if (a > b)
			return;
		if (n == 4)
		{
			lost = true;
			return;
		}
		lo[n] = a;
		hi[n] = b;
		++n;
	}
};

uint64_t ones(unsigned bits)
{
	return bits >= 64 ? ~0ull : (1ull << bits) - 1;
}

// The width an integer or pointer value compares at (0: other types).
unsigned value_bits(Type t)
{
	if (t.is_pointer())
		return 64;
	switch (t.kind())
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
	case TypeKind::I64:
		return 64;
	default:
		return 0;
	}
}

// The values x of `bits` width with `x pred k`.
ValueSet admitted(ComparePredicate pred, uint64_t k, unsigned bits)
{
	const uint64_t max = ones(bits), sign = 1ull << (bits - 1);
	k &= max;
	ValueSet s;
	const int64_t ks = bits == 64 || (k & sign) == 0 ? static_cast<int64_t>(k) : static_cast<int64_t>(k | ~max);
	const int64_t smin = bits == 64 ? INT64_MIN : -static_cast<int64_t>(sign);
	const int64_t smax = bits == 64 ? INT64_MAX : static_cast<int64_t>(sign - 1);
	auto signed_range = [&](int64_t a, int64_t b) {
		if (a > b)
			return;
		if (b >= 0)
			s.add(static_cast<uint64_t>(a > 0 ? a : 0), static_cast<uint64_t>(b));
		if (a < 0)
			s.add(static_cast<uint64_t>(a) & max, static_cast<uint64_t>(b < -1 ? b : -1) & max);
	};
	switch (pred)
	{
	case ComparePredicate::Eq:
		s.add(k, k);
		break;
	case ComparePredicate::Ne:
		if (k > 0)
			s.add(0, k - 1);
		if (k < max)
			s.add(k + 1, max);
		break;
	case ComparePredicate::Ult:
		if (k > 0)
			s.add(0, k - 1);
		break;
	case ComparePredicate::Ule:
		s.add(0, k);
		break;
	case ComparePredicate::Ugt:
		if (k < max)
			s.add(k + 1, max);
		break;
	case ComparePredicate::Uge:
		s.add(k, max);
		break;
	case ComparePredicate::Lt:
		if (ks > smin)
			signed_range(smin, ks - 1);
		break;
	case ComparePredicate::Le:
		signed_range(smin, ks);
		break;
	case ComparePredicate::Gt:
		if (ks < smax)
			signed_range(ks + 1, smax);
		break;
	case ComparePredicate::Ge:
		signed_range(ks, smax);
		break;
	}
	return s;
}

// Through an extension from `from` to `to` bits; back through one (the
// narrower values whose extension lies in the set).
ValueSet extended(const ValueSet& s, unsigned from, unsigned to, bool sign_ext)
{
	if (!sign_ext)
		return s;
	const uint64_t sign = 1ull << (from - 1), high = ones(to) & ~ones(from);
	ValueSet r;
	r.lost = s.lost;
	for (unsigned i = 0; i < s.n; ++i)
	{
		if (s.lo[i] < sign)
			r.add(s.lo[i], s.hi[i] < sign - 1 ? s.hi[i] : sign - 1);
		if (s.hi[i] >= sign)
			r.add((s.lo[i] > sign ? s.lo[i] : sign) | high, s.hi[i] | high);
	}
	return r;
}

ValueSet unextended(const ValueSet& s, unsigned from, unsigned to, bool sign_ext)
{
	const uint64_t max = ones(from), sign = 1ull << (from - 1), high = ones(to) & ~max;
	ValueSet r;
	r.lost = s.lost;
	for (unsigned i = 0; i < s.n; ++i)
	{
		if (!sign_ext)
		{
			if (s.lo[i] <= max)
				r.add(s.lo[i], s.hi[i] < max ? s.hi[i] : max);
			continue;
		}
		if (s.lo[i] < sign)
			r.add(s.lo[i], s.hi[i] < sign - 1 ? s.hi[i] : sign - 1);
		const uint64_t a = s.lo[i] > (high | sign) ? s.lo[i] : (high | sign),
					   b = s.hi[i] < (high | max) ? s.hi[i] : (high | max);
		if (a <= b && a >= (high | sign))
			r.add(a & max, b & max);
	}
	return r;
}

// 1 when every value of `f` lies in `q`, 0 when none does, -1 otherwise
// (or when either lost intervals, or `f` is empty).
int decided_by(const ValueSet& f, const ValueSet& q)
{
	if (f.lost || q.lost || f.n == 0)
		return -1;
	bool all = true, none = true;
	for (unsigned i = 0; i < f.n; ++i)
	{
		bool inside = false, overlaps = false;
		for (unsigned j = 0; j < q.n; ++j)
		{
			inside = inside || (q.lo[j] <= f.lo[i] && f.hi[i] <= q.hi[j]);
			overlaps = overlaps || (q.lo[j] <= f.hi[i] && f.lo[i] <= q.hi[j]);
		}
		all = all && inside;
		none = none && !overlaps;
	}
	return all ? 1 : none ? 0 : -1;
}

// The values of `bits` width switch `sw` sends to `to`: its cases for
// `to`, or when `to` is the default every value but the other targets'
// cases (lost past four intervals, or with a case not literal).
ValueSet switched_to(const FunctionBody& body, const Instruction& sw, BlockId to, unsigned bits)
{
	const uint64_t max = ones(bits);
	const bool by_default = sw.ops[1].index == to;
	std::vector<uint64_t> keys;
	ValueSet s;
	for (uint32_t k = 0; k + 1 < sw.extra_count; k += 2)
	{
		const Value key = body.operands[sw.extra_first + k];
		if (!key.is(ValueKind::Integer))
		{
			s.lost = true;
			return s;
		}
		if ((body.operands[sw.extra_first + k + 1].index == to) != by_default)
			keys.push_back(body.literals[key.index].low & max);
	}
	std::sort(keys.begin(), keys.end());
	keys.erase(std::unique(keys.begin(), keys.end()), keys.end());
	if (!by_default)
	{
		for (size_t i = 0; i < keys.size();)
		{
			size_t j = i;
			while (j + 1 < keys.size() && keys[j + 1] == keys[j] + 1)
				++j;
			s.add(keys[i], keys[j]);
			i = j + 1;
		}
		return s;
	}
	uint64_t from = 0;
	for (uint64_t key : keys)
	{
		if (key > from)
			s.add(from, key - 1);
		from = key + 1;
	}
	if (keys.empty() || keys.back() < max)
		s.add(from, max);
	return s;
}

// An integer extension: its source width, its own, and whether it signs.
struct Extension
{
	unsigned from;
	unsigned to;
	bool sign;
};

class JumpThreading
{
public:
	explicit JumpThreading(FunctionContext& context) : c_(context), body_(context.body), cfg_(context.cfg) {}

	bool run();
	uint32_t demoted() const { return demoted_; }

private:
	bool known_edges();
	bool known_compares();
	bool demote_phis(BlockId s, std::vector<std::pair<BlockId, BlockId>>& redirects);
	bool copy_thread(BlockId s, InstId tail, std::vector<std::pair<BlockId, BlockId>>& redirects);
	bool edge_fact(BlockId cur, BlockId to, Value& cx, ValueSet& f, unsigned& bits) const;
	Value stripped(Value v, std::vector<Extension>& chain) const;
	ValueSet related(Value cx, ValueSet f, unsigned bits, Value v, unsigned vbits) const;
	bool mergeable(BlockId p, BlockId s, Value v) const;
	BlockId copy_merge(BlockId p, BlockId q, BlockId s);
	bool dominating_facts();
	// `counted`: c_.uses holds the body's use counts as it stands.
	bool constant_phis(bool counted);
	void find_use_dominators();
	BlockId successor_holding(BlockId m, TempId t) const;
	bool evaluate(BlockId block, const Value* inputs, BlockId& target);
	bool apply_moves(BlockId m, const std::vector<BlockId>& preds, uint32_t phis,
		std::vector<std::pair<uint32_t, BlockId>>& moves, std::vector<uint8_t>& touched);
	bool copy_branch(BlockId m, BlockId p, const Value* inputs, std::vector<uint8_t>& touched);

	FunctionContext& c_;
	FunctionBody& body_;
	Cfg& cfg_;
	std::vector<Value> scratch_; // per temp: a value evaluated for one predecessor
	// constant_phis scratch: uses inside the block examined (stamped by
	// block), predecessor slots, and the phis' inputs per slot.
	std::vector<uint32_t> local_, local_stamp_, slot_of_;
	std::vector<Value> inputs_;
	std::vector<uint32_t> mark_, phi_pos_; // apply_moves' scratch (marks by epoch)
	std::vector<BlockId> use_dom_;         // find_use_dominators' result (empty until needed)
	uint32_t epoch_ = 0;
	uint32_t demoted_ = 0; // known_compares' phis demoted to slots this run
	bool touched_ = false; // the run changed the body so far
};

// A block whose only predecessor branches on `c` knows `c` on its edge: a
// branch of its own on the same value takes the known side (the dropped
// edge leaves its target's phis).
bool JumpThreading::known_edges()
{
	bool changed = false;
	for (BlockId b : body_.layout)
	{
		if (!cfg_.reachable(b) || cfg_.pred_count(b) != 1 || (cfg_.flags[b] & BF_HANDLER) != 0)
			continue;
		const BlockId p = cfg_.preds[cfg_.pred_first[b]];
		const InstId pt = body_.blocks[p].last, bt = body_.blocks[b].last;
		if (pt == kNoId || bt == kNoId || p == b)
			continue;
		const Instruction& pred_term = body_.instructions[pt];
		Instruction& term = body_.instructions[bt];
		if (pred_term.op != Opcode::Branch || term.op != Opcode::Branch ||
			pred_term.ops[1].index == pred_term.ops[2].index || !(pred_term.ops[0] == term.ops[0]) ||
			!term.ops[0].is(ValueKind::Temp))
			continue;
		const bool truth = pred_term.ops[1].index == b;
		const BlockId keep = truth ? term.ops[1].index : term.ops[2].index;
		const BlockId drop = truth ? term.ops[2].index : term.ops[1].index;
		if (keep != drop)
			remove_phi_incoming(body_, drop, b);
		term.op = Opcode::Jump;
		term.ops[0] = Value::block(keep);
		term.ops[1] = term.ops[2] = Value::none();
		++c_.stats.branches_folded;
		changed = true;
	}
	return changed;
}

// The target `block`'s terminator takes when entered with its phis'
// `inputs` (in phi order): every other instruction must fold.
bool JumpThreading::evaluate(BlockId block, const Value* inputs, BlockId& target)
{
	auto value = [&](Value v) {
		return v.is(ValueKind::Temp) && v.index < scratch_.size() && !scratch_[v.index].is(ValueKind::None)
			? scratch_[v.index]
			: v;
	};
	std::vector<TempId> touched;
	bool ok = true;
	for (InstId i = body_.blocks[block].first; i != kNoId && ok; i = body_.instructions[i].next)
	{
		const Instruction& inst = body_.instructions[i];
		if (inst.op == Opcode::Phi)
		{
			// An input not literal leaves the phi undecided: only a use fails.
			const Value in = *inputs++;
			if (in.is_literal())
			{
				scratch_[inst.result] = in;
				touched.push_back(inst.result);
			}
			continue;
		}
		if (is_terminator(inst.op))
		{
			Constant c;
			if (inst.op != Opcode::Branch || !literal_constant(body_, value(inst.ops[0]), c) ||
				c.kind != ValueKind::Integer)
			{
				ok = false;
				break;
			}
			target = (c.bits.low | c.bits.high) != 0 ? inst.ops[1].index : inst.ops[2].index;
			break;
		}
		Constant a, b, r;
		const Type type = inst.op == Opcode::Convert ? convert_source_type(inst) : inst.type;
		const bool la = literal_constant(body_, value(inst.ops[0]), a);
		const bool lb = literal_constant(body_, value(inst.ops[1]), b);
		switch (inst.op)
		{
		case Opcode::Compare:
			ok = la && lb && fold_compare(static_cast<ComparePredicate>(inst.sub), inst.type, a, b, r);
			break;
		case Opcode::Binary:
			ok = la && lb && fold_binary(static_cast<BinaryOp>(inst.sub), inst.type, a, b, r);
			break;
		case Opcode::Unary:
			ok = la && fold_unary(static_cast<UnaryOp>(inst.sub), inst.type, a, r);
			break;
		case Opcode::Convert:
			ok = la && fold_convert(static_cast<ConvertOp>(inst.sub), inst.type, type, a, r);
			break;
		case Opcode::Copy:
			ok = la && fold_copy(inst.type, a, r);
			break;
		default:
			ok = false;
			break;
		}
		if (ok)
		{
			scratch_[inst.result] = literal_value(body_, r.kind, r.float_kind, r.bits);
			touched.push_back(inst.result);
		}
	}
	for (TempId t : touched)
		scratch_[t] = Value::none();
	return ok;
}

// Per temp: the nearest common dominator of its uses outside its
// defining block (a phi's at its predecessor's end), kNoId when none.
void JumpThreading::find_use_dominators()
{
	use_dom_.assign(body_.temps.size(), kNoId);
	auto note = [&](const Value& v, BlockId at) {
		if (!v.is(ValueKind::Temp) || v.index >= use_dom_.size())
			return;
		BlockId a = use_dom_[v.index], b = at;
		if (a == kNoId)
		{
			use_dom_[v.index] = b;
			return;
		}
		while (a != b)
		{
			while (cfg_.rpo_index[a] > cfg_.rpo_index[b])
				a = cfg_.idom[a];
			while (cfg_.rpo_index[b] > cfg_.rpo_index[a])
				b = cfg_.idom[b];
		}
		use_dom_[v.index] = a;
	};
	for (BlockId b : body_.layout)
	{
		if (!cfg_.reachable(b))
			continue;
		for (InstId i = body_.blocks[b].first; i != kNoId; i = body_.instructions[i].next)
		{
			const Instruction& inst = body_.instructions[i];
			if (inst.op == Opcode::Phi)
			{
				for (uint32_t k = 0; k + 1 < inst.extra_count; k += 2)
					if (cfg_.reachable(body_.operands[inst.extra_first + k].index))
						note(body_.operands[inst.extra_first + k + 1], body_.operands[inst.extra_first + k].index);
				continue;
			}
			for_each_operand(body_, inst, [&](const Value& v) {
				if (v.is(ValueKind::Temp) &&
					(body_.temps[v.index].def == kNoId || c_.block_of[body_.temps[v.index].def] != b))
					note(v, b);
			});
		}
	}
}

// The successor of m entered from m alone whose dominance region holds
// every use of m's value t past m; kNoId when none.
BlockId JumpThreading::successor_holding(BlockId m, TempId t) const
{
	BlockId x = t < use_dom_.size() ? use_dom_[t] : kNoId;
	while (x != kNoId && x != m && cfg_.idom[x] != m)
		x = cfg_.idom[x];
	return x == kNoId || x == m || cfg_.pred_count(x) != 1 ? kNoId : x;
}

// A block that only computes its branch condition from phis: a
// predecessor whose inputs decide the branch jumps to the decided
// successor directly. A value of the block used past it must have every
// such use under one successor entered from the block alone, which no
// predecessor then enters directly (the block still dominates those
// uses); a predecessor takes over the block's branch only when its values
// are used in it alone.
bool JumpThreading::constant_phis(bool counted)
{
	std::vector<uint32_t> own;
	if (!counted)
		count_uses(body_, own);
	const std::vector<uint32_t>& uses = counted ? c_.uses : own;
	c_.entry_block_of(thread_jumps, !touched_);
	scratch_.assign(body_.temps.size(), Value::none());
	use_dom_.clear();
	local_.assign(body_.temps.size(), 0);
	local_stamp_.assign(body_.temps.size(), kNoId);
	slot_of_.assign(cfg_.block_count, kNoId);
	bool changed = false;
	std::vector<uint8_t> touched(cfg_.block_count, 0);
	for (BlockId m : body_.layout)
	{
		if (!cfg_.reachable(m) || cfg_.pred_count(m) < 1 || (cfg_.flags[m] & BF_HANDLER) != 0 || touched[m] ||
			m == body_.layout[0])
			continue;
		const InstId term = body_.blocks[m].last;
		if (term == kNoId || body_.instructions[term].op != Opcode::Branch)
			continue;
		uint32_t work = 0;
		bool has_phi = false, copyable = true;
		for (InstId i = body_.blocks[m].first; i != term && work <= kThreadWork; i = body_.instructions[i].next)
		{
			const Instruction& inst = body_.instructions[i];
			has_phi = has_phi || inst.op == Opcode::Phi;
			work += inst.op == Opcode::Phi ? 0 : 1;
			copyable = copyable && (is_pure(inst) || (inst.op == Opcode::Load && (inst.flags & IF_VOLATILE) == 0));
		}
		if (!has_phi || work > kThreadWork)
			continue;
		copyable = copyable && work <= kCopyWork;
		// Every value of the block is used only after it inside the block
		// (the phis by its own computations, those by the branch); a use by
		// one of its phis, from a back edge, is a use outside the threaded
		// path. In-block use counts, less those at or before each value.
		bool private_values = true;
		BlockId kept[2] = { kNoId, kNoId }; // successors holding uses past the block
		for (InstId i = body_.blocks[m].first; i != kNoId; i = body_.instructions[i].next)
			for_each_operand(body_, body_.instructions[i], [&](const Value& v) {
				if (v.is(ValueKind::Temp))
				{
					if (local_stamp_[v.index] != m)
					{
						local_stamp_[v.index] = m;
						local_[v.index] = 0;
					}
					++local_[v.index];
				}
			});
		for (InstId i = body_.blocks[m].first; i != term && private_values; i = body_.instructions[i].next)
		{
			const Instruction& inst = body_.instructions[i];
			for_each_operand(body_, inst, [&](const Value& v) {
				if (v.is(ValueKind::Temp))
					--local_[v.index];
			});
			if (inst.result == kNoId)
			{
				private_values = false;
				break;
			}
			if (local_stamp_[inst.result] == m && local_[inst.result] == uses[inst.result])
				continue;
			if (use_dom_.empty())
				find_use_dominators();
			const BlockId holder = successor_holding(m, inst.result);
			if (holder == kNoId || touched[holder] || (kept[0] != kNoId && kept[0] != holder && kept[1] != kNoId))
				private_values = false;
			else if (kept[0] == kNoId || kept[0] == holder)
				kept[0] = holder;
			else
				kept[1] = holder;
		}
		if (!private_values)
			continue;
		copyable = copyable && kept[0] == kNoId;
		// The phis' inputs per predecessor, in one pass over their lists.
		const uint32_t first_pred = cfg_.pred_first[m], count = cfg_.pred_count(m);
		uint32_t phis = 0;
		for (InstId i = body_.blocks[m].first; i != term && body_.instructions[i].op == Opcode::Phi;
			i = body_.instructions[i].next)
			++phis;
		for (uint32_t k = 0; k < count; ++k)
			slot_of_[cfg_.preds[first_pred + k]] = k;
		inputs_.assign(static_cast<size_t>(count) * phis, Value::none());
		uint32_t j = 0;
		for (InstId i = body_.blocks[m].first; i != term && body_.instructions[i].op == Opcode::Phi;
			i = body_.instructions[i].next, ++j)
		{
			const Instruction& phi = body_.instructions[i];
			for (uint32_t k = phi.extra_count; k >= 2; k -= 2) // the first pair of a block wins
			{
				const BlockId pred = body_.operands[phi.extra_first + k - 2].index;
				const uint32_t slot = slot_of_[pred];
				if (slot < count && cfg_.preds[first_pred + slot] == pred)
					inputs_[static_cast<size_t>(slot) * phis + j] = body_.operands[phi.extra_first + k - 1];
			}
		}
		// Every predecessor is decided first; the redirections then apply
		// per target in one pass over its phis, and this block's phis lose
		// the moved predecessors in one filter (each redirection alone
		// rewrote both lists: quadratic on a wide merge).
		const std::vector<BlockId> preds(cfg_.preds.begin() + first_pred, cfg_.preds.begin() + first_pred + count);
		// A predecessor jumping to m, or branching to it on one side, moves
		// that edge. One jumping to m with its inputs undecided takes over
		// m's computations and branch (copy_branch).
		std::vector<std::pair<uint32_t, BlockId>> moves; // (predecessor slot, target)
		std::vector<uint32_t> copies;
		for (uint32_t k = 0; k < count; ++k)
		{
			const BlockId p = preds[k];
			const InstId pt = body_.blocks[p].last;
			if (touched[p] || pt == kNoId || p == m)
				continue;
			const Instruction& pred_term = body_.instructions[pt];
			const bool jumps = pred_term.op == Opcode::Jump;
			if (!jumps && (pred_term.op != Opcode::Branch || pred_term.ops[1].index == pred_term.ops[2].index))
				continue;
			BlockId target = kNoId;
			if (evaluate(m, inputs_.data() + static_cast<size_t>(k) * phis, target))
			{
				if (target != m && target != kept[0] && target != kept[1] && (cfg_.flags[target] & BF_HANDLER) == 0)
					moves.push_back(std::make_pair(k, target));
			}
			else if (jumps && copyable)
				copies.push_back(k);
		}
		if (!moves.empty() && apply_moves(m, preds, phis, moves, touched))
		{
			changed = true;
			for (const BlockId holder : kept)
				if (holder != kNoId)
					touched[holder] = 1;
		}
		uint32_t budget = kCloneBudget;
		for (uint32_t k : copies)
		{
			if (touched[preds[k]] || budget < work)
				continue;
			if (copy_branch(m, preds[k], inputs_.data() + static_cast<size_t>(k) * phis, touched))
			{
				budget -= work;
				changed = true;
			}
		}
	}
	return changed;
}

// Predecessor `p`, jumping to m with m's phis taking `inputs`, ends in
// copies of m's computations and branch instead (m's values are used only
// in m: none is live past it). The branch's targets take p as a
// predecessor with the values m passes them.
bool JumpThreading::copy_branch(BlockId m, BlockId p, const Value* inputs, std::vector<uint8_t>& touched)
{
	const InstId term = body_.blocks[m].last;
	const BlockId on_true = body_.instructions[term].ops[1].index, on_false = body_.instructions[term].ops[2].index;
	if (on_true == on_false || on_true == m || on_false == m || on_true == p || on_false == p ||
		((cfg_.flags[on_true] | cfg_.flags[on_false]) & BF_HANDLER) != 0)
		return false;
	auto value = [&](Value v) {
		return v.is(ValueKind::Temp) && v.index < scratch_.size() && !scratch_[v.index].is(ValueKind::None)
			? scratch_[v.index]
			: v;
	};
	const Value* in = inputs;
	for (InstId i = body_.blocks[m].first; i != term && body_.instructions[i].op == Opcode::Phi;
		i = body_.instructions[i].next)
		if ((in++)->is(ValueKind::None))
			return false;
	if (scratch_.size() < body_.temps.size())
		scratch_.resize(body_.temps.size(), Value::none());
	std::vector<TempId> mapped;
	const InstId pt = body_.blocks[p].last;
	InstId at = previous_in_block(body_, p, pt);
	in = inputs;
	for (InstId i = body_.blocks[m].first; i != term; i = body_.instructions[i].next)
	{
		// (A copy: appending the copied instruction may move the instructions.)
		const Instruction inst = body_.instructions[i];
		if (inst.op == Opcode::Phi)
		{
			scratch_[inst.result] = *in++;
			mapped.push_back(inst.result);
			continue;
		}
		Instruction copy = inst;
		for (Value& v : copy.ops)
			v = value(v);
		const InstId id = append_copy(body_, body_, i, copy);
		const TempId result = append_temp(body_, body_.temps[inst.result].type, id);
		body_.instructions[id].result = result;
		insert_after(body_, p, at, id);
		// The block map constant_phis took names the copy's block (its later
		// blocks' use dominators read it).
		if (c_.block_of.size() <= id)
			c_.block_of.resize(id + 1, kNoId);
		c_.block_of[id] = p;
		at = id;
		scratch_[inst.result] = Value::temp(result);
		mapped.push_back(inst.result);
	}
	Instruction& branch = body_.instructions[pt];
	branch.op = Opcode::Branch;
	branch.ops[0] = value(body_.instructions[term].ops[0]);
	branch.ops[1] = Value::block(on_true);
	branch.ops[2] = Value::block(on_false);
	for (TempId t : mapped)
		scratch_[t] = Value::none();
	for (const BlockId to : { on_true, on_false })
		for (InstId i = body_.blocks[to].first; i != kNoId && body_.instructions[i].op == Opcode::Phi;
			i = body_.instructions[i].next)
		{
			Instruction& phi = body_.instructions[i];
			std::vector<Value> pairs(
				body_.operands.begin() + phi.extra_first, body_.operands.begin() + phi.extra_first + phi.extra_count);
			pairs.push_back(Value::block(p));
			pairs.push_back(phi_incoming(body_, phi, m));
			phi.extra_first = append_operands(body_, pairs.data(), static_cast<uint32_t>(pairs.size()));
			phi.extra_count = static_cast<uint32_t>(pairs.size());
		}
	remove_phi_incoming(body_, m, p);
	touched[p] = touched[on_true] = touched[on_false] = 1;
	++c_.stats.branches_folded;
	return true;
}

// Moves predecessors of `m` (moves: slot, target) to jump to their targets:
// each target's phis take m's input, read through m's phi for that
// predecessor (inputs_, `phis` per slot), and m's phis drop them. A
// predecessor that already reaches its target (with phis there) or is
// itself a target stays. Returns whether any moved.
bool JumpThreading::apply_moves(BlockId m, const std::vector<BlockId>& preds, uint32_t phis,
	std::vector<std::pair<uint32_t, BlockId>>& moves, std::vector<uint8_t>& touched)
{
	if (mark_.size() < cfg_.block_count)
		mark_.resize(cfg_.block_count, 0);
	// Targets, and each target's predecessors (one target at a time).
	std::vector<BlockId> targets;
	for (const std::pair<uint32_t, BlockId>& mv : moves)
		if (std::find(targets.begin(), targets.end(), mv.second) == targets.end())
			targets.push_back(mv.second);
	std::vector<uint8_t> keep(moves.size(), 1);
	for (size_t t = 0; t < targets.size(); ++t)
	{
		const BlockId to = targets[t];
		const bool has_phi = body_.blocks[to].first != kNoId &&
			body_.instructions[body_.blocks[to].first].op == Opcode::Phi;
		const uint32_t epoch = ++epoch_;
		for (uint32_t k = cfg_.pred_first[to]; k < cfg_.pred_first[to + 1]; ++k)
			mark_[cfg_.preds[k]] = epoch;
		for (size_t i = 0; i < moves.size(); ++i)
		{
			const BlockId p = preds[moves[i].first];
			if (moves[i].second == to && has_phi && mark_[p] == epoch)
				keep[i] = 0;
			if (std::find(targets.begin(), targets.end(), p) != targets.end())
				keep[i] = 0;
		}
	}
	size_t kept = 0;
	for (size_t i = 0; i < moves.size(); ++i)
		if (keep[i])
			moves[kept++] = moves[i];
	moves.resize(kept);
	if (moves.empty())
		return false;
	// m's phis by instruction, for reading an input through them.
	if (phi_pos_.size() < body_.instructions.size())
		phi_pos_.resize(body_.instructions.size(), kNoId);
	uint32_t j = 0;
	for (InstId i = body_.blocks[m].first; i != kNoId && body_.instructions[i].op == Opcode::Phi;
		i = body_.instructions[i].next)
		phi_pos_[i] = j++;
	for (const BlockId to : targets)
	{
		for (InstId i = body_.blocks[to].first; i != kNoId && body_.instructions[i].op == Opcode::Phi;
			i = body_.instructions[i].next)
		{
			Instruction& phi = body_.instructions[i];
			const Value in = phi_incoming(body_, phi, m);
			// m's own phi reads as its input for the moved predecessor; any
			// other value (defined in a block dominating m, so its
			// predecessors too) is the same on the new edge.
			const InstId def = in.is(ValueKind::Temp) && in.index >= body_.parameter_count ? body_.temps[in.index].def
																						   : kNoId;
			const bool through = def != kNoId && body_.instructions[def].op == Opcode::Phi && c_.block_of[def] == m;
			std::vector<Value> pairs(
				body_.operands.begin() + phi.extra_first, body_.operands.begin() + phi.extra_first + phi.extra_count);
			for (const std::pair<uint32_t, BlockId>& mv : moves)
				if (mv.second == to)
				{
					pairs.push_back(Value::block(preds[mv.first]));
					pairs.push_back(through ? inputs_[static_cast<size_t>(mv.first) * phis + phi_pos_[def]] : in);
				}
			phi.extra_first = append_operands(body_, pairs.data(), static_cast<uint32_t>(pairs.size()));
			phi.extra_count = static_cast<uint32_t>(pairs.size());
		}
	}
	const uint32_t epoch = ++epoch_;
	for (const std::pair<uint32_t, BlockId>& mv : moves)
	{
		const BlockId p = preds[mv.first];
		Instruction& pred_term = body_.instructions[body_.blocks[p].last];
		if (pred_term.op == Opcode::Jump)
			pred_term.ops[0] = Value::block(mv.second);
		else
			(pred_term.ops[1].index == m ? pred_term.ops[1] : pred_term.ops[2]) = Value::block(mv.second);
		mark_[p] = epoch;
		touched[p] = touched[mv.second] = 1;
		++c_.stats.branches_folded;
	}
	for (InstId i = body_.blocks[m].first; i != kNoId && body_.instructions[i].op == Opcode::Phi;
		i = body_.instructions[i].next)
	{
		Instruction& phi = body_.instructions[i];
		Value* pairs = &body_.operands[phi.extra_first];
		uint32_t out = 0;
		for (uint32_t k = 0; k < phi.extra_count; k += 2)
			if (mark_[pairs[k].index] != epoch)
			{
				pairs[out] = pairs[k];
				pairs[out + 1] = pairs[k + 1];
				out += 2;
			}
		phi.extra_count = out;
	}
	return true;
}

// A predecessor's path decides a test block's compare: a block holding
// only phis, `cmp pred x, k` and the branch on it (or only a branch on x:
// `ne x, 0`), entered from a predecessor whose path back (through unique
// predecessors, at most kFactDepth blocks) took a branch on a compare of x
// (or of a value x extends, or one extending x) with a constant (or on x),
// whose range on that edge decides the test, or one of whose dominators
// (at most kDominatorFactDepth up) is entered only along such a branch,
// jumps from that predecessor straight to the decided side (its phis take
// the test block's input). x may be a phi of the test block: each
// predecessor's input is tested. A merge block jumping to the test block
// whose own phi is that input is copied for each of its predecessors whose
// path decides the test (copy_merge), the copy redirected: a lexer's
// byte fast path then skips the loop header's character tests. The test
// block's phis move to slots first (demote_phis). A chain of keyword tests
// on one length then skips the lengths already ruled out or matched, and
// a second diamond on the test of the first (`dirty_ ? a : b` twice)
// merges into it.
const uint32_t kFactDepth = 4;
// Dominators of a predecessor searched for an entering branch.
const uint32_t kDominatorFactDepth = 8;
// Test blocks whose phis one run demotes.
const uint32_t kDemotedBlocks = 32;
// Plain instructions a threaded predecessor's copy of a test block takes.
const uint32_t kThreadCopy = 6;

bool JumpThreading::known_compares()
{
	c_.entry_uses(thread_jumps, !touched_);
	bool changed = false;
	const std::vector<BlockId> layout = body_.layout;
	for (BlockId s : layout)
	{
		if (!cfg_.reachable(s) || (cfg_.flags[s] & BF_HANDLER) != 0 || s == cfg_.rpo[0])
			continue;
		// The block: its compare and the branch, or the branch alone on a
		// compare computed elsewhere, after any phis.
		InstId first = body_.blocks[s].first;
		const InstId last = body_.blocks[s].last;
		if (first == kNoId || last == kNoId)
			continue;
		uint32_t phis = 0;
		while (first != last && body_.instructions[first].op == Opcode::Phi)
		{
			first = body_.instructions[first].next;
			++phis;
		}
		const Instruction& term = body_.instructions[last];
		// A switch alone after the phis (O2+) goes to the target whose
		// values hold all a predecessor's path admits.
		const bool switching = term.op == Opcode::Switch && first == last && c_.level >= 2 &&
			term.ops[0].is(ValueKind::Temp) && value_bits(body_.temps[term.ops[0].index].type) != 0;
		std::vector<std::pair<BlockId, ValueSet>> arms;
		InstId tail = last;
		uint32_t copied = 0;
		TempId cresult = kNoId;
		Value x = Value::none();
		unsigned qbits = 0;
		ValueSet query;
		if (switching)
		{
			x = term.ops[0];
			qbits = value_bits(body_.temps[x.index].type);
			std::vector<BlockId> targets;
			for_each_successor(body_, body_.instructions[last], [&](BlockId& b) { targets.push_back(b); });
			std::sort(targets.begin(), targets.end());
			targets.erase(std::unique(targets.begin(), targets.end()), targets.end());
			for (BlockId target : targets)
				arms.push_back(std::make_pair(target, switched_to(body_, term, target, qbits)));
		}
		else
		{
			if (term.op != Opcode::Branch || term.ops[1].index == term.ops[2].index ||
				!term.ops[0].is(ValueKind::Temp) || body_.temps[term.ops[0].index].def == kNoId)
				continue;
			const InstId def = body_.temps[term.ops[0].index].def;
			const Instruction& cmp = body_.instructions[def];
			// The compare (paired) or the branch (alone) ends the block; what
			// comes between the phis and it is copied for a threaded
			// predecessor (at most kThreadCopy plain instructions).
			bool def_here = false;
			for (InstId i = first; i != kNoId && i != last && !def_here; i = body_.instructions[i].next)
				def_here = i == def;
			if (def_here)
			{
				if (body_.instructions[def].next != last)
					continue;
				tail = def;
			}
			bool copyable = true;
			for (InstId i = first; i != tail && i != kNoId && copyable; i = body_.instructions[i].next)
			{
				const Instruction& inst = body_.instructions[i];
				copyable = ++copied <= kThreadCopy && inst.op != Opcode::Call && inst.op != Opcode::Phi &&
					!is_eh_instruction(inst.op) && inst.op != Opcode::StackAlloc && inst.op != Opcode::VaStart &&
					inst.op != Opcode::VaArg && inst.op != Opcode::AtomicLoad && inst.op != Opcode::AtomicStore &&
					inst.op != Opcode::AtomicAddFetch && inst.op != Opcode::AtomicExchange &&
					inst.op != Opcode::AtomicCompareExchange && (inst.flags & IF_VOLATILE) == 0 && i != def;
			}
			if (!copyable || (copied != 0 && c_.level < 2))
				continue;
			if (copied != 0)
				first = tail;
			const bool alone = first == last && def != last;
			const bool paired = first == def && body_.instructions[first].next == last && c_.uses[cmp.result] == 1;
			// A branch alone on a value other than a compare tests `ne x, 0`.
			const bool bare = alone && cmp.op != Opcode::Compare && body_.temps[term.ops[0].index].type.is_integer();
			if (!bare && (cmp.op != Opcode::Compare || !(alone || paired)))
				continue;
			ComparePredicate pred = bare ? ComparePredicate::Ne : static_cast<ComparePredicate>(cmp.sub);
			const Type ctype = bare ? body_.temps[term.ops[0].index].type : cmp.type;
			cresult = bare ? kNoId : cmp.result;
			x = bare ? term.ops[0] : cmp.ops[0];
			Value k = bare ? Value::none() : cmp.ops[1];
			if (!x.is(ValueKind::Temp))
			{
				std::swap(x, k);
				pred = swapped(pred);
			}
			Constant kc = { ValueKind::Integer, TypeKind::Void, Literal{ 0, 0 } };
			qbits = value_bits(ctype);
			if (qbits == 0 || !x.is(ValueKind::Temp) ||
				(!bare && (!literal_constant(body_, k, kc) || kc.kind != ValueKind::Integer)))
				continue;
			query = admitted(pred, kc.bits.low, qbits);
		}
		const BlockId on_true = switching ? kNoId : term.ops[1].index, on_false = switching ? kNoId : term.ops[2].index;
		// The target values g decide (kNoId: undecided).
		auto decide = [&](const ValueSet& g) -> BlockId {
			if (!switching)
			{
				const int r = decided_by(g, query);
				return r < 0 ? kNoId : r ? on_true : on_false;
			}
			for (const std::pair<BlockId, ValueSet>& arm : arms)
				if (decided_by(g, arm.second) == 1)
					return arm.first;
			return kNoId;
		};
		// x as each predecessor brings it: a phi of the block takes its
		// input on that edge; a value the block itself (re)defines
		// otherwise is no value a predecessor's path decided (a loop
		// header's load).
		InstId x_phi = kNoId;
		bool redefined = false;
		for (InstId i = body_.blocks[s].first; i != kNoId && !redefined; i = body_.instructions[i].next)
			if (body_.instructions[i].result == x.index)
			{
				if (body_.instructions[i].op == Opcode::Phi)
					x_phi = i;
				else
					redefined = true;
			}
		if (redefined)
			continue;
		// The target `cur`'s branch or switch decides for value v on its edge
		// to `to` (kNoId: unknown).
		auto decided_on_edge = [&](BlockId cur, BlockId to, Value v) -> BlockId {
			Value cx;
			ValueSet f;
			unsigned bits = 0;
			if (!edge_fact(cur, to, cx, f, bits))
				return kNoId;
			return decide(related(cx, f, bits, v, qbits));
		};
		// The outcome for x holding v on the edge from p into `into`: the
		// path back through unique predecessors, or a dominator of p
		// entered from a branch alone (v holds what that branch tested).
		auto path_outcome = [&](BlockId p, BlockId into, Value v) -> BlockId {
			if (!v.is(ValueKind::Temp))
				return kNoId;
			BlockId outcome = kNoId;
			BlockId cur = p, to = into;
			for (uint32_t depth = 0; depth < kFactDepth && outcome == kNoId; ++depth)
			{
				outcome = decided_on_edge(cur, to, v);
				if (cfg_.pred_count(cur) != 1)
					break;
				to = cur;
				cur = cfg_.preds[cfg_.pred_first[cur]];
			}
			BlockId d = p;
			for (uint32_t depth = 0; depth < kDominatorFactDepth && outcome == kNoId && d != kNoId;
				++depth, d = cfg_.idom[d])
				if (cfg_.pred_count(d) == 1)
					outcome = decided_on_edge(cfg_.preds[cfg_.pred_first[d]], d, v);
			return outcome;
		};
		// The outcome on each predecessor's edge; a merge block jumping
		// here whose own phi brings x is copied for each of its
		// predecessors whose path decides the test (copy_merge), the copy
		// redirected in turn.
		std::vector<std::pair<BlockId, BlockId>> redirects; // (predecessor, target)
		std::vector<std::pair<BlockId, BlockId>> merges;    // (merge block, its predecessor)
		std::vector<BlockId> merge_targets;
		for (uint32_t pi = cfg_.pred_first[s]; pi < cfg_.pred_first[s + 1]; ++pi)
		{
			const BlockId p = cfg_.preds[pi];
			if (p == s)
				continue;
			const Value v = x_phi == kNoId ? x : phi_incoming(body_, body_.instructions[x_phi], p);
			const BlockId outcome = path_outcome(p, s, v);
			if (outcome != kNoId)
			{
				redirects.push_back(std::make_pair(p, outcome));
				continue;
			}
			if (x_phi == kNoId || !mergeable(p, s, v))
				continue;
			const Instruction& vphi = body_.instructions[body_.temps[v.index].def];
			for (uint32_t qi = cfg_.pred_first[p]; qi < cfg_.pred_first[p + 1]; ++qi)
			{
				const BlockId q = cfg_.preds[qi];
				if (q == p || q == s || (cfg_.flags[q] & BF_HANDLER) != 0)
					continue;
				const BlockId o = path_outcome(q, p, phi_incoming(body_, vphi, q));
				if (o != kNoId)
				{
					merges.push_back(std::make_pair(p, q));
					merge_targets.push_back(o);
				}
			}
		}
		if (!merges.empty())
		{
			for (size_t m = 0; m < merges.size(); ++m)
			{
				const BlockId copy = copy_merge(merges[m].first, merges[m].second, s);
				if (copy != kNoId)
					redirects.push_back(std::make_pair(copy, merge_targets[m]));
			}
			c_.invalidate_cfg();
			c_.ensure_cfg();
			changed = true;
		}
		if (redirects.empty())
			continue;
		if (copied != 0)
		{
			if (copy_thread(s, tail, redirects))
				changed = true;
			continue;
		}
		// The phis go to slots their predecessors store (promoted again
		// once the pass is done), so a bypassing predecessor leaves its
		// value where every use finds it.
		if (phis != 0 && !demote_phis(s, redirects))
			continue;
		for (const std::pair<BlockId, BlockId>& r : redirects)
		{
			const BlockId p = r.first, target = r.second;
			// The target's phis: the predecessor brings what the test block
			// brought (one input per predecessor: a disagreeing existing one
			// keeps the edge).
			bool ok = true;
			for (InstId i = body_.blocks[target].first; i != kNoId && body_.instructions[i].op == Opcode::Phi && ok;
				i = body_.instructions[i].next)
			{
				const Value from_s = phi_incoming(body_, body_.instructions[i], s);
				const Value from_p = phi_incoming(body_, body_.instructions[i], p);
				ok = !from_s.is(ValueKind::None) && !(cresult != kNoId && from_s == Value::temp(cresult)) &&
					(from_p.is(ValueKind::None) || from_p == from_s);
			}
			if (!ok)
				continue;
			bool had_edge = false;
			Instruction& pt = body_.instructions[body_.blocks[p].last];
			for_each_successor(body_, pt, [&](BlockId& b) { had_edge = had_edge || b == target; });
			for (InstId i = body_.blocks[target].first;
				i != kNoId && body_.instructions[i].op == Opcode::Phi && !had_edge; i = body_.instructions[i].next)
			{
				Instruction& phi = body_.instructions[i];
				std::vector<Value> pairs(body_.operands.begin() + phi.extra_first,
					body_.operands.begin() + phi.extra_first + phi.extra_count);
				pairs.push_back(Value::block(p));
				pairs.push_back(phi_incoming(body_, phi, s));
				phi.extra_first = append_operands(body_, pairs.data(), static_cast<uint32_t>(pairs.size()));
				phi.extra_count = static_cast<uint32_t>(pairs.size());
			}
			for_each_successor(body_, body_.instructions[body_.blocks[p].last], [&](BlockId& b) {
				if (b == s)
					b = target;
			});
			++c_.stats.branches_folded;
			changed = true;
		}
		if (!redirects.empty() && changed)
		{
			c_.invalidate_cfg();
			c_.ensure_cfg();
		}
	}
	return changed;
}

// The values a branch ending `cur` admits on its edge to `to`: those of
// `cx` (of `bits` width) in `f`, from a compare of cx with a constant (or
// the branch value itself, nonzero when taken). False: none.
bool JumpThreading::edge_fact(BlockId cur, BlockId to, Value& cx, ValueSet& f, unsigned& bits) const
{
	const InstId t = body_.blocks[cur].last;
	if (t == kNoId)
		return false;
	const Instruction& br = body_.instructions[t];
	if (br.op == Opcode::Switch && br.ops[0].is(ValueKind::Temp))
	{
		bits = value_bits(body_.temps[br.ops[0].index].type);
		bool reaches = br.ops[1].index == to;
		for (uint32_t k = 1; k < br.extra_count; k += 2)
			reaches = reaches || body_.operands[br.extra_first + k].index == to;
		if (bits == 0 || !reaches)
			return false;
		cx = br.ops[0];
		f = switched_to(body_, br, to, bits);
		return true;
	}
	if (br.op != Opcode::Branch || br.ops[1].index == br.ops[2].index || !br.ops[0].is(ValueKind::Temp) ||
		(br.ops[1].index != to && br.ops[2].index != to))
		return false;
	const bool taken = br.ops[1].index == to;
	const Value c = br.ops[0];
	const InstId d = c.index < body_.parameter_count ? kNoId : body_.temps[c.index].def;
	if (d != kNoId && body_.instructions[d].op == Opcode::Compare && !body_.instructions[d].type.is_float())
	{
		const Instruction& cmp = body_.instructions[d];
		Value a = cmp.ops[0], k = cmp.ops[1];
		ComparePredicate p = static_cast<ComparePredicate>(cmp.sub);
		if (!a.is(ValueKind::Temp))
		{
			std::swap(a, k);
			p = swapped(p);
		}
		Constant kc;
		bits = value_bits(cmp.type);
		if (bits != 0 && a.is(ValueKind::Temp) && literal_constant(body_, k, kc) && kc.kind == ValueKind::Integer)
		{
			cx = a;
			f = admitted(taken ? p : inverted(p), kc.bits.low, bits);
			return true;
		}
		return false;
	}
	bits = value_bits(body_.temps[c.index].type);
	if (bits == 0)
		return false;
	cx = c;
	f = admitted(taken ? ComparePredicate::Ne : ComparePredicate::Eq, 0, bits);
	return true;
}

// v without the integer extensions defining it (the outermost first in
// `chain`).
Value JumpThreading::stripped(Value v, std::vector<Extension>& chain) const
{
	chain.clear();
	while (v.is(ValueKind::Temp) && v.index >= body_.parameter_count && chain.size() < 4)
	{
		const InstId d = body_.temps[v.index].def;
		if (d == kNoId || body_.instructions[d].op != Opcode::Convert)
			break;
		const Instruction& inst = body_.instructions[d];
		const ConvertOp op = static_cast<ConvertOp>(inst.sub);
		const unsigned from = value_bits(convert_source_type(inst)), to = value_bits(inst.type);
		if ((op != ConvertOp::ZExt && op != ConvertOp::SExt) || from == 0 || to == 0 || from >= to ||
			!convert_source_type(inst).is_integer() || !inst.type.is_integer())
			break;
		chain.push_back(Extension{ from, to, op == ConvertOp::SExt });
		v = inst.ops[0];
	}
	return v;
}

// The values v (of `vbits` width) may hold when cx (of `bits` width)
// holds one of `f`: through the extension chains down to their common
// root and back up (lost when unrelated).
ValueSet JumpThreading::related(Value cx, ValueSet f, unsigned bits, Value v, unsigned vbits) const
{
	ValueSet unknown;
	unknown.lost = true;
	if (cx == v)
		return bits == vbits ? f : unknown;
	std::vector<Extension> down, up;
	const Value a = stripped(cx, down), b = stripped(v, up);
	if (!(a == b))
		return unknown;
	unsigned w = bits;
	for (const Extension& e : down)
	{
		if (e.to != w)
			return unknown;
		f = unextended(f, e.from, e.to, e.sign);
		w = e.from;
	}
	for (size_t i = up.size(); i-- > 0;)
	{
		if (up[i].from != w)
			return unknown;
		f = extended(f, up[i].from, up[i].to, up[i].sign);
		w = up[i].to;
	}
	return w == vbits ? f : unknown;
}

// Plain instructions a merge block copied for one predecessor holds.
const uint32_t kMergeCopy = 8;

// Whether merge block p (jumping only to s, entered from several blocks)
// may be copied for one of its predecessors: v is one of its phis, its
// other instructions are few and plain, and nothing outside it reads its
// values but s's phis.
bool JumpThreading::mergeable(BlockId p, BlockId s, Value v) const
{
	if (!v.is(ValueKind::Temp) || v.index < body_.parameter_count || body_.temps[v.index].def == kNoId || p == s ||
		(cfg_.flags[p] & BF_HANDLER) != 0 || cfg_.pred_count(p) < 2 || cfg_.pred_count(s) < 2)
		return false;
	const InstId vd = body_.temps[v.index].def;
	if (body_.instructions[vd].op != Opcode::Phi || c_.block_of[vd] != p)
		return false;
	const InstId last = body_.blocks[p].last;
	if (last == kNoId || body_.instructions[last].op != Opcode::Jump || body_.instructions[last].ops[0].index != s)
		return false;
	uint32_t plain = 0;
	for (InstId i = body_.blocks[p].first; i != last; i = body_.instructions[i].next)
	{
		const Instruction& inst = body_.instructions[i];
		if (inst.op == Opcode::Phi)
			continue;
		if (++plain > kMergeCopy || inst.op == Opcode::Call || is_eh_instruction(inst.op) ||
			inst.op == Opcode::StackAlloc || inst.op == Opcode::VaStart || inst.op == Opcode::VaArg ||
			inst.op == Opcode::AtomicLoad || inst.op == Opcode::AtomicStore || inst.op == Opcode::AtomicAddFetch ||
			inst.op == Opcode::AtomicExchange || inst.op == Opcode::AtomicCompareExchange ||
			(inst.flags & IF_VOLATILE) != 0)
			return false;
	}
	// p's values are read in p, or by s's phis on the edge from p.
	for (BlockId b : body_.layout)
	{
		if (b == p)
			continue;
		for (InstId i = body_.blocks[b].first; i != kNoId; i = body_.instructions[i].next)
		{
			const Instruction& inst = body_.instructions[i];
			bool bad = false;
			uint32_t position = 0;
			for_each_operand(body_, inst, [&](const Value& u) {
				const uint32_t at = position++;
				if (!u.is(ValueKind::Temp) || u.index < body_.parameter_count || body_.temps[u.index].def == kNoId ||
					c_.block_of[body_.temps[u.index].def] != p)
					return;
				const bool from_p = inst.op == Opcode::Phi && b == s && at >= 3 &&
					body_.operands[inst.extra_first + (at - 3) - 1].index == p;
				bad = bad || !from_p;
			});
			if (bad)
				return false;
		}
	}
	return true;
}

// A copy of merge block p for its predecessor q, jumping to s: p's phis
// read as q's inputs; q enters the copy instead, and s's phis take what p
// brought them as the copy computes it. The new block, or kNoId.
BlockId JumpThreading::copy_merge(BlockId p, BlockId q, BlockId s)
{
	const BlockId copy = append_block(body_, BlockKind::Thread);
	std::vector<BlockId> layout;
	for (BlockId b : body_.layout)
	{
		layout.push_back(b);
		if (b == q)
			layout.push_back(copy);
	}
	body_.layout.swap(layout);
	std::vector<std::pair<TempId, Value>> renamed;
	auto mapped = [&](Value v) {
		for (const std::pair<TempId, Value>& n : renamed)
			if (v.is(ValueKind::Temp) && v.index == n.first)
				return n.second;
		return v;
	};
	InstId at = kNoId;
	for (InstId i = body_.blocks[p].first; i != kNoId; i = body_.instructions[i].next)
	{
		Instruction inst = body_.instructions[i];
		if (inst.op == Opcode::Phi)
		{
			renamed.push_back(std::make_pair(inst.result, phi_incoming(body_, inst, q)));
			continue;
		}
		if (inst.op == Opcode::Jump)
			break;
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
			renamed.push_back(std::make_pair(old, Value::temp(fresh)));
		}
		insert_after(body_, copy, at, id);
		at = id;
	}
	Instruction jump = make_instruction(Opcode::Jump, Type::scalar(TypeKind::Void));
	jump.ops[0] = Value::block(s);
	insert_after(body_, copy, at, append_instruction(body_, jump));
	for (InstId i = body_.blocks[s].first; i != kNoId && body_.instructions[i].op == Opcode::Phi;
		i = body_.instructions[i].next)
	{
		Instruction& phi = body_.instructions[i];
		std::vector<Value> pairs(
			body_.operands.begin() + phi.extra_first, body_.operands.begin() + phi.extra_first + phi.extra_count);
		pairs.push_back(Value::block(copy));
		pairs.push_back(mapped(phi_incoming(body_, phi, p)));
		phi.extra_first = append_operands(body_, pairs.data(), static_cast<uint32_t>(pairs.size()));
		phi.extra_count = static_cast<uint32_t>(pairs.size());
	}
	for_each_successor(body_, body_.instructions[body_.blocks[q].last], [&](BlockId& t) {
		if (t == p)
			t = copy;
	});
	remove_phi_incoming(body_, p, q);
	map_instruction_blocks(body_, c_.block_of);
	c_.invalidate_block_of();
	++c_.stats.branches_folded;
	return copy;
}

// Moves test block `s`'s phis to fresh slots (promoted again once the pass
// is done): each predecessor stores its input on its edge to `s` (in a new
// block when the predecessor has other successors) and each use loads the
// slot (a phi's at the end of its incoming block; one in `s` itself, its
// compare, before it). A predecessor in `redirects` whose edge was split
// is replaced by the new block. Not when `s` is its own predecessor, an
// input is another of the phis, a use is in a handler block or a phi's
// input on an edge from `s`, or the run's budget is spent.
// A test block with plain instructions before its compare and branch: a
// predecessor whose path decides the branch gets a copy of them that
// jumps to the decided side. Every value the block defines and anything
// outside it (or its own later instructions) reads goes to a slot first
// -- phis stored on the predecessors' edges, others right after their
// definition -- so the copy stores its own and each use loads (promoted
// again once the pass is done, thread_jumps). Not when the block is its
// own predecessor or a predecessor or use is in a handler block, or the
// run's budget is spent.
bool JumpThreading::copy_thread(BlockId s, InstId tail, std::vector<std::pair<BlockId, BlockId>>& redirects)
{
	if (demoted_ == kDemotedBlocks)
		return false;
	// Not a loop header (a copy for the back edge peels the test into
	// the loop with every value through slots).
	for (uint32_t k = cfg_.pred_first[s]; k < cfg_.pred_first[s + 1]; ++k)
		if (cfg_.preds[k] == s || (cfg_.flags[cfg_.preds[k]] & BF_HANDLER) != 0 ||
			(cfg_.reachable(cfg_.preds[k]) && cfg_.dominates(s, cfg_.preds[k])))
			return false;
	// Addresses of slots and globals move to the entry: computed once,
	// they need no slot or phi when the block is copied.
	const BlockId entry = cfg_.rpo[0];
	for (InstId i = body_.blocks[s].first, prev = kNoId; i != kNoId && i != tail;)
	{
		const InstId next = body_.instructions[i].next;
		const Instruction& inst = body_.instructions[i];
		if (inst.op == Opcode::Addr && (inst.ops[0].is(ValueKind::Slot) || inst.ops[0].is(ValueKind::Symbol)))
		{
			unlink_after(body_, s, prev, i);
			insert_after(body_, entry, previous_in_block(body_, entry, body_.blocks[entry].last), i);
		}
		else
			prev = i;
		i = next;
	}
	map_instruction_blocks(body_, c_.block_of);
	c_.invalidate_block_of();
	// The values the block defines.
	std::vector<InstId> defined;
	for (InstId i = body_.blocks[s].first; i != kNoId && i != tail; i = body_.instructions[i].next)
		if (body_.instructions[i].result != kNoId)
			defined.push_back(i);
	auto of_s = [&](const Value& v) -> InstId {
		if (!v.is(ValueKind::Temp))
			return kNoId;
		const InstId def = body_.temps[v.index].def;
		return def != kNoId && std::find(defined.begin(), defined.end(), def) != defined.end() ? def : kNoId;
	};
	for (InstId i = body_.blocks[s].first; i != kNoId && body_.instructions[i].op == Opcode::Phi;
		i = body_.instructions[i].next)
	{
		const Instruction& phi = body_.instructions[i];
		for (uint32_t k = 0; k + 1 < phi.extra_count; k += 2)
			if (of_s(body_.operands[phi.extra_first + k + 1]) != kNoId)
				return false;
	}
	struct Use
	{
		InstId inst;
		BlockId block;
		uint32_t position;
		bool phi;
		InstId def;
	};
	std::vector<Use> uses;
	bool ok = true;
	for (BlockId b : body_.layout)
		for (InstId i = body_.blocks[b].first; i != kNoId && ok; i = body_.instructions[i].next)
		{
			const Instruction& inst = body_.instructions[i];
			uint32_t position = 0;
			for_each_operand(body_, inst, [&](const Value& v) {
				const uint32_t at = position++;
				const InstId def = of_s(v);
				if (def == kNoId)
					return;
				const bool phi_input = inst.op == Opcode::Phi && at >= 3;
				const BlockId from = phi_input ? body_.operands[inst.extra_first + (at - 3) - 1].index : b;
				if ((phi_input && b == s) || (cfg_.flags[from] & BF_HANDLER) != 0)
					ok = false;
				uses.push_back(Use{ i, from, at, phi_input, def });
			});
		}
	if (!ok)
		return false;
	++demoted_;
	// Phi stores go on the predecessors' edges (a new block on one from a
	// predecessor with other successors).
	std::vector<std::pair<BlockId, BlockId>> edge;
	for (uint32_t k = cfg_.pred_first[s]; k < cfg_.pred_first[s + 1]; ++k)
	{
		const BlockId p = cfg_.preds[k];
		bool seen = false;
		for (const std::pair<BlockId, BlockId>& e : edge)
			seen = seen || e.first == p;
		if (seen)
			continue;
		BlockId at = p;
		if (cfg_.succ_count(p) != 1)
		{
			at = append_block(body_, BlockKind::Split);
			Instruction jump = make_instruction(Opcode::Jump, Type::scalar(TypeKind::Void));
			jump.ops[0] = Value::block(s);
			insert_after(body_, at, kNoId, append_instruction(body_, jump));
			std::vector<BlockId> layout;
			for (BlockId b : body_.layout)
			{
				layout.push_back(b);
				if (b == p)
					layout.push_back(at);
			}
			body_.layout.swap(layout);
			for_each_successor(body_, body_.instructions[body_.blocks[p].last], [&](BlockId& t) {
				if (t == s)
					t = at;
			});
			for (std::pair<BlockId, BlockId>& r : redirects)
				if (r.first == p)
					r.first = at;
		}
		edge.push_back(std::make_pair(p, at));
	}
	// Demote each defined value.
	for (InstId d : defined)
	{
		const Instruction value = body_.instructions[d];
		const Type type = body_.temps[value.result].type;
		const SlotId slot = static_cast<SlotId>(body_.slots.size());
		body_.slots.push_back(Slot{ 0, SlotKind::Demote, 0, 0, type });
		if (value.op == Opcode::Phi)
		{
			for (uint32_t k = 0; k + 1 < value.extra_count; k += 2)
			{
				const BlockId from = body_.operands[value.extra_first + k].index;
				BlockId at = from;
				for (const std::pair<BlockId, BlockId>& e : edge)
					if (e.first == from)
						at = e.second;
				Instruction store = make_instruction(Opcode::Store, type);
				store.ops[0] = body_.operands[value.extra_first + k + 1];
				store.ops[1] = Value::slot(slot);
				store.location = value.location;
				insert_after(
					body_, at, previous_in_block(body_, at, body_.blocks[at].last), append_instruction(body_, store));
			}
		}
		else
		{
			Instruction store = make_instruction(Opcode::Store, type);
			store.ops[0] = Value::temp(value.result);
			store.ops[1] = Value::slot(slot);
			store.location = value.location;
			insert_after(body_, s, d, append_instruction(body_, store));
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
			const InstId before = u.phi ? body_.blocks[u.block].last : u.inst;
			insert_after(body_, u.block, previous_in_block(body_, u.block, before), id);
			Instruction& again = body_.instructions[u.inst];
			(u.position < 3 ? again.ops[u.position] : body_.operands[again.extra_first + u.position - 3]) = Value::temp(
				body_.instructions[id].result);
		}
		if (value.op == Opcode::Phi)
			unlink_after(body_, s, previous_in_block(body_, s, d), d);
	}
	// Each threaded predecessor jumps to its own copy of the block's plain
	// instructions (the paired compare aside), which jumps on.
	const InstId skip = body_.instructions[tail].op == Opcode::Compare ? tail : kNoId;
	for (const std::pair<BlockId, BlockId>& r : redirects)
	{
		const BlockId p = r.first, target = r.second;
		const BlockId copy = append_block(body_, BlockKind::Thread);
		std::vector<BlockId> layout;
		for (BlockId b : body_.layout)
		{
			layout.push_back(b);
			if (b == p)
				layout.push_back(copy);
		}
		body_.layout.swap(layout);
		std::vector<std::pair<TempId, TempId>> renamed;
		auto mapped = [&](Value v) {
			for (const std::pair<TempId, TempId>& n : renamed)
				if (v.is(ValueKind::Temp) && v.index == n.first)
					return Value::temp(n.second);
			return v;
		};
		InstId at = kNoId;
		for (InstId i = body_.blocks[s].first; i != kNoId; i = body_.instructions[i].next)
		{
			Instruction inst = body_.instructions[i];
			if (inst.op == Opcode::Branch || inst.op == Opcode::Jump)
				break;
			if (i == skip)
				continue;
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
			insert_after(body_, copy, at, id);
			at = id;
		}
		Instruction jump = make_instruction(Opcode::Jump, Type::scalar(TypeKind::Void));
		jump.ops[0] = Value::block(target);
		insert_after(body_, copy, at, append_instruction(body_, jump));
		// The target's phis take what the block brought them, as the copy
		// computed it.
		for (InstId i = body_.blocks[target].first; i != kNoId && body_.instructions[i].op == Opcode::Phi;
			i = body_.instructions[i].next)
		{
			Instruction& phi = body_.instructions[i];
			std::vector<Value> pairs(
				body_.operands.begin() + phi.extra_first, body_.operands.begin() + phi.extra_first + phi.extra_count);
			pairs.push_back(Value::block(copy));
			pairs.push_back(mapped(phi_incoming(body_, phi, s)));
			phi.extra_first = append_operands(body_, pairs.data(), static_cast<uint32_t>(pairs.size()));
			phi.extra_count = static_cast<uint32_t>(pairs.size());
		}
		for_each_successor(body_, body_.instructions[body_.blocks[p].last], [&](BlockId& t) {
			if (t == s)
				t = copy;
		});
		++c_.stats.branches_folded;
	}
	c_.invalidate_cfg();
	c_.ensure_cfg();
	return true;
}

bool JumpThreading::demote_phis(BlockId s, std::vector<std::pair<BlockId, BlockId>>& redirects)
{
	if (demoted_ == kDemotedBlocks)
		return false;
	for (uint32_t k = cfg_.pred_first[s]; k < cfg_.pred_first[s + 1]; ++k)
		if (cfg_.preds[k] == s || (cfg_.flags[cfg_.preds[k]] & BF_HANDLER) != 0)
			return false;
	// A diamond or triangle whose arms only compute (at most two
	// instructions each; the branching block itself is an empty arm)
	// stays: select formation turns its phis into selects. Its arms hang
	// from s's immediate dominator.
	bool computing = cfg_.pred_count(s) == 2;
	const BlockId head = cfg_.idom[s];
	for (uint32_t k = cfg_.pred_first[s]; k < cfg_.pred_first[s + 1] && computing; ++k)
	{
		const BlockId p = cfg_.preds[k];
		if (cfg_.succ_count(p) == 2)
		{
			computing = p == head;
			continue;
		}
		computing = cfg_.pred_count(p) == 1 && cfg_.preds[cfg_.pred_first[p]] == head;
		unsigned count = 0;
		for (InstId i = body_.blocks[p].first; i != body_.blocks[p].last && computing; i = body_.instructions[i].next)
		{
			const Instruction& inst = body_.instructions[i];
			const bool divides = inst.op == Opcode::Binary &&
				(inst.sub == static_cast<uint8_t>(BinaryOp::Div) || inst.sub == static_cast<uint8_t>(BinaryOp::Mod) ||
					inst.sub == static_cast<uint8_t>(BinaryOp::UDiv) ||
					inst.sub == static_cast<uint8_t>(BinaryOp::UMod));
			computing = ++count <= 2 && !divides &&
				(inst.op == Opcode::Copy || inst.op == Opcode::Index || inst.op == Opcode::Addr ||
					inst.op == Opcode::Unary || inst.op == Opcode::Compare || inst.op == Opcode::Select ||
					inst.op == Opcode::Binary || inst.op == Opcode::Convert);
		}
	}
	if (computing)
		return false;
	std::vector<InstId> phis;
	for (InstId i = body_.blocks[s].first; i != kNoId && body_.instructions[i].op == Opcode::Phi;
		i = body_.instructions[i].next)
		phis.push_back(i);
	auto of_s = [&](const Value& v) {
		if (!v.is(ValueKind::Temp))
			return false;
		const InstId def = body_.temps[v.index].def;
		return def != kNoId && std::find(phis.begin(), phis.end(), def) != phis.end();
	};
	for (InstId i : phis)
	{
		const Instruction& phi = body_.instructions[i];
		for (uint32_t k = 0; k + 1 < phi.extra_count; k += 2)
			if (of_s(body_.operands[phi.extra_first + k + 1]))
				return false;
	}
	// The uses: (instruction, its block or a phi input's incoming block,
	// operand position: ops[0..2], then extra operands).
	struct Use
	{
		InstId inst;
		BlockId block;
		uint32_t position;
		bool phi;
	};
	std::vector<Use> uses;
	for (BlockId b : body_.layout)
		for (InstId i = body_.blocks[b].first; i != kNoId; i = body_.instructions[i].next)
		{
			const Instruction& inst = body_.instructions[i];
			uint32_t position = 0;
			bool ok = true;
			for_each_operand(body_, inst, [&](const Value& v) {
				const uint32_t at = position++;
				if (!of_s(v))
					return;
				const bool phi_input = inst.op == Opcode::Phi && at >= 3;
				const BlockId from = phi_input ? body_.operands[inst.extra_first + (at - 3) - 1].index : b;
				if ((phi_input && from == s) || (b == s && inst.op == Opcode::Phi) ||
					(cfg_.flags[from] & BF_HANDLER) != 0)
					ok = false;
				uses.push_back(Use{ i, from, at, phi_input });
			});
			if (!ok)
				return false;
		}
	++demoted_;
	// The stores' blocks: the predecessor, or a new block on its edge.
	std::vector<std::pair<BlockId, BlockId>> edge; // (predecessor, block storing)
	for (uint32_t k = cfg_.pred_first[s]; k < cfg_.pred_first[s + 1]; ++k)
	{
		const BlockId p = cfg_.preds[k];
		bool seen = false;
		for (const std::pair<BlockId, BlockId>& e : edge)
			seen = seen || e.first == p;
		if (seen)
			continue;
		BlockId at = p;
		if (cfg_.succ_count(p) != 1)
		{
			at = append_block(body_, BlockKind::Split);
			Instruction jump = make_instruction(Opcode::Jump, Type::scalar(TypeKind::Void));
			jump.ops[0] = Value::block(s);
			insert_after(body_, at, kNoId, append_instruction(body_, jump));
			std::vector<BlockId> layout;
			for (BlockId b : body_.layout)
			{
				layout.push_back(b);
				if (b == p)
					layout.push_back(at);
			}
			body_.layout.swap(layout);
			for_each_successor(body_, body_.instructions[body_.blocks[p].last], [&](BlockId& t) {
				if (t == s)
					t = at;
			});
			for (std::pair<BlockId, BlockId>& r : redirects)
				if (r.first == p)
					r.first = at;
		}
		edge.push_back(std::make_pair(p, at));
	}
	for (InstId i : phis)
	{
		const Instruction phi = body_.instructions[i];
		const SlotId slot = static_cast<SlotId>(body_.slots.size());
		body_.slots.push_back(Slot{ 0, SlotKind::Phi, 0, 0, phi.type });
		for (uint32_t k = 0; k + 1 < phi.extra_count; k += 2)
		{
			const BlockId from = body_.operands[phi.extra_first + k].index;
			BlockId at = from;
			for (const std::pair<BlockId, BlockId>& e : edge)
				if (e.first == from)
					at = e.second;
			Instruction store = make_instruction(Opcode::Store, phi.type);
			store.ops[0] = body_.operands[phi.extra_first + k + 1];
			store.ops[1] = Value::slot(slot);
			store.location = phi.location;
			const InstId id = append_instruction(body_, store);
			insert_after(body_, at, previous_in_block(body_, at, body_.blocks[at].last), id);
		}
		for (const Use& u : uses)
		{
			Instruction& user = body_.instructions[u.inst];
			Value& operand = u.position < 3 ? user.ops[u.position] : body_.operands[user.extra_first + u.position - 3];
			if (!(operand == Value::temp(phi.result)))
				continue;
			Instruction load = make_instruction(Opcode::Load, phi.type);
			load.ops[0] = Value::slot(slot);
			load.location = phi.location;
			const InstId id = append_instruction(body_, load);
			body_.instructions[id].result = append_temp(body_, phi.type, id);
			const InstId before = u.phi ? body_.blocks[u.block].last : u.inst;
			insert_after(body_, u.block, previous_in_block(body_, u.block, before), id);
			Instruction& again = body_.instructions[u.inst];
			(u.position < 3 ? again.ops[u.position] : body_.operands[again.extra_first + u.position - 3]) = Value::temp(
				body_.instructions[id].result);
		}
		unlink_after(body_, s, kNoId, i);
	}
	c_.invalidate_cfg();
	c_.ensure_cfg();
	return true;
}

// O2+: a branch on a compare, or on any value, decides facts in each block
// entered only along one of its edges and in the blocks that block
// dominates: the compare's result, and for `cmp eq|ne x, k` (integers and
// pointers, k a literal) x itself on the equal side (k) and x != k on the
// other; a value branched on is 0 on its false side and nonzero on its
// true side. Uses there take the decided values (phis keep theirs: their
// inputs flow from predecessors), and `cmp eq|ne x, k'` folds against a
// known x != k'. One dominator-tree walk with an undo log.
bool JumpThreading::dominating_facts()
{
	const DomChildren& tree = c_.ensure_dom();
	std::vector<Value> known(body_.temps.size(), Value::none());
	std::vector<std::pair<TempId, Value>> unequal; // (x, k): x != k
	std::vector<TempId> undo;
	bool changed = false;
	auto literal_of = [&](Value v, Literal& out) {
		Constant c;
		if (!literal_constant(body_, v, c) || c.kind != ValueKind::Integer)
			return false;
		out = c.bits;
		return true;
	};
	auto set = [&](TempId t, Value v) {
		if (t < known.size() && known[t].is(ValueKind::None))
		{
			known[t] = v;
			undo.push_back(t);
		}
	};
	auto enter = [&](BlockId b) {
		if (b == cfg_.rpo[0] || cfg_.pred_count(b) != 1 || (cfg_.flags[b] & BF_HANDLER) != 0)
			return;
		const BlockId p = cfg_.preds[cfg_.pred_first[b]];
		const InstId t = body_.blocks[p].last;
		if (t == kNoId || body_.instructions[t].op != Opcode::Branch)
			return;
		const Instruction& br = body_.instructions[t];
		if (br.ops[1].index == br.ops[2].index || !br.ops[0].is(ValueKind::Temp) || br.ops[0].index >= known.size())
			return;
		const bool taken = br.ops[1].index == b;
		const TempId c = br.ops[0].index;
		const InstId def = body_.temps[c].def;
		const Type type = body_.temps[c].type;
		if (!(type.is_integer() || type.is_pointer()) || type.kind() == TypeKind::I128)
			return;
		if (def == kNoId || body_.instructions[def].op != Opcode::Compare)
		{
			if (!taken)
				set(c, integer_value(body_, Literal{ 0, 0 }));
			else
				unequal.push_back(std::make_pair(c, integer_value(body_, Literal{ 0, 0 })));
			return;
		}
		set(c, integer_value(body_, Literal{ taken ? 1u : 0u, 0 }));
		const Instruction& cmp = body_.instructions[def];
		const ComparePredicate pred = static_cast<ComparePredicate>(cmp.sub);
		if ((pred != ComparePredicate::Eq && pred != ComparePredicate::Ne) || cmp.type.is_float() ||
			cmp.type.kind() == TypeKind::I128)
			return;
		Value x = cmp.ops[0], k = cmp.ops[1];
		Literal bits;
		if (!x.is(ValueKind::Temp))
			std::swap(x, k);
		if (!x.is(ValueKind::Temp) || x.index >= known.size() || !literal_of(k, bits) ||
			body_.temps[x.index].type != cmp.type)
			return;
		if ((pred == ComparePredicate::Eq) == taken)
			set(x.index, k);
		else
			unequal.push_back(std::make_pair(x.index, k));
	};
	auto rewrite = [&](BlockId b) {
		for (InstId i = body_.blocks[b].first; i != kNoId; i = body_.instructions[i].next)
		{
			Instruction& inst = body_.instructions[i];
			if (inst.op == Opcode::Phi)
				continue;
			for_each_operand(body_, inst, [&](Value& v) {
				if (v.is(ValueKind::Temp) && v.index < known.size() && !known[v.index].is(ValueKind::None))
				{
					v = known[v.index];
					changed = true;
				}
			});
			if (inst.op != Opcode::Compare || inst.result == kNoId || inst.result >= known.size() || unequal.empty())
				continue;
			const ComparePredicate pred = static_cast<ComparePredicate>(inst.sub);
			if (pred != ComparePredicate::Eq && pred != ComparePredicate::Ne)
				continue;
			Value x = inst.ops[0], k = inst.ops[1];
			if (!x.is(ValueKind::Temp))
				std::swap(x, k);
			Literal bits;
			if (!x.is(ValueKind::Temp) || !literal_of(k, bits))
				continue;
			for (const std::pair<TempId, Value>& f : unequal)
			{
				Literal other;
				if (f.first != x.index || !literal_of(f.second, other) ||
					!same_constant(inst.type, Constant{ ValueKind::Integer, TypeKind::Void, bits },
						Constant{ ValueKind::Integer, TypeKind::Void, other }))
					continue;
				set(inst.result, integer_value(body_, Literal{ pred == ComparePredicate::Ne ? 1u : 0u, 0 }));
				break;
			}
		}
	};
	// (block, next child, undo mark, unequal mark)
	struct Frame
	{
		BlockId block;
		uint32_t child;
		size_t undo, unequal;
	};
	std::vector<Frame> stack;
	auto visit = [&](BlockId b) {
		stack.push_back(Frame{ b, 0, undo.size(), unequal.size() });
		enter(b);
		// (No fact in force: nothing to rewrite.)
		if (!undo.empty() || !unequal.empty())
			rewrite(b);
	};
	visit(cfg_.rpo[0]);
	while (!stack.empty())
	{
		Frame& top = stack.back();
		if (tree.first[top.block] + top.child < tree.first[top.block + 1])
		{
			const BlockId child = tree.children[tree.first[top.block] + top.child++];
			visit(child);
			continue;
		}
		for (size_t u = undo.size(); u > top.undo; --u)
			known[undo[u - 1]] = Value::none();
		undo.resize(top.undo);
		unequal.resize(top.unequal);
		stack.pop_back();
	}
	return changed;
}

bool JumpThreading::run()
{
	c_.ensure_cfg();
	bool changed = c_.level >= 2 && dominating_facts();
	changed = known_edges() || changed;
	if (changed)
	{
		c_.invalidate_cfg();
		c_.ensure_cfg();
	}
	touched_ = changed;
	// (known_compares counts uses first: still current when it changes
	// nothing.)
	bool counted = false;
	if (c_.level >= 2)
	{
		if (known_compares())
		{
			changed = true;
			c_.invalidate_cfg();
			c_.ensure_cfg();
		}
		else
			counted = true;
	}
	touched_ = changed;
	changed = constant_phis(counted) || changed;
	return changed;
}

} // namespace

bool thread_jumps(FunctionContext& context)
{
	JumpThreading threading(context);
	const bool changed = threading.run();
	// A block every predecessor now bypasses is unreachable, its phis
	// empty: the cleanup removes it. Demoted phis return to SSA form.
	if (changed)
	{
		context.invalidate_cfg();
		cleanup_cfg(context);
		if (threading.demoted() != 0)
			promote_slots(context);
	}
	return changed;
}

// O2+: a branch on whether two xors of loads with constants or-ed together
// are zero -- an expanded equality memcmp against a literal -- tests the
// first chunk, then (in a new block) the second: most mismatches leave
// after one compare with an immediate instead of xoring and or-ing both
// chunks. The xors, the or and the compare have no other uses; the second
// load moves into the new block when it has no other use either and
// nothing after it in its block writes memory. Each target's phis take the
// new block's edge with the branch block's value.
bool split_or_branches(FunctionContext& context)
{
	FunctionBody& body = context.body;
	const std::vector<uint32_t>& uses = context.entry_uses(split_or_branches);
	auto def_of = [&](Value v) -> const Instruction* {
		return v.is(ValueKind::Temp) && v.index >= body.parameter_count && body.temps[v.index].def != kNoId
			? &body.instructions[body.temps[v.index].def]
			: nullptr;
	};
	auto single = [&](Value v) { return v.is(ValueKind::Temp) && v.index < uses.size() && uses[v.index] == 1; };
	auto xor_of_load = [&](Value v, Value& loaded, Value& constant) {
		const Instruction* x = def_of(v);
		if (x == nullptr || !single(v) || x->op != Opcode::Binary || static_cast<BinaryOp>(x->sub) != BinaryOp::Xor)
			return false;
		for (unsigned k = 0; k < 2; ++k)
		{
			const Instruction* load = def_of(x->ops[k]);
			if (load != nullptr && load->op == Opcode::Load && (load->flags & IF_VOLATILE) == 0 &&
				x->ops[1 - k].is(ValueKind::Integer))
			{
				loaded = x->ops[k];
				constant = x->ops[1 - k];
				return true;
			}
		}
		return false;
	};
	bool changed = false;
	const std::vector<BlockId> layout = body.layout;
	std::vector<BlockId> new_layout;
	for (BlockId b : layout)
	{
		new_layout.push_back(b);
		const InstId last = body.blocks[b].last;
		if (last == kNoId || body.instructions[last].op != Opcode::Branch)
			continue;
		const Instruction term = body.instructions[last];
		const BlockId t = term.ops[1].index, f = term.ops[2].index;
		const Instruction* cmp = def_of(term.ops[0]);
		if (t == f || cmp == nullptr || !single(term.ops[0]) || cmp->op != Opcode::Compare || !cmp->type.is_integer())
			continue;
		const ComparePredicate pred = static_cast<ComparePredicate>(cmp->sub);
		if ((pred != ComparePredicate::Eq && pred != ComparePredicate::Ne) || !cmp->ops[1].is(ValueKind::Integer) ||
			body.literals[cmp->ops[1].index].low != 0 || body.literals[cmp->ops[1].index].high != 0)
			continue;
		const Instruction* orr = def_of(cmp->ops[0]);
		if (orr == nullptr || !single(cmp->ops[0]) || orr->op != Opcode::Binary ||
			static_cast<BinaryOp>(orr->sub) != BinaryOp::Or)
			continue;
		Value l0, k0, l1, k1;
		if (!xor_of_load(orr->ops[0], l0, k0) || !xor_of_load(orr->ops[1], l1, k1))
			continue;
		// Every one of them in this block; the second load movable to the end.
		const InstId cmp_id = body.temps[term.ops[0].index].def, or_id = body.temps[cmp->ops[0].index].def;
		const InstId x0_id = body.temps[orr->ops[0].index].def, x1_id = body.temps[orr->ops[1].index].def;
		// The second load moves when it is this block's own (a load shared
		// with other compares stays where it is).
		const InstId l1_id = body.temps[l1.index].def;
		bool here[5] = { false, false, false, false, false };
		bool past_load = false, writes = false;
		for (InstId i = body.blocks[b].first; i != kNoId; i = body.instructions[i].next)
		{
			here[0] = here[0] || i == cmp_id;
			here[1] = here[1] || i == or_id;
			here[2] = here[2] || i == x0_id;
			here[3] = here[3] || i == x1_id;
			here[4] = here[4] || i == l1_id;
			if (past_load && i != last && may_write_memory(context.program, body.instructions[i]))
				writes = true;
			past_load = past_load || i == l1_id;
		}
		if (!here[0] || !here[1] || !here[2] || !here[3])
			continue;
		const bool move_load = here[4] && single(l1) && !writes;
		const Type type = body.instructions[x0_id].type;
		const Type type1 = body.instructions[x1_id].type;
		// The first chunk's test replaces the or-ed compare; the second's
		// goes to a new block with its load.
		const bool equal_taken = pred == ComparePredicate::Eq; // t when all equal
		Instruction& first = body.instructions[cmp_id];
		first.type = type;
		first.sub = static_cast<uint8_t>(equal_taken ? ComparePredicate::Eq : ComparePredicate::Ne);
		first.ops[0] = l0;
		first.ops[1] = k0;
		const BlockId split = append_block(body, BlockKind::Split);
		const BlockId mismatch = equal_taken ? f : t;
		body.instructions[last].ops[1] = Value::block(equal_taken ? split : t);
		body.instructions[last].ops[2] = Value::block(equal_taken ? f : split);
		// Unlink the or and both xors; move the second load.
		for (InstId gone : { or_id, x0_id, x1_id })
			unlink_after(body, b, previous_in_block(body, b, gone), gone);
		InstId after = kNoId;
		if (move_load)
		{
			unlink_after(body, b, previous_in_block(body, b, l1_id), l1_id);
			insert_after(body, split, kNoId, l1_id);
			after = l1_id;
		}
		Instruction second = make_instruction(Opcode::Compare, type1);
		second.sub = static_cast<uint8_t>(equal_taken ? ComparePredicate::Eq : ComparePredicate::Ne);
		second.ops[0] = l1;
		second.ops[1] = k1;
		second.location = term.location;
		const InstId at = append_instruction(body, second);
		body.instructions[at].result = append_temp(body, body.temps[term.ops[0].index].type, at);
		insert_after(body, split, after, at);
		Instruction branch = make_instruction(Opcode::Branch, Type::scalar(TypeKind::Void));
		branch.ops[0] = Value::temp(body.instructions[at].result);
		branch.ops[1] = Value::block(t);
		branch.ops[2] = Value::block(f);
		branch.location = term.location;
		insert_after(body, split, at, append_instruction(body, branch));
		// The targets' phis: the one the branch block no longer reaches
		// (all-equal for eq, the second chunk's for ne) takes the new block
		// in its place; the other gains the new block with the branch
		// block's value.
		rename_phi_incoming(body, mismatch == f ? t : f, b, split);
		for (InstId i = body.blocks[mismatch].first; i != kNoId && body.instructions[i].op == Opcode::Phi;
			i = body.instructions[i].next)
		{
			const Value v = phi_incoming(body, body.instructions[i], b);
			std::vector<Value> pairs(body.operands.begin() + body.instructions[i].extra_first,
				body.operands.begin() + body.instructions[i].extra_first + body.instructions[i].extra_count);
			pairs.push_back(Value::block(split));
			pairs.push_back(v);
			body.instructions[i].extra_first = append_operands(body, pairs.data(), static_cast<uint32_t>(pairs.size()));
			body.instructions[i].extra_count = static_cast<uint32_t>(pairs.size());
		}
		new_layout.push_back(split);
		changed = true;
	}
	if (changed)
	{
		body.layout.swap(new_layout);
		context.invalidate_cfg();
	}
	return changed;
}

// O2+: a block that only branches on a compare of a phi with a value
// defined before it (a width chosen as 1, 2 or 3 on three paths, then
// tested) gives each predecessor its own compare of that predecessor's
// incoming value, in a new block on the edge: the phi's copies go and a
// literal becomes the compare's immediate. The targets take no phis; at
// most kPhiComparePreds predecessors.
const uint32_t kPhiComparePreds = 4;

bool specialize_phi_compares(FunctionContext& context)
{
	FunctionBody& body = context.body;
	const std::vector<uint32_t>& uses = context.entry_uses(specialize_phi_compares);
	context.ensure_cfg();
	Cfg& cfg = context.cfg;
	auto has_phi = [&](BlockId b) {
		const InstId first = body.blocks[b].first;
		return first != kNoId && body.instructions[first].op == Opcode::Phi;
	};
	bool changed = false;
	const std::vector<BlockId> layout = body.layout;
	std::vector<BlockId> extra;
	for (BlockId j : layout)
	{
		const InstId phi_id = body.blocks[j].first;
		if (phi_id == kNoId || cfg.pred_count(j) < 2 || cfg.pred_count(j) > kPhiComparePreds || j == layout[0])
			continue;
		const Instruction& phi = body.instructions[phi_id];
		const InstId cmp_id = phi.next;
		if (phi.op != Opcode::Phi || cmp_id == kNoId || !phi.type.is_integer() || phi.result >= uses.size() ||
			uses[phi.result] != 1)
			continue;
		const Instruction& cmp = body.instructions[cmp_id];
		const InstId br_id = cmp.next;
		if (cmp.op != Opcode::Compare || br_id == kNoId || cmp.result >= uses.size() || uses[cmp.result] != 1)
			continue;
		const Instruction& br = body.instructions[br_id];
		if (br.op != Opcode::Branch || br.next != kNoId || !(br.ops[0] == Value::temp(cmp.result)))
			continue;
		const unsigned side = cmp.ops[0] == Value::temp(phi.result) ? 0 : cmp.ops[1] == Value::temp(phi.result) ? 1 : 2;
		if (side == 2 || cmp.ops[1 - side] == Value::temp(phi.result))
			continue;
		const BlockId t = br.ops[1].index, f = br.ops[2].index;
		if (t == f || has_phi(t) || has_phi(f) || (cfg.flags[j] & BF_HANDLER) != 0)
			continue;
		// Each predecessor's edge gets its block.
		std::vector<BlockId> preds;
		for (uint32_t k = cfg.pred_first[j]; k < cfg.pred_first[j + 1]; ++k)
			if (std::find(preds.begin(), preds.end(), cfg.preds[k]) == preds.end())
				preds.push_back(cfg.preds[k]);
		bool ok = true;
		for (BlockId p : preds)
			ok = ok && (cfg.flags[p] & BF_HANDLER) == 0 && body.blocks[p].last != kNoId &&
				(body.instructions[body.blocks[p].last].op == Opcode::Branch ||
					body.instructions[body.blocks[p].last].op == Opcode::Jump);
		if (!ok)
			continue;
		const Instruction phi_copy = phi, cmp_copy = cmp, br_copy = br;
		for (BlockId p : preds)
		{
			const Value incoming = phi_incoming(body, phi_copy, p);
			const BlockId e = append_block(body, BlockKind::Thread);
			Instruction c = cmp_copy;
			c.next = kNoId;
			c.ops[side] = incoming;
			const InstId c_id = append_instruction(body, c);
			body.instructions[c_id].result = append_temp(body, body.temps[cmp_copy.result].type, c_id);
			insert_after(body, e, kNoId, c_id);
			Instruction b2 = br_copy;
			b2.next = kNoId;
			b2.ops[0] = Value::temp(body.instructions[c_id].result);
			insert_after(body, e, c_id, append_instruction(body, b2));
			for_each_successor(body, body.instructions[body.blocks[p].last], [&](BlockId& s) {
				if (s == j)
					s = e;
			});
			extra.push_back(e);
		}
		changed = true;
	}
	if (changed)
	{
		body.layout.insert(body.layout.end(), extra.begin(), extra.end());
		context.invalidate_cfg();
		cleanup_cfg(context);
	}
	return changed;
}

// O2+, last: a compare whose every use is a branch condition, with uses
// in blocks other than its own, is computed again before each such
// branch (its operands dominate them): each branch then tests its own
// fused compare instead of a materialized Boolean the function keeps live
// (a compare used by one later branch only moves there).
bool split_branch_compares(FunctionContext& context)
{
	FunctionBody& body = context.body;
	context.entry_uses(split_branch_compares);
	context.entry_block_of(split_branch_compares);
	std::vector<uint32_t>& uses = context.uses;
	std::vector<uint32_t> branch_uses(body.temps.size(), 0), select_uses(body.temps.size(), 0);
	for (BlockId b : body.layout)
	{
		const InstId last = body.blocks[b].last;
		if (last != kNoId && body.instructions[last].op == Opcode::Branch &&
			body.instructions[last].ops[0].is(ValueKind::Temp))
			++branch_uses[body.instructions[last].ops[0].index];
		for (InstId i = body.blocks[b].first; i != kNoId; i = body.instructions[i].next)
			if (body.instructions[i].op == Opcode::Select && body.instructions[i].ops[0].is(ValueKind::Temp))
				++select_uses[body.instructions[i].ops[0].index];
	}
	bool changed = false;
	for (BlockId b : body.layout)
	{
		const InstId last = body.blocks[b].last;
		if (last == kNoId || body.instructions[last].op != Opcode::Branch ||
			!body.instructions[last].ops[0].is(ValueKind::Temp))
			continue;
		const TempId c = body.instructions[last].ops[0].index;
		const InstId def = body.temps[c].def;
		if (def == kNoId || c >= branch_uses.size() || uses[c] < 1 || branch_uses[c] + select_uses[c] != uses[c] ||
			def >= context.block_of.size() || context.block_of[def] == b)
			continue;
		const Instruction cmp = body.instructions[def];
		if (cmp.op != Opcode::Compare || cmp.type.kind() == TypeKind::I128 || cmp.type.kind() == TypeKind::F80)
			continue;
		Instruction copy = cmp;
		copy.next = kNoId;
		const InstId at = append_instruction(body, copy);
		const TempId result = append_temp(body, body.temps[c].type, at);
		body.instructions[at].result = result;
		insert_after(body, b, previous_in_block(body, b, last), at);
		body.instructions[last].ops[0] = Value::temp(result);
		changed = true;
	}
	// So does a select's compare: each select reading one (used only as
	// conditions) not just before it gets its copy there, which the back
	// end fuses into its cmov.
	for (BlockId b : body.layout)
		for (InstId prev = kNoId, i = body.blocks[b].first; i != kNoId; prev = i, i = body.instructions[i].next)
		{
			if (body.instructions[i].op != Opcode::Select || !body.instructions[i].ops[0].is(ValueKind::Temp))
				continue;
			const TempId c = body.instructions[i].ops[0].index;
			const InstId def = body.temps[c].def;
			if (def == kNoId || def == prev || uses[c] < 1 || branch_uses[c] + select_uses[c] != uses[c])
				continue;
			const Instruction cmp = body.instructions[def];
			if (cmp.op != Opcode::Compare || cmp.type.kind() == TypeKind::I128 || cmp.type.kind() == TypeKind::F80 ||
				cmp.type.is_float())
				continue;
			Instruction copy = cmp;
			copy.next = kNoId;
			const InstId at = append_instruction(body, copy);
			const TempId result = append_temp(body, body.temps[c].type, at);
			body.instructions[at].result = result;
			insert_after(body, b, prev, at);
			body.instructions[i].ops[0] = Value::temp(result);
			prev = at;
			changed = true;
		}
	// A single-bit mask `shl 1, k` shared by several and-tests, or
	// computed in another block (an `and` used only by a compare with zero
	// that the branch of its block takes): each such and takes a mask
	// computed beside it, which the back end folds into the branch's bt
	// (no shared mask kept live).
	if (changed)
	{
		count_uses(body, uses);
		map_instruction_blocks(body, context.block_of);
		context.invalidate_uses();
		context.invalidate_block_of();
	}
	for (BlockId b : body.layout)
	{
		const InstId last = body.blocks[b].last;
		if (last == kNoId || body.instructions[last].op != Opcode::Branch ||
			!body.instructions[last].ops[0].is(ValueKind::Temp))
			continue;
		// The branch tests the and itself, or its compare with zero.
		TempId x = body.instructions[last].ops[0].index;
		const InstId def = body.temps[x].def;
		if (def != kNoId && body.instructions[def].op == Opcode::Compare)
		{
			const Instruction& cmp = body.instructions[def];
			const ComparePredicate pred = static_cast<ComparePredicate>(cmp.sub);
			Constant zero;
			if (uses[x] != 1 || context.block_of[def] != b ||
				(pred != ComparePredicate::Eq && pred != ComparePredicate::Ne) || !cmp.ops[0].is(ValueKind::Temp) ||
				!literal_constant(body, cmp.ops[1], zero) || zero.kind != ValueKind::Integer || zero.bits.low != 0 ||
				zero.bits.high != 0)
				continue;
			x = cmp.ops[0].index;
		}
		const InstId and_id = body.temps[x].def;
		if (and_id == kNoId || uses[x] != 1 || context.block_of[and_id] != b ||
			body.instructions[and_id].op != Opcode::Binary ||
			static_cast<BinaryOp>(body.instructions[and_id].sub) != BinaryOp::And)
			continue;
		for (unsigned k = 0; k < 2; ++k)
		{
			const Value m = body.instructions[and_id].ops[k];
			const InstId shl = m.is(ValueKind::Temp) ? body.temps[m.index].def : kNoId;
			if (shl == kNoId || body.instructions[shl].op != Opcode::Binary ||
				static_cast<BinaryOp>(body.instructions[shl].sub) != BinaryOp::Shl ||
				body.instructions[shl].type != body.instructions[and_id].type)
				continue;
			Constant one;
			if (!literal_constant(body, body.instructions[shl].ops[0], one) || one.kind != ValueKind::Integer ||
				one.bits.low != 1 || one.bits.high != 0)
				continue;
			if (uses[m.index] == 1 && context.block_of[shl] == b)
				break;
			Instruction copy = body.instructions[shl];
			copy.next = kNoId;
			const InstId at = append_instruction(body, copy);
			const TempId mask = append_temp(body, body.temps[m.index].type, at);
			body.instructions[at].result = mask;
			insert_after(body, b, previous_in_block(body, b, and_id), at);
			body.instructions[and_id].ops[k] = Value::temp(mask);
			changed = true;
			break;
		}
	}
	return changed;
}

} // namespace opt
