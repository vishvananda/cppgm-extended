#include "opt/integer_patterns.h"
#include "lowir/builder.h"
#include "opt/scalar_fold.h"
namespace opt {
using namespace lowir;
namespace {
class IntegerPatterns
{
public:
	IntegerPatterns(FunctionBody& body, const std::vector<uint32_t>& uses, IntegerPatternFacts& facts)
		: body_(body), uses_(uses), facts_(facts)
	{}
	bool variable_power_division(BlockId block, InstId& prev, InstId id);
	bool round_power_alignment(BlockId block, InstId& prev, InstId id);

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
		constant(def->ops[0], type, c) && is_integer(type, c, 1) && possible_bits(def->ops[1], type) < width)
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

} // namespace
bool simplify_integer_arithmetic(FunctionBody& body, const std::vector<uint32_t>& uses, IntegerPatternFacts& facts,
	BlockId block, InstId& previous, InstId instruction)
{
	IntegerPatterns patterns(body, uses, facts);
	const BinaryOp op = static_cast<BinaryOp>(body.instructions[instruction].sub);
	return (op == BinaryOp::Mul || op == BinaryOp::Shl)
		? patterns.round_power_alignment(block, previous, instruction)
		: patterns.variable_power_division(block, previous, instruction);
}
} // namespace opt
