#include "lowir/builder.h"
#include "opt/loops.h"
#include "opt/passes.h"
#include "opt/scalar_fold.h"

#include <algorithm>

namespace opt {

using namespace lowir;

namespace {

// Loop-invariant code motion: pure instructions whose operands the loop does
// not compute move to its preheader.
class LoopMotion
{
public:
	explicit LoopMotion(FunctionContext& context) : c_(context), body_(context.body), cfg_(context.cfg) {}

	bool run();

private:
	bool invariant(uint32_t loop, Value v) const;
	void note_register_uses();
	bool literal_index(const Instruction& inst) const;
	bool speculable(const Instruction& inst) const;
	bool costly(const Instruction& inst) const;
	bool one_reader(TempId t);
	bool worth_moving(const Instruction& inst, bool calls);
	void find_feeders(uint32_t loop);
	bool sweep(uint32_t loop, bool apply);
	struct Term
	{
		Value value;
		bool negative; // subtracted
	};
	bool chain_link(Value v, BinaryOp family, Type type, BlockId block);
	bool read_chain(uint32_t loop, InstId root, BlockId block);
	bool widened(uint32_t loop, InstId root) const;
	Value combine(const std::vector<Term>& terms, BinaryOp family, size_t& next, bool& negative);
	bool reassociate(uint32_t loop, bool calls, bool apply);

