#include <vector>

#include "lowir/builder.h"
#include "opt/loops.h"
#include "opt/passes.h"
#include "opt/scalar_fold.h"

namespace opt {

using namespace lowir;

namespace {

// The most passes one group runs, and the most instructions a group holds
// (its passes' own, then the group's test and branch).
const unsigned kGroupPasses = 8;
const unsigned kGroupSize = 28;
// The greatest step of a tested induction (bytes of a pointer).
const int64_t kGroupStep = int64_t(1) << 16;

// O2+, after the passes that read a loop's shape: a short straight loop that
// leaves when an induction meets a bound runs its passes in groups. Before
// the loop, a group loop runs U passes with one test, as long as U passes
// remain: the distance to the bound, less what is short of a whole group, is
// its range (the induction steps by a power of two, so the range is one
// `and`). The loop itself then runs the passes left, fewer than U; with
// fewer than U from the start the group loop does not run.
//
// The loop is a chain of blocks, tested by its header on an induction's phi
// (`while`) or by its latch (`do`), with no call. It stays while the
// induction differs from the bound, or while it is before the bound in the
// direction it steps (then the bound is first tested as the loop would). The
// test may read the induction truncated (an `int` counter kept in 64 bits):
// the distances are then the narrow type's.
//
// In a group, an induction's value in each pass is the group's first plus
// the steps so far (an address's displacement, with no step between).
class ShortLoopUnrolling
{
public:
	explicit ShortLoopUnrolling(FunctionContext& context) : c_(context), body_(context.body), cfg_(context.cfg) {}
	bool run();

private:
	struct Shape
	{
		BlockId pre = kNoId, header = kNoId, latch = kNoId, exit = kNoId;
		bool bottom = false;   // tested by the latch
		bool stepped = false;  // the test reads the stepped value (a latch may read either)
		bool narrowed = false; // the test reads the value truncated, to `narrow`
		Type narrow;
		InstId compare = kNoId; // the test, or none: the branch reads `truth` itself (`!= 0`)
		Value truth;
		bool stays_true = false;
		ComparePredicate stays = ComparePredicate::Ne; // `induction stays bound`: the loop goes on
		TempId iv = kNoId;                             // the tested induction's phi
		Value init, bound, next;                       // its first value, the bound, its stepped value
		int64_t step = 0;                              // bytes of a pointer, units of an integer
		unsigned passes = 0;                           // per group
		std::vector<BlockId> chain;                    // header first
	};

	bool in(uint32_t loop, Value v) const
	{
		if (!v.is(ValueKind::Temp) || v.index < body_.parameter_count)
			return false;
		// (An instruction this pass made stands outside every loop found.)
		const InstId def = body_.temps[v.index].def;
		return def != kNoId && def < block_of_.size() && in_loop(forest_, loop, block_of_[def]);
	}
	bool wanted(uint32_t loop) const;
	bool chain(uint32_t loop, Shape& s) const;
	bool tested(uint32_t loop, Shape& s) const;
	bool sized(Shape& s) const;
	void rewrite(const Shape& s);
	void join_exit(const Shape& s, BlockId left);
	Value emit(BlockId block, InstId& after, Instruction inst, Type result, bool placed = false);
	Value mapped(Value v) const
	{
		return v.is(ValueKind::Temp) && v.index < map_.size() && !map_[v.index].is(ValueKind::None) ? map_[v.index] : v;
	}

