#include <cstring>

#include "lowir/builder.h"
#include "opt/passes.h"
#include "opt/scalar_fold.h"

namespace opt {

using namespace lowir;

namespace {

// `sin(x)` and `cos(x)` of one value share their work: `sincos` gives both for
// about the price of one. The first call becomes `sincos`, and the calls it
// dominates read what that wrote.
class SinCos
{
public:
	explicit SinCos(FunctionContext& context) : c_(context), body_(context.body) {}

	bool run();

private:
	struct Call
	{
		InstId id;
		BlockId block;
		Value x;
		MathFunction function;
		bool done;
	};

	SymbolId sincos_symbol(TypeKind kind);
	void combine(size_t first, const std::vector<size_t>& later, Value sine, Value cosine);
	Value emit(BlockId block, InstId& at, Instruction inst, Type type, uint32_t location);

	FunctionContext& c_;
	FunctionBody& body_;
	std::vector<Call> calls_;
};

// `sincos` or `sincosf`, declared on first use (kNoId when the unit gives that
// name to something else).
SymbolId SinCos::sincos_symbol(TypeKind kind)
{
	Program& program = c_.program;
	const char* name = kind == TypeKind::F64 ? "sincos" : "sincosf";
	const NameId id = program.names.intern(name, static_cast<uint32_t>(std::strlen(name)));
	const SymbolId symbol = program.symbol_for(id);
	const Symbol& known = program.symbols[symbol];
	if (known.kind != SymbolKind::Unresolved)
		return known.kind == SymbolKind::FunctionDeclaration && known.metadata.linkage == ir_model::LLM_C ? symbol
																										  : kNoId;
	Signature sig;
	sig.first_parameter = static_cast<uint32_t>(program.parameters.size());
	sig.parameter_count = 3;
	sig.result = Type::scalar(TypeKind::Void);
	sig.boundary.unwind = ir_model::CUM_NO;
	const Type types[3] = { Type::scalar(kind), Type::scalar(TypeKind::Ptr), Type::scalar(TypeKind::Ptr) };
	for (const Type& type : types)
	{
		Parameter p = Parameter();
		p.name = 0;
		p.type = type;
		p.pass = ir_model::PPM_DIRECT;
		p.object_bytes = 0;
		program.parameters.push_back(p);
	}
	program.declare_function(symbol, SymbolKind::FunctionDeclaration, program.add_signature(sig));
	SymbolMetadata& m = program.symbols[symbol].metadata;
	m.binding = ir_model::SBM_STRONG;
	m.linkage = ir_model::LLM_C;
	m.object = id;
	return symbol;
}

Value SinCos::emit(BlockId block, InstId& at, Instruction inst, Type type, uint32_t location)
{
	inst.location = location;
	const InstId id = append_instruction(body_, inst);
	if (!type.is_void())
		body_.instructions[id].result = append_temp(body_, type, id);
	insert_after(body_, block, at, id);
	at = id;
	return type.is_void() ? Value::none() : Value::temp(body_.instructions[id].result);
}

bool SinCos::run()
{
	for (BlockId b : body_.layout)
		for (InstId i = body_.blocks[b].first; i != kNoId; i = body_.instructions[i].next)
		{
			const Instruction& inst = body_.instructions[i];
			if (inst.op != Opcode::Call || !inst.ops[0].is(ValueKind::Symbol) || inst.extra_count != 1 ||
				inst.result == kNoId)
				continue;
			const MathFunction function = c_.program.math_function(inst.ops[0].index);
			if ((function != MathFunction::Sin && function != MathFunction::Cos) ||
				(inst.type.kind() != TypeKind::F32 && inst.type.kind() != TypeKind::F64))
				continue;
			const Value x = body_.operands[inst.extra_first];
			if (x.is(ValueKind::Temp) && body_.temps[x.index].type == inst.type)
				calls_.push_back(Call{ i, b, x, function, false });
		}
	if (calls_.size() < 2)
		return false;
	const Cfg& cfg = c_.ensure_cfg();
	bool changed = false;
	std::vector<size_t> later;
	for (size_t first = 0; first < calls_.size(); ++first)
	{
		if (calls_[first].done)
			continue;
		later.clear();
		bool other = false;
		const Type type = body_.instructions[calls_[first].id].type;
		for (size_t k = first + 1; k < calls_.size(); ++k)
		{
			const Call& call = calls_[k];
			if (call.done || !(call.x == calls_[first].x) || !(body_.instructions[call.id].type == type) ||
				!cfg.dominates(calls_[first].block, call.block))
				continue;
			later.push_back(k);
			other = other || call.function != calls_[first].function;
		}
		if (!other)
			continue;
		const SymbolId symbol = sincos_symbol(type.kind());
		if (symbol == kNoId)
			return changed;
		const Type pointer = Type::scalar(TypeKind::Ptr);
		const uint32_t slot = static_cast<uint32_t>(body_.slots.size());
		body_.slots.push_back(Slot{ 0, SlotKind::Aggregate, 0, 0, type });
		body_.slots.push_back(Slot{ 0, SlotKind::Aggregate, 0, 0, type });
		const Call& head = calls_[first];
		const uint32_t location = body_.instructions[head.id].location;
		InstId at = previous_in_block(body_, head.block, head.id);
		Instruction addr = make_instruction(Opcode::Addr, pointer);
		addr.ops[0] = Value::slot(slot);
		const Value sine_at = emit(head.block, at, addr, pointer, location);
		addr.ops[0] = Value::slot(slot + 1);
		const Value cosine_at = emit(head.block, at, addr, pointer, location);
		Instruction call = make_instruction(Opcode::Call, Type::scalar(TypeKind::Void));
		call.ops[0] = Value::symbol(symbol);
		const Value args[3] = { head.x, sine_at, cosine_at };
		call.extra_first = append_operands(body_, args, 3);
		call.extra_count = 3;
		emit(head.block, at, call, Type::scalar(TypeKind::Void), location);
		Instruction load = make_instruction(Opcode::Load, type);
		load.ops[0] = sine_at;
		const Value sine = emit(head.block, at, load, type, location);
		load.ops[0] = cosine_at;
		const Value cosine = emit(head.block, at, load, type, location);
		combine(first, later, sine, cosine);
		changed = true;
	}
	if (changed)
	{
		c_.invalidate_block_of();
		c_.invalidate_cfg();
	}
	return changed;
}

// The calls give way to the two values.
void SinCos::combine(size_t first, const std::vector<size_t>& later, Value sine, Value cosine)
{
	auto drop = [&](size_t k) {
		Call& call = calls_[k];
		const TempId result = body_.instructions[call.id].result;
		replace_all_uses(body_, result, call.function == MathFunction::Sin ? sine : cosine);
		unlink_after(body_, call.block, previous_in_block(body_, call.block, call.id), call.id);
		call.done = true;
		++c_.stats.math_calls_shared;
	};
	drop(first);
	for (size_t k : later)
		drop(k);
}

} // namespace

namespace {

// `sqrt(x)` differs from the instruction only where it sets `errno`, for a
// negative `x`. The instruction runs always; the call stays on that path alone,
// out of the way of the loop around it. An `x` that cannot be negative takes
// the instruction alone: a square, a root, a sum, product or
// quotient of such values, or a phi or select of them (at most kSignDepth
// steps), or a value a dominating branch found not below a non-negative
// literal (at most kSignDominators blocks up).
const unsigned kSignDepth = 6;
const unsigned kSignDominators = 8;

class SqrtGuards
{
public:
	explicit SqrtGuards(FunctionContext& context) : c_(context), body_(context.body), cfg_(context.cfg) {}

