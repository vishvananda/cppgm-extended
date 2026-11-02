#include <algorithm>

#include "lowir/builder.h"
#include "opt/loops.h"
#include "opt/passes.h"

namespace opt {

using namespace lowir;

namespace {

// At most this many new counters per loop: each takes a register for the loop.
const unsigned kCountersPerLoop = 8;
// The most addresses sink_steps follows from one phi.
const size_t kCarried = 16;
// The most additions one counter takes in.
const unsigned kShiftedTerms = 4;

// A product of a counter and a literal the addressing modes cannot do, or a
// value the loop does not change, becomes a counter of its own: a loop adds to
// it where it multiplied.
class StrengthReduction
{
public:
	explicit StrengthReduction(FunctionContext& context) : c_(context), body_(context.body), cfg_(context.cfg) {}

	bool run();
	bool sink();

private:
	struct Counter
	{
		TempId phi, next;
		InstId step;
		Value start;
		int64_t by;
		Value stride; // a step the loop does not change, or none: `by` is the step
		bool no_wrap;
	};
	struct Scaled
	{
		uint32_t counter;
		Value factor;
		Value offset; // what the counter adds before the multiply, or none
		Type type;
		bool extended;
		TempId phi, next;
		Value by; // the step times the factor
	};

	void read_counters(uint32_t loop);
	int32_t counter_of(Value v, bool& extended) const;
	int32_t offset_counter_of(uint32_t loop, Value v, Value& offset) const;
	void reduce(uint32_t loop);
	Value emit_after(BlockId block, InstId& at, Instruction inst, Type type);
	void add_counter(const Loop& l, Scaled& s, bool no_wrap);
	Value multiply(Value a, Value b, Type type, BlockId block, InstId& at);
	Value product(Value start, Value factor, Type type, bool extended, BlockId block, InstId& at);
	bool outside(uint32_t loop, Value v) const;
	bool same_factor(Value a, Value b) const;
	bool wants_preheader(uint32_t loop) const;
	void read_uses();
	void sink_steps(uint32_t loop);
	void shift_counters(uint32_t loop);
	void shift_counter(const Loop& l, uint32_t loop, InstId phi_id, InstId& at);

