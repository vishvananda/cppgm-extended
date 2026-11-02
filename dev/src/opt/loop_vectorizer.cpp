#include "lowir/builder.h"
#include "opt/loops.h"
#include "opt/passes.h"
#include "opt/scalar_fold.h"

#include <algorithm>

namespace opt {

using namespace lowir;

namespace {

// The most instructions of a loop this pass reads, and the overlap tests one
// loop's vector form may wait on.
const size_t kBodyInstructions = 64;
const size_t kOverlapTests = 8;
// How deep a truncation's arithmetic is rewritten.
const unsigned kNarrowDepth = 6;

// Loop vectorization at O3: a loop of one block, whose count a counter and an
// invariant bound give, runs in sixteen-byte steps of the x86-64 baseline. The
// lanes hold what the scalar loop computes pass by pass, so each operation
// is the one on each lane. The widest value sets how many passes one wide pass
// does, and a narrower value keeps that many lanes at the bottom of its
// register. The vector loop stops with at least one pass left:
// the scalar loop then runs last, on every path, and so gives every value
// the code after it reads, and the vector loop only sets where it starts.
//
//   preheader:  enough passes?          (to the scalar loop if not)
//   guard:      the arrays apart?       (only when the types do not say)
//   vector_pre: lanes of what the loop does not change
//   wide_loop:  the lanes, the counter stepping by the lanes
//   exit:       reductions added across, then the scalar preheader
class LoopVectorizer
{
public:
	explicit LoopVectorizer(FunctionContext& context) : c_(context), body_(context.body), cfg_(context.cfg) {}

	bool run();

private:
	struct Induction
	{
		TempId phi, next;
		Value start;
		int64_t step;
		Type type;
		bool no_wrap;
	};
	struct Reduction
	{
		TempId phi, next;
		Value start, operand;
		BinaryOp op;
		Type type;
		bool ordered; // floating: the sum stays scalar, lane by lane in pass order
		Value next_sum;
	};
	// A value as offset + scale * pass + term_scale * term.
	struct Linear
	{
		int64_t offset = 0, scale = 0, term_scale = 0;
		Value term = Value::none();
	};
	struct Access
	{
		InstId inst;
		Type type;
		bool store, typed;
		Linear at;
		Value base, object;
		uint32_t group;
	};
	// The accesses of one base, apart by known offsets.
	struct Group
	{
		uint32_t lowest;
		int64_t low, high, scale;
	};

	bool in_body(Value v) const;
	bool narrow_fits(Value v, Type type, unsigned depth) const;
	Value narrowed(Value v, Type type, InstId& tail);
	void narrow_arithmetic();
	bool read_phis();
	bool read_exit();
	bool read_code();
	void mark(Value v, std::vector<uint8_t>& marks);
	void mark_uses();
	bool integer_term(Value v, Linear& out, int64_t factor, unsigned depth) const;
	bool address(Value v, Linear& out, Value& base, Value& object) const;
	bool read_accesses();
	bool supported(const Instruction& inst) const;
	bool choose_lanes();
	bool same_group(const Access& a, const Access& b) const;
	bool add_overlap_test(uint32_t a, uint32_t b);
	bool read_dependences();
	bool analyze(uint32_t loop);

	void at(BlockId block, InstId tail);
	Value emit(Instruction inst, Type type);
	void emit_void(const Instruction& inst);
	void finish_block(Instruction terminator);
	Value literal(int64_t value) { return integer_value(body_, integer_literal(value)); }
	Value vector(VectorOp op, Value a = Value::none(), Value b = Value::none(), int immediate = -1);
	Value compare(ComparePredicate predicate, Type type, Value a, Value b);
	Value binary(BinaryOp op, Type type, Value a, Value b, Type result);
	Value step_counter(Value v, int64_t by);
	Value convert(ConvertOp op, Type to, Type from, Value v);
	Value exact_scalar(Value v, Type from);
	Value broadcast(Value v, Type type);
	Value wide(Value v, Type type);
	Value scalar(Value v) const;
	Value widen_load(Value address, unsigned bytes, InstId origin);
	void widen_store(Value value, Value address, unsigned bytes, InstId origin);
	Value lane_address(Value address, Type type);
	Value extend(Value v, unsigned from, unsigned to, bool is_signed);
	Value narrow(Value v, unsigned from, unsigned to);
	void widen_convert(const Instruction& inst);
	Value widen_multiply(Value a, Value b);
	Value widen_mask(const Instruction& test, bool& swapped);
	void widen_binary(const Instruction& inst);
	void widen_unary(const Instruction& inst);
	void widen(const Instruction& inst, InstId origin);
	void clone_scalars(bool also_widen);
	Value overlap_guard(Value distance);
	Value add_across(const Reduction& r, Value v);
	Value add_in_order(const Reduction& r, Value sum, Value lanes);
	bool lanes_pay_for_order() const;
	bool halves(const Instruction& inst, unsigned& shift) const;
	void make_phi(BlockId block, Type type, std::initializer_list<Value> pairs, Value& out);
	BlockId place_block(BlockKind kind);
	void vectorize(uint32_t loop);

	FunctionContext& c_;
	FunctionBody& body_;
	Cfg& cfg_;
	LoopForest forest_;
	std::vector<uint8_t> done_; // per block: a loop this pass has handled

	// The loop being read.
	BlockId b_ = kNoId;
	std::vector<Induction> inductions_;
	std::vector<Reduction> reductions_;
	std::vector<InstId> code_;
	std::vector<Access> accesses_;
	std::vector<Group> groups_;
	std::vector<std::pair<uint32_t, uint32_t>> tests_;
	std::vector<uint8_t> in_scalar_, in_wide_; // per temp
	uint32_t counter_ = 0;
	bool backward_ = false; // accesses step down: the top lane holds the first pass
	Value bound_ = Value::none();
	int64_t bound_add_ = 0;
	bool unsigned_counter_ = false;
	unsigned lane_bytes_ = 0, lanes_ = 0;