	bool run();

private:
	bool guarded(InstId call) const;
	bool never_negative(Value x, Type type, unsigned depth) const;
	bool not_negative_literal(Value v, Type type) const;
	bool tested_not_negative(BlockId block, Value x) const;
	void guard(BlockId block, InstId before, InstId call, BlockId& rest);

	FunctionContext& c_;
	FunctionBody& body_;
	const Cfg& cfg_;
};

// Whether v is a literal neither negative nor NaN, read as `type`.
bool SqrtGuards::not_negative_literal(Value v, Type type) const
{
	Constant k, ge;
	return literal_constant(body_, v, k) &&
		fold_compare(
			ComparePredicate::Ge, type, k, integer_constant(Type::scalar(TypeKind::I64), integer_literal(0)), ge) &&
		is_integer(Type::scalar(TypeKind::I64), ge, 1);
}

// Whether x < 0 is false for every value x may hold (x may be NaN or -0).
bool SqrtGuards::never_negative(Value x, Type type, unsigned depth) const
{
	if (x.is_literal())
		return not_negative_literal(x, type);
	if (!x.is(ValueKind::Temp) || x.index < body_.parameter_count || body_.temps[x.index].def == kNoId ||
		depth == kSignDepth)
		return false;
	const Instruction& d = body_.instructions[body_.temps[x.index].def];
	if (!d.type.is_float())
		return d.op == Opcode::Convert && static_cast<ConvertOp>(d.sub) == ConvertOp::UIToFP;
	switch (d.op)
	{
	case Opcode::Unary:
		return static_cast<UnaryOp>(d.sub) == UnaryOp::Sqrt;
	case Opcode::Binary:
		switch (static_cast<BinaryOp>(d.sub))
		{
		case BinaryOp::Mul:
			return d.ops[0] == d.ops[1] ||
				(never_negative(d.ops[0], d.type, depth + 1) && never_negative(d.ops[1], d.type, depth + 1));
		case BinaryOp::Add:
		case BinaryOp::Div:
			return never_negative(d.ops[0], d.type, depth + 1) && never_negative(d.ops[1], d.type, depth + 1);
		default:
			return false;
		}
	case Opcode::Convert:
		return static_cast<ConvertOp>(d.sub) == ConvertOp::UIToFP ||
			((static_cast<ConvertOp>(d.sub) == ConvertOp::FPExt ||
				 static_cast<ConvertOp>(d.sub) == ConvertOp::FPTrunc) &&
				never_negative(d.ops[0], convert_source_type(d), depth + 1));
	case Opcode::Select:
		return never_negative(d.ops[1], d.type, depth + 1) && never_negative(d.ops[2], d.type, depth + 1);
	case Opcode::Copy:
		return never_negative(d.ops[0], d.type, depth + 1);
	case Opcode::Phi:
		for (uint32_t k = 1; k < d.extra_count; k += 2)
		{
			const Value in = body_.operands[d.extra_first + k];
			if (!(in == x) && !never_negative(in, d.type, depth + 1))
				return false;
		}
		return true;
	default:
		return false;
	}
}

// Whether a block dominating `block`, entered only from a branch, is on the
// side of a compare of x with a non-negative literal k where x >= k, x > k,
// !(x < k) or !(x <= k) holds.
bool SqrtGuards::tested_not_negative(BlockId block, Value x) const
{
	BlockId d = block;
	for (unsigned up = 0; up < kSignDominators && d != kNoId; ++up, d = cfg_.idom[d])
	{
		if (cfg_.pred_count(d) != 1)
			continue;
		const BlockId p = cfg_.preds[cfg_.pred_first[d]];
		const Instruction& br = body_.instructions[body_.blocks[p].last];
		if (br.op != Opcode::Branch || !br.ops[0].is(ValueKind::Temp) || br.ops[1] == br.ops[2] ||
			body_.temps[br.ops[0].index].def == kNoId)
			continue;
		const Instruction& cmp = body_.instructions[body_.temps[br.ops[0].index].def];
		if (cmp.op != Opcode::Compare || !cmp.type.is_float())
			continue;
		const bool taken = br.ops[1].index == d;
		ComparePredicate pred = static_cast<ComparePredicate>(cmp.sub);
		Value k;
		if (cmp.ops[0] == x)
			k = cmp.ops[1];
		else if (cmp.ops[1] == x)
		{
			k = cmp.ops[0];
			pred = swapped(pred);
		}
		else
			continue;
		const bool holds = taken ? (pred == ComparePredicate::Ge || pred == ComparePredicate::Gt)
								 : (pred == ComparePredicate::Lt || pred == ComparePredicate::Le);
		if (holds && not_negative_literal(k, cmp.type))
			return true;
	}
	return false;
}

bool SqrtGuards::guarded(InstId id) const
{
	const Instruction& call = body_.instructions[id];
	if (call.op != Opcode::Call || !call.ops[0].is(ValueKind::Symbol) || call.result == kNoId ||
		call.extra_count != 1 || c_.program.math_function(call.ops[0].index) != MathFunction::Sqrt)
		return false;
	const TypeKind kind = call.type.kind();
	const Value x = body_.operands[call.extra_first];
	return (kind == TypeKind::F32 || kind == TypeKind::F64) && x.is(ValueKind::Temp) &&
		body_.temps[x.index].type == call.type;
}

// `block` ends at a branch on the sign of the operand, and `rest` holds what
// followed the call, starting with a phi of the two results under the call's
// own name.
void SqrtGuards::guard(BlockId block, InstId before, InstId id, BlockId& rest)
{
	const Instruction call = body_.instructions[id];
	const Value x = body_.operands[call.extra_first];
	const Type type = call.type;
	rest = append_block(body_, BlockKind::Split);
	const BlockId slow = append_block(body_, BlockKind::Split);
	// What follows the call moves, and the successors' phis with it.
	const InstId last = body_.blocks[block].last;
	const InstId moved = call.next;
	body_.blocks[rest].first = moved;
	body_.blocks[rest].last = last;
	for_each_successor(
		body_, body_.instructions[last], [&](BlockId& target) { rename_phi_incoming(body_, target, block, rest); });
	InstId at = before;
	auto emit = [&](BlockId b, InstId& after, Instruction inst, Type result) {
		inst.location = call.location;
		const InstId made = append_instruction(body_, inst);
		const TempId temp = result.kind() == TypeKind::Void ? kNoId : append_temp(body_, result, made);
		body_.instructions[made].result = temp;
		insert_after(body_, b, after, made);
		after = made;
		return Value::temp(temp);
	};
	// The call leaves `block` with what followed it.
	body_.blocks[block].last = before;
	if (before == kNoId)
		body_.blocks[block].first = kNoId;
	else
		body_.instructions[before].next = kNoId;
	Instruction root = make_instruction(Opcode::Unary, type);
	root.sub = static_cast<uint8_t>(UnaryOp::Sqrt);
	root.ops[0] = x;
	const Value fast = emit(block, at, root, type);
	Instruction below = make_instruction(Opcode::Compare, type);
	below.sub = static_cast<uint8_t>(ComparePredicate::Lt);
	below.ops[0] = x;
	below.ops[1] = phi_input(body_, type, integer_value(body_, integer_literal(0)));
	const Value negative = emit(block, at, below, Type::scalar(TypeKind::I64));
	Instruction branch = make_instruction(Opcode::Branch, Type::scalar(TypeKind::Void));
	branch.ops[0] = negative;
	branch.ops[1] = Value::block(slow);
	branch.ops[2] = Value::block(rest);
	emit(block, at, branch, Type::scalar(TypeKind::Void));
	// The call, under a new name, and its way back.
	const TempId result = call.result;
	const TempId slow_result = append_temp(body_, type, id);
	body_.instructions[id].next = kNoId;
	body_.instructions[id].result = slow_result;
	InstId tail = kNoId;
	insert_after(body_, slow, tail, id);
	tail = id;
	Instruction back = make_instruction(Opcode::Jump, Type::scalar(TypeKind::Void));
	back.ops[0] = Value::block(rest);
	emit(slow, tail, back, Type::scalar(TypeKind::Void));
	const Value pairs[4] = { Value::block(block), fast, Value::block(slow), Value::temp(slow_result) };
	Instruction phi = make_instruction(Opcode::Phi, type);
	phi.extra_first = append_operands(body_, pairs, 4);
	phi.extra_count = 4;
	phi.result = result;
	phi.location = call.location;
	const InstId merge = append_instruction(body_, phi);
	body_.temps[result].def = merge;
	insert_after(body_, rest, kNoId, merge);
	// In the layout, what follows the call stays where it was and the call
	// goes last.
	std::vector<BlockId> layout;
	layout.reserve(body_.layout.size() + 2);
	for (BlockId b : body_.layout)
	{
		layout.push_back(b);
		if (b == block)
			layout.push_back(rest);
	}
	layout.push_back(slow);
	body_.layout.swap(layout);
}

bool SqrtGuards::run()
{
	bool changed = false;
	c_.ensure_cfg();
	// (The blocks of the calls gather past `end`.)
	size_t end = body_.layout.size();
	for (size_t at = 0; at < end; ++at)
	{
		const BlockId block = body_.layout[at];
		InstId before = kNoId;
		for (InstId i = body_.blocks[block].first; i != kNoId; before = i, i = body_.instructions[i].next)
			if (guarded(i))
			{
				const Value x = body_.operands[body_.instructions[i].extra_first];
				if (never_negative(x, body_.instructions[i].type, 0) || tested_not_negative(block, x))
				{
					// The instruction alone.
					Instruction& call = body_.instructions[i];
					call.op = Opcode::Unary;
					call.sub = static_cast<uint8_t>(UnaryOp::Sqrt);
					call.ops[0] = x;
					call.ops[1] = call.ops[2] = Value::none();
					call.extra_first = call.extra_count = 0;
					call.flags = 0;
					changed = true;
					continue;
				}
				BlockId rest;
				guard(block, before, i, rest);
				++end;
				changed = true;
				c_.invalidate_cfg();
				c_.ensure_cfg();
				break; // (`rest` comes next in the layout)
			}
	}
	if (changed)
	{
		c_.invalidate_cfg();
		c_.invalidate_block_of();
	}
	return changed;
}

} // namespace

bool guard_sqrt_calls(FunctionContext& context)
{
	SqrtGuards pass(context);
	return pass.run();
}

bool combine_sin_cos(FunctionContext& context)
{
	SinCos pass(context);
	return pass.run();
}

} // namespace opt
