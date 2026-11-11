#include "opt/integer_patterns.h"
#include "lowir/builder.h"
#include "opt/scalar_fold.h"
namespace opt {
using namespace lowir;
namespace {
uint64_t returned_powers(const FunctionBody& body, Value value, Type type, unsigned& budget)
{
	if (budget == 0 || !type.is_integer() || type.kind() == TypeKind::I128)
		return 0;
	--budget;
	Constant c;
	Literal bits;
	if (literal_constant(body, value, c) && integer_bits(type, c, bits))
		return bits.high == 0 && bits.low != 0 && (bits.low & (bits.low - 1)) == 0 ? bits.low : 0;
	if (!value.is(ValueKind::Temp) || value.index < body.parameter_count || body.temps[value.index].def == kNoId)
		return 0;
	const Instruction& def = body.instructions[body.temps[value.index].def];
	if (def.type != type)
		return 0;
	if (def.op == Opcode::Phi || def.op == Opcode::Select)
	{
		uint64_t out = 0;
		const unsigned count = def.op == Opcode::Phi ? def.extra_count / 2 : 2;
		for (unsigned i = 0; i < count; ++i)
		{
			const Value operand = def.op == Opcode::Phi ? body.operands[def.extra_first + i * 2 + 1] : def.ops[i + 1];
			const uint64_t choice = returned_powers(body, operand, type, budget);
			if (choice == 0)
				return 0;
			out |= choice;
		}
		return out;
	}
	if (def.op == Opcode::Binary && static_cast<BinaryOp>(def.sub) == BinaryOp::Shl &&
		literal_constant(body, def.ops[0], c) && is_integer(type, c, 1))
	{
		const unsigned width = lowir::integer_bits(type.kind());
		return width == 64 ? ~uint64_t(0) : (uint64_t(1) << width) - 1;
	}
	if (def.op == Opcode::Copy)
	{
		const Type from = def.ops[0].is(ValueKind::Temp) ? body.temps[def.ops[0].index].type : type;
		return storage_bytes(from) == storage_bytes(type) ? returned_powers(body, def.ops[0], from, budget) : 0;
	}
	if (def.op == Opcode::Convert)
	{
		const Type from = convert_source_type(def);
		const ConvertOp op = static_cast<ConvertOp>(def.sub);
		uint64_t powers = returned_powers(body, def.ops[0], from, budget);
		const unsigned width = lowir::integer_bits(type.kind()), input = lowir::integer_bits(from.kind());
		if (op == ConvertOp::ZExt)
			return powers;
		if (op == ConvertOp::Trunc)
			return width == 64 || (powers >> width) == 0 ? powers : 0;
		if (op == ConvertOp::SExt)
			return (powers >> (input - 1)) == 0 ? powers : 0;
	}
	return 0;
}

class IntegerPatterns
{
public:
	IntegerPatterns(FunctionBody& body, const std::vector<uint32_t>& uses, IntegerPatternFacts& facts)
		: body_(body), uses_(uses), facts_(facts)
	{}
	bool variable_power_division(BlockId block, InstId& prev, InstId id);
	bool round_power_alignment(BlockId block, InstId& prev, InstId id);
	bool narrow_truncation(Instruction& inst);
	bool successor_compare(Instruction& inst);
	bool advanced_compare(Instruction& inst);
	bool top_range_compare(Instruction& inst);

private:
	bool power_of_two(Value value, Type type, unsigned& budget, unsigned width = 64);
	Value resolve(Value v) { return facts_.resolve(v); }
	uint64_t possible_bits(Value v, Type t) { return facts_.possible_bits(v, t); }
	bool constant(Value v, Type, Constant& out) { return literal_constant(body_, v, out); }
	const Instruction* producer(Value v) const
	{
		return v.is(ValueKind::Temp) && v.index >= body_.parameter_count && body_.temps[v.index].def != kNoId
			? &body_.instructions[body_.temps[v.index].def]
			: nullptr;
	}
	FunctionBody& body_;
	const std::vector<uint32_t>& uses_;
	IntegerPatternFacts& facts_;
};
// A bounded proof over typed SSA alternatives. Cycles, unknown values and
// zero all fail; no runtime assertion is introduced.
bool IntegerPatterns::power_of_two(Value value, Type type, unsigned& budget, unsigned width)
{
	if (budget == 0)
		return false;
	--budget;
	value = resolve(value);
	width = std::min(width, lowir::integer_bits(type.kind()));
	Constant c;
	Literal bits;
	if (constant(value, type, c) && integer_bits(type, c, bits))
		return width != 0 && bits.high == 0 && bits.low != 0 && (width == 64 || (bits.low >> width) == 0) &&
			(bits.low & (bits.low - 1)) == 0;
	const Instruction* def = producer(value);
	if (def == nullptr || def->type != type)
		return false;
	if (def->op == Opcode::Call && def->ops[0].is(ValueKind::Symbol))
	{
		const uint64_t powers = facts_.returned_power_bits(def->ops[0].index, type);
		return powers != 0 && (width == 64 || (powers >> width) == 0);
	}
	if (def->op == Opcode::Copy)
	{
		const Type from = def->ops[0].is(ValueKind::Temp) ? body_.temps[def->ops[0].index].type : type;
		return from.is_integer() && storage_bytes(from) == storage_bytes(type) &&
			power_of_two(def->ops[0], from, budget, width);
	}
	if (def->op == Opcode::Convert)
	{
		const ConvertOp op = static_cast<ConvertOp>(def->sub);
		const Type from = convert_source_type(*def);
		if (!from.is_integer() || from.kind() == TypeKind::I128 ||
			(op != ConvertOp::ZExt && op != ConvertOp::SExt && op != ConvertOp::Trunc))
			return false;
		if (op == ConvertOp::SExt)
			width = std::min(width, lowir::integer_bits(from.kind()) - 1);
		return power_of_two(def->ops[0], from, budget, width);
	}
	if (def->op == Opcode::Select)
		return power_of_two(def->ops[1], type, budget, width) && power_of_two(def->ops[2], type, budget, width);
	if (def->op == Opcode::Phi && def->extra_count != 0)
	{
		for (uint32_t k = 1; k < def->extra_count; k += 2)
			if (!power_of_two(body_.operands[def->extra_first + k], type, budget, width))
				return false;
		return true;
	}
	if (def->op == Opcode::Binary && static_cast<BinaryOp>(def->sub) == BinaryOp::Shl &&
		constant(def->ops[0], type, c) && is_integer(type, c, 1) &&
		(width == lowir::integer_bits(type.kind()) || possible_bits(def->ops[1], type) < width))
		return true;
	return false;
}

bool IntegerPatterns::variable_power_division(BlockId block, InstId& prev, InstId id)
{
	const Instruction inst = body_.instructions[id];
	if (inst.op != Opcode::Binary || (inst.type.kind() != TypeKind::U32 && inst.type.kind() != TypeKind::I64) ||
		(static_cast<BinaryOp>(inst.sub) != BinaryOp::UDiv && static_cast<BinaryOp>(inst.sub) != BinaryOp::UMod) ||
		resolve(inst.ops[1]).is(ValueKind::Integer))
		return false;
	unsigned budget = 128;
	if (!power_of_two(inst.ops[1], inst.type, budget))
		return false;
	const bool division = static_cast<BinaryOp>(inst.sub) == BinaryOp::UDiv;
	Instruction helper = make_instruction(division ? Opcode::Unary : Opcode::Binary, inst.type);
	helper.sub = static_cast<uint8_t>(
		division ? static_cast<uint8_t>(UnaryOp::CountTrailingZeros) : static_cast<uint8_t>(BinaryOp::Sub));
	helper.ops[0] = inst.ops[1];
	if (!division)
		helper.ops[1] = integer_value(body_, Literal{ 1, 0 });
	helper.location = inst.location;
	const InstId at = append_instruction(body_, helper);
	const TempId temp = append_temp(body_, inst.type, at);
	body_.instructions[at].result = temp;
	insert_after(body_, block, prev, at);
	prev = at;
	Instruction& changed = body_.instructions[id];
	changed.sub = static_cast<uint8_t>(division ? BinaryOp::UShr : BinaryOp::And);
	changed.ops[1] = Value::temp(temp);
	changed.flags = static_cast<decltype(changed.flags)>(changed.flags & ~(IF_EXACT | IF_NO_WRAP));
	return true;
}

// Unsigned ceil division followed by its power-of-two multiple is
// rounding in the type's modular arithmetic, including zero and wraparound.
bool IntegerPatterns::round_power_alignment(BlockId block, InstId& prev, InstId id)
{
	const Instruction inst = body_.instructions[id];
	if (inst.op != Opcode::Binary ||
		(static_cast<BinaryOp>(inst.sub) != BinaryOp::Mul && static_cast<BinaryOp>(inst.sub) != BinaryOp::Shl) ||
		(inst.type.kind() != TypeKind::U32 && inst.type.kind() != TypeKind::I64))
		return false;
	const bool scaled = static_cast<BinaryOp>(inst.sub) == BinaryOp::Shl;
	unsigned scale = 0;
	if (scaled)
	{
		Constant c;
		Literal count;
		if (!constant(inst.ops[1], inst.type, c) || !integer_bits(inst.type, c, count) || count.high != 0 ||
			count.low >= lowir::integer_bits(inst.type.kind()))
			return false;
		scale = static_cast<unsigned>(count.low);
	}
	for (unsigned side = 0; side < (scaled ? 1u : 2u); ++side)
	{
		Value divisor = scaled ? Value::none() : resolve(inst.ops[1 - side]);
		const Instruction* sum = producer(inst.ops[side]);
		if (sum == nullptr || sum->op != Opcode::Binary || sum->type != inst.type ||
			static_cast<BinaryOp>(sum->sub) != BinaryOp::Add || uses_[sum->result] != 1)
			continue;
		for (unsigned arm = 0; arm < 2; ++arm)
		{
			const Value flag = resolve(sum->ops[arm]);
			const Instruction* quotient = producer(sum->ops[1 - arm]);
			if (quotient == nullptr || quotient->op != Opcode::Binary || quotient->type != inst.type ||
				uses_[quotient->result] != 1)
				continue;
			const BinaryOp qop = static_cast<BinaryOp>(quotient->sub);
			bool divided = qop == BinaryOp::UDiv && resolve(quotient->ops[1]) == divisor;
			if (!divided && !scaled && qop == BinaryOp::UDiv)
			{
				Constant a, b;
				divided = constant(quotient->ops[1], inst.type, a) && constant(divisor, inst.type, b) &&
					same_constant(inst.type, a, b);
			}
			if (scaled && (qop == BinaryOp::UDiv || qop == BinaryOp::UShr))
			{
				Constant c;
				Literal bits;
				divided = constant(quotient->ops[1], inst.type, c) && integer_bits(inst.type, c, bits) &&
					bits.high == 0 && bits.low == (qop == BinaryOp::UDiv ? (static_cast<uint64_t>(1) << scale) : scale);
			}
			if (!scaled && qop == BinaryOp::UShr)
			{
				const Instruction* scan = producer(quotient->ops[1]);
				divided = scan != nullptr && scan->op == Opcode::Unary &&
					static_cast<UnaryOp>(scan->sub) == UnaryOp::CountTrailingZeros && resolve(scan->ops[0]) == divisor;
				Constant d, c;
				Literal bits, count;
				if (!divided && constant(divisor, inst.type, d) && integer_bits(inst.type, d, bits) && bits.high == 0 &&
					bits.low != 0 && (bits.low & (bits.low - 1)) == 0 && constant(quotient->ops[1], inst.type, c) &&
					integer_bits(inst.type, c, count))
				{
					unsigned shift = 0;
					while ((static_cast<uint64_t>(1) << shift) != bits.low)
						++shift;
					divided = count.high == 0 && count.low == shift;
				}
			}
			if (!divided)
				continue;
			const Instruction* sub = producer(quotient->ops[0]);
			if (sub == nullptr || sub->op != Opcode::Binary || sub->type != inst.type ||
				static_cast<BinaryOp>(sub->sub) != BinaryOp::Sub || !(resolve(sub->ops[1]) == flag) ||
				uses_[sub->result] != 1)
				continue;
			const Value x = resolve(sub->ops[0]);
			const Instruction* truth = producer(flag);
			for (unsigned depth = 0;
				truth != nullptr && depth < 4 && (truth->op == Opcode::Copy || truth->op == Opcode::Convert); ++depth)
			{
				if (!truth->type.is_integer())
					break;
				if (truth->op == Opcode::Convert)
				{
					const ConvertOp op = static_cast<ConvertOp>(truth->sub);
					if (op != ConvertOp::Trunc && op != ConvertOp::ZExt)
						break;
				}
				truth = producer(truth->ops[0]);
			}
			Constant zero;
			if (truth == nullptr || truth->op != Opcode::Compare || truth->type != inst.type ||
				static_cast<ComparePredicate>(truth->sub) != ComparePredicate::Ne || !(resolve(truth->ops[0]) == x) ||
				!constant(truth->ops[1], inst.type, zero) || !is_integer(inst.type, zero, 0))
				continue;
			unsigned budget = 128;
			if (!scaled && !power_of_two(divisor, inst.type, budget))
				continue;
			if (scaled)
				divisor = integer_value(body_, Literal{ static_cast<uint64_t>(1) << scale, 0 });
			auto emit = [&](Opcode op, uint8_t sub, Value a, Value b) {
				Instruction helper = make_instruction(op, inst.type);
				helper.sub = sub;
				helper.ops[0] = a;
				helper.ops[1] = b;
				helper.location = inst.location;
				const InstId at = append_instruction(body_, helper);
				const TempId temp = append_temp(body_, inst.type, at);
				body_.instructions[at].result = temp;
				insert_after(body_, block, prev, at);
				prev = at;
				return Value::temp(temp);
			};
			const Value pad = emit(
				Opcode::Binary, static_cast<uint8_t>(BinaryOp::Sub), divisor, integer_value(body_, Literal{ 1, 0 }));
			const Value rounded = emit(Opcode::Binary, static_cast<uint8_t>(BinaryOp::Add), x, pad);
			const Value mask = emit(Opcode::Unary, static_cast<uint8_t>(UnaryOp::Neg), divisor, Value::none());
			Instruction& changed = body_.instructions[id];
			changed.sub = static_cast<uint8_t>(BinaryOp::And);
			changed.ops[0] = rounded;
			changed.ops[1] = mask;
			changed.flags = 0;
			return true;
		}
	}
	return false;
}

// A truncation of arithmetic on extensions of its own type is that arithmetic
// in the narrow type: the low bits of each operation depend only on the low
// bits of its operands.
bool IntegerPatterns::narrow_truncation(Instruction& inst)
{
	if (inst.op != Opcode::Convert || static_cast<ConvertOp>(inst.sub) != ConvertOp::Trunc || !inst.type.is_integer() ||
		inst.type.kind() == TypeKind::I1)
		return false;
	const Instruction* def = producer(inst.ops[0]);
	if (def == nullptr || def->op != Opcode::Binary || !def->type.is_integer())
		return false;
	const BinaryOp op = static_cast<BinaryOp>(def->sub);
	const bool shift = op == BinaryOp::Shl;
	// (No lanes multiply bytes: the wider form may still run in lanes.)
	const bool product = op == BinaryOp::Mul && storage_bytes(inst.type) > 1;
	if (op != BinaryOp::Add && op != BinaryOp::Sub && op != BinaryOp::And && op != BinaryOp::Or &&
		op != BinaryOp::Xor && !shift && !product)
		return false;
	Value narrow[2] = { Value::none(), Value::none() };
	for (unsigned k = 0; k < 2; ++k)
	{
		const Value v = def->ops[k];
		Constant c;
		Literal bits;
		if (constant(v, def->type, c))
		{
			if (!integer_bits(def->type, c, bits) || (shift && k == 1 && bits.low >= 8 * storage_bytes(inst.type)))
				return false;
			narrow[k] = integer_value(body_, normalize_integer(inst.type, bits));
			continue;
		}
		const Instruction* wide = producer(v);
		if ((shift && k == 1) || wide == nullptr || wide->op != Opcode::Convert ||
			(static_cast<ConvertOp>(wide->sub) != ConvertOp::SExt &&
				static_cast<ConvertOp>(wide->sub) != ConvertOp::ZExt) ||
			!wide->ops[0].is(ValueKind::Temp) || body_.temps[wide->ops[0].index].type != inst.type)
			return false;
		narrow[k] = wide->ops[0];
	}
	if (narrow[0].is(ValueKind::Integer) && narrow[1].is(ValueKind::Integer))
		return false;
	Instruction rewritten = make_instruction(Opcode::Binary, inst.type);
	rewritten.sub = static_cast<uint8_t>(op);
	rewritten.ops[0] = narrow[0];
	rewritten.ops[1] = narrow[1];
	rewritten.result = inst.result;
	rewritten.next = inst.next;
	rewritten.location = inst.location;
	inst = rewritten;
	return true;
}

// Bounded address algebra over typed SSA; no source reconstruction.
class PointerDistance
{
public:
	PointerDistance(FunctionBody& body, IntegerPatternFacts& facts, uint64_t stride)
		: body_(body), facts_(facts), stride_(stride)
	{}
	bool collect(Value& address, bool negative);
	Value emit(BlockId block, InstId& prev, uint32_t location);

private:
	struct Term
	{
		Value value;
		bool negative;
	};
	const Instruction* producer(Value value) const
	{
		return value.is(ValueKind::Temp) && value.index >= body_.parameter_count &&
				body_.temps[value.index].def != kNoId
			? &body_.instructions[body_.temps[value.index].def]
			: nullptr;
	}
	bool record(Value value, uint64_t bytes, bool negative);
	FunctionBody& body_;
	IntegerPatternFacts& facts_;
	uint64_t stride_;
	uint64_t magnitude_ = 0;
	Term terms_[6];
	unsigned count_ = 0;
};

bool PointerDistance::record(Value value, uint64_t bytes, bool negative)
{
	value = facts_.resolve(value);
	const Instruction* def = producer(value);
	if (def != nullptr && def->op == Opcode::Binary && def->type.kind() == TypeKind::I64 &&
		static_cast<BinaryOp>(def->sub) == BinaryOp::Sub && def->ops[0].is(ValueKind::Integer) &&
		body_.literals[def->ops[0].index].low == 0)
	{
		value = facts_.resolve(def->ops[1]);
		negative = !negative;
		def = producer(value);
	}
	else if (def != nullptr && def->op == Opcode::Unary && def->type.kind() == TypeKind::I64 &&
		static_cast<UnaryOp>(def->sub) == UnaryOp::Neg)
	{
		value = facts_.resolve(def->ops[0]);
		negative = !negative;
		def = producer(value);
	}
	if (bytes != stride_ && def != nullptr && def->op == Opcode::Binary && def->type.kind() == TypeKind::I64 &&
		def->ops[1].is(ValueKind::Integer))
	{
		uint64_t scale = body_.literals[def->ops[1].index].low;
		const BinaryOp operation = static_cast<BinaryOp>(def->sub);
		if (operation == BinaryOp::Shl && scale < 63)
			scale = uint64_t(1) << scale;
		else if (operation != BinaryOp::Mul)
			return false;
		if (bytes == 0 || stride_ % bytes != 0 || scale != stride_ / bytes)
			return false;
		value = facts_.resolve(def->ops[0]);
		def = producer(value);
		bytes = stride_;
	}
	if (count_ == 6)
		return false;
	uint64_t bound = 0;
	if (value.is(ValueKind::Integer))
	{
		const int64_t literal = static_cast<int64_t>(body_.literals[value.index].low);
		if (bytes != stride_)
		{
			// Restrict the constant conversion too: neither product nor
			// division may silently wrap while proving an exact quotient.
			const uint64_t absolute = literal < 0 ? uint64_t(0) - uint64_t(literal) : uint64_t(literal);
			if (bytes == 0 || absolute > uint64_t(INT64_MAX) / bytes || (absolute * bytes) % stride_ != 0)
				return false;
			const uint64_t units = absolute * bytes / stride_;
			value = integer_value(body_,
				Literal{ literal < 0 ? uint64_t(0) - units : units, literal < 0 && units != 0 ? ~uint64_t(0) : 0 });
		}
		const uint64_t bits = body_.literals[value.index].low;
		bound = int64_t(bits) < 0 ? uint64_t(0) - bits : bits;
	}
	else
	{
		if (bytes != stride_ || !value.is(ValueKind::Temp) || body_.temps[value.index].type.kind() != TypeKind::I64)
			return false;
		bound = facts_.possible_bits(value, Type::scalar(TypeKind::I64));
		if (bound > uint64_t(INT64_MAX) && def != nullptr && def->op == Opcode::Convert &&
			static_cast<ConvertOp>(def->sub) == ConvertOp::SExt)
		{
			const unsigned width = integer_bits(convert_source_type(*def).kind());
			if (width > 0 && width < 64)
				bound = uint64_t(1) << (width - 1);
		}
	}
	if (stride_ != 1 && (bound > uint64_t(INT64_MAX) / stride_ || magnitude_ > uint64_t(INT64_MAX) / stride_ - bound))
		return false;
	magnitude_ += bound;
	terms_[count_++] = Term{ value, negative };
	return true;
}

bool PointerDistance::collect(Value& address, bool negative)
{
	address = facts_.resolve(address);
	for (unsigned depth = 0; depth < 4; ++depth)
	{
		const Instruction* index = producer(address);
		if (index == nullptr || index->op != Opcode::Index)
			return true;
		// record() may extend only the literal pool, not instructions.
		if (!record(index->ops[1], storage_bytes(index->type), negative))
			return false;
		address = facts_.resolve(index->ops[0]);
	}
	return false;
}

Value PointerDistance::emit(BlockId block, InstId& prev, uint32_t location)
{
	Value value = integer_value(body_, Literal{ 0, 0 });
	for (unsigned i = 0; i < count_; ++i)
	{
		Instruction next = make_instruction(Opcode::Binary, Type::scalar(TypeKind::I64));
		next.sub = static_cast<uint8_t>(terms_[i].negative ? BinaryOp::Sub : BinaryOp::Add);
		next.ops[0] = value;
		next.ops[1] = terms_[i].value;
		next.location = location;
		const InstId id = append_instruction(body_, next);
		body_.instructions[id].result = append_temp(body_, next.type, id);
		insert_after(body_, block, prev, id);
		prev = id;
		value = Value::temp(body_.instructions[id].result);
	}
	return value;
}

// `x + 1 > y` and `x + 1 <= y`, unsigned, of an x that is not its type's
// greatest value (the sum does not wrap), are `x >= y` and `x < y`: a
// vector's `size + 1 > capacity` compares the two words it loaded.
bool IntegerPatterns::successor_compare(Instruction& inst)
{
	if (inst.op != Opcode::Compare || !inst.type.is_integer() || inst.type.kind() == TypeKind::I128)
		return false;
	// The sum on the left: ugt, ule; on the right: ult, uge.
	unsigned side = 0;
	ComparePredicate to = ComparePredicate::Uge;
	switch (static_cast<ComparePredicate>(inst.sub))
	{
	case ComparePredicate::Ugt:
		break;
	case ComparePredicate::Ule:
		to = ComparePredicate::Ult;
		break;
	case ComparePredicate::Ult:
		side = 1;
		to = ComparePredicate::Ule;
		break;
	case ComparePredicate::Uge:
		side = 1;
		to = ComparePredicate::Ugt;
		break;
	default:
		return false;
	}
	const Instruction* sum = producer(inst.ops[side]);
	Constant one;
	if (sum == nullptr || sum->op != Opcode::Binary || static_cast<BinaryOp>(sum->sub) != BinaryOp::Add ||
		sum->type != inst.type || !sum->ops[0].is(ValueKind::Temp) || !constant(sum->ops[1], inst.type, one) ||
		!is_integer(inst.type, one, 1))
		return false;
	const unsigned width = lowir::integer_bits(inst.type.kind());
	const uint64_t all = width >= 64 ? ~static_cast<uint64_t>(0) : (static_cast<uint64_t>(1) << width) - 1;
	if (possible_bits(sum->ops[0], inst.type) == all)
		return false;
	inst.ops[side] = sum->ops[0];
	inst.sub = static_cast<uint8_t>(to);
	return true;
}

// `p + n == p`, of an inbounds `index` (its address calculation does not
// wrap) or of a product that cannot reach the address width, is `n == 0`:
// an empty range's `begin + size != begin` tests the size alone. A
// literal factor of n that cannot wrap its type leaves n's other factor.
bool IntegerPatterns::advanced_compare(Instruction& inst)
{
	const ComparePredicate pred = static_cast<ComparePredicate>(inst.sub);
	if (inst.op != Opcode::Compare || !inst.type.is_pointer() ||
		(pred != ComparePredicate::Eq && pred != ComparePredicate::Ne))
		return false;
	for (unsigned side = 0; side < 2; ++side)
	{
		const Instruction* step = producer(inst.ops[side]);
		if (step == nullptr || step->op != Opcode::Index || !step->ops[1].is(ValueKind::Temp) ||
			!(resolve(step->ops[0]) == resolve(inst.ops[1 - side])) || storage_bytes(step->type) == 0)
			continue;
		Value n = resolve(step->ops[1]);
		Type type = body_.temps[n.index].type;
		if (!type.is_integer() || type.kind() == TypeKind::I128 || type.kind() == TypeKind::I1)
			return false;
		const unsigned width = lowir::integer_bits(type.kind());
		const uint64_t all = width >= 64 ? ~static_cast<uint64_t>(0) : (static_cast<uint64_t>(1) << width) - 1;
		if (step->index_bounds() != IndexBounds::InBounds &&
			(width != 64 || possible_bits(n, type) > all / storage_bytes(step->type)))
			return false;
		for (unsigned depth = 0; depth < 2; ++depth)
		{
			const Instruction* scaled = producer(n);
			Constant c;
			Literal k;
			if (scaled == nullptr || scaled->op != Opcode::Binary || scaled->type != type ||
				!scaled->ops[0].is(ValueKind::Temp) || !constant(scaled->ops[1], type, c) ||
				!integer_bits(type, c, k) || k.high != 0 || k.low == 0)
				break;
			const BinaryOp op = static_cast<BinaryOp>(scaled->sub);
			const uint64_t from = possible_bits(scaled->ops[0], type);
			if (!(op == BinaryOp::Mul && from <= all / k.low) &&
				!(op == BinaryOp::Shl && k.low < width && from <= (all >> k.low)))
				break;
			n = resolve(scaled->ops[0]);
		}
		inst.ops[0] = n;
		inst.ops[1] = integer_value(body_, Literal{ 0, 0 });
		inst.type = type;
		return true;
	}
	return false;
}

// `x - a < n`, unsigned, of a range [a, a + n) that ends at the type's
// greatest value is `x >= a` (and its negation `x < a`): the two markers
// -2 and -1 of a pointer set take one compare of the value itself.
bool IntegerPatterns::top_range_compare(Instruction& inst)
{
	if (inst.op != Opcode::Compare || !inst.type.is_integer() || inst.type.kind() == TypeKind::I128 ||
		inst.type.kind() == TypeKind::I1)
		return false;
	const unsigned width = lowir::integer_bits(inst.type.kind());
	const uint64_t all = width >= 64 ? ~static_cast<uint64_t>(0) : (static_cast<uint64_t>(1) << width) - 1;
	Constant c;
	Literal bits;
	if (!constant(inst.ops[1], inst.type, c) || !integer_bits(inst.type, c, bits))
		return false;
	uint64_t count = bits.low & all;
	bool inside = true;
	switch (static_cast<ComparePredicate>(inst.sub))
	{
	case ComparePredicate::Ult:
		break;
	case ComparePredicate::Uge:
		inside = false;
		break;
	case ComparePredicate::Ugt:
		inside = false;
		[[fallthrough]];
	case ComparePredicate::Ule:
		if (count == all)
			return false;
		++count;
		break;
	default:
		return false;
	}
	const Instruction* moved = producer(inst.ops[0]);
	if (count == 0 || moved == nullptr || moved->op != Opcode::Binary || moved->type != inst.type ||
		!moved->ops[0].is(ValueKind::Temp) || !constant(moved->ops[1], inst.type, c) ||
		!integer_bits(inst.type, c, bits))
		return false;
	const BinaryOp op = static_cast<BinaryOp>(moved->sub);
	if (op != BinaryOp::Sub && op != BinaryOp::Add)
		return false;
	const uint64_t low = (op == BinaryOp::Sub ? bits.low : 0 - bits.low) & all;
	if (((low + count) & all) != 0)
		return false;
	inst.ops[0] = moved->ops[0];
	inst.ops[1] = integer_value(body_, normalize_integer(inst.type, Literal{ low, 0 }));
	inst.sub = static_cast<uint8_t>(inside ? ComparePredicate::Uge : ComparePredicate::Ult);
	return true;
}

} // namespace

bool simplify_integer_compare(
	FunctionBody& body, const std::vector<uint32_t>& uses, IntegerPatternFacts& facts, Instruction& instruction)
{
	IntegerPatterns patterns(body, uses, facts);
	return patterns.successor_compare(instruction) || patterns.advanced_compare(instruction) ||
		patterns.top_range_compare(instruction);
}

bool simplify_integer_conversion(
	FunctionBody& body, const std::vector<uint32_t>& uses, IntegerPatternFacts& facts, Instruction& instruction)
{
	IntegerPatterns patterns(body, uses, facts);
	return patterns.narrow_truncation(instruction);
}

bool simplify_pointer_distance(FunctionBody& body, IntegerPatternFacts& facts, BlockId block, InstId& prev, InstId id)
{
	const Instruction seen = body.instructions[id];
	if (seen.op != Opcode::Binary || seen.type.kind() != TypeKind::I64 ||
		static_cast<BinaryOp>(seen.sub) != BinaryOp::Shr || (seen.flags & IF_EXACT) == 0)
		return false;
	const Instruction* difference = &seen;
	uint64_t stride = 1;
	if (seen.type.kind() == TypeKind::I64 && static_cast<BinaryOp>(seen.sub) == BinaryOp::Shr &&
		(seen.flags & IF_EXACT) != 0 && seen.ops[1].is(ValueKind::Integer) &&
		body.literals[seen.ops[1].index].low < 63 && seen.ops[0].is(ValueKind::Temp) &&
		body.temps[seen.ops[0].index].def != kNoId)
	{
		stride = uint64_t(1) << body.literals[seen.ops[1].index].low;
		difference = &body.instructions[body.temps[seen.ops[0].index].def];
	}
	if (!difference->type.is_pointer() || static_cast<BinaryOp>(difference->sub) != BinaryOp::Sub ||
		difference->op != Opcode::Binary)
		return false;
	Value a = difference->ops[0], b = difference->ops[1];
	PointerDistance proof(body, facts, stride);
	if (!proof.collect(a, false) || !proof.collect(b, true) || !(a == b) ||
		!(a.is(ValueKind::Temp) || a.is(ValueKind::Symbol) || a.is(ValueKind::Slot)))
		return false;
	const Value value = proof.emit(block, prev, seen.location);
	Instruction& changed = body.instructions[id];
	changed.op = Opcode::Copy;
	changed.type = Type::scalar(TypeKind::I64);
	changed.sub = 0;
	changed.ops[0] = value;
	changed.ops[1] = Value::none();
	changed.flags = 0;
	return true;
}

uint64_t infer_return_power_bits(const Program& program, SymbolId id)
{
	const Symbol& symbol = program.symbols[id];
	if (symbol.kind != SymbolKind::FunctionDefinition || (symbol.metadata.flags & SF_REPLACEABLE) != 0 ||
		(symbol.metadata.binding != ir_model::SBM_INTERNAL && symbol.metadata.binding != ir_model::SBM_STRONG))
		return 0;
	const Function& function = program.functions[symbol.detail];
	const Type type = program.signatures[function.signature].result;
	if (function.body == nullptr || !type.is_integer() || type.kind() == TypeKind::I128 ||
		function.body->layout.size() > 128)
		return 0;
	unsigned budget = 512;
	uint64_t out = 0;
	for (BlockId b : function.body->layout)
	{
		const InstId last = function.body->blocks[b].last;
		if (last == kNoId || function.body->instructions[last].op != Opcode::Return)
			continue;
		const Instruction& ret = function.body->instructions[last];
		const uint64_t powers = ret.type == type ? returned_powers(*function.body, ret.ops[0], type, budget) : 0;
		if (powers == 0)
			return 0;
		out |= powers;
	}
	return out;
}

bool simplify_integer_arithmetic(FunctionBody& body, const std::vector<uint32_t>& uses, IntegerPatternFacts& facts,
	BlockId block, InstId& previous, InstId instruction)
{
	IntegerPatterns patterns(body, uses, facts);
	const BinaryOp op = static_cast<BinaryOp>(body.instructions[instruction].sub);
	return (op == BinaryOp::Mul || op == BinaryOp::Shl)
		? patterns.round_power_alignment(block, previous, instruction)
		: patterns.variable_power_division(block, previous, instruction);
}
// A scalar extraction from an ABI two-word pack needs only its half.
bool simplify_packed_wide_part(FunctionBody& body, Instruction& inst, bool truncation)
{
	const Type type = truncation ? convert_source_type(inst) : inst.type;
	const auto producer = [&](Value v) -> const Instruction* {
		return v.is(ValueKind::Temp) && v.index >= body.parameter_count && body.temps[v.index].def != kNoId
			? &body.instructions[body.temps[v.index].def]
			: nullptr;
	};
	if (truncation && storage_bytes(inst.type) > 8)
		return false;
	auto count64 = [&](Value v) {
		Constant value;
		return literal_constant(body, v, value) && value.bits.low == 64 && value.bits.high == 0;
	};
	if (!truncation && !count64(inst.ops[1]))
		return false;
	auto zero_half = [&](Value v, bool high) {
		Constant value;
		if (literal_constant(body, v, value))
			return (high ? value.bits.high : value.bits.low) == 0;
		const Instruction* d = producer(v);
		if (d == nullptr || d->type != type)
			return false;
		if (high && d->op == Opcode::Convert && static_cast<ConvertOp>(d->sub) == ConvertOp::ZExt &&
			convert_source_type(*d).is_integer() && storage_bytes(convert_source_type(*d)) <= 8)
			return true;
		return d->op == Opcode::Binary && count64(d->ops[1]) &&
			static_cast<BinaryOp>(d->sub) == (high ? BinaryOp::UShr : BinaryOp::Shl);
	};
	const Instruction* d = producer(inst.ops[0]);
	if (d == nullptr || d->type != type || d->op != Opcode::Binary)
		return false;
	if (!truncation && static_cast<BinaryOp>(d->sub) == BinaryOp::Shl && count64(d->ops[1]) &&
		zero_half(d->ops[0], true))
	{
		inst.op = Opcode::Copy;
		inst.ops[0] = d->ops[0];
		inst.ops[1] = Value::none();
		inst.sub = 0;
		inst.flags = 0;
		inst.immediate = 0;
		return true;
	}
	if (static_cast<BinaryOp>(d->sub) != BinaryOp::Or)
		return false;
	for (unsigned k = 0; k < 2; ++k)
		if (zero_half(d->ops[k], !truncation))
		{
			inst.ops[0] = d->ops[1 - k];
			return true;
		}
	return false;
}

} // namespace opt