	// The vector form being written.
	BlockId block_ = kNoId, vector_pre_ = kNoId;
	InstId tail_ = kNoId, pre_tail_ = kNoId;
	std::vector<Value> scalars_, wides_; // per temp of the loop
	std::vector<std::pair<std::pair<Value, TypeKind>, Value>> broadcasts_;
};

unsigned lane_bytes(Type type)
{
	switch (type.kind())
	{
	case TypeKind::I8:
	case TypeKind::U8:
		return 1;
	case TypeKind::I16:
	case TypeKind::U16:
		return 2;
	case TypeKind::I32:
	case TypeKind::U32:
	case TypeKind::F32:
		return 4;
	case TypeKind::I64:
	case TypeKind::F64:
		return 8;
	default:
		return 0;
	}
}

// The operation on each lane of a binary operator, or false.
bool lane_operation(BinaryOp op, Type type, VectorOp& out)
{
	const unsigned bytes = lane_bytes(type);
	if (type.is_float())
	{
		const bool wide_lanes = bytes == 8;
		switch (op)
		{
		case BinaryOp::Add:
			out = wide_lanes ? VectorOp::Addpd : VectorOp::Addps;
			return true;
		case BinaryOp::Sub:
			out = wide_lanes ? VectorOp::Subpd : VectorOp::Subps;
			return true;
		case BinaryOp::Mul:
			out = wide_lanes ? VectorOp::Mulpd : VectorOp::Mulps;
			return true;
		case BinaryOp::Div:
			out = wide_lanes ? VectorOp::Divpd : VectorOp::Divps;
			return true;
		default:
			return false;
		}
	}
	switch (op)
	{
	case BinaryOp::Add:
		out = bytes == 1 ? VectorOp::Paddb
			: bytes == 2 ? VectorOp::Paddw
			: bytes == 4 ? VectorOp::Paddd
						 : VectorOp::Paddq;
		return true;
	case BinaryOp::Sub:
		out = bytes == 1 ? VectorOp::Psubb
			: bytes == 2 ? VectorOp::Psubw
			: bytes == 4 ? VectorOp::Psubd
						 : VectorOp::Psubq;
		return true;
	case BinaryOp::And:
		out = VectorOp::Pand;
		return true;
	case BinaryOp::Or:
		out = VectorOp::Por;
		return true;
	case BinaryOp::Xor:
		out = VectorOp::Pxor;
		return true;
	case BinaryOp::Mul:
		out = VectorOp::Pmullw;
		return bytes == 2;
	default:
		return false;
	}
}

// The lane shift by an immediate of a shift operator, or false.
bool lane_shift(BinaryOp op, Type type, VectorOp& out)
{
	const unsigned bytes = lane_bytes(type);
	if (type.is_float() || bytes < 2)
		return false;
	switch (op)
	{
	case BinaryOp::Shl:
		out = bytes == 2 ? VectorOp::Psllwi : bytes == 4 ? VectorOp::Pslldi : VectorOp::Psllqi;
		return true;
	case BinaryOp::UShr:
		out = bytes == 2 ? VectorOp::Psrlwi : bytes == 4 ? VectorOp::Psrldi : VectorOp::Psrlqi;
		return true;
	case BinaryOp::Shr:
		out = bytes == 2 ? VectorOp::Psrawi : VectorOp::Psradi;
		return bytes <= 4;
	default:
		return false;
	}
}

bool named(Value object)
{
	return object.is(ValueKind::Slot) || object.is(ValueKind::Symbol);
}

ComparePredicate swapped_operands(ComparePredicate p)
{
	switch (p)
	{
	case ComparePredicate::Lt:
		return ComparePredicate::Gt;
	case ComparePredicate::Le:
		return ComparePredicate::Ge;
	case ComparePredicate::Gt:
		return ComparePredicate::Lt;
	case ComparePredicate::Ge:
		return ComparePredicate::Le;
	case ComparePredicate::Ult:
		return ComparePredicate::Ugt;
	case ComparePredicate::Ule:
		return ComparePredicate::Uge;
	case ComparePredicate::Ugt:
		return ComparePredicate::Ult;
	case ComparePredicate::Uge:
		return ComparePredicate::Ule;
	default:
		return p;
	}
}

ComparePredicate negated(ComparePredicate p)
{
	switch (p)
	{
	case ComparePredicate::Eq:
		return ComparePredicate::Ne;
	case ComparePredicate::Ne:
		return ComparePredicate::Eq;
	case ComparePredicate::Lt:
		return ComparePredicate::Ge;
	case ComparePredicate::Le:
		return ComparePredicate::Gt;
	case ComparePredicate::Gt:
		return ComparePredicate::Le;
	case ComparePredicate::Ge:
		return ComparePredicate::Lt;
	case ComparePredicate::Ult:
		return ComparePredicate::Uge;
	case ComparePredicate::Ule:
		return ComparePredicate::Ugt;
	case ComparePredicate::Ugt:
		return ComparePredicate::Ule;
	default:
		return ComparePredicate::Ult;
	}
}

bool LoopVectorizer::in_body(Value v) const
{
	if (!v.is(ValueKind::Temp) || v.index < body_.parameter_count)
		return false;
	const InstId def = body_.temps[v.index].def;
	return def != kNoId && c_.block_of[def] == b_;
}

// Whether the low bytes of `v` can be computed in `type`: wide arithmetic only
// a truncation reads takes narrower lanes, and more of them.
bool LoopVectorizer::narrow_fits(Value v, Type type, unsigned depth) const
{
	if (v.is(ValueKind::Integer))
		return true;
	if (!in_body(v) || depth == 0)
		return false;
	const Instruction& def = body_.instructions[body_.temps[v.index].def];
	const unsigned bytes = lane_bytes(type);
	if (def.op == Opcode::Convert)
	{
		const ConvertOp op = static_cast<ConvertOp>(def.sub);
		const unsigned from = lane_bytes(convert_source_type(def));
		if (op == ConvertOp::SExt || op == ConvertOp::ZExt)
			return from >= bytes && (from == bytes || narrow_fits(def.ops[0], type, depth - 1));
		return op == ConvertOp::Trunc && narrow_fits(def.ops[0], type, depth - 1);
	}
	if (def.op != Opcode::Binary || !def.type.is_integer())
		return false;
	switch (static_cast<BinaryOp>(def.sub))
	{
	case BinaryOp::Mul:
		if (bytes != 2)
			return false;
		break;
	case BinaryOp::Add:
	case BinaryOp::Sub:
	case BinaryOp::And:
	case BinaryOp::Or:
	case BinaryOp::Xor:
		break;
	default:
		return false;
	}
	return narrow_fits(def.ops[0], type, depth - 1) && narrow_fits(def.ops[1], type, depth - 1);
}

// `v` in `type`, computed before what follows `tail`.
Value LoopVectorizer::narrowed(Value v, Type type, InstId& tail)
{
	if (v.is(ValueKind::Integer))
		return integer_value(body_, normalize_integer(type, body_.literals[v.index]));
	const Instruction def = body_.instructions[body_.temps[v.index].def];
	auto add = [&](Instruction inst) {
		const InstId id = append_instruction(body_, inst);
		const TempId result = append_temp(body_, type, id);
		body_.instructions[id].result = result;
		insert_after(body_, b_, tail, id);
		c_.block_of.resize(body_.instructions.size(), kNoId);
		c_.block_of[id] = b_;
		tail = id;
		return Value::temp(result);
	};
	if (def.op == Opcode::Convert)
	{
		if (lane_bytes(def.type) != lane_bytes(type) && lane_bytes(convert_source_type(def)) != lane_bytes(type))
			return narrowed(def.ops[0], type, tail);
		if (static_cast<ConvertOp>(def.sub) == ConvertOp::Trunc && lane_bytes(def.type) != lane_bytes(type))
			return narrowed(def.ops[0], type, tail);
		const Value source = static_cast<ConvertOp>(def.sub) == ConvertOp::Trunc ? v : def.ops[0];
		if (body_.temps[source.index].type.kind() == type.kind())
			return source;
		Instruction copy = make_instruction(Opcode::Copy, type);
		copy.ops[0] = source;
		return add(copy);
	}
	Instruction op = make_instruction(Opcode::Binary, type);
	op.sub = def.sub;
	op.location = def.location;
	op.ops[0] = narrowed(def.ops[0], type, tail);
	op.ops[1] = narrowed(def.ops[1], type, tail);
	return add(op);
}

// Before the block is read: a truncation of arithmetic becomes the arithmetic
// in the narrow type, and what the block reads of it reads that.
void LoopVectorizer::narrow_arithmetic()
{
	bool changed = false;
	InstId prev = kNoId;
	for (InstId i = body_.blocks[b_].first; i != kNoId; prev = i, i = body_.instructions[i].next)
	{
		const Instruction inst = body_.instructions[i];
		if (inst.op == Opcode::Call)
			return;
		if (inst.op != Opcode::Convert || static_cast<ConvertOp>(inst.sub) != ConvertOp::Trunc ||
			lane_bytes(inst.type) == 0 || lane_bytes(inst.type) > 2 || !in_body(inst.ops[0]) ||
			body_.instructions[body_.temps[inst.ops[0].index].def].op != Opcode::Binary ||
			!narrow_fits(inst.ops[0], inst.type, kNarrowDepth))
			continue;
		InstId tail = prev;
		const Value value = narrowed(inst.ops[0], inst.type, tail);
		Instruction& copy = body_.instructions[i];
		copy.op = Opcode::Copy;
		copy.sub = 0;
		copy.ops[0] = value;
		prev = tail;
		const Value self = Value::temp(inst.result);
		for (InstId k = body_.instructions[i].next; k != kNoId; k = body_.instructions[k].next)
			for (Value& operand : body_.instructions[k].ops)
				if (operand == self)
					operand = value;
		for (InstId k = body_.blocks[b_].first; body_.instructions[k].op == Opcode::Phi; k = body_.instructions[k].next)
			for (uint32_t e = 1; e < body_.instructions[k].extra_count; e += 2)
				if (body_.operands[body_.instructions[k].extra_first + e - 1].index == b_ &&
					body_.operands[body_.instructions[k].extra_first + e] == self)
					body_.operands[body_.instructions[k].extra_first + e] = value;
		changed = true;
	}
	if (!changed)
		return;
	// The wide arithmetic goes: a reduction's phi may have no other reader.
	std::vector<uint32_t> uses;
	count_uses(body_, uses);
	std::vector<InstId> order;
	for (InstId i = body_.blocks[b_].first; i != kNoId; i = body_.instructions[i].next)
		order.push_back(i);
	for (size_t k = order.size(); k-- > 0;)
	{
		const Instruction inst = body_.instructions[order[k]];
		if (inst.op == Opcode::Phi || inst.result == kNoId || !is_pure(inst) || uses[inst.result] != 0)
			continue;
		unlink_after(body_, b_, previous_in_block(body_, b_, order[k]), order[k]);
		for_each_operand(body_, inst, [&](const Value& v) {
			if (v.is(ValueKind::Temp))
				--uses[v.index];
		});
	}
}

// The block's phis: each a counter of a literal step or a reduction.
bool LoopVectorizer::read_phis()
{
	inductions_.clear();
	reductions_.clear();
	for (InstId i = body_.blocks[b_].first; i != kNoId; i = body_.instructions[i].next)
	{
		const Instruction& phi = body_.instructions[i];
		if (phi.op != Opcode::Phi)
			break;
		if (phi.extra_count != 4)
			return false;
		const BlockId entry = body_.operands[phi.extra_first].index == b_ ? body_.operands[phi.extra_first + 2].index
																		  : body_.operands[phi.extra_first].index;
		const Value start = phi_incoming(body_, phi, entry), next = phi_incoming(body_, phi, b_);
		if (!next.is(ValueKind::Temp) || !in_body(next))
			return false;
		const Instruction& step = body_.instructions[body_.temps[next.index].def];
		const Value self = Value::temp(phi.result);
		if (step.op == Opcode::Index && step.ops[0] == self && step.ops[1].is(ValueKind::Integer))
		{
			inductions_.push_back(Induction{ phi.result, next.index, start,
				static_cast<int64_t>(body_.literals[step.ops[1].index].low) *
					static_cast<int64_t>(storage_bytes(step.type)),
				phi.type, true });
			continue;
		}
		if (step.op != Opcode::Binary)
			return false;
		const BinaryOp op = static_cast<BinaryOp>(step.sub);
		const bool left = step.ops[0] == self, right = step.ops[1] == self;
		if (left && !right && (op == BinaryOp::Add || op == BinaryOp::Sub) && step.ops[1].is(ValueKind::Integer) &&
			(phi.type.kind() == TypeKind::I32 || phi.type.kind() == TypeKind::U32 || phi.type.kind() == TypeKind::I64))
		{
			const int64_t by = static_cast<int64_t>(body_.literals[step.ops[1].index].low);
			inductions_.push_back(Induction{ phi.result, next.index, start, op == BinaryOp::Add ? by : -by, phi.type,
				(step.flags & IF_NO_WRAP) != 0 || phi.type.kind() == TypeKind::I64 });
			continue;
		}
		const bool ordered = phi.type.is_float();
		const bool commutes = op == BinaryOp::Add || (ordered && op == BinaryOp::Mul) ||
			(!ordered && (op == BinaryOp::And || op == BinaryOp::Or || op == BinaryOp::Xor));
		if ((left == right) || (right && !commutes) || (!commutes && op != BinaryOp::Sub) || lane_bytes(phi.type) == 0)
			return false;
		reductions_.push_back(
			Reduction{ phi.result, next.index, start, step.ops[left ? 1 : 0], op, phi.type, ordered, Value::none() });
	}
	return !inductions_.empty();
}

// The block's branch: the counter against an invariant bound. What is kept is
// where the lanes must stop: the counter must stay on this side of
// bound_ + bound_add_.
bool LoopVectorizer::read_exit()
{
	const Instruction& term = body_.instructions[body_.blocks[b_].last];
	if (term.op != Opcode::Branch)
		return false;
	const bool stays_on_true = term.ops[1].index == b_;
	if (stays_on_true == (term.ops[2].index == b_))
		return false;
	Value tested = term.ops[0], bound = literal(0);
	ComparePredicate predicate = ComparePredicate::Ne;
	if (in_body(tested))
	{
		const Instruction& def = body_.instructions[body_.temps[tested.index].def];
		if (def.op == Opcode::Compare)
		{
			predicate = static_cast<ComparePredicate>(def.sub);
			tested = def.ops[0];
			bound = def.ops[1];
		}
	}
	if (!stays_on_true)
		predicate = negated(predicate);
	auto counter = [&](Value v, uint32_t& index, bool& next) {
		for (uint32_t k = 0; k < inductions_.size(); ++k)
			if (v.is(ValueKind::Temp) && (v.index == inductions_[k].phi || v.index == inductions_[k].next))
			{
				index = k;
				next = v.index == inductions_[k].next;
				return true;
			}
		return false;
	};
	bool next = false;
	if (!counter(tested, counter_, next))
	{
		std::swap(tested, bound);
		predicate = swapped_operands(predicate);
		if (!counter(tested, counter_, next))
			return false;
	}
	if (in_body(bound))
		return false;
	const Induction& iv = inductions_[counter_];
	const int64_t d = iv.step;
	if (d == 0)
		return false;
	const bool up = d > 0;
	int64_t add = next ? 0 : d;
	switch (predicate)
	{
	case ComparePredicate::Lt:
	case ComparePredicate::Ult:
		if (!up)
			return false;
		break;
	case ComparePredicate::Gt:
	case ComparePredicate::Ugt:
		if (up)
			return false;
		break;
	case ComparePredicate::Le:
	case ComparePredicate::Ule:
		if (!up)
			return false;
		add += 1;
		break;
	case ComparePredicate::Ge:
	case ComparePredicate::Uge:
		if (up)
			return false;
		add -= 1;
		break;
	case ComparePredicate::Ne:
		if (!iv.type.is_pointer() && d != 1 && d != -1)
			return false;
		break;
	default:
		return false;
	}
	bound_ = bound;
	bound_add_ = add;
	unsigned_counter_ = iv.type.is_pointer() || predicate == ComparePredicate::Ult ||
		predicate == ComparePredicate::Ule || predicate == ComparePredicate::Ugt || predicate == ComparePredicate::Uge;
	return true;
}

// The block's instructions: all pure, or plain loads and stores.
bool LoopVectorizer::read_code()
{
	code_.clear();
	const InstId last = body_.blocks[b_].last;
	for (InstId i = body_.blocks[b_].first; i != kNoId && i != last; i = body_.instructions[i].next)
	{
		const Instruction& inst = body_.instructions[i];
		if (inst.op == Opcode::Phi)
			continue;
		if (code_.size() == kBodyInstructions || inst.type.is_vector() || inst.type.is_object())
			return false;
		switch (inst.op)
		{
		case Opcode::Load:
		case Opcode::Store:
			if ((inst.flags & IF_VOLATILE) != 0)
				return false;
			break;
		case Opcode::Binary:
		case Opcode::Unary:
		case Opcode::Compare:
		case Opcode::Select:
		case Opcode::Convert:
		case Opcode::Copy:
		case Opcode::Index:
			break;
		default:
			return false;
		}
		code_.push_back(i);
	}
	return !code_.empty();
}

void LoopVectorizer::mark(Value v, std::vector<uint8_t>& marks)
{
	if (in_body(v))
		marks[v.index] = 1;
}

// What must be a lane value and what a scalar one (of the first lane): the
// operands of each, from the loop's effects back. Each value's uses follow
// it, so one walk back through the block does.
void LoopVectorizer::mark_uses()
{
	in_scalar_.assign(body_.temps.size(), 0);
	in_wide_.assign(body_.temps.size(), 0);
	for (const Reduction& r : reductions_)
		in_wide_[r.next] = 1;
	for (size_t at = code_.size(); at-- > 0;)
	{
		const Instruction& inst = body_.instructions[code_[at]];
		if (inst.op == Opcode::Store)
		{
			mark(inst.ops[0], in_wide_);
			mark(inst.ops[1], in_scalar_);
			continue;
		}
		if (inst.op == Opcode::Load)
		{
			mark(inst.ops[0], in_scalar_);
			continue;
		}
		if (inst.result == kNoId)
			continue;
		for (const Value& v : inst.ops)
		{
			if (in_wide_[inst.result])
				mark(v, in_wide_);
			if (in_scalar_[inst.result])
				mark(v, in_scalar_);
		}
	}
}

// Adds factor * v to `out`, as far as it is linear in the pass.
bool LoopVectorizer::integer_term(Value v, Linear& out, int64_t factor, unsigned depth) const
{
	if (depth == 0)
		return false;
	if (v.is(ValueKind::Integer))
	{
		out.offset += static_cast<int64_t>(body_.literals[v.index].low) * factor;
		return true;
	}
	if (!in_body(v))
	{
		if (out.term.is(ValueKind::None))
			out.term = v;
		else if (!(out.term == v))
			return false;
		out.term_scale += factor;
		return true;
	}
	const Instruction& def = body_.instructions[body_.temps[v.index].def];
	const bool no_wrap = (def.flags & IF_NO_WRAP) != 0 || lane_bytes(def.type) == 8;
	switch (def.op)
	{
	case Opcode::Phi:
		for (const Induction& iv : inductions_)
			if (iv.phi == v.index && !iv.type.is_pointer() && (iv.no_wrap || &iv == &inductions_[counter_]))
			{
				out.scale += iv.step * factor;
				return integer_term(iv.start, out, factor, depth - 1);
			}
		return false;
	case Opcode::Binary:
	{
		const BinaryOp op = static_cast<BinaryOp>(def.sub);
		if (!no_wrap)
			return false;
		if (op == BinaryOp::Add)
			return integer_term(def.ops[0], out, factor, depth - 1) && integer_term(def.ops[1], out, factor, depth - 1);
		if (op == BinaryOp::Sub)
			return integer_term(def.ops[0], out, factor, depth - 1) &&
				integer_term(def.ops[1], out, -factor, depth - 1);
		if (op == BinaryOp::Mul && def.ops[1].is(ValueKind::Integer))
			return integer_term(
				def.ops[0], out, factor * static_cast<int64_t>(body_.literals[def.ops[1].index].low), depth - 1);
		if (op == BinaryOp::Shl && def.ops[1].is(ValueKind::Integer))
			return integer_term(def.ops[0], out, factor << body_.literals[def.ops[1].index].low, depth - 1);
		return false;
	}
	case Opcode::Convert:
	{
		const ConvertOp op = static_cast<ConvertOp>(def.sub);
		const TypeKind from = convert_source_type(def).kind();
		if (op == ConvertOp::SExt || (op == ConvertOp::ZExt && (from == TypeKind::U32 || from == TypeKind::U16)))
			return integer_term(def.ops[0], out, factor, depth - 1);
		return false;
	}
	case Opcode::Copy:
		return def.type.is_integer() && integer_term(def.ops[0], out, factor, depth - 1);
	default:
		return false;
	}
}

// An address as `base` the loop does not change, plus a linear value.
bool LoopVectorizer::address(Value v, Linear& out, Value& base, Value& object) const
{
	for (unsigned step = 0; step < 8; ++step)
	{
		if (!in_body(v))
		{
			const Located located = locate(body_, v);
			object = located.root;
			base = located.known ? located.root : v;
			if (located.known)
				out.offset += located.offset;
			return true;
		}
		const Instruction& def = body_.instructions[body_.temps[v.index].def];
		if (def.op == Opcode::Phi)
		{
			for (const Induction& iv : inductions_)
				if (iv.phi == v.index && iv.type.is_pointer())
				{
					out.scale += iv.step;
					v = iv.start;
					goto stepped;
				}
			return false;
		}
		if (def.op == Opcode::Copy && def.type.is_pointer())
			v = def.ops[0];
		else if (def.op == Opcode::Index)
		{
			if (!integer_term(def.ops[1], out, static_cast<int64_t>(storage_bytes(def.type)), 8))
				return false;
			v = def.ops[0];
		}
		else
			return false;
	stepped:;
	}
	return false;
}

bool LoopVectorizer::read_accesses()
{
	accesses_.clear();
	for (InstId id : code_)
	{
		const Instruction& inst = body_.instructions[id];
		if (inst.op != Opcode::Load && inst.op != Opcode::Store)
			continue;
		Access a{ id, inst.type, inst.op == Opcode::Store, (inst.flags & IF_TYPED) != 0, Linear(), Value::none(),
			Value::none(), 0 };
		if (!address(inst.ops[a.store ? 1 : 0], a.at, a.base, a.object))
			return false;
		const int64_t bytes = static_cast<int64_t>(lane_bytes(inst.type));
		// One direction: lanes hold their passes in one order, in every access.
		if (bytes == 0 || (a.at.scale != bytes && a.at.scale != -bytes) ||
			(!accesses_.empty() && (a.at.scale < 0) != backward_))
			return false;
		backward_ = a.at.scale < 0;
		a.at.scale = bytes;
		accesses_.push_back(a);
	}
	for (const Reduction& r : reductions_)
	{
		// The running value is the operator's alone: a lane holds its own
		// pass's part only.
		uint32_t reads = 0;
		for (InstId id : code_)
		{
			// (A copy for the code after the loop reads the scalar loop's value.)
			const Instruction& inst = body_.instructions[id];
			if (inst.op == Opcode::Copy && !in_wide_[inst.result] && !in_scalar_[inst.result])
				continue;
			for (const Value& v : inst.ops)
				reads += v.is(ValueKind::Temp) && (v.index == r.phi || v.index == r.next);
		}
		if (reads != 1 || body_.instructions[body_.blocks[b_].last].ops[0] == Value::temp(r.next))
			return false;
	}
	return true;
}

// A signed 32-bit division by a literal power of two (shifts of the lanes).
bool LoopVectorizer::halves(const Instruction& inst, unsigned& shift) const
{
	if (inst.op != Opcode::Binary || static_cast<BinaryOp>(inst.sub) != BinaryOp::Div ||
		inst.type.kind() != TypeKind::I32 || !inst.ops[1].is(ValueKind::Integer))
		return false;
	const uint64_t divisor = body_.literals[inst.ops[1].index].low;
	if (divisor < 2 || divisor > (1u << 30) || (divisor & (divisor - 1)) != 0)
		return false;
	shift = 0;
	while ((1ull << shift) != divisor)
		++shift;
	return true;
}

// Whether a lane form of the instruction exists.
bool LoopVectorizer::supported(const Instruction& inst) const
{
	VectorOp op;
	switch (inst.op)
	{
	case Opcode::Load:
	case Opcode::Store:
		return true;
	case Opcode::Copy:
		return lane_bytes(inst.type) != 0 && in_body(inst.ops[0]) &&
			lane_bytes(body_.temps[inst.ops[0].index].type) == lane_bytes(inst.type);
	case Opcode::Binary:
	{
		const BinaryOp operation = static_cast<BinaryOp>(inst.sub);
		if (lane_shift(operation, inst.type, op))
			return inst.ops[1].is(ValueKind::Integer) &&
				body_.literals[inst.ops[1].index].low < lane_bytes(inst.type) * 8;
		if (operation == BinaryOp::Mul && !inst.type.is_float() && lane_bytes(inst.type) == 4)
			return true;
		unsigned shift;
		if (halves(inst, shift))
			return true;
		return lane_operation(operation, inst.type, op);
	}
	case Opcode::Unary:
	{
		const UnaryOp operation = static_cast<UnaryOp>(inst.sub);
		return operation == UnaryOp::Neg || operation == UnaryOp::BitNot ||
			(operation == UnaryOp::Sqrt && inst.type.is_float());
	}
	case Opcode::Convert:
	{
		const ConvertOp operation = static_cast<ConvertOp>(inst.sub);
		const Type source = convert_source_type(inst);
		const TypeKind from = source.kind(), to = inst.type.kind();
		switch (operation)
		{
		case ConvertOp::SExt:
		case ConvertOp::ZExt:
		case ConvertOp::Trunc:
			return lane_bytes(source) != 0 && !source.is_float() && !inst.type.is_float();
		case ConvertOp::SIToFP:
			return from == TypeKind::I32;
		case ConvertOp::FPToSI:
			return to == TypeKind::I32;
		case ConvertOp::FPExt:
		case ConvertOp::FPTrunc:
			return from != TypeKind::F80 && to != TypeKind::F80;
		default:
			return false;
		}
	}
	case Opcode::Select:
	{
		if (!in_body(inst.ops[0]))
			return false;
		const Instruction& test = body_.instructions[body_.temps[inst.ops[0].index].def];
		if (test.op != Opcode::Compare || lane_bytes(test.type) != lane_bytes(inst.type))
			return false;
		const ComparePredicate p = static_cast<ComparePredicate>(test.sub);
		if (test.type.is_float())
			return p == ComparePredicate::Eq || p == ComparePredicate::Ne || p == ComparePredicate::Lt ||
				p == ComparePredicate::Le || p == ComparePredicate::Gt || p == ComparePredicate::Ge;
		return lane_bytes(test.type) != 8 &&
			(p == ComparePredicate::Eq || p == ComparePredicate::Ne || p == ComparePredicate::Lt ||
				p == ComparePredicate::Le || p == ComparePredicate::Gt || p == ComparePredicate::Ge);
	}
	case Opcode::Compare: // (A select reads it.)
		return true;
	default:
		return false;
	}
}

// The widest lane of all that needs lanes, and a lane form of each.
bool LoopVectorizer::choose_lanes()
{
	lane_bytes_ = 0;
	auto lane = [&](unsigned bytes) {
		lane_bytes_ = std::max(lane_bytes_, bytes);
		return bytes != 0;
	};
	for (InstId id : code_)
	{
		const Instruction& inst = body_.instructions[id];
		const bool wide = inst.op == Opcode::Store || inst.op == Opcode::Load ||
			(inst.result != kNoId && in_wide_[inst.result]);
		if (!wide)
			continue;
		if (!lane(lane_bytes(inst.type)) || !supported(inst))
			return false;
		if (inst.op == Opcode::Convert && !lane(lane_bytes(convert_source_type(inst))))
			return false;
	}
	for (const Reduction& r : reductions_)
		if (!lane(lane_bytes(r.type)))
			return false;
	for (const Induction& iv : inductions_)
		if (in_wide_[iv.phi])
		{
			if (!lane(lane_bytes(iv.type)) || iv.step != static_cast<int32_t>(iv.step))
				return false;
		}
	if (lane_bytes_ == 0 || (accesses_.empty() && reductions_.empty()) || !lanes_pay_for_order())
		return false;
	lanes_ = 16 / lane_bytes_;
	return true;
}

// A floating sum adds its lanes one at a time, as many additions as the scalar
// loop and a shuffle more: only a costlier operation in lanes pays for that.
bool LoopVectorizer::lanes_pay_for_order() const
{
	bool ordered = false;
	for (const Reduction& r : reductions_)
		ordered = ordered || r.ordered;
	if (!ordered)
		return true;
	for (InstId id : code_)
	{
		const Instruction& inst = body_.instructions[id];
		if (!inst.type.is_float() || !in_wide_[inst.result])
			continue;
		const BinaryOp operation = static_cast<BinaryOp>(inst.sub);
		if ((inst.op == Opcode::Binary && (operation == BinaryOp::Mul || operation == BinaryOp::Div)) ||
			(inst.op == Opcode::Unary && static_cast<UnaryOp>(inst.sub) == UnaryOp::Sqrt))
			return true;
	}
	return false;
}

bool LoopVectorizer::same_group(const Access& a, const Access& b) const
{
	return a.base == b.base && a.at.term == b.at.term && a.at.term_scale == b.at.term_scale;
}

bool LoopVectorizer::add_overlap_test(uint32_t a, uint32_t b)
{
	const std::pair<uint32_t, uint32_t> test(std::min(a, b), std::max(a, b));
	if (std::find(tests_.begin(), tests_.end(), test) == tests_.end())
		tests_.push_back(test);
	return tests_.size() <= kOverlapTests;
}

// Whether the lanes keep every order of accesses that matters. Two accesses
// of one array apart by less than the vector need the load of the later
// element first; of two arrays, a test of their ranges unless the
// objects or types keep them apart.
bool LoopVectorizer::read_dependences()
{
	groups_.clear();
	tests_.clear();
	for (Access& a : accesses_)
	{
		uint32_t g = 0;
		for (; g < groups_.size(); ++g)
			if (same_group(a, accesses_[groups_[g].lowest]))
				break;
		if (g == groups_.size())
			groups_.push_back(
				Group{ static_cast<uint32_t>(&a - accesses_.data()), a.at.offset, a.at.offset, a.at.scale });
		else
		{
			if (a.at.offset < groups_[g].low)
			{
				groups_[g].low = a.at.offset;
				groups_[g].lowest = static_cast<uint32_t>(&a - accesses_.data());
			}
			groups_[g].high = std::max(groups_[g].high, a.at.offset);
			groups_[g].scale = std::max(groups_[g].scale, a.at.scale);
		}
		a.group = g;
	}
	for (size_t i = 0; i < accesses_.size(); ++i)
		for (size_t j = i + 1; j < accesses_.size(); ++j)
		{
			const Access& first = accesses_[i];
			const Access& later = accesses_[j];
			if (!first.store && !later.store)
				continue;
			if (first.group == later.group)
			{
				const int64_t apart = (later.at.offset - first.at.offset) * (backward_ ? -1 : 1);
				const int64_t whole = static_cast<int64_t>(lanes_) * std::max(first.at.scale, later.at.scale);
				if ((apart == 0 && first.at.scale == later.at.scale) || apart >= whole || apart <= -whole)
					continue;
				// (The earlier access reads ahead of what the later writes.)
				if (!first.store && later.store && apart < 0 && first.at.scale == later.at.scale)
					continue;
				return false;
			}
			if (!(first.object == later.object) && named(first.object) && named(later.object))
				continue;
			if (first.typed && later.typed && typed_types_apart(first.type, later.type))
				continue;
			if (!add_overlap_test(first.group, later.group))
				return false;
		}
	return true;
}

bool LoopVectorizer::analyze(uint32_t loop)
{
	const Loop& l = forest_.loops[loop];
	if (l.blocks.size() != 1 || done_[l.header] != 0 || forest_.innermost[l.header] != loop)
		return false;
	b_ = l.header;
	narrow_arithmetic();
	return read_phis() && read_exit() && read_code() && (mark_uses(), read_accesses()) && choose_lanes() &&
		read_dependences();
}

void LoopVectorizer::at(BlockId block, InstId tail)
{
	block_ = block;
	tail_ = tail;
}

Value LoopVectorizer::emit(Instruction inst, Type type)
{
	const InstId id = append_instruction(body_, inst);
	const TempId result = append_temp(body_, type, id);
	body_.instructions[id].result = result;
	insert_after(body_, block_, tail_, id);
	tail_ = id;
	return Value::temp(result);
}

void LoopVectorizer::emit_void(const Instruction& inst)
{
	const InstId id = append_instruction(body_, inst);
	insert_after(body_, block_, tail_, id);
	tail_ = id;
}

void LoopVectorizer::finish_block(Instruction terminator)
{
	emit_void(terminator);
}

Value LoopVectorizer::vector(VectorOp op, Value a, Value b, int immediate)
{
	const Type type = vector_result_type(op);
	Instruction inst = make_instruction(Opcode::Vector, type);
	inst.sub = static_cast<uint8_t>(op);
	inst.ops[0] = a;
	inst.ops[1] = b;
	if (immediate >= 0)
		inst.ops[2] = literal(immediate);
	return emit(inst, type);
}

Value LoopVectorizer::compare(ComparePredicate predicate, Type type, Value a, Value b)
{
	Instruction inst = make_instruction(Opcode::Compare, type);
	inst.sub = static_cast<uint8_t>(predicate);
	inst.ops[0] = a;
	inst.ops[1] = b;
	return emit(inst, Type::scalar(TypeKind::I1));
}

Value LoopVectorizer::binary(BinaryOp op, Type type, Value a, Value b, Type result)
{
	Instruction inst = make_instruction(Opcode::Binary, type);
	inst.sub = static_cast<uint8_t>(op);
	inst.ops[0] = a;
	inst.ops[1] = b;
	return emit(inst, result);
}

// A counter value stepped by a literal number of counter units.
Value LoopVectorizer::step_counter(Value v, int64_t by)
{
	const Type type = inductions_[counter_].type;
	if (type.is_pointer())
	{
		Instruction inst = make_instruction(Opcode::Index, Type::scalar(TypeKind::I8));
		inst.ops[0] = v;
		inst.ops[1] = literal(by);
		return emit(inst, type);
	}
	return binary(BinaryOp::Add, type, v, literal(by), type);
}

Value LoopVectorizer::convert(ConvertOp op, Type to, Type from, Value v)
{
	Instruction inst = make_instruction(Opcode::Convert, to);
	inst.sub = static_cast<uint8_t>(op);
	inst.ops[0] = v;
	set_convert_source_type(inst, from);
	return emit(inst, to);
}

// The value as the scalar type `from<t>` takes.
Value LoopVectorizer::exact_scalar(Value v, Type from)
{
	const TypeKind kind = from.kind();
	if (!v.is(ValueKind::Temp) && !body_.temps.empty() && kind != TypeKind::U32 && lane_bytes(from) >= 4)
		return v;
	if (kind == TypeKind::I32 || kind == TypeKind::I64 || kind == TypeKind::F32 || kind == TypeKind::F64)
		return v;
	if (!v.is(ValueKind::Temp))
		return v;
	const Type wanted = Type::scalar(lane_bytes(from) == 8 ? TypeKind::I64 : TypeKind::I32);
	if (lane_bytes(from) == 4)
	{
		Instruction copy = make_instruction(Opcode::Copy, wanted);
		copy.ops[0] = v;
		return emit(copy, wanted);
	}
	return convert(ConvertOp::ZExt, wanted, from, v);
}

// `v`, which the loop does not change, in every lane: made before the loop.
Value LoopVectorizer::broadcast(Value v, Type type)
{
	const std::pair<Value, TypeKind> key(v, type.kind());
	for (const auto& known : broadcasts_)
		if (known.first == key)
			return known.second;
	const BlockId block = block_;
	const InstId tail = tail_;
	at(vector_pre_, pre_tail_);
	Value out = Value::none();
	const unsigned bytes = lane_bytes(type);
	const Value value = phi_input(body_, type, v);
	if (v.is(ValueKind::Integer) && body_.literals[v.index].low == 0 && body_.literals[v.index].high == 0)
		out = vector(VectorOp::Pzero);
	else if (type.is_float())
		out = vector(VectorOp::Pshufd, vector(bytes == 8 ? VectorOp::Fromf64 : VectorOp::Fromf32, value), Value::none(),
			bytes == 8 ? 0x44 : 0);
	else if (bytes == 8)
		out = vector(VectorOp::Pshufd, vector(VectorOp::Fromi64, exact_scalar(value, type)), Value::none(), 0x44);
	else if (bytes == 4)
		out = vector(VectorOp::Pshufd, vector(VectorOp::Fromi32, exact_scalar(value, type)), Value::none(), 0);
	else
	{
		// A word, or a byte twice over, four times in a register's low half.
		Value low = exact_scalar(value, type);
		if (bytes == 1)
			low = v.is(ValueKind::Integer)
				? literal(static_cast<int64_t>(body_.literals[v.index].low & 0xff) * 0x0101)
				: binary(BinaryOp::Mul, Type::scalar(TypeKind::I32), low, literal(0x0101), Type::scalar(TypeKind::I32));
		else if (v.is(ValueKind::Integer))
			low = literal(static_cast<int64_t>(body_.literals[v.index].low & 0xffff));
		out = vector(VectorOp::Pshufd, vector(VectorOp::Pshuflw, vector(VectorOp::Fromi32, low), Value::none(), 0),
			Value::none(), 0);
	}
	pre_tail_ = tail_;
	at(block, tail);
	broadcasts_.push_back(std::make_pair(key, out));
	return out;
}

Value LoopVectorizer::wide(Value v, Type type)
{
	if (in_body(v))
		return wides_[v.index];
	return broadcast(v, type);
}

Value LoopVectorizer::scalar(Value v) const
{
	return v.is(ValueKind::Temp) && v.index < scalars_.size() && !scalars_[v.index].is(ValueKind::None)
		? scalars_[v.index]
		: v;
}

// The bytes at `address` at the bottom of a register.
Value LoopVectorizer::widen_load(Value address, unsigned bytes, InstId origin)
{
	const TypeKind kind = bytes == 16 ? TypeKind::V128
		: bytes == 8                  ? TypeKind::I64
		: bytes == 4                  ? TypeKind::I32
									  : TypeKind::U16;
	Instruction load = make_instruction(Opcode::Load, Type::scalar(kind));
	load.ops[0] = address;
	load.location = body_.instructions[origin].location;
	const Value loaded = emit(load, Type::scalar(kind));
	if (bytes == 16)
		return loaded;
	if (bytes == 8)
		return vector(VectorOp::Fromi64, loaded);
	if (bytes == 4)
		return vector(VectorOp::Fromi32, loaded);
	return vector(VectorOp::Fromi32, convert(ConvertOp::ZExt, Type::scalar(TypeKind::I32), Type::scalar(kind), loaded));
}

void LoopVectorizer::widen_store(Value value, Value address, unsigned bytes, InstId origin)
{
	Value out = value;
	TypeKind kind = TypeKind::V128;
	if (bytes == 8)
	{
		out = vector(VectorOp::Toi64, value);
		kind = TypeKind::I64;
	}
	else if (bytes == 4)
	{
		out = vector(VectorOp::Toi32, value);
		kind = TypeKind::I32;
	}
	else if (bytes == 2)
	{
		kind = TypeKind::I16;
		out = convert(
			ConvertOp::Trunc, Type::scalar(kind), Type::scalar(TypeKind::I32), vector(VectorOp::Toi32, value));
	}
	Instruction store = make_instruction(Opcode::Store, Type::scalar(kind));
	store.ops[0] = out;
	store.ops[1] = address;
	store.location = body_.instructions[origin].location;
	emit_void(store);
}

// Integer lanes twice as wide, as often as it takes.
Value LoopVectorizer::extend(Value v, unsigned from, unsigned to, bool is_signed)
{
	const Value zero = broadcast(literal(0), Type::scalar(TypeKind::I32));
	for (; from < to; from *= 2)
	{
		if (from == 1)
			v = is_signed ? vector(VectorOp::Psrawi, vector(VectorOp::Punpcklbw, v, v), Value::none(), 8)
						  : vector(VectorOp::Punpcklbw, v, zero);
		else if (from == 2)
			v = is_signed ? vector(VectorOp::Psradi, vector(VectorOp::Punpcklwd, v, v), Value::none(), 16)
						  : vector(VectorOp::Punpcklwd, v, zero);
		else
			v = vector(VectorOp::Punpckldq, v, is_signed ? vector(VectorOp::Pcmpgtd, zero, v) : zero);
	}
	return v;
}

// Integer lanes half as wide, as often as it takes, each cut to its low bits.
Value LoopVectorizer::narrow(Value v, unsigned from, unsigned to)
{
	for (; from > to; from /= 2)
	{
		if (from == 2)
		{
			const Value kept = vector(VectorOp::Pand, v, broadcast(literal(0xff), Type::scalar(TypeKind::I16)));
			v = vector(VectorOp::Packuswb, kept, kept);
		}
		else if (from == 4)
		{
			const Value kept = vector(
				VectorOp::Psradi, vector(VectorOp::Pslldi, v, Value::none(), 16), Value::none(), 16);
			v = vector(VectorOp::Packssdw, kept, kept);
		}
		else
			v = vector(VectorOp::Pshufd, v, Value::none(), 0x08);
	}
	return v;
}

void LoopVectorizer::widen_convert(const Instruction& inst)
{
	const Type source = convert_source_type(inst);
	const unsigned from = lane_bytes(source), to = lane_bytes(inst.type);
	const Value v = wide(inst.ops[0], source);
	Value out = Value::none();
	switch (static_cast<ConvertOp>(inst.sub))
	{
	case ConvertOp::SExt:
		out = extend(v, from, to, true);
		break;
	case ConvertOp::ZExt:
		out = extend(v, from, to, false);
		break;
	case ConvertOp::Trunc:
		out = narrow(v, from, to);
		break;
	case ConvertOp::SIToFP:
		out = vector(to == 8 ? VectorOp::Cvtdq2pd : VectorOp::Cvtdq2ps, v);
		break;
	case ConvertOp::FPToSI:
		out = vector(from == 8 ? VectorOp::Cvttpd2dq : VectorOp::Cvttps2dq, v);
		break;
	case ConvertOp::FPExt:
		out = vector(VectorOp::Cvtps2pd, v);
		break;
	default:
		out = vector(VectorOp::Cvtpd2ps, v);
		break;
	}
	wides_[inst.result] = out;
}

// i32 lanes multiplied: SSE2 multiplies the even lanes only.
Value LoopVectorizer::widen_multiply(Value a, Value b)
{
	const Value even = vector(VectorOp::Pmuludq, a, b);
	const Value high_a = vector(VectorOp::Psrlqi, a, Value::none(), 32);
	const Value high_b = vector(VectorOp::Psrlqi, b, Value::none(), 32);
	const Value odd = vector(VectorOp::Pmuludq, high_a, high_b);
	const Value low_even = vector(VectorOp::Pshufd, even, Value::none(), 0x08);
	const Value low_odd = vector(VectorOp::Pshufd, odd, Value::none(), 0x08);
	return vector(VectorOp::Punpckldq, low_even, low_odd);
}

// The mask of a compare's lanes. Set `swapped` when it is of the opposite test.
Value LoopVectorizer::widen_mask(const Instruction& test, bool& swapped)
{
	const Value a = wide(test.ops[0], test.type), b = wide(test.ops[1], test.type);
	const ComparePredicate p = static_cast<ComparePredicate>(test.sub);
	swapped = false;
	if (test.type.is_float())
	{
		const VectorOp op = lane_bytes(test.type) == 8 ? VectorOp::Cmppd : VectorOp::Cmpps;
		switch (p)
		{
		case ComparePredicate::Eq:
			return vector(op, a, b, 0);
		case ComparePredicate::Ne:
			return vector(op, a, b, 4);
		case ComparePredicate::Lt:
			return vector(op, a, b, 1);
		case ComparePredicate::Le:
			return vector(op, a, b, 2);
		case ComparePredicate::Gt:
			return vector(op, b, a, 1);
		default:
			return vector(op, b, a, 2);
		}
	}
	const unsigned bytes = lane_bytes(test.type);
	const VectorOp equal = bytes == 1 ? VectorOp::Pcmpeqb : bytes == 2 ? VectorOp::Pcmpeqw : VectorOp::Pcmpeqd;
	const VectorOp greater = bytes == 1 ? VectorOp::Pcmpgtb : bytes == 2 ? VectorOp::Pcmpgtw : VectorOp::Pcmpgtd;
	swapped = p == ComparePredicate::Ne || p == ComparePredicate::Le || p == ComparePredicate::Ge;
	switch (p)
	{
	case ComparePredicate::Eq:
	case ComparePredicate::Ne:
		return vector(equal, a, b);
	case ComparePredicate::Gt:
	case ComparePredicate::Le:
		return vector(greater, a, b);
	default:
		return vector(greater, b, a);
	}
}

void LoopVectorizer::widen_binary(const Instruction& inst)
{
	for (Reduction& r : reductions_)
		if (r.ordered && r.next == inst.result)
		{
			const Value lanes = wide(r.operand, r.type);
			r.next_sum = add_in_order(r, wides_[r.phi], lanes);
			return;
		}
	unsigned shift;
	if (halves(inst, shift))
	{
		// Negative lanes round toward zero: they add the divisor less one.
		const Value a = wide(inst.ops[0], inst.type);
		const Value sign = vector(VectorOp::Psradi, a, Value::none(), 31);
		const Value bias = vector(VectorOp::Psrldi, sign, Value::none(), 32 - static_cast<int>(shift));
		const Value rounded = vector(VectorOp::Paddd, a, bias);
		wides_[inst.result] = vector(VectorOp::Psradi, rounded, Value::none(), static_cast<int>(shift));
		return;
	}
	const BinaryOp operation = static_cast<BinaryOp>(inst.sub);
	VectorOp op;
	if (lane_shift(operation, inst.type, op))
	{
		wides_[inst.result] = vector(
			op, wide(inst.ops[0], inst.type), Value::none(), static_cast<int>(body_.literals[inst.ops[1].index].low));
		return;
	}
	const Value a = wide(inst.ops[0], inst.type), b = wide(inst.ops[1], inst.type);
	if (lane_operation(operation, inst.type, op))
		wides_[inst.result] = vector(op, a, b);
	else
		wides_[inst.result] = widen_multiply(a, b);
}

void LoopVectorizer::widen_unary(const Instruction& inst)
{
	const UnaryOp operation = static_cast<UnaryOp>(inst.sub);
	const Value a = wide(inst.ops[0], inst.type);
	const unsigned bytes = lane_bytes(inst.type);
	const bool is_float = inst.type.is_float();
	if (operation == UnaryOp::Sqrt)
		wides_[inst.result] = vector(bytes == 8 ? VectorOp::Sqrtpd : VectorOp::Sqrtps, a);
	else if (operation == UnaryOp::BitNot)
		wides_[inst.result] = vector(VectorOp::Pxor, a, broadcast(literal(-1), Type::scalar(TypeKind::I32)));
	else if (is_float)
	{
		const Type bits = Type::scalar(bytes == 8 ? TypeKind::I64 : TypeKind::I32);
		wides_[inst.result] = vector(VectorOp::Pxor, a, broadcast(literal(bytes == 8 ? INT64_MIN : INT32_MIN), bits));
	}
	else
	{
		VectorOp op;
		lane_operation(BinaryOp::Sub, inst.type, op);
		wides_[inst.result] = vector(op, broadcast(literal(0), inst.type), a);
	}
}

// Where the lanes of an access lie: its element in this pass is the highest
// of them when the accesses step down.
Value LoopVectorizer::lane_address(Value address, Type type)
{
	if (!backward_)
		return address;
	Instruction low = make_instruction(Opcode::Index, Type::scalar(TypeKind::I8));
	low.ops[0] = address;
	low.ops[1] = literal(-static_cast<int64_t>((lanes_ - 1) * lane_bytes(type)));
	return emit(low, Type::scalar(TypeKind::Ptr));
}

void LoopVectorizer::widen(const Instruction& inst, InstId origin)
{
	switch (inst.op)
	{
	case Opcode::Load:
		wides_[inst.result] = widen_load(
			lane_address(scalar(inst.ops[0]), inst.type), lanes_ * lane_bytes(inst.type), origin);
		break;
	case Opcode::Store:
	{
		const Value value = wide(inst.ops[0], inst.type);
		widen_store(value, lane_address(scalar(inst.ops[1]), inst.type), lanes_ * lane_bytes(inst.type), origin);
		break;
	}
	case Opcode::Copy:
		wides_[inst.result] = wides_[inst.ops[0].index];
		break;
	case Opcode::Binary:
		widen_binary(inst);
		break;
	case Opcode::Unary:
		widen_unary(inst);
		break;
	case Opcode::Convert:
		widen_convert(inst);
		break;
	case Opcode::Select:
	{
		bool swapped;
		const Instruction test = body_.instructions[body_.temps[inst.ops[0].index].def];
		const Value mask = widen_mask(test, swapped);
		const Value chosen = wide(inst.ops[swapped ? 2 : 1], inst.type),
					other = wide(inst.ops[swapped ? 1 : 2], inst.type);
		const Value taken = vector(VectorOp::Pand, mask, chosen);
		const Value not_taken = vector(VectorOp::Pandn, mask, other);
		wides_[inst.result] = vector(VectorOp::Por, taken, not_taken);
		break;
	}
	default:
		break; // (A compare: its select makes the mask.)
	}
}

// The block's code once more, at the current point. The scalar copies read
// `scalars_`, which starts at the counters' values there.
void LoopVectorizer::clone_scalars(bool also_widen)
{
	for (InstId id : code_)
	{
		const Instruction inst = body_.instructions[id];
		if (inst.result != kNoId && in_scalar_[inst.result])
		{
			Instruction copy = inst;
			for (Value& v : copy.ops)
				v = scalar(v);
			copy.next = kNoId;
			scalars_[inst.result] = emit(copy, body_.temps[inst.result].type);
		}
		if (also_widen && (inst.op == Opcode::Store || inst.op == Opcode::Load || in_wide_[inst.result]))
			widen(inst, id);
	}
}

// Whether the ranges the loop touches are apart, tested at the current point.
Value LoopVectorizer::overlap_guard(Value distance)
{
	for (const Induction& iv : inductions_)
		scalars_[iv.phi] = iv.start;
	clone_scalars(false);
	const Type pointer = Type::scalar(TypeKind::Ptr), i64 = Type::scalar(TypeKind::I64);
	const Induction& counter = inductions_[counter_];
	const int64_t unit = counter.step < 0 ? -counter.step : counter.step;
	Value passes = distance;
	if (lane_bytes(counter.type) == 4)
	{
		Instruction bits = make_instruction(Opcode::Copy, Type::scalar(TypeKind::U32));
		bits.ops[0] = passes;
		passes = convert(ConvertOp::ZExt, i64, Type::scalar(TypeKind::U32), emit(bits, Type::scalar(TypeKind::U32)));
	}
	for (unsigned shift = 0; shift < 63; ++shift)
		if ((int64_t(1) << shift) == unit && shift != 0)
			passes = binary(BinaryOp::UShr, i64, passes, literal(shift), i64);
	std::vector<Value> first(groups_.size(), Value::none()), last(groups_.size(), Value::none());
	auto ends = [&](uint32_t g) {
		if (!first[g].is(ValueKind::None))
			return;
		const Access& a = accesses_[groups_[g].lowest];
		const Instruction& inst = body_.instructions[a.inst];
		const Value lowest = scalar(inst.ops[a.store ? 1 : 0]);
		const int64_t spread = groups_[g].high - groups_[g].low;
		const Value bytes = binary(BinaryOp::Mul, i64, passes, literal(groups_[g].scale), i64);
		auto step = [&](Value from, Value by) {
			Instruction index = make_instruction(Opcode::Index, Type::scalar(TypeKind::I8));
			index.ops[0] = from;
			index.ops[1] = by;
			return emit(index, pointer);
		};
		if (backward_)
		{
			const Value down = binary(BinaryOp::Sub, i64, literal(groups_[g].scale), bytes, i64);
			first[g] = step(lowest, down);
			last[g] = step(lowest, literal(spread + groups_[g].scale));
		}
		else
		{
			first[g] = lowest;
			last[g] = step(step(lowest, bytes), literal(spread));
		}
	};
	Value all = Value::none();
	for (const std::pair<uint32_t, uint32_t>& test : tests_)
	{
		ends(test.first);
		ends(test.second);
		const Value below = compare(ComparePredicate::Ule, pointer, last[test.first], first[test.second]);
		const Value above = compare(ComparePredicate::Ule, pointer, last[test.second], first[test.first]);
		const Value apart = binary(BinaryOp::Or, i64, below, above, i64);
		all = all.is(ValueKind::None) ? apart : binary(BinaryOp::And, i64, all, apart, i64);
	}
	return all;
}

// The floating lanes added to `sum` in the order of their passes.
Value LoopVectorizer::add_in_order(const Reduction& r, Value sum, Value lanes)
{
	const unsigned bytes = lane_bytes(r.type);
	for (unsigned k = 0; k < lanes_; ++k)
	{
		const unsigned lane = backward_ ? lanes_ - 1 - k : k;
		const Value moved = lane == 0 ? lanes
									  : vector(VectorOp::Pshufd, lanes, Value::none(), bytes == 8 ? 0xee : 0x55 * lane);
		const Value value = vector(bytes == 8 ? VectorOp::Tof64 : VectorOp::Tof32, moved);
		sum = binary(r.op, r.type, sum, value, r.type);
	}
	return sum;
}

// A reduction's lanes as one value.
Value LoopVectorizer::add_across(const Reduction& r, Value v)
{
	const Type type = r.type;
	const unsigned bytes = lane_bytes(type);
	VectorOp op;
	lane_operation(r.op == BinaryOp::Sub ? BinaryOp::Add : r.op, type, op);
	Value sum = v;
	// Each step folds the upper half of what is left onto the lower.
	unsigned width = lanes_ * bytes;
	if (width == 16)
	{
		sum = vector(op, sum, vector(VectorOp::Pshufd, sum, Value::none(), 0x4e));
		width = 8;
	}
	if (width == 8 && bytes < 8)
	{
		sum = vector(op, sum, vector(VectorOp::Pshufd, sum, Value::none(), 0xb1));
		width = 4;
	}
	if (width == 4 && bytes < 4)
	{
		sum = vector(op, sum, vector(VectorOp::Psrldi, sum, Value::none(), 16));
		width = 2;
	}
	if (width == 2 && bytes < 2)
		sum = vector(op, sum, vector(VectorOp::Psrlwi, sum, Value::none(), 8));
	const Value out = vector(bytes == 8 ? VectorOp::Toi64 : VectorOp::Toi32, sum);
	if (bytes < 4)
		return convert(ConvertOp::Trunc, type, Type::scalar(TypeKind::I32), out);
	if (type.kind() != TypeKind::U32)
		return out;
	Instruction copy = make_instruction(Opcode::Copy, type);
	copy.ops[0] = out;
	return emit(copy, type);
}

void LoopVectorizer::make_phi(BlockId block, Type type, std::initializer_list<Value> pairs, Value& out)
{
	const BlockId here = block_;
	const InstId tail = tail_;
	Instruction phi = make_instruction(Opcode::Phi, type);
	phi.extra_first = append_operands(body_, pairs.begin(), static_cast<uint32_t>(pairs.size()));
	phi.extra_count = static_cast<uint32_t>(pairs.size());
	at(block, kNoId);
	out = emit(phi, type);
	at(here, tail == kNoId && here == block ? tail_ : tail);
}

BlockId LoopVectorizer::place_block(BlockKind kind)
{
	const BlockId block = append_block(body_, kind);
	const auto where = std::find(body_.layout.begin(), body_.layout.end(), b_);
	body_.layout.insert(where, block);
	return block;
}

void LoopVectorizer::vectorize(uint32_t loop)
{
	const BlockId pre = forest_.loops[loop].preheader;
	const Induction counter = inductions_[counter_];
	const bool up = counter.step > 0;
	const Type type = counter.type, v128 = Type::scalar(TypeKind::V128);
	const Type i64 = Type::scalar(TypeKind::I64);
	const Type distance_type = type.is_pointer() ? i64 : type;
	const int64_t span = static_cast<int64_t>(lanes_) * (up ? counter.step : -counter.step);
	const ComparePredicate below = unsigned_counter_ ? ComparePredicate::Ult : ComparePredicate::Lt;
	const BlockId guard = tests_.empty() ? kNoId : place_block(BlockKind::LoopPreheader);
	vector_pre_ = place_block(BlockKind::LoopPreheader);
	const BlockId wide_loop = place_block(BlockKind::WideLoop);
	const BlockId exit = place_block(BlockKind::LoopExit);
	const BlockId scalar_pre = place_block(BlockKind::LoopPreheader);
	scalars_.assign(body_.temps.size(), Value::none());
	wides_.assign(body_.temps.size(), Value::none());
	broadcasts_.clear();

	// Enough passes for the lanes and at least one more.
	at(pre, previous_in_block(body_, pre, body_.blocks[pre].last));
	const Value start = counter.start;
	Value bound = bound_;
	if (bound_add_ != 0)
		bound = step_counter(bound, bound_add_);
	Instruction difference = make_instruction(Opcode::Binary, type);
	difference.sub = static_cast<uint8_t>(BinaryOp::Sub);
	difference.ops[0] = up ? bound : start;
	difference.ops[1] = up ? start : bound;
	const Value distance = emit(difference, distance_type);
	const Value before = up ? compare(below, type, start, bound) : compare(below, type, bound, start);
	const Value enough = compare(ComparePredicate::Ugt, distance_type, distance, literal(span));
	const Value test = binary(BinaryOp::And, i64, before, enough, i64);
	Instruction& jump = body_.instructions[body_.blocks[pre].last];
	jump.op = Opcode::Branch;
	jump.ops[0] = test;
	jump.ops[1] = Value::block(guard != kNoId ? guard : vector_pre_);
	jump.ops[2] = Value::block(scalar_pre);

	if (guard != kNoId)
	{
		at(guard, kNoId);
		const Value apart = overlap_guard(distance);
		Instruction branch = make_instruction(Opcode::Branch, Type::scalar(TypeKind::Void));
		branch.ops[0] = apart;
		branch.ops[1] = Value::block(vector_pre_);
		branch.ops[2] = Value::block(scalar_pre);
		finish_block(branch);
		scalars_.assign(body_.temps.size(), Value::none());
	}

	// What the lanes start from.
	at(vector_pre_, kNoId);
	const Value limit = step_counter(bound, up ? -span : span);
	std::vector<Value> wide_start(reductions_.size(), Value::none());
	for (size_t k = 0; k < reductions_.size(); ++k)
	{
		const Reduction& r = reductions_[k];
		if (r.ordered)
			wide_start[k] = r.start;
		else if (r.op == BinaryOp::And)
			wide_start[k] = Value::none();
		else if (r.start.is(ValueKind::Integer) && body_.literals[r.start.index].low == 0 &&
			body_.literals[r.start.index].high == 0)
			wide_start[k] = vector(VectorOp::Pzero);
		else if (r.start.is(ValueKind::Integer) && lane_bytes(r.type) < 4)
		{
			// (Only the first lane holds the start: the others are zero.)
			const uint64_t mask = lane_bytes(r.type) == 1 ? 0xff : 0xffff;
			wide_start[k] = vector(
				VectorOp::Fromi32, literal(static_cast<int64_t>(body_.literals[r.start.index].low & mask)));
		}
		else
			wide_start[k] = vector(
				lane_bytes(r.type) == 8 ? VectorOp::Fromi64 : VectorOp::Fromi32, exact_scalar(r.start, r.type));
	}
	pre_tail_ = tail_;
	for (size_t k = 0; k < reductions_.size(); ++k)
		if (wide_start[k].is(ValueKind::None))
			wide_start[k] = broadcast(reductions_[k].start, reductions_[k].type);

	// Lanes of a counter that is data: its first value in each.
	std::vector<Value> wide_counter(inductions_.size(), Value::none());
	for (size_t k = 0; k < inductions_.size(); ++k)
	{
		const Induction& iv = inductions_[k];
		if (!in_wide_[iv.phi])
			continue;
		at(vector_pre_, pre_tail_);
		const uint64_t d = static_cast<uint64_t>(iv.step);
		// The pass each lane holds, counted from the first.
		auto pass = [&](unsigned lane) { return (backward_ ? lanes_ - 1 - lane : lane) * d; };
		auto bits = [&](uint64_t value) {
			return value == 0 ? vector(VectorOp::Pzero)
							  : vector(VectorOp::Fromi64, literal(static_cast<int64_t>(value)));
		};
		Value steps = Value::none();
		if (lane_bytes(iv.type) == 8)
		{
			const Value low = bits(pass(0));
			steps = vector(VectorOp::Punpcklqdq, low, bits(pass(1)));
		}
		else
		{
			const Value low = bits((pass(1) << 32) | (pass(0) & 0xffffffffu));
			steps = lanes_ == 2 ? low
								: vector(VectorOp::Punpcklqdq, low, bits((pass(3) << 32) | (pass(2) & 0xffffffffu)));
		}
		pre_tail_ = tail_;
		const Value from = broadcast(iv.start, iv.type);
		at(vector_pre_, pre_tail_);
		VectorOp add;
		lane_operation(BinaryOp::Add, iv.type, add);
		wide_counter[k] = vector(add, from, steps);
		pre_tail_ = tail_;
	}

	// The wide loop.
	at(wide_loop, kNoId);
	std::vector<Value> counters(inductions_.size(), Value::none()), sums(reductions_.size(), Value::none());
	std::vector<Value> lane_counters(inductions_.size(), Value::none());
	for (size_t k = 0; k < inductions_.size(); ++k)
	{
		const Induction& iv = inductions_[k];
		make_phi(wide_loop, iv.type, { Value::block(vector_pre_), iv.start, Value::block(wide_loop), Value::none() },
			counters[k]);
		scalars_[iv.phi] = counters[k];
		if (in_wide_[iv.phi])
		{
			make_phi(wide_loop, v128,
				{ Value::block(vector_pre_), wide_counter[k], Value::block(wide_loop), Value::none() },
				lane_counters[k]);
			wides_[iv.phi] = lane_counters[k];
		}
	}
	for (size_t k = 0; k < reductions_.size(); ++k)
	{
		const Reduction& r = reductions_[k];
		make_phi(wide_loop, r.ordered ? r.type : v128,
			{ Value::block(vector_pre_), wide_start[k], Value::block(wide_loop), Value::none() }, sums[k]);
		wides_[r.phi] = sums[k];
	}
	at(wide_loop, body_.blocks[wide_loop].last);
	clone_scalars(true);
	auto set_back_edge = [&](Value phi, Value next) {
		Instruction& inst = body_.instructions[body_.temps[phi.index].def];
		body_.operands[inst.extra_first + 3] = next;
	};
	std::vector<Value> next_values(inductions_.size(), Value::none());
	Value next_counter = Value::none();
	for (size_t k = 0; k < inductions_.size(); ++k)
	{
		const Induction& iv = inductions_[k];
		const int64_t by = iv.step * static_cast<int64_t>(lanes_);
		Value next = Value::none();
		if (iv.type.is_pointer())
		{
			Instruction inst = make_instruction(Opcode::Index, Type::scalar(TypeKind::I8));
			inst.ops[0] = counters[k];
			inst.ops[1] = literal(by);
			next = emit(inst, iv.type);
		}
		else
			next = binary(BinaryOp::Add, iv.type, counters[k], literal(by), iv.type);
		set_back_edge(counters[k], next);
		next_values[k] = next;
		if (k == counter_)
			next_counter = next;
		if (in_wide_[iv.phi])
		{
			VectorOp add;
			lane_operation(BinaryOp::Add, iv.type, add);
			set_back_edge(lane_counters[k], vector(add, lane_counters[k], broadcast(literal(by), iv.type)));
		}
	}
	for (size_t k = 0; k < reductions_.size(); ++k)
		set_back_edge(sums[k], reductions_[k].ordered ? reductions_[k].next_sum : wides_[reductions_[k].next]);
	const Value more = up ? compare(below, type, next_counter, limit) : compare(below, type, limit, next_counter);
	Instruction back = make_instruction(Opcode::Branch, Type::scalar(TypeKind::Void));
	back.ops[0] = more;
	back.ops[1] = Value::block(wide_loop);
	back.ops[2] = Value::block(exit);
	finish_block(back);

	// The vector preheader ends now that nothing more is added to it.
	at(vector_pre_, pre_tail_);
	Instruction enter = make_instruction(Opcode::Jump, Type::scalar(TypeKind::Void));
	enter.ops[0] = Value::block(wide_loop);
	finish_block(enter);

	// What the scalar loop starts from.
	at(exit, kNoId);
	std::vector<Value> finals(reductions_.size(), Value::none());
	for (size_t k = 0; k < reductions_.size(); ++k)
		finals[k] = reductions_[k].ordered ? reductions_[k].next_sum
										   : add_across(reductions_[k], wides_[reductions_[k].next]);
	Instruction leave = make_instruction(Opcode::Jump, Type::scalar(TypeKind::Void));
	leave.ops[0] = Value::block(scalar_pre);
	finish_block(leave);

	at(scalar_pre, kNoId);
	auto scalar_start = [&](TempId phi_temp, Value start_value, Value from_lanes) {
		Value joined = Value::none();
		if (guard != kNoId)
			make_phi(scalar_pre, body_.temps[phi_temp].type,
				{ Value::block(pre), start_value, Value::block(guard), start_value, Value::block(exit), from_lanes },
				joined);
		else
			make_phi(scalar_pre, body_.temps[phi_temp].type,
				{ Value::block(pre), start_value, Value::block(exit), from_lanes }, joined);
		Instruction& phi = body_.instructions[body_.temps[phi_temp].def];
		for (uint32_t k = 0; k + 1 < phi.extra_count; k += 2)
			if (body_.operands[phi.extra_first + k].index == pre)
				body_.operands[phi.extra_first + k + 1] = joined;
	};
	for (size_t k = 0; k < inductions_.size(); ++k)
		scalar_start(inductions_[k].phi, inductions_[k].start, next_values[k]);
	for (size_t k = 0; k < reductions_.size(); ++k)
		scalar_start(reductions_[k].phi, reductions_[k].start, finals[k]);
	Instruction into = make_instruction(Opcode::Jump, Type::scalar(TypeKind::Void));
	into.ops[0] = Value::block(b_);
	at(scalar_pre, body_.blocks[scalar_pre].last);
	finish_block(into);
	rename_phi_incoming(body_, b_, pre, scalar_pre);
	++c_.stats.loops_vectorized;
}

bool LoopVectorizer::run()
{
	c_.ensure_cfg();
	find_loops(body_, cfg_, forest_);
	if (forest_.loops.empty())
		return false;
	c_.entry_block_of(vectorize_loops);
	bool changed = false;
	for (unsigned round = 0; round < 32; ++round)
	{
		done_.resize(body_.blocks.size(), 0);
		bool one = false;
		for (uint32_t loop = 0; loop < forest_.loops.size() && !one; ++loop)
		{
			if (!analyze(loop))
				continue;
			// (A preheader changes the graph: the loop comes round again.)
			if (forest_.loops[loop].preheader == kNoId)
			{
				one = make_loop_preheader(body_, cfg_, forest_, loop);
				done_[b_] = one ? 0 : 1;
				continue;
			}
			done_[b_] = 1;
			vectorize(loop);
			one = true;
		}
		if (!one)
			break;
		changed = true;
		c_.invalidate_cfg();
		c_.ensure_cfg();
		find_loops(body_, cfg_, forest_);
		map_instruction_blocks(body_, c_.block_of);
		c_.invalidate_block_of();
	}
	return changed;
}

} // namespace

bool vectorize_loops(FunctionContext& context)
{
	LoopVectorizer pass(context);
	return pass.run();
}

} // namespace opt
