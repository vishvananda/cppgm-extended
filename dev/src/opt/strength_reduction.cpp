#include "lowir/builder.h"
#include "opt/loops.h"
#include "opt/passes.h"

namespace opt {

using namespace lowir;

namespace {

// At most this many new counters per loop: each takes a register for the loop.
const unsigned kCountersPerLoop = 8;

// A product of a counter and a literal the addressing modes cannot do, or a
// value the loop does not change, becomes a counter of its own: a loop adds to
// it where it multiplied.
class StrengthReduction
{
public:
	explicit StrengthReduction(FunctionContext& context) : c_(context), body_(context.body), cfg_(context.cfg) {}

	bool run();

private:
	struct Counter
	{
		TempId phi, next;
		InstId step;
		Value start;
		int64_t by;
		bool no_wrap;
	};
	struct Scaled
	{
		uint32_t counter;
		Value factor;
		Type type;
		bool extended;
		TempId phi, next;
	};

	void read_counters(uint32_t loop);
	int32_t counter_of(Value v, bool& extended) const;
	void reduce(uint32_t loop);
	Value emit_after(BlockId block, InstId& at, Instruction inst, Type type);
	void add_counter(const Loop& l, Scaled& s, bool no_wrap);
	Value multiply(Value a, Value b, Type type, BlockId block, InstId& at);
	Value product(Value start, Value factor, Type type, bool extended, BlockId block, InstId& at);
	bool outside(uint32_t loop, Value v) const;
	bool same_factor(Value a, Value b) const;
	bool wants_preheader(uint32_t loop) const;

	FunctionContext& c_;
	FunctionBody& body_;
	Cfg& cfg_;
	LoopForest forest_;
	std::vector<Counter> counters_;
	std::vector<Scaled> scaled_;
	bool changed_ = false;
};

// Whether the addressing modes or one `lea` already give the product.
bool cheap_factor(int64_t factor)
{
	const uint64_t size = factor < 0 ? static_cast<uint64_t>(0) - static_cast<uint64_t>(factor)
									 : static_cast<uint64_t>(factor);
	return size == 3 || size == 5 || size == 9 || (size & (size - 1)) == 0;
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
		if (add.op != Opcode::Binary || (op != BinaryOp::Add && op != BinaryOp::Sub) ||
			!(add.ops[0] == Value::temp(phi.result)) || !add.ops[1].is(ValueKind::Integer))
			continue;
		const int64_t by = static_cast<int64_t>(body_.literals[add.ops[1].index].low);
		counters_.push_back(Counter{ phi.result, next.index, step, start, op == BinaryOp::Add ? by : -by,
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
		if (counters_[k].phi == temp && (!extended || counters_[k].no_wrap))
			return static_cast<int32_t>(k);
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
	const Value start = product(counter.start, s.factor, s.type, s.extended, l.preheader, at);
	const BlockId stepping = c_.block_of[counter.step];
	Instruction add = make_instruction(Opcode::Binary, s.type);
	add.sub = static_cast<uint8_t>(BinaryOp::Add);
	add.flags = no_wrap ? IF_NO_WRAP : 0;
	add.ops[1] = multiply(integer_value(body_, integer_literal(counter.by)), s.factor, s.type, l.preheader, at);
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
	// A value carried across a call costs more than the multiply it saves.
	for (BlockId b : l.blocks)
		for (InstId i = body_.blocks[b].first; i != kNoId; i = body_.instructions[i].next)
			if (body_.instructions[i].op == Opcode::Call)
				return;
	for (BlockId b : l.blocks)
		for (InstId i = body_.blocks[b].first; i != kNoId; i = body_.instructions[i].next)
		{
			Instruction& inst = body_.instructions[i];
			if (inst.op != Opcode::Binary || static_cast<BinaryOp>(inst.sub) != BinaryOp::Mul ||
				(inst.type.kind() != TypeKind::I32 && inst.type.kind() != TypeKind::I64))
				continue;
			bool extended = false;
			int32_t counter = counter_of(inst.ops[0], extended);
			Value factor = inst.ops[1];
			if (counter < 0)
			{
				counter = counter_of(inst.ops[1], extended);
				factor = inst.ops[0];
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
					scaled_[k].type.kind() == inst.type.kind() && scaled_[k].extended == extended)
					break;
			if (k == scaled_.size())
			{
				if (scaled_.size() == kCountersPerLoop)
					continue;
				scaled_.push_back(Scaled{ static_cast<uint32_t>(counter), factor, inst.type, extended, 0, 0 });
				add_counter(l, scaled_.back(), (inst.flags & IF_NO_WRAP) != 0 && counters_[counter].no_wrap);
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
			if (inst.op == Opcode::Call)
				return false;
			if (inst.op == Opcode::Binary && static_cast<BinaryOp>(inst.sub) == BinaryOp::Mul &&
				(header_phi(inst.ops[0]) || header_phi(inst.ops[1])))
				return true;
		}
	return false;
}

bool StrengthReduction::run()
{
	c_.ensure_cfg();
	find_loops(body_, cfg_, forest_);
	if (forest_.loops.empty())
		return false;
	c_.entry_block_of(reduce_strength);
	// (Each preheader made changes the graph: the loops are found again.)
	for (unsigned round = 0; round < 16; ++round)
	{
		bool one = false;
		for (uint32_t loop = 0; loop < forest_.loops.size() && !one; ++loop)
			one = wants_preheader(loop) && make_loop_preheader(body_, cfg_, forest_, loop);
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
	if (changed_)
		c_.invalidate_block_of();
	return changed_;
}

} // namespace

bool reduce_strength(FunctionContext& context)
{
	StrengthReduction pass(context);
	return pass.run();
}

} // namespace opt
