#include "lowir/builder.h"
#include "opt/loops.h"
#include "opt/passes.h"

namespace opt {

using namespace lowir;

namespace {

// The most values outside the loop one index adds to its counter.
const unsigned kSumTerms = 2;
// How far one index's sum is followed.
const unsigned kSumDepth = 4;

// An `int` counter that never wraps equals, extended, a 64-bit counter of the
// same step. Registers hold narrow values extended, so each addition to the
// narrow counter that indexes memory costs an extension, and the sum of a
// counter and a value the loop does not change costs an addition as well. The
// 64-bit counter needs neither: the value goes into the base, or the literal
// into the displacement, before the loop or at the access. What still reads
// the narrow value reads an exact truncation, which costs nothing.
class CounterWidening
{
public:
	explicit CounterWidening(FunctionContext& context) : c_(context), body_(context.body), cfg_(context.cfg) {}

	bool run();

private:
	struct Counter
	{
		InstId phi_id, step;
		TempId phi, next;
		Value start, by;
		bool is_signed, down;
		bool wanted;
		Value wide, wide_next;
	};
	// counter + offset + terms, each extended.
	struct Sum
	{
		uint32_t counter;
		bool stepped; // of the counter's next value
		int64_t offset;
		Value terms[kSumTerms];
		unsigned count;
	};
	struct Read
	{
		InstId id, previous; // (never moves)
		Sum sum;
	};
	struct Rebased
	{
		Value base, term, result;
		Type type;
	};
	struct Extended
	{
		Value value, result;
	};

	bool outside(uint32_t loop, Value v) const;
	int64_t literal_of(Value v, bool is_signed) const;
	void read_counters(uint32_t loop);
	bool sum_of(uint32_t loop, Value v, Sum& sum, unsigned depth) const;
	bool add_term(Value v, bool is_signed, Sum& sum) const;
	void read_loop(uint32_t loop);
	bool wanted(uint32_t loop);
	void forget_counters();
	void widen(uint32_t loop);
	void widen_counter(const Loop& l, Counter& counter);
	void rewrite(const Loop& l, uint32_t loop, Read& read);
	Value full_sum(const Read& read, BlockId block, InstId& at);
	Value extended(const Loop& l, Value v, bool is_signed);
	Value rebased(const Loop& l, Value base, Type type, Value term);
	Value emit_after(BlockId block, InstId& at, Instruction inst, Type type);