	FunctionContext& c_;
	FunctionBody& body_;
	Cfg& cfg_;
	LoopForest forest_;
	std::vector<Counter> counters_;
	std::vector<Scaled> scaled_;
	std::vector<TempId> carried_;               // sink_steps: the phi and addresses stepping from it
	std::vector<uint32_t> uses_;                // per temp
	std::vector<InstId> first_reader_, reader_; // per temp: the first and last in layout order
	bool changed_ = false;
};

// Whether the addressing modes or one `lea` already give the product.
bool cheap_factor(int64_t factor)
{
	const uint64_t size = factor < 0 ? static_cast<uint64_t>(0) - static_cast<uint64_t>(factor)
									 : static_cast<uint64_t>(factor);
	return size == 3 || size == 5 || size == 9 || size == 1 || size == 2 || size == 4 || size == 8;
}

void StrengthReduction::read_counters(uint32_t loop)
{
	const Loop& l = forest_.loops[loop];
	counters_.clear();
	for (InstId i = body_.blocks[l.header].first; i != kNoId; i = body_.instructions[i].next)
	{
		const Instruction& phi = body_.instructions[i];
		if (phi.op != Opcode::Phi)
			break;
		if (phi.extra_count != 4 || (phi.type.kind() != TypeKind::I32 && phi.type.kind() != TypeKind::I64))
			continue;
		const Value start = phi_incoming(body_, phi, l.preheader);
		const BlockId latch = body_.operands[phi.extra_first].index == l.preheader
			? body_.operands[phi.extra_first + 2].index
			: body_.operands[phi.extra_first].index;
		const Value next = phi_incoming(body_, phi, latch);
		if (!next.is(ValueKind::Temp) || next.index < body_.parameter_count ||
			!in_loop(forest_, loop, c_.block_of[body_.temps[next.index].def]))
			continue;
		const InstId step = body_.temps[next.index].def;
		const Instruction& add = body_.instructions[step];
		const BinaryOp op = static_cast<BinaryOp>(add.sub);
		if (add.op != Opcode::Binary || (op != BinaryOp::Add && op != BinaryOp::Sub))
			continue;
		int64_t by = 0;
		Value stride = Value::none();
		const Value self = Value::temp(phi.result);
		if (add.ops[0] == self && add.ops[1].is(ValueKind::Integer))
		{
			by = static_cast<int64_t>(body_.literals[add.ops[1].index].low);
			by = op == BinaryOp::Add ? by : -by;
		}
		else if (op == BinaryOp::Add && (add.ops[0] == self || add.ops[1] == self))
		{
			stride = add.ops[add.ops[0] == self ? 1 : 0];
			if (!stride.is(ValueKind::Temp) || !outside(loop, stride))
				continue;
		}
		else
			continue;
		counters_.push_back(Counter{ phi.result, next.index, step, start, by, stride,
			(add.flags & IF_NO_WRAP) != 0 || phi.type.kind() == TypeKind::I64 });
	}
}

// The counter `v` reads, directly or sign-extended.
int32_t StrengthReduction::counter_of(Value v, bool& extended) const
{
	extended = false;
	if (!v.is(ValueKind::Temp))
		return -1;
	TempId temp = v.index;
	const InstId def = temp >= body_.parameter_count ? body_.temps[temp].def : kNoId;
	if (def != kNoId && body_.instructions[def].op == Opcode::Convert &&
		static_cast<ConvertOp>(body_.instructions[def].sub) == ConvertOp::SExt &&
		body_.instructions[def].ops[0].is(ValueKind::Temp))
	{
		extended = true;
		temp = body_.instructions[def].ops[0].index;
	}
	for (size_t k = 0; k < counters_.size(); ++k)
		if (counters_[k].phi == temp &&
			(!extended || (counters_[k].no_wrap && counters_[k].stride.is(ValueKind::None))))
			return static_cast<int32_t>(k);
	return -1;
}

// The counter `v` adds what the loop does not change to, in the counter's
// type: the product of the sum is the counter's product plus the addend's.
int32_t StrengthReduction::offset_counter_of(uint32_t loop, Value v, Value& offset) const
{
	if (!v.is(ValueKind::Temp) || v.index < body_.parameter_count || body_.temps[v.index].def == kNoId)
		return -1;
	const Instruction& add = body_.instructions[body_.temps[v.index].def];
	if (add.op != Opcode::Binary || static_cast<BinaryOp>(add.sub) != BinaryOp::Add)
		return -1;
	for (unsigned k = 0; k < 2; ++k)
	{
		bool extended = false;
		const int32_t counter = counter_of(add.ops[k], extended);
		if (counter >= 0 && !extended && outside(loop, add.ops[1 - k]) &&
			body_.temps[counters_[static_cast<size_t>(counter)].phi].type.kind() == add.type.kind())
		{
			offset = add.ops[1 - k];
			return counter;
		}
	}
	return -1;
}

Value StrengthReduction::emit_after(BlockId block, InstId& at, Instruction inst, Type type)
{
	const InstId id = append_instruction(body_, inst);
	body_.instructions[id].result = append_temp(body_, type, id);
	insert_after(body_, block, at, id);
	c_.block_of.resize(body_.instructions.size(), kNoId);
	c_.block_of[id] = block;
	at = id;
	return Value::temp(body_.instructions[id].result);
}

// Whether the loop does not compute `v`.
bool StrengthReduction::outside(uint32_t loop, Value v) const
{
	if (!v.is(ValueKind::Temp) || v.index < body_.parameter_count)
		return true;
	const InstId def = body_.temps[v.index].def;
	return def != kNoId && c_.block_of[def] != kNoId && !in_loop(forest_, loop, c_.block_of[def]);
}

bool StrengthReduction::same_factor(Value a, Value b) const
{
	if (a.is(ValueKind::Integer) && b.is(ValueKind::Integer))
		return body_.literals[a.index].low == body_.literals[b.index].low;
	return a == b;
}

Value StrengthReduction::multiply(Value a, Value b, Type type, BlockId block, InstId& at)
{
	if (a.is(ValueKind::Integer) && b.is(ValueKind::Integer))
		return integer_value(
			body_, integer_literal(static_cast<int64_t>(body_.literals[a.index].low * body_.literals[b.index].low)));
	if (a.is(ValueKind::Integer))
		std::swap(a, b);
	if (b.is(ValueKind::Integer) && body_.literals[b.index].low == 1)
		return a;
	if (b.is(ValueKind::Integer) && body_.literals[b.index].low == 0)
		return b;
	Instruction mul = make_instruction(Opcode::Binary, type);
	mul.sub = static_cast<uint8_t>(BinaryOp::Mul);
	mul.ops[0] = a;
	mul.ops[1] = b;
	return emit_after(block, at, mul, type);
}

// start * factor at `at`, in type `type`.
Value StrengthReduction::product(Value start, Value factor, Type type, bool extended, BlockId block, InstId& at)
{
	Value from = start;
	if (extended && start.is(ValueKind::Integer))
		from = integer_value(body_, integer_literal(static_cast<int32_t>(body_.literals[start.index].low)));
	else if (extended)
	{
		Instruction widen = make_instruction(Opcode::Convert, type);
		widen.sub = static_cast<uint8_t>(ConvertOp::SExt);
		set_convert_source_type(widen, Type::scalar(TypeKind::I32));
		widen.ops[0] = start;
		from = emit_after(block, at, widen, type);
	}
	return multiply(from, factor, type, block, at);
}

void StrengthReduction::add_counter(const Loop& l, Scaled& s, bool no_wrap)
{
	const Counter counter = counters_[s.counter];
	InstId at = previous_in_block(body_, l.preheader, body_.blocks[l.preheader].last);
	Value first = counter.start;
	const bool zero = first.is(ValueKind::Integer) && body_.literals[first.index].low == 0;
	if (zero && !s.offset.is(ValueKind::None))
		first = s.offset;
	else if (!s.offset.is(ValueKind::None))
	{
		Instruction sum = make_instruction(Opcode::Binary, s.type);
		sum.sub = static_cast<uint8_t>(BinaryOp::Add);
		sum.ops[0] = counter.start;
		sum.ops[1] = s.offset;
		first = emit_after(l.preheader, at, sum, s.type);
	}
	const Value start = product(first, s.factor, s.type, s.extended, l.preheader, at);
	const BlockId stepping = c_.block_of[counter.step];
	Instruction add = make_instruction(Opcode::Binary, s.type);
	add.sub = static_cast<uint8_t>(BinaryOp::Add);
	add.flags = no_wrap ? IF_NO_WRAP : 0;
	// (Products of one counter and factor step by one value.)
	s.by = Value::none();
	for (const Scaled& other : scaled_)
		if (&other != &s && other.counter == s.counter && same_factor(other.factor, s.factor) &&
			other.type.kind() == s.type.kind() && other.extended == s.extended)
			s.by = other.by;
	if (s.by.is(ValueKind::None))
		s.by = multiply(
			counter.stride.is(ValueKind::None) ? integer_value(body_, integer_literal(counter.by)) : counter.stride,
			s.factor, s.type, l.preheader, at);
	add.ops[1] = s.by;
	InstId after = counter.step;
	const InstId step = append_instruction(body_, add);
	s.next = append_temp(body_, s.type, step);
	body_.instructions[step].result = s.next;
	insert_after(body_, stepping, after, step);
	c_.block_of.resize(body_.instructions.size(), kNoId);
	c_.block_of[step] = stepping;

	const Instruction& old = body_.instructions[body_.temps[counter.phi].def];
	BlockId latch = body_.operands[old.extra_first].index;
	if (latch == l.preheader)
		latch = body_.operands[old.extra_first + 2].index;
	const Value pairs[4] = { Value::block(l.preheader), start, Value::block(latch), Value::temp(s.next) };
	Instruction phi = make_instruction(Opcode::Phi, s.type);
	phi.extra_first = append_operands(body_, pairs, 4);
	phi.extra_count = 4;
	InstId front = kNoId;
	s.phi = emit_after(l.header, front, phi, s.type).index;
	body_.instructions[step].ops[0] = Value::temp(s.phi);
}

void StrengthReduction::reduce(uint32_t loop)
{
	const Loop& l = forest_.loops[loop];
	read_counters(loop);
	if (counters_.empty())
		return;
	scaled_.clear();
	// A value carried across a call costs more than the multiply it saves,
	// unless the call is on a rare path.
	for (BlockId b : l.blocks)
		for (InstId i = body_.blocks[b].first; i != kNoId; i = body_.instructions[i].next)
			if (body_.instructions[i].op == Opcode::Call && !rare_float_path(body_, cfg_, b))
				return;
	for (BlockId b : l.blocks)
		for (InstId i = body_.blocks[b].first; i != kNoId; i = body_.instructions[i].next)
		{
			Instruction& inst = body_.instructions[i];
			if (inst.op != Opcode::Binary || static_cast<BinaryOp>(inst.sub) != BinaryOp::Mul ||
				(inst.type.kind() != TypeKind::I32 && inst.type.kind() != TypeKind::I64))
				continue;
			bool extended = false;
			Value offset = Value::none();
			int32_t counter = -1;
			Value factor = Value::none();
			for (unsigned side = 0; side < 2 && counter < 0; ++side)
			{
				factor = inst.ops[1 - side];
				counter = counter_of(inst.ops[side], extended);
				if (counter < 0)
					counter = offset_counter_of(loop, inst.ops[side], offset);
			}
			if (counter < 0 || !outside(loop, factor))
				continue;
			if (factor.is(ValueKind::Integer))
			{
				const int64_t literal = static_cast<int64_t>(body_.literals[factor.index].low);
				if (cheap_factor(literal) || literal != static_cast<int32_t>(literal))
					continue;
			}
			const Type start_type = body_.temps[counters_[static_cast<size_t>(counter)].phi].type;
			if (!extended && !(start_type.kind() == inst.type.kind()))
				continue;
			size_t k = 0;
			for (; k < scaled_.size(); ++k)
				if (scaled_[k].counter == static_cast<uint32_t>(counter) && same_factor(scaled_[k].factor, factor) &&
					scaled_[k].type.kind() == inst.type.kind() && scaled_[k].extended == extended &&
					(scaled_[k].offset.is(ValueKind::None)
							? offset.is(ValueKind::None)
							: !offset.is(ValueKind::None) && same_factor(scaled_[k].offset, offset)))
					break;
			if (k == scaled_.size())
			{
				if (scaled_.size() == kCountersPerLoop)
					continue;
				scaled_.push_back(
					Scaled{ static_cast<uint32_t>(counter), factor, offset, inst.type, extended, 0, 0, Value::none() });
				add_counter(l, scaled_.back(),
					(inst.flags & IF_NO_WRAP) != 0 && counters_[counter].no_wrap && offset.is(ValueKind::None));
			}
			// (`inst` still names its instruction: adding appends to storage.)
			Instruction& mul = body_.instructions[i];
			mul.op = Opcode::Copy;
			mul.sub = 0;
			mul.flags = 0;
			mul.ops[0] = Value::temp(scaled_[k].phi);
			mul.ops[1] = Value::none();
			++c_.stats.multiplies_reduced;
			changed_ = true;
		}
}

// Whether the loop multiplies one of its header's phis, or what extends one.
bool StrengthReduction::wants_preheader(uint32_t loop) const
{
	const Loop& l = forest_.loops[loop];
	if (l.preheader != kNoId)
		return false;
	auto header_phi = [&](Value v) {
		if (!v.is(ValueKind::Temp) || v.index < body_.parameter_count)
			return false;
		InstId def = body_.temps[v.index].def;
		if (def != kNoId && body_.instructions[def].op == Opcode::Convert &&
			body_.instructions[def].ops[0].is(ValueKind::Temp) &&
			body_.instructions[def].ops[0].index >= body_.parameter_count)
			def = body_.temps[body_.instructions[def].ops[0].index].def;
		return def != kNoId && body_.instructions[def].op == Opcode::Phi && c_.block_of[def] == l.header;
	};
	for (BlockId b : l.blocks)
		for (InstId i = body_.blocks[b].first; i != kNoId; i = body_.instructions[i].next)
		{
			const Instruction& inst = body_.instructions[i];
			if (inst.op == Opcode::Call && !rare_float_path(body_, cfg_, b))
				return false;
			if (inst.op == Opcode::Binary && static_cast<BinaryOp>(inst.sub) == BinaryOp::Mul &&
				(header_phi(inst.ops[0]) || header_phi(inst.ops[1])))
				return true;
		}
	return false;
}

void StrengthReduction::read_uses()
{
	uses_.assign(body_.temps.size(), 0);
	first_reader_.assign(body_.temps.size(), kNoId);
	reader_.assign(body_.temps.size(), kNoId);
	for (BlockId b : body_.layout)
		for (InstId i = body_.blocks[b].first; i != kNoId; i = body_.instructions[i].next)
			for_each_operand(body_, body_.instructions[i], [&](const Value& v) {
				if (!v.is(ValueKind::Temp))
					return;
				if (uses_[v.index]++ == 0)
					first_reader_[v.index] = i;
				reader_[v.index] = i;
			});
}

// A counter of any invariant step read only through additions of what the
// loop does not change: it starts at their sum instead, and the additions go.
void StrengthReduction::shift_counter(const Loop& l, uint32_t loop, InstId phi_id, InstId& at)
{
	const Instruction phi = body_.instructions[phi_id];
	const uint32_t entry = body_.operands[phi.extra_first].index == l.preheader ? 0 : 2;
	if (body_.operands[phi.extra_first + entry].index != l.preheader)
		return;
	const Value next = body_.operands[phi.extra_first + (2 - entry) + 1];
	const Value self = Value::temp(phi.result);
	if (!next.is(ValueKind::Temp) || next.index < body_.parameter_count || uses_[phi.result] != 2 ||
		uses_[next.index] != 1)
		return;
	const InstId step = body_.temps[next.index].def;
	const Instruction& add = body_.instructions[step];
	if (step == kNoId || add.op != Opcode::Binary || static_cast<BinaryOp>(add.sub) != BinaryOp::Add ||
		!in_loop(forest_, loop, c_.block_of[step]))
		return;
	const bool first = add.ops[0] == self;
	if ((!first && !(add.ops[1] == self)) || !outside(loop, add.ops[first ? 1 : 0]))
		return;
	InstId read = first_reader_[phi.result] == step ? reader_[phi.result] : first_reader_[phi.result];
	Value value = self;
	Value terms[kShiftedTerms];
	unsigned count = 0;
	InstId last_add = kNoId;
	bool no_wrap = true;
	for (unsigned depth = 0; depth < kShiftedTerms * 2 && read != kNoId; ++depth)
	{
		const Instruction& inst = body_.instructions[read];
		if (!in_loop(forest_, loop, c_.block_of[read]) || inst.type.kind() != phi.type.kind())
			break;
		if (inst.op == Opcode::Binary && static_cast<BinaryOp>(inst.sub) == BinaryOp::Add && count < kShiftedTerms)
		{
			const Value other = inst.ops[0] == value ? inst.ops[1] : inst.ops[0];
			if (!outside(loop, other))
				break;
			terms[count++] = other;
			last_add = read;
			no_wrap = no_wrap && (inst.flags & IF_NO_WRAP) != 0 && c_.block_of[read] == l.header;
		}
		else if (inst.op != Opcode::Copy)
			break;
		value = Value::temp(inst.result);
		read = uses_[inst.result] == 1 ? reader_[inst.result] : kNoId;
	}
	if (last_add == kNoId)
		return;
	Value start = body_.operands[phi.extra_first + entry + 1];
	for (unsigned k = 0; k < count; ++k)
	{
		const bool zero = start.is(ValueKind::Integer) && body_.literals[start.index].low == 0 &&
			body_.literals[start.index].high == 0;
		if (zero)
		{
			start = terms[k];
			continue;
		}
		Instruction sum = make_instruction(Opcode::Binary, phi.type);
		sum.sub = static_cast<uint8_t>(BinaryOp::Add);
		sum.ops[0] = start;
		sum.ops[1] = terms[k];
		start = emit_after(l.preheader, at, sum, phi.type);
	}
	const Instruction& placed = body_.instructions[phi_id];
	body_.operands[placed.extra_first + entry + 1] = start;
	const Value shifted = Value::temp(body_.instructions[last_add].result);
	Instruction copy = make_instruction(Opcode::Copy, phi.type);
	copy.result = shifted.index;
	copy.ops[0] = self;
	copy.next = body_.instructions[last_add].next;
	copy.location = body_.instructions[last_add].location;
	body_.instructions[last_add] = copy;
	for (BlockId b : l.blocks)
		for (InstId i = body_.blocks[b].first; i != kNoId; i = body_.instructions[i].next)
			if (i != last_add)
				for_each_operand(body_, body_.instructions[i], [&](Value& v) {
					if (v == shifted)
						v = self;
				});
	if (!no_wrap)
		body_.instructions[step].flags &= static_cast<uint8_t>(~IF_NO_WRAP);
	++c_.stats.counters_shifted;
	changed_ = true;
}

void StrengthReduction::shift_counters(uint32_t loop)
{
	const Loop& l = forest_.loops[loop];
	InstId at = previous_in_block(body_, l.preheader, body_.blocks[l.preheader].last);
	for (InstId i = body_.blocks[l.header].first; i != kNoId; i = body_.instructions[i].next)
	{
		const Instruction& phi = body_.instructions[i];
		if (phi.op != Opcode::Phi)
			break;
		if (phi.extra_count == 4 && (phi.type.kind() == TypeKind::I32 || phi.type.kind() == TypeKind::I64))
			shift_counter(l, loop, i, at);
	}
}

// In a one-block loop, a step of a phi that runs before the phi's last reader
// holds both values at once, and the back edge copies one to the other: the
// step (with the compare only the branch reads, when that reads the step) moves
// after that reader.
void StrengthReduction::sink_steps(uint32_t loop)
{
	const Loop& l = forest_.loops[loop];
	const BlockId b = l.header;
	if (l.blocks.size() != 1)
		return;
	const InstId term = body_.blocks[b].last;
	std::vector<InstId> order;
	auto read_order = [&]() {
		order.clear();
		for (InstId i = body_.blocks[b].first; i != kNoId; i = body_.instructions[i].next)
			order.push_back(i);
	};
	auto reads = [&](InstId i, TempId t) {
		bool found = false;
		for_each_operand(body_, body_.instructions[i], [&](const Value& v) { found = found || v == Value::temp(t); });
		return found;
	};
	read_order();
	const size_t phis = static_cast<size_t>(std::find_if(order.begin(), order.end(), [&](InstId i) {
		return body_.instructions[i].op != Opcode::Phi;
	}) - order.begin());
	for (size_t p = 0; p < phis; ++p)
	{
		const Instruction phi = body_.instructions[order[p]];
		const Value next = phi_incoming(body_, phi, b);
		if (!next.is(ValueKind::Temp) || next.index < body_.parameter_count)
			continue;
		const InstId step = body_.temps[next.index].def;
		const size_t at = static_cast<size_t>(std::find(order.begin() + phis, order.end(), step) - order.begin());
		if (at == order.size() || !is_pure(body_.instructions[step]) || !reads(step, phi.result))
			continue;
		// (An address steps from the phi at the access that reads it.)
		size_t last = at;
		carried_.assign(1, phi.result);
		for (size_t k = phis; k < order.size(); ++k)
		{
			if (k == at)
				continue;
			const Instruction& inst = body_.instructions[order[k]];
			bool read = false;
			for (TempId t : carried_)
				read = read || reads(order[k], t);
			if (!read)
				continue;
			last = std::max(last, k);
			if (inst.op == Opcode::Index && inst.result != kNoId && carried_.size() < kCarried)
				carried_.push_back(inst.result);
		}
		if (last == at || order[last] == term)
			continue;
		// What reads the step before `last`: at most the branch's compare,
		// perhaps through the truncation of a widened counter.
		InstId compare = kNoId, narrow = kNoId;
		bool blocked = false;
		for (size_t k = at + 1; k <= last && !blocked; ++k)
		{
			if (!reads(order[k], next.index) &&
				(narrow == kNoId || !reads(order[k], body_.instructions[narrow].result)))
				continue;
			const Instruction& reader = body_.instructions[order[k]];
			if (narrow == kNoId && reader.op == Opcode::Convert && (reader.flags & IF_EXACT) != 0 &&
				reads(order[k], next.index))
			{
				narrow = order[k];
				continue;
			}
			const Value tested = body_.instructions[term].op == Opcode::Branch ? body_.instructions[term].ops[0]
																			   : Value::none();
			blocked = compare != kNoId || reader.op != Opcode::Compare || !(tested == Value::temp(reader.result));
			for (size_t j = k + 1; j + 1 < order.size() && !blocked; ++j)
				blocked = reads(order[j], reader.result);
			compare = order[k];
		}
		if (blocked)
			continue;
		const InstId tail = order[last];
		unlink_after(body_, b, previous_in_block(body_, b, step), step);
		insert_after(body_, b, tail, step);
		InstId placed = step;
		if (narrow != kNoId)
		{
			unlink_after(body_, b, previous_in_block(body_, b, narrow), narrow);
			insert_after(body_, b, placed, narrow);
			placed = narrow;
		}
		if (compare != kNoId)
		{
			unlink_after(body_, b, previous_in_block(body_, b, compare), compare);
			insert_after(body_, b, placed, compare);
		}
		changed_ = true;
		read_order();
	}
}

bool StrengthReduction::run()
{
	c_.ensure_cfg();
	find_loops(body_, cfg_, forest_);
	if (forest_.loops.empty())
		return false;
	c_.entry_block_of(reduce_strength);
	// (The preheaders made change the graph: the loops are found again.)
	for (unsigned round = 0; round < 16; ++round)
	{
		bool one = false;
		for (uint32_t loop = 0; loop < forest_.loops.size(); ++loop)
			one = (wants_preheader(loop) && make_loop_preheader(body_, cfg_, forest_, loop)) || one;
		if (!one)
			break;
		changed_ = true;
		c_.invalidate_cfg();
		c_.ensure_cfg();
		find_loops(body_, cfg_, forest_);
		map_instruction_blocks(body_, c_.block_of);
		c_.invalidate_block_of();
		c_.entry_block_of(reduce_strength);
	}
	for (uint32_t loop = static_cast<uint32_t>(forest_.loops.size()); loop-- > 0;)
		if (forest_.loops[loop].preheader != kNoId)
			reduce(loop);
	read_uses();
	for (uint32_t loop = static_cast<uint32_t>(forest_.loops.size()); loop-- > 0;)
		if (forest_.loops[loop].preheader != kNoId)
			shift_counters(loop);
	if (changed_)
		c_.invalidate_block_of();
	return changed_;
}

bool StrengthReduction::sink()
{
	c_.ensure_cfg();
	find_loops(body_, cfg_, forest_);
	for (uint32_t loop = static_cast<uint32_t>(forest_.loops.size()); loop-- > 0;)
		sink_steps(loop);
	return changed_;
}

} // namespace

bool reduce_strength(FunctionContext& context)
{
	StrengthReduction pass(context);
	return pass.run();
}

bool sink_counter_steps(FunctionContext& context)
{
	StrengthReduction pass(context);
	return pass.sink();
}

} // namespace opt