	FunctionContext& c_;
	FunctionBody& body_;
	Cfg& cfg_;
	LoopForest forest_;
	std::vector<BlockId> block_of_;
	std::vector<uint32_t> uses_;
	std::vector<uint8_t> escapes_; // per temp: read outside the loop that defines it
	std::vector<Value> map_;       // per temp of the loop: its value in the pass being copied
	std::vector<uint8_t> member_;  // per block: of the loop being rewritten
	uint32_t location_ = 0;
};

// A loop the pass takes once it has a preheader (any one of its entries
// shows what the others bring too: a value of outside the loop).
bool ShortLoopUnrolling::wanted(uint32_t loop) const
{
	const Loop& l = forest_.loops[loop];
	Shape s;
	for (uint32_t k = cfg_.pred_first[l.header]; k < cfg_.pred_first[l.header + 1] && s.pre == kNoId; ++k)
		if (!in_loop(forest_, loop, cfg_.preds[k]))
			s.pre = cfg_.preds[k];
	return l.preheader == kNoId && chain(loop, s) && tested(loop, s) && sized(s);
}

// The loop's blocks as one chain from the header to the latch, left by one
// branch: the header's (then the latch only jumps) or the latch's. (`s.pre`
// is the preheader, or the entry given for a loop with none.)
bool ShortLoopUnrolling::chain(uint32_t loop, Shape& s) const
{
	const Loop& l = forest_.loops[loop];
	if (l.preheader != kNoId)
		s.pre = l.preheader;
	if (l.latches.size() != 1 || s.pre == kNoId)
		return false;
	s.header = l.header;
	s.latch = l.latches[0];
	// (Not a loop this pass or the fill widening made, nor what one left.)
	if (body_.blocks[s.header].kind == BlockKind::Unrolled || body_.blocks[s.pre].kind == BlockKind::WideTail)
		return false;
	// (Nor the passes a lane loop left, fewer than its lanes: entered from
	// the lane loop, or from its exit block.)
	for (uint32_t k = cfg_.pred_first[s.pre]; k < cfg_.pred_first[s.pre + 1]; ++k)
	{
		const BlockId p = cfg_.preds[k];
		if (body_.blocks[p].kind == BlockKind::WideLoop)
			return false;
		for (uint32_t j = cfg_.pred_first[p]; body_.blocks[p].kind == BlockKind::LoopExit && j < cfg_.pred_first[p + 1];
			++j)
			if (body_.blocks[cfg_.preds[j]].kind == BlockKind::WideLoop)
				return false;
	}
	auto last = [&](BlockId b) -> const Instruction& { return body_.instructions[body_.blocks[b].last]; };
	s.bottom = s.header == s.latch || last(s.header).op != Opcode::Branch;
	const BlockId exiting = s.bottom ? s.latch : s.header;
	const Instruction& branch = last(exiting);
	if (branch.op != Opcode::Branch || !branch.ops[0].is(ValueKind::Temp))
		return false;
	const BlockId on_true = branch.ops[1].index, on_false = branch.ops[2].index;
	s.stays_true = in_loop(forest_, loop, on_true);
	if (s.stays_true == in_loop(forest_, loop, on_false))
		return false;
	s.exit = s.stays_true ? on_false : on_true;
	const BlockId stays = s.stays_true ? on_true : on_false;
	if (s.bottom && stays != s.header)
		return false;
	s.chain.clear();
	for (BlockId b = s.header;;)
	{
		if (forest_.innermost[b] != loop || s.chain.size() == l.blocks.size())
			return false;
		s.chain.push_back(b);
		if (b == s.latch)
			break;
		BlockId next = stays;
		if (b != exiting)
		{
			if (last(b).op != Opcode::Jump)
				return false;
			next = last(b).ops[0].index;
		}
		if (next == s.header || cfg_.pred_first[next + 1] - cfg_.pred_first[next] != 1)
			return false;
		b = next;
	}
	if (s.chain.size() != l.blocks.size() || (!s.bottom && last(s.latch).op != Opcode::Jump))
		return false;
	// The test: a compare the branch alone reads, or the value the branch
	// reads itself.
	const InstId compare = body_.temps[branch.ops[0].index].def;
	if (compare == kNoId || branch.ops[0].index < body_.parameter_count)
		return false;
	s.truth = branch.ops[0];
	s.compare = kNoId;
	if (body_.instructions[compare].op != Opcode::Compare)
		return body_.temps[s.truth.index].type.is_integer() || body_.temps[s.truth.index].type.is_pointer();
	if (block_of_[compare] != exiting || uses_[branch.ops[0].index] != 1)
		return false;
	s.compare = compare;
	return true;
}

// The test's induction: a header phi from the preheader's value, stepped once
// per pass by a power of two, compared with a value of outside the loop. A
// header tests the phi; a latch the stepped value, or the phi for difference.
bool ShortLoopUnrolling::tested(uint32_t loop, Shape& s) const
{
	// (A value the branch reads itself is compared with zero.)
	Instruction cmp = make_instruction(Opcode::Compare, body_.temps[s.truth.index].type);
	cmp.sub = static_cast<uint8_t>(ComparePredicate::Ne);
	cmp.ops[0] = s.truth;
	cmp.ops[1] = integer_value(body_, Literal{ 0, 0 });
	if (s.compare != kNoId)
		cmp = body_.instructions[s.compare];
	for (unsigned side = 0; side < 2; ++side)
	{
		Value value = cmp.ops[side];
		const Value bound = cmp.ops[1 - side];
		if (!value.is(ValueKind::Temp) || in(loop, bound) || !in(loop, value))
			continue;
		const Instruction* def = &body_.instructions[body_.temps[value.index].def];
		const bool narrowed = def->op == Opcode::Convert && static_cast<ConvertOp>(def->sub) == ConvertOp::Trunc &&
			def->type == cmp.type && in(loop, def->ops[0]);
		if (narrowed)
		{
			value = def->ops[0];
			def = &body_.instructions[body_.temps[value.index].def];
		}
		const bool stepped = def->op != Opcode::Phi;
		if (stepped)
		{
			if (!s.bottom || !def->ops[0].is(ValueKind::Temp) || !in(loop, def->ops[0]))
				continue;
			def = &body_.instructions[body_.temps[def->ops[0].index].def];
		}
		const Instruction& phi = *def;
		if (phi.op != Opcode::Phi || phi.extra_count != 4 || block_of_[body_.temps[phi.result].def] != s.header)
			continue;
		const Value next = phi_incoming(body_, phi, s.latch);
		if (!next.is(ValueKind::Temp) || !in(loop, next) || (stepped && next.index != value.index))
			continue;
		const Instruction& step = body_.instructions[body_.temps[next.index].def];
		if (!step.ops[0].is(ValueKind::Temp) || step.ops[0].index != phi.result || !step.ops[1].is(ValueKind::Integer))
			continue;
		int64_t by = 0;
		if (step.op == Opcode::Index && phi.type.is_pointer() && !narrowed)
		{
			const int64_t count = static_cast<int64_t>(body_.literals[step.ops[1].index].low);
			const int64_t size = static_cast<int64_t>(storage_bytes(step.type));
			if (count > kGroupStep || count < -kGroupStep || size > kGroupStep)
				continue;
			by = count * size;
		}
		else if (step.op == Opcode::Binary && step.type == phi.type && (narrowed || cmp.type == phi.type) &&
			phi.type.is_integer() && phi.type.kind() != TypeKind::I128 && cmp.type.is_integer() &&
			lowir::integer_bits(cmp.type.kind()) >= 32 &&
			(static_cast<BinaryOp>(step.sub) == BinaryOp::Add || static_cast<BinaryOp>(step.sub) == BinaryOp::Sub))
		{
			by = static_cast<int64_t>(normalize_integer(step.type, body_.literals[step.ops[1].index]).low);
			// (An unsigned type's literal reads as the step it wraps to.)
			const unsigned bits = lowir::integer_bits(step.type.kind());
			if (bits < 64)
				by = static_cast<int64_t>(static_cast<uint64_t>(by) << (64 - bits)) >> (64 - bits);
			if (static_cast<BinaryOp>(step.sub) == BinaryOp::Sub)
				by = -by;
		}
		const int64_t size = by < 0 ? -by : by;
		if (size == 0 || size > kGroupStep || (size & (size - 1)) != 0)
			continue;
		// It stays while the induction differs from the bound, or is before
		// it (the value the test reads: the stepped one's order alone).
		ComparePredicate stays = static_cast<ComparePredicate>(cmp.sub);
		stays = s.stays_true ? stays : inverted(stays);
		stays = side == 0 ? stays : swapped(stays);
		const bool before = by > 0 ? stays == ComparePredicate::Lt || stays == ComparePredicate::Ult
								   : stays == ComparePredicate::Gt || stays == ComparePredicate::Ugt;
		if (stays != ComparePredicate::Ne && !(before && (stepped || !s.bottom)))
			continue;
		s.stays = stays;
		s.stepped = stepped;
		s.narrowed = narrowed;
		s.narrow = cmp.type;
		s.iv = phi.result;
		s.init = phi_incoming(body_, phi, s.pre);
		s.bound = bound;
		s.next = next;
		s.step = by;
		return true;
	}
	return false;
}

// What each pass copies is plain (no call, no object over two words moved
// whole), and U passes fit a group. Phis stand only in the header, each for the
// preheader's value and the latch's.
bool ShortLoopUnrolling::sized(Shape& s) const
{
	unsigned size = 0;
	for (BlockId b : s.chain)
		for (InstId i = body_.blocks[b].first; i != kNoId; i = body_.instructions[i].next)
		{
			const Instruction& inst = body_.instructions[i];
			if (inst.op == Opcode::Phi)
			{
				if (b != s.header || inst.extra_count != 4)
					return false;
				continue;
			}
			if (i == s.compare || i == body_.blocks[b].last)
				continue;
			// (An object of at most two words zeroed or copied whole: a
			// store a word, with a load for a copy.)
			if ((inst.op == Opcode::ZeroInit || inst.op == Opcode::CopyObject) && inst.extra_count == 0 &&
				inst.immediate <= 16)
			{
				size += (inst.immediate > 8 ? 2 : 1) * (inst.op == Opcode::CopyObject ? 2 : 1);
				continue;
			}
			if (inst.extra_count != 0 || inst.type.is_object())
				return false;
			switch (inst.op)
			{
			case Opcode::Copy:
			case Opcode::Addr:
				break;
			case Opcode::Index:
				size += inst.ops[1].is(ValueKind::Integer) ? 0 : 1;
				break;
			case Opcode::Convert:
				size += static_cast<ConvertOp>(inst.sub) == ConvertOp::Trunc ? 0 : 1;
				break;
			case Opcode::Load:
			case Opcode::Store:
			case Opcode::Binary:
			case Opcode::Unary:
			case Opcode::Compare:
			case Opcode::Select:
				++size;
				break;
			default:
				return false;
			}
		}
	if (size == 0)
		return false;
	s.passes = kGroupPasses;
	while (s.passes > 1 && size * s.passes + 2 > kGroupSize)
		s.passes /= 2;
	return s.passes > 1;
}

// (`placed`: a copy, which keeps its source's location.)
Value ShortLoopUnrolling::emit(BlockId block, InstId& after, Instruction inst, Type result, bool placed)
{
	if (!placed)
		inst.location = location_;
	inst.next = kNoId;
	const InstId id = append_instruction(body_, inst);
	if (!result.is_void())
		body_.instructions[id].result = append_temp(body_, result, id);
	insert_after(body_, block, after, id);
	after = id;
	return result.is_void() ? Value::none() : Value::temp(body_.instructions[id].result);
}

void ShortLoopUnrolling::rewrite(const Shape& s)
{
	const Type none = Type::scalar(TypeKind::Void), i64 = Type::scalar(TypeKind::I64);
	const Type type = body_.temps[s.iv].type;
	const bool pointer = type.is_pointer();
	// (A pointer's distances and its stop as the integers they are.)
	const Type word = pointer ? i64 : s.narrowed ? s.narrow : type;
	const bool ordered = s.stays != ComparePredicate::Ne;
	location_ = body_.instructions[body_.blocks[s.header].last].location;
	auto literal = [&](int64_t v) { return integer_value(body_, normalize_integer(word, integer_literal(v))); };
	auto binary = [&](BinaryOp op, Value a, Value b) {
		Instruction inst = make_instruction(Opcode::Binary, word);
		inst.sub = static_cast<uint8_t>(op);
		inst.ops[0] = a;
		inst.ops[1] = b;
		return inst;
	};
	auto compare = [&](ComparePredicate pred, Value a, Value b) {
		Instruction inst = make_instruction(Opcode::Compare, word);
		inst.sub = static_cast<uint8_t>(pred);
		inst.ops[0] = a;
		inst.ops[1] = b;
		return inst;
	};
	auto branch = [&](Value on, BlockId yes, BlockId no) {
		Instruction inst = make_instruction(Opcode::Branch, none);
		inst.ops[0] = on;
		inst.ops[1] = Value::block(yes);
		inst.ops[2] = Value::block(no);
		return inst;
	};
	auto jump = [&](BlockId to) {
		Instruction inst = make_instruction(Opcode::Jump, none);
		inst.ops[0] = Value::block(to);
		return inst;
	};
	// The induction's value as the test reads it.
	auto as_word = [&](BlockId b, InstId& after, Value v) {
		if (pointer)
		{
			Instruction copy = make_instruction(Opcode::Copy, i64);
			copy.ops[0] = v;
			return emit(b, after, copy, i64);
		}
		if (!s.narrowed)
			return v;
		if (v.is(ValueKind::Integer))
			return integer_value(body_, normalize_integer(word, body_.literals[v.index]));
		Instruction narrow = make_instruction(Opcode::Convert, word);
		narrow.sub = static_cast<uint8_t>(ConvertOp::Trunc);
		narrow.ops[0] = v;
		set_convert_source_type(narrow, type);
		return emit(b, after, narrow, word);
	};
	// An ordered loop that does not run at all has no distance: the bound
	// is tested first as the loop tests it (from zero the distance says it).
	const bool zero = s.init.is(ValueKind::Integer) && body_.literals[s.init.index].low == 0 &&
		body_.literals[s.init.index].high == 0 && s.step > 0;
	const bool guarded = ordered && !zero;
	const BlockId sized = guarded ? append_block(body_, BlockKind::LoopPreheader) : s.pre;
	const BlockId enter = append_block(body_, BlockKind::LoopPreheader),
				  group = append_block(body_, BlockKind::Unrolled);
	const BlockId left = s.bottom ? append_block(body_, BlockKind::LoopExit) : kNoId;
	const BlockId tail = append_block(body_, BlockKind::WideTail);
	const BlockId out = s.bottom ? left : group; // where the group loop leaves from
	InstId at = previous_in_block(body_, s.pre, body_.blocks[s.pre].last);
	const Value from = as_word(s.pre, at, s.init);
	Value to = pointer ? as_word(s.pre, at, s.bound) : s.bound;
	if (guarded)
	{
		const Value runs = emit(s.pre, at, compare(s.stays, from, to), i64);
		Instruction& entry = body_.instructions[body_.blocks[s.pre].last];
		entry = branch(runs, sized, tail);
		entry.location = location_;
		at = kNoId;
	}
	// The whole groups' range, and whether there is one. (A latch that
	// tests the phi leaves one step after the phi met the bound.)
	if (s.bottom && !s.stepped)
		to = emit(sized, at, binary(BinaryOp::Add, to, literal(s.step)), word);
	const uint64_t stride = static_cast<uint64_t>(s.step < 0 ? -s.step : s.step) * s.passes;
	const Value distance = emit(
		sized, at, s.step > 0 ? binary(BinaryOp::Sub, to, from) : binary(BinaryOp::Sub, from, to), word);
	const Value range = emit(sized, at, binary(BinaryOp::And, distance, literal(-static_cast<int64_t>(stride))), word);
	// (By a signed order, a distance past the type's half has no group.)
	const ComparePredicate some = s.stays == ComparePredicate::Lt ? ComparePredicate::Gt : ComparePredicate::Ne;
	const Value any = emit(sized, at, compare(some, range, literal(0)), i64);
	if (guarded)
		emit(sized, at, branch(any, enter, tail), none);
	else
	{
		Instruction& entry = body_.instructions[body_.blocks[s.pre].last];
		entry = branch(any, enter, tail);
		entry.location = location_;
	}
	// The group loop's preheader: where its induction stops.
	InstId e = kNoId;
	const Value stop = emit(enter, e, binary(s.step > 0 ? BinaryOp::Add : BinaryOp::Sub, from, range), word);
	emit(enter, e, jump(group), none);
	// The group loop: the header's phis, then the passes.
	std::vector<InstId> phis, group_phis;
	for (InstId i = body_.blocks[s.header].first; i != kNoId && body_.instructions[i].op == Opcode::Phi;
		i = body_.instructions[i].next)
		phis.push_back(i);
	map_.assign(body_.temps.size(), Value::none());
	member_.assign(body_.blocks.size(), 0);
	for (BlockId b : s.chain)
		member_[b] = 1;
	InstId g = kNoId;
	std::vector<Value> first(phis.size());
	// Each induction's step: the phi and a literal.
	std::vector<InstId> step_of(phis.size(), kNoId);
	for (size_t k = 0; k < phis.size(); ++k)
	{
		const Instruction phi = body_.instructions[phis[k]];
		first[k] = emit(group, g, make_instruction(Opcode::Phi, phi.type), phi.type);
		group_phis.push_back(g);
		map_[phi.result] = first[k];
		const Value next = phi_incoming(body_, phi, s.latch);
		if (!next.is(ValueKind::Temp) || next.index < body_.parameter_count || body_.temps[next.index].def == kNoId)
			continue;
		const InstId def = body_.temps[next.index].def;
		const Instruction& step = body_.instructions[def];
		const bool sum = step.op == Opcode::Binary && step.type == phi.type && phi.type.is_integer() &&
			phi.type.kind() != TypeKind::I128 && phi.type.kind() != TypeKind::I1 &&
			(static_cast<BinaryOp>(step.sub) == BinaryOp::Add || static_cast<BinaryOp>(step.sub) == BinaryOp::Sub);
		if (def < block_of_.size() && member_[block_of_[def]] && (sum || step.op == Opcode::Index) &&
			step.ops[0] == Value::temp(phi.result) && step.ops[1].is(ValueKind::Integer))
			step_of[k] = def;
	}
	// The tested induction's steps stay in its type between the first value
	// and the bound when the test's order is the type's (or it starts at
	// zero, unsigned, or signed below a positive bound).
	const bool is_signed = !is_unsigned(type.kind());
	const bool same_order = is_signed ? s.stays == ComparePredicate::Lt || s.stays == ComparePredicate::Gt
									  : s.stays == ComparePredicate::Ult || s.stays == ComparePredicate::Ugt;
	const bool held = !pointer && !s.narrowed &&
		(guarded ? same_order
				 : zero && (is_signed ? s.stays == ComparePredicate::Lt : s.stays != ComparePredicate::Lt));
	std::vector<Value> stepped(phis.size());
	for (unsigned pass = 0; pass < s.passes; ++pass)
	{
		if (pass != 0)
			for (size_t k = 0; k < phis.size(); ++k)
				map_[body_.instructions[phis[k]].result] = stepped[k];
		for (BlockId b : s.chain)
			for (InstId i = body_.blocks[b].first; i != kNoId; i = body_.instructions[i].next)
			{
				// (By value: a copy's instruction may move the instructions.)
				Instruction inst = body_.instructions[i];
				if (inst.op == Opcode::Phi || i == s.compare || i == body_.blocks[b].last)
					continue;
				size_t k = 0;
				while (k < phis.size() && step_of[k] != i)
					++k;
				if (k < phis.size())
				{
					// (A literal of an unsigned type reads as the step it wraps to.)
					const Type of = inst.op == Opcode::Index ? i64 : inst.type;
					const Literal by = normalize_integer(of, body_.literals[inst.ops[1].index]);
					inst.ops[0] = first[k];
					inst.ops[1] = integer_value(
						body_, normalize_integer(of, integer_literal(static_cast<int64_t>(by.low * (pass + 1)))));
					if (held && inst.result == s.next.index)
						inst.flags |= IF_NO_WRAP;
				}
				else
					for (unsigned o = 0; o < 3; ++o)
						inst.ops[o] = mapped(inst.ops[o]);
				const TempId result = inst.result;
				const Value copy = emit(group, g, inst, result == kNoId ? none : body_.temps[result].type, true);
				if (result != kNoId)
					map_[result] = copy;
			}
		for (size_t k = 0; k < phis.size(); ++k)
			stepped[k] = mapped(phi_incoming(body_, body_.instructions[phis[k]], s.latch));
	}
	const Value reached = as_word(group, g, mapped(s.next));
	const Value more = emit(group, g, compare(ComparePredicate::Ne, reached, stop), i64);
	emit(group, g, branch(more, group, s.bottom ? left : tail), none);
	for (size_t k = 0; k < phis.size(); ++k)
	{
		const Value pairs[4] = { Value::block(enter), phi_incoming(body_, body_.instructions[phis[k]], s.pre),
			Value::block(group), stepped[k] };
		body_.instructions[group_phis[k]].extra_first = append_operands(body_, pairs, 4);
		body_.instructions[group_phis[k]].extra_count = 4;
	}
	if (s.bottom)
		join_exit(s, left);
	// The loop's own preheader: each phi's value from the entry or the groups.
	InstId t = kNoId;
	rename_phi_incoming(body_, s.header, s.pre, tail);
	for (size_t k = 0; k < phis.size(); ++k)
	{
		const Type of = body_.instructions[phis[k]].type;
		const Value start = phi_incoming(body_, body_.instructions[phis[k]], tail);
		std::vector<Value> pairs = { Value::block(s.pre), start, Value::block(out), stepped[k] };
		if (guarded)
		{
			pairs.push_back(Value::block(sized));
			pairs.push_back(start);
		}
		const Value joined = emit(tail, t, make_instruction(Opcode::Phi, of), of);
		body_.instructions[t].extra_first = append_operands(body_, pairs.data(), static_cast<uint32_t>(pairs.size()));
		body_.instructions[t].extra_count = static_cast<uint32_t>(pairs.size());
		const Instruction& phi = body_.instructions[phis[k]];
		for (uint32_t o = 0; o < phi.extra_count; o += 2)
			if (body_.operands[phi.extra_first + o] == Value::block(tail))
				body_.operands[phi.extra_first + o + 1] = joined;
	}
	emit(tail, t, jump(s.header), none);
	std::vector<BlockId> layout;
	for (BlockId b : body_.layout)
	{
		layout.push_back(b);
		if (b != s.pre)
			continue;
		if (guarded)
			layout.push_back(sized);
		layout.push_back(enter);
		layout.push_back(group);
		if (s.bottom)
			layout.push_back(left);
		layout.push_back(tail);
	}
	body_.layout.swap(layout);
}

// A latch-tested loop is done when the groups reached the bound: `left`
// holds the latch's own test, of the last pass's values, and leaves to the
// exit as the latch does. The exit's phis take that edge's values; a value of
// the loop read after it otherwise is read through a phi the exit gets (the
// exit stands before every such reader: the loop leaves only to it).
void ShortLoopUnrolling::join_exit(const Shape& s, BlockId left)
{
	const Type none = Type::scalar(TypeKind::Void);
	InstId x = kNoId;
	Value on = mapped(s.truth);
	if (s.compare != kNoId)
	{
		Instruction test = body_.instructions[s.compare];
		for (unsigned k = 0; k < 2; ++k)
			test.ops[k] = mapped(test.ops[k]);
		on = emit(left, x, test, body_.temps[test.result].type);
	}
	Instruction leave = body_.instructions[body_.blocks[s.latch].last];
	const BlockId tail = body_.instructions[body_.blocks[s.pre].last].ops[2].index;
	leave.ops[0] = on;
	for (unsigned k = 1; k < 3; ++k)
		if (leave.ops[k].index == s.header)
			leave.ops[k] = Value::block(tail);
	emit(left, x, leave, none);
	for (InstId i = body_.blocks[s.exit].first; i != kNoId && body_.instructions[i].op == Opcode::Phi;
		i = body_.instructions[i].next)
	{
		const Instruction phi = body_.instructions[i];
		std::vector<Value> pairs(
			body_.operands.begin() + phi.extra_first, body_.operands.begin() + phi.extra_first + phi.extra_count);
		pairs.push_back(Value::block(left));
		pairs.push_back(mapped(phi_incoming(body_, phi, s.latch)));
		body_.instructions[i].extra_first = append_operands(body_, pairs.data(), static_cast<uint32_t>(pairs.size()));
		body_.instructions[i].extra_count = static_cast<uint32_t>(pairs.size());
	}
	// The values read after the loop, and a phi for each.
	std::vector<TempId> read;
	for (BlockId b : s.chain)
		for (InstId i = body_.blocks[b].first; i != kNoId; i = body_.instructions[i].next)
			if (body_.instructions[i].result != kNoId && escapes_[body_.instructions[i].result])
				read.push_back(body_.instructions[i].result);
	if (read.empty())
		return;
	std::vector<Value> through(body_.temps.size(), Value::none());
	std::vector<InstId> joins;
	for (TempId v : read)
	{
		const Type of = body_.temps[v].type;
		const InstId id = append_instruction(body_, make_instruction(Opcode::Phi, of));
		body_.instructions[id].result = append_temp(body_, of, id);
		body_.instructions[id].location = location_;
		through[v] = Value::temp(body_.instructions[id].result);
		joins.push_back(id);
	}
	auto outside = [&](BlockId b) { return b >= member_.size() || !member_[b]; };
	auto replaced = [&](Value& v) {
		if (v.is(ValueKind::Temp) && v.index < through.size() && !through[v.index].is(ValueKind::None))
			v = through[v.index];
	};
	for (BlockId b : body_.layout)
		for (InstId i = body_.blocks[b].first; i != kNoId; i = body_.instructions[i].next)
		{
			Instruction& inst = body_.instructions[i];
			if (inst.op != Opcode::Phi)
			{
				if (!outside(b))
					continue;
				for (unsigned k = 0; k < 3; ++k)
					replaced(inst.ops[k]);
				for (uint32_t k = 0; k < inst.extra_count; ++k)
					replaced(body_.operands[inst.extra_first + k]);
				continue;
			}
			for (uint32_t k = 1; k < inst.extra_count; k += 2)
				if (outside(body_.operands[inst.extra_first + k - 1].index))
					replaced(body_.operands[inst.extra_first + k]);
		}
	// (The exit's other predecessors come from after it: the value is the
	// phi's own.)
	InstId after = kNoId;
	for (size_t k = 0; k < read.size(); ++k)
	{
		std::vector<Value> pairs;
		for (uint32_t p = cfg_.pred_first[s.exit]; p < cfg_.pred_first[s.exit + 1]; ++p)
		{
			pairs.push_back(Value::block(cfg_.preds[p]));
			pairs.push_back(cfg_.preds[p] == s.latch ? Value::temp(read[k]) : through[read[k]]);
		}
		pairs.push_back(Value::block(left));
		pairs.push_back(mapped(Value::temp(read[k])));
		body_.instructions[joins[k]].extra_first = append_operands(
			body_, pairs.data(), static_cast<uint32_t>(pairs.size()));
		body_.instructions[joins[k]].extra_count = static_cast<uint32_t>(pairs.size());
		insert_after(body_, s.exit, after, joins[k]);
		after = joins[k];
	}
}

bool ShortLoopUnrolling::run()
{
	// (A loop to take is first given the preheader it lacks: the graph
	// changes, and the loops are found again.)
	bool changed = false;
	for (unsigned round = 0; round < 2; ++round)
	{
		c_.ensure_cfg();
		find_loops(body_, cfg_, forest_);
		if (forest_.loops.empty())
			return false;
		map_instruction_blocks(body_, block_of_);
		count_uses(body_, uses_);
		bool made = false;
		for (uint32_t loop = 0; round == 0 && loop < forest_.loops.size(); ++loop)
			made = (wanted(loop) && make_loop_preheader(body_, cfg_, forest_, loop)) || made;
		if (!made)
			break;
		changed = true;
		c_.invalidate_cfg();
		c_.invalidate_block_of();
		c_.invalidate_uses();
	}
	// Temporaries read outside the loop that defines them (a phi reads in
	// the block its value comes from).
	escapes_.assign(body_.temps.size(), 0);
	auto read = [&](Value v, BlockId at) {
		if (!v.is(ValueKind::Temp) || v.index < body_.parameter_count || body_.temps[v.index].def == kNoId)
			return;
		const uint32_t loop = forest_.innermost[block_of_[body_.temps[v.index].def]];
		if (loop != kNoId && !in_loop(forest_, loop, at))
			escapes_[v.index] = 1;
	};
	for (BlockId b : body_.layout)
		for (InstId i = body_.blocks[b].first; i != kNoId; i = body_.instructions[i].next)
		{
			const Instruction& inst = body_.instructions[i];
			for (unsigned k = 0; k < 3; ++k)
				read(inst.ops[k], b);
			for (uint32_t k = 0; k < inst.extra_count; ++k)
				if (inst.op != Opcode::Phi)
					read(body_.operands[inst.extra_first + k], b);
				else if (k % 2 == 1)
					read(body_.operands[inst.extra_first + k], body_.operands[inst.extra_first + k - 1].index);
		}
	// (The loops as found: a rewrite adds blocks no other loop holds, and
	// changes its own loop's preheader, the header's phis and the exit's.)
	const size_t loops = forest_.loops.size();
	for (uint32_t loop = 0; loop < loops; ++loop)
	{
		Shape s;
		if (!chain(loop, s) || !tested(loop, s) || !sized(s))
			continue;
		rewrite(s);
		changed = true;
	}
	if (!changed)
		return false;
	c_.invalidate_cfg();
	c_.invalidate_block_of();
	c_.invalidate_uses();
	return true;
}

} // namespace

bool unroll_short_loops(FunctionContext& context)
{
	return ShortLoopUnrolling(context).run();
}

} // namespace opt