	FunctionContext& c_;
	FunctionBody& body_;
	Cfg& cfg_;
	LoopForest forest_;
	std::vector<Counter> counters_;
	std::vector<Read> reads_;
	std::vector<Rebased> rebased_;
	std::vector<Extended> extended_;
	std::vector<int32_t> role_; // per temp: 2k for counter k's phi, 2k + 1 for its next value, else -1
	InstId preheader_end_ = kNoId;
	uint32_t current_ = 0;
	bool changed_ = false;
};

bool CounterWidening::outside(uint32_t loop, Value v) const
{
	if (!v.is(ValueKind::Temp) || v.index < body_.parameter_count)
		return true;
	const InstId def = body_.temps[v.index].def;
	return def != kNoId && c_.block_of[def] != kNoId && !in_loop(forest_, loop, c_.block_of[def]);
}

// A 32-bit literal's value at the counter's signedness.
int64_t CounterWidening::literal_of(Value v, bool is_signed) const
{
	const uint32_t bits = static_cast<uint32_t>(body_.literals[v.index].low);
	return is_signed ? static_cast<int64_t>(static_cast<int32_t>(bits)) : static_cast<int64_t>(bits);
}

void CounterWidening::read_counters(uint32_t loop)
{
	const Loop& l = forest_.loops[loop];
	counters_.clear();
	if (l.latches.size() != 1)
		return;
	for (InstId i = body_.blocks[l.header].first; i != kNoId; i = body_.instructions[i].next)
	{
		const Instruction& phi = body_.instructions[i];
		if (phi.op != Opcode::Phi)
			break;
		const TypeKind kind = phi.type.kind();
		if (phi.extra_count != 4 || (kind != TypeKind::I32 && kind != TypeKind::U32))
			continue;
		const uint32_t back = body_.operands[phi.extra_first].index == l.latches[0] ? 0 : 2;
		if (body_.operands[phi.extra_first + back].index != l.latches[0])
			continue;
		const Value next = body_.operands[phi.extra_first + back + 1];
		if (!next.is(ValueKind::Temp) || next.index < body_.parameter_count || body_.temps[next.index].def == kNoId)
			continue;
		const InstId step = body_.temps[next.index].def;
		const Instruction& add = body_.instructions[step];
		if (add.op != Opcode::Binary || (add.flags & IF_NO_WRAP) == 0 || add.type.kind() != kind ||
			!in_loop(forest_, loop, c_.block_of[step]))
			continue;
		const BinaryOp op = static_cast<BinaryOp>(add.sub);
		const Value self = Value::temp(phi.result);
		Value by;
		if (op == BinaryOp::Add && add.ops[0] == self)
			by = add.ops[1];
		else if (op == BinaryOp::Add && add.ops[1] == self)
			by = add.ops[0];
		else if (op == BinaryOp::Sub && add.ops[0] == self && add.ops[1].is(ValueKind::Integer))
			by = add.ops[1];
		else
			continue;
		if (!outside(loop, by) || !(by.is(ValueKind::Integer) || by.is(ValueKind::Temp)) ||
			(by.is(ValueKind::Temp) && body_.temps[by.index].type.kind() != kind))
			continue;
		const Value start = body_.operands[phi.extra_first + (2 - back) + 1];
		if (start.is(ValueKind::Temp) && body_.temps[start.index].type.kind() != kind)
			continue;
		role_[phi.result] = static_cast<int32_t>(counters_.size() * 2);
		role_[next.index] = static_cast<int32_t>(counters_.size() * 2 + 1);
		counters_.push_back(Counter{ i, step, phi.result, next.index, start, by, kind == TypeKind::I32,
			op == BinaryOp::Sub, false, Value::none(), Value::none() });
	}
}

void CounterWidening::forget_counters()
{
	for (const Counter& counter : counters_)
		role_[counter.phi] = role_[counter.next] = -1;
}

bool CounterWidening::add_term(Value v, bool is_signed, Sum& sum) const
{
	if (v.is(ValueKind::Integer))
	{
		sum.offset += literal_of(v, is_signed);
		return true;
	}
	if (!v.is(ValueKind::Temp) || sum.count == kSumTerms ||
		body_.temps[v.index].type.kind() != (is_signed ? TypeKind::I32 : TypeKind::U32))
		return false;
	sum.terms[sum.count++] = v;
	return true;
}

// Whether `v` is a counter plus values the loop does not change, each addition
// free of wrapping (so the extended sum is the sum of the extensions).
bool CounterWidening::sum_of(uint32_t loop, Value v, Sum& sum, unsigned depth) const
{
	if (!v.is(ValueKind::Temp) || v.index >= role_.size() || v.index < body_.parameter_count)
		return false;
	const int32_t role = role_[v.index];
	if (role >= 0)
	{
		sum.counter = static_cast<uint32_t>(role) / 2;
		sum.stepped = (role & 1) != 0;
		return true;
	}
	const InstId def = body_.temps[v.index].def;
	if (def == kNoId || depth == 0)
		return false;
	const Instruction& inst = body_.instructions[def];
	if (inst.op != Opcode::Binary || (inst.flags & IF_NO_WRAP) == 0 || !in_loop(forest_, loop, c_.block_of[def]))
		return false;
	const BinaryOp op = static_cast<BinaryOp>(inst.sub);
	const TypeKind kind = inst.type.kind();
	if (kind != TypeKind::I32 && kind != TypeKind::U32)
		return false;
	const bool is_signed = kind == TypeKind::I32;
	if (op == BinaryOp::Sub && inst.ops[1].is(ValueKind::Integer))
	{
		if (!sum_of(loop, inst.ops[0], sum, depth - 1) || counters_[sum.counter].is_signed != is_signed)
			return false;
		sum.offset -= literal_of(inst.ops[1], is_signed);
		return true;
	}
	if (op != BinaryOp::Add)
		return false;
	const bool first = outside(loop, inst.ops[1]);
	if (!first && !outside(loop, inst.ops[0]))
		return false;
	if (!sum_of(loop, inst.ops[first ? 0 : 1], sum, depth - 1) || counters_[sum.counter].is_signed != is_signed)
		return false;
	return add_term(inst.ops[first ? 1 : 0], is_signed, sum);
}

// The reads the wide counter serves. Only an index or an extension makes a
// counter worth widening.
void CounterWidening::read_loop(uint32_t loop)
{
	const Loop& l = forest_.loops[loop];
	reads_.clear();
	for (BlockId b : l.blocks)
	{
		InstId previous = kNoId;
		for (InstId i = body_.blocks[b].first; i != kNoId; previous = i, i = body_.instructions[i].next)
		{
			const Instruction& inst = body_.instructions[i];
			Read read = Read();
			read.id = i;
			read.previous = previous;
			Value v = Value::none();
			if (inst.op == Opcode::Index)
				v = inst.ops[1];
			else if (inst.op == Opcode::Convert && inst.type.kind() == TypeKind::I64 &&
				(static_cast<ConvertOp>(inst.sub) == ConvertOp::SExt ||
					static_cast<ConvertOp>(inst.sub) == ConvertOp::ZExt))
				v = inst.ops[0];
			if (v.is(ValueKind::None) || !sum_of(loop, v, read.sum, kSumDepth))
				continue;
			Counter& counter = counters_[read.sum.counter];
			if (inst.op == Opcode::Convert &&
				(convert_source_type(inst).kind() != (counter.is_signed ? TypeKind::I32 : TypeKind::U32) ||
					(static_cast<ConvertOp>(inst.sub) == ConvertOp::SExt) != counter.is_signed))
				continue;
			if (inst.op == Opcode::Index && body_.temps[v.index].type.kind() != body_.temps[counter.phi].type.kind())
				continue;
			counter.wanted = true;
			reads_.push_back(read);
		}
	}
}

// Whether the loop lacks the preheader a widening needs.
bool CounterWidening::wanted(uint32_t loop)
{
	if (forest_.loops[loop].preheader != kNoId)
		return false;
	read_counters(loop);
	bool any = false;
	if (!counters_.empty())
	{
		read_loop(loop);
		for (const Counter& counter : counters_)
			any = any || counter.wanted;
	}
	forget_counters();
	return any;
}

Value CounterWidening::emit_after(BlockId block, InstId& at, Instruction inst, Type type)
{
	const InstId id = append_instruction(body_, inst);
	body_.instructions[id].result = append_temp(body_, type, id);
	insert_after(body_, block, at, id);
	c_.block_of.resize(body_.instructions.size(), kNoId);
	c_.block_of[id] = block;
	at = id;
	return Value::temp(body_.instructions[id].result);
}

// `v` extended, before the loop.
Value CounterWidening::extended(const Loop& l, Value v, bool is_signed)
{
	if (v.is(ValueKind::Integer))
		return integer_value(body_, integer_literal(literal_of(v, is_signed)));
	for (const Extended& known : extended_)
		if (known.value == v)
			return known.result;
	Instruction widen = make_instruction(Opcode::Convert, Type::scalar(TypeKind::I64));
	widen.sub = static_cast<uint8_t>(is_signed ? ConvertOp::SExt : ConvertOp::ZExt);
	set_convert_source_type(widen, Type::scalar(is_signed ? TypeKind::I32 : TypeKind::U32));
	widen.ops[0] = v;
	const Value result = emit_after(l.preheader, preheader_end_, widen, Type::scalar(TypeKind::I64));
	extended_.push_back(Extended{ v, result });
	return result;
}

// `base` advanced `term` elements, before the loop.
Value CounterWidening::rebased(const Loop& l, Value base, Type type, Value term)
{
	for (const Rebased& known : rebased_)
		if (known.base == base && known.term == term && known.type == type)
			return known.result;
	Instruction index = make_instruction(Opcode::Index, type);
	index.ops[0] = base;
	index.ops[1] = term;
	const Value result = emit_after(l.preheader, preheader_end_, index, body_.pointer_type());
	rebased_.push_back(Rebased{ base, term, result, type });
	return result;
}

// The phi and the step become 64-bit in place (nothing before them moves);
// truncations give what still reads the narrow values their definitions.
void CounterWidening::widen_counter(const Loop& l, Counter& counter)
{
	const Type i64 = Type::scalar(TypeKind::I64);
	const Type narrow = body_.temps[counter.phi].type;
	counter.wide = Value::temp(append_temp(body_, i64, counter.phi_id));
	counter.wide_next = Value::temp(append_temp(body_, i64, counter.step));
	const Value start = extended(l, counter.start, counter.is_signed);
	const Value by = extended(l, counter.by, counter.is_signed);
	auto narrowed = [&](Value wide, TempId result, BlockId block, InstId after) {
		Instruction trunc = make_instruction(Opcode::Convert, narrow);
		trunc.sub = static_cast<uint8_t>(ConvertOp::Trunc);
		trunc.flags = IF_EXACT;
		set_convert_source_type(trunc, i64);
		trunc.ops[0] = wide;
		trunc.result = result;
		const InstId id = append_instruction(body_, trunc);
		body_.temps[result].def = id;
		insert_after(body_, block, after, id);
		c_.block_of.resize(body_.instructions.size(), kNoId);
		c_.block_of[id] = block;
	};
	{
		Instruction& step = body_.instructions[counter.step];
		const BinaryOp op = static_cast<BinaryOp>(step.sub);
		step.type = i64;
		step.flags = IF_NO_WRAP;
		step.ops[0] = counter.wide;
		step.ops[1] = by;
		step.sub = static_cast<uint8_t>(op);
		step.result = counter.wide_next.index;
	}
	narrowed(counter.wide_next, counter.next, c_.block_of[counter.step], counter.step);
	{
		Instruction& phi = body_.instructions[counter.phi_id];
		phi.type = i64;
		phi.result = counter.wide.index;
		for (uint32_t k = 0; k < 4; k += 2)
			body_.operands[phi.extra_first + k + 1] = body_.operands[phi.extra_first + k].index == l.latches[0]
				? counter.wide_next
				: start;
	}
	InstId last_phi = counter.phi_id;
	for (InstId i = body_.instructions[last_phi].next; i != kNoId && body_.instructions[i].op == Opcode::Phi;
		i = body_.instructions[i].next)
		last_phi = i;
	narrowed(counter.wide, counter.phi, l.header, last_phi);
	++c_.stats.counters_widened;
}

// The extended value of a read's whole sum, at `at`.
Value CounterWidening::full_sum(const Read& read, BlockId block, InstId& at)
{
	const Counter& counter = counters_[read.sum.counter];
	const Loop& l = forest_.loops[current_];
	const Type i64 = Type::scalar(TypeKind::I64);
	Value sum = read.sum.stepped ? counter.wide_next : counter.wide;
	auto add = [&](Value term) {
		Instruction inst = make_instruction(Opcode::Binary, i64);
		inst.sub = static_cast<uint8_t>(BinaryOp::Add);
		inst.ops[0] = sum;
		inst.ops[1] = term;
		sum = emit_after(block, at, inst, i64);
	};
	for (unsigned k = 0; k < read.sum.count; ++k)
		add(extended(l, read.sum.terms[k], counter.is_signed));
	if (read.sum.offset != 0)
		add(integer_value(body_, integer_literal(read.sum.offset)));
	return sum;
}

void CounterWidening::rewrite(const Loop& l, uint32_t loop, Read& read)
{
	const Counter& counter = counters_[read.sum.counter];
	if (!counter.wanted)
		return;
	const BlockId block = c_.block_of[read.id];
	InstId at = read.previous;
	const Type i64 = Type::scalar(TypeKind::I64);
	const Opcode op = body_.instructions[read.id].op;
	if (op == Opcode::Convert)
	{
		const Value sum = full_sum(read, block, at);
		Instruction& inst = body_.instructions[read.id];
		Instruction copy = make_instruction(Opcode::Copy, i64);
		copy.ops[0] = sum;
		copy.result = inst.result;
		copy.next = inst.next;
		copy.location = inst.location;
		inst = copy;
		return;
	}
	Value base = body_.instructions[read.id].ops[0];
	const Type type = body_.instructions[read.id].type;
	// (A literal step is an access's displacement from the phi: nothing then
	// holds the next value before the back edge.)
	if (read.sum.stepped && counter.by.is(ValueKind::Integer))
	{
		const int64_t by = literal_of(counter.by, counter.is_signed);
		read.sum.offset += counter.down ? -by : by;
		read.sum.stepped = false;
	}
	Value offset = read.sum.stepped ? counter.wide_next : counter.wide;
	if (outside(loop, base) && read.sum.count != 0)
	{
		// (A literal stays at the access: its displacement costs nothing.)
		for (unsigned k = 0; k < read.sum.count; ++k)
			base = rebased(l, base, type, read.sum.terms[k]);
		read.sum.count = 0;
	}
	else if (read.sum.count != 0)
	{
		const int64_t literal = read.sum.offset;
		read.sum.offset = 0;
		offset = full_sum(read, block, at);
		read.sum.offset = literal;
	}
	Instruction& inst = body_.instructions[read.id];
	if (read.sum.offset == 0)
	{
		inst.ops[0] = base;
		inst.ops[1] = offset;
		return;
	}
	Instruction scaled = inst;
	scaled.ops[0] = base;
	scaled.ops[1] = offset;
	const Value advanced = emit_after(block, at, scaled, body_.pointer_type());
	Instruction& index = body_.instructions[read.id];
	index.ops[0] = advanced;
	index.ops[1] = integer_value(body_, integer_literal(read.sum.offset));
}

void CounterWidening::widen(uint32_t loop)
{
	const Loop& l = forest_.loops[loop];
	if (l.preheader == kNoId)
		return;
	read_counters(loop);
	if (counters_.empty())
		return;
	read_loop(loop);
	bool any = false;
	for (const Counter& counter : counters_)
		any = any || counter.wanted;
	if (any)
	{
		extended_.clear();
		rebased_.clear();
		preheader_end_ = previous_in_block(body_, l.preheader, body_.blocks[l.preheader].last);
		current_ = loop;
		for (Counter& counter : counters_)
			if (counter.wanted)
				widen_counter(l, counter);
		for (Read& read : reads_)
			rewrite(l, loop, read);
		changed_ = true;
	}
	forget_counters();
}

bool CounterWidening::run()
{
	c_.ensure_cfg();
	find_loops(body_, cfg_, forest_);
	if (forest_.loops.empty())
		return false;
	c_.entry_block_of(widen_counters);
	role_.assign(body_.temps.size(), -1);
	// (The preheaders made change the graph: the loops are found again.)
	for (unsigned round = 0; round < 16; ++round)
	{
		bool one = false;
		for (uint32_t loop = 0; loop < forest_.loops.size(); ++loop)
			one = (wanted(loop) && make_loop_preheader(body_, cfg_, forest_, loop)) || one;
		if (!one)
			break;
		changed_ = true;
		c_.invalidate_cfg();
		c_.ensure_cfg();
		find_loops(body_, cfg_, forest_);
		map_instruction_blocks(body_, c_.block_of);
		c_.invalidate_block_of();
		c_.entry_block_of(widen_counters);
	}
	// Inner loops first: what they extend before they start, an outer counter
	// then serves.
	for (uint32_t loop = static_cast<uint32_t>(forest_.loops.size()); loop-- > 0;)
		widen(loop);
	if (changed_)
		c_.invalidate_block_of();
	return changed_;
}

} // namespace

bool widen_counters(FunctionContext& context)
{
	CounterWidening pass(context);
	return pass.run();
}

} // namespace opt