	FunctionContext& c_;
	FunctionBody& body_;
	Cfg& cfg_;
	LoopForest forest_;
	std::vector<uint8_t> in_register_; // per temp: some use needs it in a register
	std::vector<uint8_t> feeds_;       // per temp of the loop at hand: could leave (1), a costly one reads it (2)
	std::vector<TempId> fed_;          // the temps feeds_ marks
	// reassociate's state: use counts, per instruction the walk that read it
	// as part of a chain, a chain's operands and instructions (its root
	// first), and the block's regrouped chains' instructions in their new
	// order.
	std::vector<uint32_t> uses_;
	std::vector<uint32_t> read_;
	uint32_t walk_ = 0;
	std::vector<Term> terms_, invariants_, variants_;
	std::vector<InstId> links_, order_, block_insts_;
	std::vector<std::pair<size_t, size_t>> chains_; // per regrouped chain of the block: its range of order_
	std::vector<uint32_t> chain_;                   // per instruction: 1 + its regrouped chain, or 0
};

// The operation family of a chain step: addition and subtraction form one;
// multiplication, and, or and xor are each their own. Integers only: their
// arithmetic wraps, so every grouping of a chain gives the same bits.
bool chain_family(const Instruction& inst, BinaryOp& family)
{
	if (inst.op != Opcode::Binary || inst.result == kNoId || !inst.type.is_integer() ||
		inst.type.kind() == TypeKind::I128)
		return false;
	const BinaryOp op = static_cast<BinaryOp>(inst.sub);
	family = op == BinaryOp::Sub ? BinaryOp::Add : op;
	return op == BinaryOp::Add || op == BinaryOp::Sub || op == BinaryOp::Mul || op == BinaryOp::And ||
		op == BinaryOp::Or || op == BinaryOp::Xor;
}

bool LoopMotion::invariant(uint32_t loop, Value v) const
{
	if (!v.is(ValueKind::Temp) || v.index < body_.parameter_count)
		return true;
	const InstId def = body_.temps[v.index].def;
	return def != kNoId && c_.block_of[def] != kNoId && !in_loop(forest_, loop, c_.block_of[def]);
}

bool LoopMotion::literal_index(const Instruction& inst) const
{
	return inst.op == Opcode::Index && inst.ops[1].is(ValueKind::Integer);
}

// An address that only reaches loads and stores, through literal steps, fits
// in their operands: in a register of its own it saves nothing.
void LoopMotion::note_register_uses()
{
	in_register_.assign(body_.temps.size(), 0);
	for (BlockId b : body_.layout)
		for (InstId i = body_.blocks[b].first; i != kNoId; i = body_.instructions[i].next)
		{
			const Instruction& inst = body_.instructions[i];
			uint32_t position = 0;
			for_each_operand(body_, inst, [&](const Value& v) {
				const uint32_t at = position++;
				const bool address = (inst.op == Opcode::Load && at == 0) || (inst.op == Opcode::Store && at == 1) ||
					(literal_index(inst) && at == 0);
				if (v.is(ValueKind::Temp) && !address)
					in_register_[v.index] = 1;
			});
		}
	// (Layout order has definitions first but for phis: two walks back reach
	// the addresses a literal step's use needs.)
	for (unsigned walk = 0; walk < 2; ++walk)
		for (size_t at = body_.layout.size(); at-- > 0;)
			for (InstId i = body_.blocks[body_.layout[at]].first; i != kNoId; i = body_.instructions[i].next)
			{
				const Instruction& inst = body_.instructions[i];
				if (literal_index(inst) && in_register_[inst.result] && inst.ops[0].is(ValueKind::Temp))
					in_register_[inst.ops[0].index] = 1;
			}
}

// Whether `inst` may run on a path that would not have reached it: integer
// division traps unless a literal divisor rules that out.
bool LoopMotion::speculable(const Instruction& inst) const
{
	if (inst.op != Opcode::Binary || !inst.type.is_integer())
		return true;
	const BinaryOp op = static_cast<BinaryOp>(inst.sub);
	if (op != BinaryOp::Div && op != BinaryOp::Mod && op != BinaryOp::UDiv && op != BinaryOp::UMod)
		return true;
	if (!inst.ops[1].is(ValueKind::Integer))
		return false;
	const Literal divisor = normalize_integer(inst.type, body_.literals[inst.ops[1].index]);
	const bool is_signed = op == BinaryOp::Div || op == BinaryOp::Mod;
	return (divisor.low != 0 || divisor.high != 0) && !(is_signed && static_cast<int64_t>(divisor.low) == -1);
}

// A computation that pays for a value held across a loop's calls.
bool LoopMotion::costly(const Instruction& inst) const
{
	if (inst.op != Opcode::Binary)
		return false;
	const BinaryOp op = static_cast<BinaryOp>(inst.sub);
	return op == BinaryOp::Mul || op == BinaryOp::Div || op == BinaryOp::Mod || op == BinaryOp::UDiv ||
		op == BinaryOp::UMod;
}

// Whether temp `t` has one reader. (Counted when first asked: most loops
// have no candidate.)
bool LoopMotion::one_reader(TempId t)
{
	if (uses_.size() != body_.temps.size())
		count_uses(body_, uses_);
	return uses_[t] == 1;
}

// A loop with a call would carry the value across it in memory or in one of
// the few registers calls keep: only a costly computation pays for that.
bool LoopMotion::worth_moving(const Instruction& inst, bool calls)
{
	if (inst.type.kind() == TypeKind::I128 ||
		(inst.op == Opcode::Convert && convert_source_type(inst).kind() == TypeKind::I128))
		return false; // (a call of the runtime's)
	// Virtual locals survive calls without spilling. Their address arithmetic
	// is explicit too, rather than folded into a load/store operand.
	calls = calls && !c_.costs.locals_survive_calls;
	switch (inst.op)
	{
	case Opcode::Addr:
		// An A64 symbol address uses a page relocation and a second
		// instruction. A base needed for a variable index cannot fold into
		// the load, and may stay in a call-preserved register across trips.
		if (calls && target_model::is_aarch64(c_.program.target) && inst.ops[0].is(ValueKind::Symbol) &&
			c_.program.symbols[inst.ops[0].index].metadata.storage != ir_model::GSM_THREAD_LOCAL &&
			!in_register_.empty() && in_register_[inst.result])
			return true;
		return !calls && (!c_.costs.scaled_memory_operands || in_register_[inst.result] != 0);
	case Opcode::Index:
		return !calls && (!c_.costs.scaled_memory_operands || !literal_index(inst) || in_register_[inst.result] != 0);
	case Opcode::Binary:
		return !calls || costly(inst);
	case Opcode::Compare:
	case Opcode::Copy:
	case Opcode::Unary:
	case Opcode::Select:
	case Opcode::Convert:
		return !calls;
	case Opcode::Vector:
		// Arm can multiply by a repeated lane without materializing the
		// broadcast. Leave it beside the multiply, its one reader, to keep
		// that opportunity. (A broadcast other instructions read too is
		// materialized wherever it stays.)
		if (!calls && inst.sub == static_cast<unsigned>(VectorOp::Pshufd) && inst.next != kNoId &&
			inst.ops[2].is(ValueKind::Integer) && target_model::is_aarch64(c_.program.target) &&
			one_reader(inst.result))
		{
			const auto& next = body_.instructions[inst.next];
			const uint64_t mask = body_.literals[inst.ops[2].index].low;
			const Value shuffled = Value::temp(inst.result);
			if (mask <= 255 && mask % 0x55 == 0 && next.op == Opcode::Vector &&
				next.sub == static_cast<unsigned>(VectorOp::Pmulld) &&
				((next.ops[0] == shuffled) != (next.ops[1] == shuffled)))
				return false;
		}
		return !calls;
	default:
		return false;
	}
}

// In a loop with calls, the cheap computations a costly one needs to leave
// it: each reads only what the loop does not compute or other values that
// could leave, and a costly instruction that could leave reads it (through
// others of them). They leave with it: a cheap value alone stays, since it
// would only be held across the calls.
void LoopMotion::find_feeders(uint32_t loop)
{
	for (TempId t : fed_)
		feeds_[t] = 0;
	fed_.clear();
	if (feeds_.size() < body_.temps.size())
		feeds_.resize(body_.temps.size(), 0);
	const Loop& l = forest_.loops[loop];
	const auto plain = [&](const Instruction& inst) {
		const bool kind = inst.op == Opcode::Binary || inst.op == Opcode::Compare || inst.op == Opcode::Copy ||
			inst.op == Opcode::Unary || inst.op == Opcode::Select || inst.op == Opcode::Convert;
		return kind && inst.result != kNoId && !inst.type.is_object() && inst.type.kind() != TypeKind::F80 &&
			inst.type.kind() != TypeKind::I128 &&
			!(inst.op == Opcode::Convert && convert_source_type(inst).kind() == TypeKind::I128) && speculable(inst);
	};
	for (bool changed = true; changed;)
	{
		changed = false;
		for (BlockId b : l.blocks)
			for (InstId i = body_.blocks[b].first; i != kNoId; i = body_.instructions[i].next)
			{
				const Instruction& inst = body_.instructions[i];
				if (!plain(inst) || feeds_[inst.result] != 0)
					continue;
				bool leaves = true;
				for_each_operand(body_, inst, [&](const Value& v) {
					leaves = leaves && (invariant(loop, v) || (v.index < feeds_.size() && feeds_[v.index] != 0));
				});
				if (!leaves)
					continue;
				feeds_[inst.result] = 1;
				fed_.push_back(inst.result);
				changed = true;
			}
	}
	// (fed_ has definitions before their readers.)
	for (size_t k = fed_.size(); k-- > 0;)
	{
		const Instruction& inst = body_.instructions[body_.temps[fed_[k]].def];
		if (!costly(inst) && (feeds_[fed_[k]] & 2) == 0)
			continue;
		for_each_operand(body_, inst, [&](const Value& v) {
			if (v.is(ValueKind::Temp) && v.index < feeds_.size() && feeds_[v.index] != 0)
				feeds_[v.index] |= 2;
		});
	}
}

// Whether `v` is an inner step of a chain: computed in `block` by the
// family's operation on `type`, for its one reader alone.
bool LoopMotion::chain_link(Value v, BinaryOp family, Type type, BlockId block)
{
	if (!v.is(ValueKind::Temp) || v.index < body_.parameter_count)
		return false;
	const InstId def = body_.temps[v.index].def;
	BinaryOp op = BinaryOp::Add;
	if (def == kNoId || c_.block_of[def] != block || !chain_family(body_.instructions[def], op) || op != family ||
		body_.instructions[def].type != type)
		return false;
	return one_reader(v.index);
}

// A reader in the loop extends the chain's value: an index, an integer
// extension, or the phi of a counter that steps by the chain.
bool LoopMotion::widened(uint32_t loop, InstId root) const
{
	const Value value = Value::temp(body_.instructions[root].result);
	const bool step = variants_.size() == 1 && variants_[0].value.is(ValueKind::Temp);
	for (BlockId b : forest_.loops[loop].blocks)
		for (InstId i = body_.blocks[b].first; i != kNoId; i = body_.instructions[i].next)
		{
			const Instruction& inst = body_.instructions[i];
			const auto op = static_cast<ConvertOp>(inst.sub);
			if ((inst.op == Opcode::Index && inst.ops[1] == value) ||
				(inst.op == Opcode::Convert && inst.ops[0] == value &&
					(op == ConvertOp::SExt || op == ConvertOp::ZExt)))
				return true;
			if (inst.op == Opcode::Phi && step && inst.result == variants_[0].value.index)
				for (uint32_t k = 1; k < inst.extra_count; k += 2)
					if (body_.operands[inst.extra_first + k] == value)
						return true;
		}
	return false;
}

// Reads the chain `root` ends into terms_ (its operands, left to right,
// each marked when it is subtracted) and links_ (its instructions, the
// root first), marking them read. False beyond kChainTerms operands, and
// when its invariant operands do not need regrouping: fewer than two,
// literals only (value simplification folds those), or nothing else, and
// when a narrow value that widens would lose its steps' no-wrap facts.
bool LoopMotion::read_chain(uint32_t loop, InstId root, BlockId block)
{
	constexpr size_t kChainTerms = 16;
	const Instruction& last = body_.instructions[root];
	BinaryOp family = BinaryOp::Add;
	chain_family(last, family);
	terms_.clear();
	links_.clear();
	std::pair<InstId, bool> stack[kChainTerms];
	size_t depth = 0;
	stack[depth++] = { root, false };
	bool fits = true;
	while (depth > 0)
	{
		const auto at = stack[--depth];
		const Instruction& inst = body_.instructions[at.first];
		read_[at.first] = walk_;
		links_.push_back(at.first);
		const bool subtract = static_cast<BinaryOp>(inst.sub) == BinaryOp::Sub;
		// (The right operand first: the stack then yields the left one.)
		for (unsigned side = 2; side-- > 0;)
		{
			const bool negative = at.second != (side == 1 && subtract);
			const Value v = inst.ops[side];
			if (fits && chain_link(v, family, last.type, block) && depth < kChainTerms)
				stack[depth++] = { body_.temps[v.index].def, negative };
			else
				terms_.push_back({ v, negative });
			fits = fits && terms_.size() + depth <= kChainTerms;
		}
	}
	if (!fits)
		return false;
	std::reverse(terms_.begin(), terms_.end());
	invariants_.clear();
	variants_.clear();
	bool named = false;
	for (const Term& term : terms_)
	{
		const bool fixed = invariant(loop, term.value);
		(fixed ? invariants_ : variants_).push_back(term);
		named = named || (fixed && !term.value.is_literal());
	}
	// (Two invariants that one instruction combines already leave the loop.)
	if (invariants_.size() < 2 || variants_.empty() || !named || links_.size() < 2)
		return false;
	// A regrouped step cannot keep a no-wrap fact (the invariants' own sum
	// may wrap). A narrow value that widens needs it, or it would extend on
	// every pass.
	bool no_wrap = false;
	for (InstId link : links_)
		no_wrap = no_wrap || (body_.instructions[link].flags & IF_NO_WRAP) != 0;
	return !no_wrap || last.type.kind() == TypeKind::I64 || !widened(loop, root);
}

// Folds `terms` left to right with the chain's next instructions, starting
// at the first term that is added. Returns the result; `negative` when all
// of the terms are subtracted (the result is then their sum).
Value LoopMotion::combine(const std::vector<Term>& terms, BinaryOp family, size_t& next, bool& negative)
{
	size_t first = 0;
	while (first < terms.size() && terms[first].negative)
		++first;
	negative = first == terms.size();
	if (negative)
		first = 0;
	Value sum = terms[first].value;
	for (size_t k = 0; k < terms.size(); ++k)
	{
		if (k == first)
			continue;
		const InstId id = links_[next++];
		Instruction& step = body_.instructions[id];
		step.sub = static_cast<uint8_t>(terms[k].negative && !negative ? BinaryOp::Sub : family);
		step.ops[0] = sum;
		step.ops[1] = terms[k].value;
		step.flags &= static_cast<uint8_t>(~(IF_NO_WRAP | IF_EXACT));
		order_.push_back(id);
		sum = Value::temp(step.result);
	}
	return sum;
}

// A chain of one integer operation in a loop (x + v1 + v2, x * v1 * v2,
// each step's result read only by the next) whose operands include two or
// more values the loop does not compute: those combine first, and their
// combination then leaves the loop (x + (v1 + v2)). The chain keeps its
// instructions, placed just before its root (its last one): the invariants'
// combination, then the loop's own operands in their order, the
// combination joining the first of them. Regrouped steps lose their
// no-wrap facts. Without `apply`: whether any chain would change.
bool LoopMotion::reassociate(uint32_t loop, bool calls, bool apply)
{
	bool any = false;
	for (BlockId b : forest_.loops[loop].blocks)
	{
		// A chain's steps mark the links they read; the steps with a link
		// that no later step reads are the roots.
		++walk_;
		block_insts_.clear();
		for (InstId i = body_.blocks[b].first; i != kNoId; i = body_.instructions[i].next)
		{
			const Instruction& inst = body_.instructions[i];
			BinaryOp family = BinaryOp::Add;
			if (!chain_family(inst, family))
				continue;
			bool linked = false;
			for (unsigned side = 0; side < 2; ++side)
				if (chain_link(inst.ops[side], family, inst.type, b))
				{
					read_[body_.temps[inst.ops[side].index].def] = walk_;
					linked = true;
				}
			if (linked)
				block_insts_.push_back(i);
		}
		order_.clear();
		chains_.clear();
		for (size_t k = block_insts_.size(); k-- > 0;)
		{
			const InstId i = block_insts_[k];
			BinaryOp family = BinaryOp::Add;
			if (read_[i] == walk_ || !worth_moving(body_.instructions[i], calls) || !read_chain(loop, i, b))
				continue;
			chain_family(body_.instructions[i], family);
			if (!apply)
				return true;
			// The root last: it keeps its result for its readers.
			std::rotate(links_.begin(), links_.begin() + 1, links_.end());
			const size_t start = order_.size();
			size_t next = 0;
			bool negative = false;
			const Value fixed = combine(invariants_, family, next, negative);
			variants_.insert(variants_.begin() + 1, Term{ fixed, negative });
			combine(variants_, family, next, negative);
			chains_.push_back({ start, order_.size() });
			for (size_t at = start; at < order_.size(); ++at)
				chain_[order_[at]] = static_cast<uint32_t>(chains_.size());
		}
		if (chains_.empty())
			continue;
		any = true;
		// The block again, each regrouped chain's steps just before its root
		// (the last of them).
		InstId prev = kNoId;
		auto append = [&](InstId i) {
			if (prev == kNoId)
				body_.blocks[b].first = i;
			else
				body_.instructions[prev].next = i;
			prev = i;
		};
		for (InstId i = body_.blocks[b].first, after = kNoId; i != kNoId; i = after)
		{
			after = body_.instructions[i].next;
			const uint32_t chain = chain_[i];
			if (chain == 0)
				append(i);
			else if (order_[chains_[chain - 1].second - 1] == i)
				for (size_t at = chains_[chain - 1].first; at < chains_[chain - 1].second; ++at)
					append(order_[at]);
		}
		body_.instructions[prev].next = kNoId;
		body_.blocks[b].last = prev;
		for (InstId i : order_)
			chain_[i] = 0;
	}
	return any;
}

// Moves what the loop's blocks hold to its preheader, repeating while a move
// frees another. Without `apply`: whether anything would move.
bool LoopMotion::sweep(uint32_t loop, bool apply)
{
	const Loop& l = forest_.loops[loop];
	if (apply && l.preheader == kNoId)
		return false;
	bool calls = false;
	for (BlockId b : l.blocks)
		for (InstId i = body_.blocks[b].first; i != kNoId && !calls; i = body_.instructions[i].next)
			calls = body_.instructions[i].op == Opcode::Call || body_.instructions[i].op == Opcode::Asm;
	const bool feeders = calls && !c_.costs.locals_survive_calls;
	if (feeders)
		find_feeders(loop);
	const BlockId pre = l.preheader;
	InstId tail = apply ? previous_in_block(body_, pre, body_.blocks[pre].last) : kNoId;
	if (!apply && reassociate(loop, calls, false))
		return true;
	bool any = false;
	for (bool moved = true; moved;)
	{
		// (Chains regrouped now leave in this round.)
		moved = false;
		any = (apply && reassociate(loop, calls, true)) || any;
		for (BlockId b : l.blocks)
		{
			// Everything before a point of the header runs whenever the
			// preheader does (the header's instructions up to the first that
			// may leave).
			bool certain = b == l.header;
			InstId prev = kNoId;
			for (InstId i = body_.blocks[b].first; i != kNoId;)
			{
				const Instruction& inst = body_.instructions[i];
				const InstId next = inst.next;
				bool move = is_pure(inst) && inst.op != Opcode::Phi && !inst.type.is_object() &&
					inst.type.kind() != TypeKind::F80 &&
					(worth_moving(inst, calls) || (feeders && (feeds_[inst.result] & 2) != 0)) &&
					(certain || speculable(inst));
				if (move)
					for_each_operand(body_, inst, [&](const Value& v) { move = move && invariant(loop, v); });
				if (!move)
				{
					certain = certain && (is_pure(inst) || inst.op == Opcode::Load || inst.op == Opcode::Store);
					prev = i;
					i = next;
					continue;
				}
				if (!apply)
					return true;
				unlink_after(body_, b, prev, i);
				insert_after(body_, pre, tail, i);
				if (!certain)
				{
					body_.instructions[i].flags &= static_cast<uint8_t>(~(IF_NO_WRAP | IF_EXACT | IF_NONNULL));
					if (body_.instructions[i].op == Opcode::Index)
						body_.instructions[i].set_index_bounds(IndexBounds::None);
				}
				tail = i;
				c_.block_of[i] = pre;
				++c_.stats.invariants_hoisted;
				moved = any = true;
				i = next;
			}
		}
	}
	return any;
}

bool LoopMotion::run()
{
	c_.ensure_cfg();
	find_loops(body_, cfg_, forest_);
	if (forest_.loops.empty())
		return false;
	c_.entry_block_of(hoist_loop_invariants);
	note_register_uses();
	uses_.clear();
	read_.assign(body_.instructions.size(), 0);
	chain_.assign(body_.instructions.size(), 0);
	// A preheader for each loop with something to move, then the loops are
	// found again. (A loop's preheader changes only its header's entry edge.)
	bool changed = false;
	for (unsigned round = 0; round < 16; ++round)
	{
		bool one = false;
		for (uint32_t loop = 0; loop < forest_.loops.size(); ++loop)
			one = (forest_.loops[loop].preheader == kNoId && sweep(loop, false) &&
					  make_loop_preheader(body_, cfg_, forest_, loop)) ||
				one;
		if (!one)
			break;
		changed = true;
		c_.invalidate_cfg();
		c_.ensure_cfg();
		find_loops(body_, cfg_, forest_);
		map_instruction_blocks(body_, c_.block_of);
		c_.invalidate_block_of();
		uses_.clear();
		read_.resize(body_.instructions.size(), 0);
		chain_.resize(body_.instructions.size(), 0);
	}
	// Innermost first: what moves into an inner preheader may leave the loop
	// around it too.
	for (uint32_t loop = static_cast<uint32_t>(forest_.loops.size()); loop-- > 0;)
		changed = sweep(loop, true) || changed;
	return changed;
}

} // namespace

bool hoist_loop_invariants(FunctionContext& context)
{
	LoopMotion pass(context);
	return pass.run();
}

} // namespace opt
