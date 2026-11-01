// The SSE family of GCC's x86 builtins (lex/gnu_builtins.h's VectorLanewise).
// An operation with a `vector` instruction (lower/sse_instructions.cpp) is that
// instruction. Any other is unrolled over the lanes of the slots its operands
// are evaluated into, once each and in order. A lane
// is read and written as bits wherever the operation does not compute on its
// value, so a signaling NaN moves unchanged and masks stay whole. The
// reciprocal estimates are exact: any value within the hardware's error
// bound is a valid estimate.

#include "lower/function_lowering.h"
#include "lower/sse_instructions.h"

#include "sema/conversions.h"
#include "sema/expression_facts.h"

namespace lower {

namespace {

const syntax::Expr* WithoutParentheses(const syntax::Expr* expr)
{
	while (expr->kind == syntax::ExprKind::Paren)
		expr = static_cast<const syntax::ParenExpr*>(expr)->inner;
	return expr;
}

lowir::Type BitsType(uint64_t bytes)
{
	return lowir::Type::scalar(bytes == 1 ? lowir::TypeKind::I8
			: bytes == 2                  ? lowir::TypeKind::I16
			: bytes == 4                  ? lowir::TypeKind::I32
										  : lowir::TypeKind::I64);
}

const lowir::Type kI32 = lowir::Type::scalar(lowir::TypeKind::I32);
const lowir::Type kI64 = lowir::Type::scalar(lowir::TypeKind::I64);

} // namespace

struct FunctionLowering::LanewiseCall
{
	VectorLanewiseOp operation;
	bool scalar;
	const sema::Type* result;
	uint32_t count;
	const sema::Type* types[4];
	lowir::Value addresses[4];
	RValue values[4];

	uint64_t lanes() const { return result->unqualified->bound; }
	const sema::Type* lane() const { return result->unqualified->element; }
};

RValue FunctionLowering::select_value(const RValue& condition, const RValue& first, const RValue& second)
{
	lowir::Instruction choice = lowir::make_instruction(lowir::Opcode::Select, first.type);
	choice.ops[0] = condition.value;
	choice.ops[1] = first.value;
	choice.ops[2] = second.value;
	RValue selected = first;
	selected.value = emit_value(choice, first.type);
	return selected;
}

RValue FunctionLowering::lane_bits(lowir::Value address, uint64_t bytes, uint64_t lane)
{
	Place place = Place();
	place.storage = index(
		lowir::Type::scalar(lowir::TypeKind::I8), ir_model::IPK_FIELD, address, integer(kI64, lane * bytes))
						.value;
	place.untyped = true;
	return load(BitsType(bytes), place);
}

void FunctionLowering::set_lane_bits(lowir::Value address, uint64_t bytes, uint64_t lane, const RValue& value)
{
	Place place = Place();
	place.storage = index(
		lowir::Type::scalar(lowir::TypeKind::I8), ir_model::IPK_FIELD, address, integer(kI64, lane * bytes))
						.value;
	place.untyped = true;
	store(BitsType(bytes), value, place);
}

RValue FunctionLowering::finish_lanes(const sema::Type* vector, lowir::SlotId slot)
{
	Place place = Place();
	place.storage = lowir::Value::slot(slot);
	return load(lowering_.storage_type(vector), place);
}

RValue FunctionLowering::vector_instruction(
	lowir::VectorOp op, const RValue* a, const RValue* b, const RValue* c, int32_t immediate, lowir::Type result)
{
	lowir::Instruction inst = lowir::make_instruction(lowir::Opcode::Vector, result);
	inst.sub = static_cast<uint8_t>(op);
	if (a != nullptr)
		inst.ops[0] = a->value;
	if (b != nullptr)
		inst.ops[1] = b->value;
	if (c != nullptr)
		inst.ops[2] = c->value;
	else if (immediate >= 0)
		inst.ops[2] = integer(kI32, static_cast<uint64_t>(immediate)).value;
	RValue value = RValue();
	value.type = result;
	value.value = emit_value(inst, result);
	return value;
}

bool FunctionLowering::direct_vector_call(LanewiseCall& call, const syntax::CallExpr& expr, RValue& result)
{
	using lowir::VectorShape;
	static_cast<void>(expr);
	const sema::Type* shape = call.result->unqualified;
	SseForm form;
	form.scalar = call.scalar;
	form.vector_result = shape->kind == sema::TypeKind::Vector;
	if (form.vector_result && !lowering_.register_vector(shape))
		return false;
	for (uint32_t i = 0; i < call.count; ++i)
		if (call.types[i]->kind == sema::TypeKind::Vector && !lowering_.register_vector(call.types[i]))
			return false;
	if (form.vector_result)
	{
		form.floating = sema::IsFloating(shape->element->unqualified);
		form.lane_bytes = static_cast<uint32_t>(lowering_.size_of(shape->element, 0));
	}
	// The source lanes of a conversion: the last vector operand.
	for (uint32_t i = 0; i < call.count; ++i)
		if (call.types[i]->kind == sema::TypeKind::Vector)
		{
			form.source_floating = sema::IsFloating(call.types[i]->element->unqualified);
			form.source_bytes = static_cast<uint32_t>(lowering_.size_of(call.types[i]->element, 0));
		}
	if (call.count == 0 || call.types[0]->kind != sema::TypeKind::Vector)
		return false;
	form.second_vector = call.count > 1 && call.types[1]->kind == sema::TypeKind::Vector;
	SseInstruction use;
	if (!SseInstructionOf(call.operation, form, use))
		return false;
	int32_t immediate = use.immediate;
	if (use.literal_operand >= 0)
	{
		uint64_t bits = 0;
		if (!is_integer_literal(call.values[use.literal_operand], bits))
			return false;
		// GCC's palignr count is in bits, on 64-bit lanes.
		const uint64_t count = use.op == lowir::VectorOp::Palignr && call.types[0]->bound != 16 ? bits >> 3 : bits;
		if (count > 255)
			return false;
		immediate = static_cast<int32_t>(count);
	}
	const lowir::Type type = form.vector_result ? lowir::Type::scalar(lowir::TypeKind::V128)
												: lowering_.storage_type(call.result);
	const RValue* first = &call.values[use.swapped ? 1 : 0];
	const RValue* second = call.count > 1 && call.types[1]->kind == sema::TypeKind::Vector
		? &call.values[use.swapped ? 0 : 1]
		: nullptr;
	if (use.repeated)
		second = first;
	const RValue* third = lowir::shape_of(use.op) == VectorShape::Blend ? &call.values[2] : nullptr;
	result = vector_instruction(use.op, first, second, third, immediate, type);
	return true;
}

RValue FunctionLowering::lanewise_vector(const syntax::CallExpr& expr)
{
	LanewiseCall call = LanewiseCall();
	call.operation = static_cast<VectorLanewiseOp>(expr.facts->value >> 8 & 0xff);
	call.scalar = (expr.facts->value >> 16 & 1) != 0;
	call.result = expr.facts->type;
	call.count = static_cast<uint32_t>(expr.arguments.size);
	for (uint32_t i = 0; i < call.count; ++i)
	{
		const sema::ExprFacts* argument = WithoutParentheses(expr.arguments[i])->facts;
		call.types[i] = (argument->conversion != nullptr ? argument->conversion->result : argument->type)->unqualified;
		call.values[i] = operand(*expr.arguments[i], ConversionContext::Argument);
	}
	RValue direct;
	if (direct_vector_call(call, expr, direct))
		return direct;
	for (uint32_t i = 0; i < call.count; ++i)
		if (call.types[i]->kind == sema::TypeKind::Vector)
			call.addresses[i] = vector_storage(call.values[i]);
	switch (call.operation)
	{
	case VectorLanewiseOp::Add:
	case VectorLanewiseOp::Sub:
	case VectorLanewiseOp::Mul:
	case VectorLanewiseOp::Div:
	case VectorLanewiseOp::Min:
	case VectorLanewiseOp::Max:
	case VectorLanewiseOp::Sqrt:
	case VectorLanewiseOp::Reciprocal:
	case VectorLanewiseOp::ReciprocalSqrt:
	case VectorLanewiseOp::AddSub:
		return lanewise_floating(call);
	case VectorLanewiseOp::And:
	case VectorLanewiseOp::Or:
	case VectorLanewiseOp::Xor:
	case VectorLanewiseOp::AndNot:
		return lanewise_bitwise(call);
	case VectorLanewiseOp::CompareEq:
	case VectorLanewiseOp::CompareLt:
	case VectorLanewiseOp::CompareLe:
	case VectorLanewiseOp::CompareGt:
	case VectorLanewiseOp::CompareGe:
	case VectorLanewiseOp::CompareNe:
	case VectorLanewiseOp::CompareNotLt:
	case VectorLanewiseOp::CompareNotLe:
	case VectorLanewiseOp::CompareNotGt:
	case VectorLanewiseOp::CompareNotGe:
	case VectorLanewiseOp::CompareOrdered:
	case VectorLanewiseOp::CompareUnordered:
		return lanewise_mask(call);
	case VectorLanewiseOp::OrderedEq:
	case VectorLanewiseOp::OrderedLt:
	case VectorLanewiseOp::OrderedLe:
	case VectorLanewiseOp::OrderedGt:
	case VectorLanewiseOp::OrderedGe:
	case VectorLanewiseOp::OrderedNe:
	case VectorLanewiseOp::TestZero:
	case VectorLanewiseOp::TestCarry:
	case VectorLanewiseOp::TestMixed:
		return lanewise_ordered(call);
	case VectorLanewiseOp::Convert:
	case VectorLanewiseOp::ConvertNearest:
	case VectorLanewiseOp::ConvertLow:
	case VectorLanewiseOp::Extend:
	case VectorLanewiseOp::ExtendUnsigned:
		return lanewise_convert(call);
	case VectorLanewiseOp::MoveScalar:
	case VectorLanewiseOp::MoveHighToLow:
	case VectorLanewiseOp::MoveLowToHigh:
	case VectorLanewiseOp::UnpackHigh:
	case VectorLanewiseOp::UnpackLow:
	case VectorLanewiseOp::DuplicateOdd:
	case VectorLanewiseOp::DuplicateEven:
	case VectorLanewiseOp::MoveLow:
		return lanewise_move(call);
	case VectorLanewiseOp::AddSaturate:
	case VectorLanewiseOp::AddSaturateUnsigned:
	case VectorLanewiseOp::SubSaturate:
	case VectorLanewiseOp::SubSaturateUnsigned:
	case VectorLanewiseOp::Average:
		return lanewise_saturate(call);
	case VectorLanewiseOp::MulHigh:
	case VectorLanewiseOp::MulHighUnsigned:
	case VectorLanewiseOp::MulHighRound:
	case VectorLanewiseOp::MulWide:
	case VectorLanewiseOp::MulWideUnsigned:
	case VectorLanewiseOp::MulAdd:
	case VectorLanewiseOp::MulAddBytes:
		return lanewise_multiply(call);
	case VectorLanewiseOp::MaxSigned:
	case VectorLanewiseOp::MinSigned:
	case VectorLanewiseOp::MaxUnsigned:
	case VectorLanewiseOp::MinUnsigned:
		return lanewise_extreme(call);
	case VectorLanewiseOp::ShiftLeft:
	case VectorLanewiseOp::ShiftRightLogical:
	case VectorLanewiseOp::ShiftRightArithmetic:
		return lanewise_shift(call);
	case VectorLanewiseOp::Abs:
	case VectorLanewiseOp::Sign:
	case VectorLanewiseOp::SumAbsoluteDifference:
		return lanewise_magnitude(call);
	case VectorLanewiseOp::ByteShuffle:
	case VectorLanewiseOp::ByteAlign:
		return lanewise_bytes(call);
	case VectorLanewiseOp::HorizontalAdd:
	case VectorLanewiseOp::HorizontalSub:
	case VectorLanewiseOp::HorizontalAddSaturate:
	case VectorLanewiseOp::HorizontalSubSaturate:
		return lanewise_horizontal(call);
	case VectorLanewiseOp::Blend:
	case VectorLanewiseOp::BlendVariable:
		return lanewise_blend(call);
	case VectorLanewiseOp::Store:
	case VectorLanewiseOp::StoreValue:
	case VectorLanewiseOp::Load:
	case VectorLanewiseOp::LoadHigh:
	case VectorLanewiseOp::LoadLow:
	case VectorLanewiseOp::StoreHigh:
	case VectorLanewiseOp::StoreLow:
	case VectorLanewiseOp::Fence:
	case VectorLanewiseOp::Hint:
		return lanewise_memory(call);
	}
	throw LoweringError(expr.location, "an unknown packed operation");
}

RValue FunctionLowering::lanewise_floating(const LanewiseCall& call)
{
	// A scalar form computes lane 0 and keeps the first operand's others.
	// GCC's minimum and maximum are `a < b ? a : b` and `a > b ? a : b`, so
	// with a NaN or two zeros they answer the second operand, by bits.
	const sema::Type* vector = call.result->unqualified;
	const sema::Type* lane = call.lane();
	const lowir::Type type = lowering_.storage_type(lane);
	const uint64_t bytes = lowering_.size_of(lane, 0);
	const bool wide = bytes == 8;
	const bool binary_form = call.count == 2;
	lowir::SlotId slot = lowir::kNoId;
	const lowir::Value out = zeroed_vector(vector, slot);
	const lowir::Value a = call.addresses[0];
	const RValue one = convert_op(lowir::ConvertOp::SIToFP, type, integer(kI32, 1));
	for (uint64_t i = 0; i < call.lanes(); ++i)
	{
		if (call.scalar && i != 0)
		{
			set_lane_bits(out, bytes, i, lane_bits(a, bytes, i));
			continue;
		}
		const RValue x = vector_lane(a, vector, i);
		const RValue y = binary_form ? vector_lane(call.addresses[1], vector, i) : RValue();
		switch (call.operation)
		{
		case VectorLanewiseOp::Add:
			set_vector_lane(out, vector, i, arithmetic_op(static_cast<uint32_t>(TokenKind::OP_PLUS), lane, x, y));
			break;
		case VectorLanewiseOp::Sub:
			set_vector_lane(out, vector, i, arithmetic_op(static_cast<uint32_t>(TokenKind::OP_MINUS), lane, x, y));
			break;
		case VectorLanewiseOp::Mul:
			set_vector_lane(out, vector, i, arithmetic_op(static_cast<uint32_t>(TokenKind::OP_STAR), lane, x, y));
			break;
		case VectorLanewiseOp::Div:
			set_vector_lane(out, vector, i, arithmetic_op(static_cast<uint32_t>(TokenKind::OP_DIV), lane, x, y));
			break;
		case VectorLanewiseOp::AddSub:
			set_vector_lane(out, vector, i,
				arithmetic_op(
					static_cast<uint32_t>(i % 2 == 0 ? TokenKind::OP_MINUS : TokenKind::OP_PLUS), lane, x, y));
			break;
		case VectorLanewiseOp::Sqrt:
			set_vector_lane(out, vector, i, soft_call(wide ? SoftFloatHelper::Sqrt : SoftFloatHelper::SqrtF, x));
			break;
		case VectorLanewiseOp::Reciprocal:
			set_vector_lane(out, vector, i, arithmetic_op(static_cast<uint32_t>(TokenKind::OP_DIV), lane, one, x));
			break;
		case VectorLanewiseOp::ReciprocalSqrt:
			set_vector_lane(out, vector, i,
				arithmetic_op(static_cast<uint32_t>(TokenKind::OP_DIV), lane, one,
					soft_call(wide ? SoftFloatHelper::Sqrt : SoftFloatHelper::SqrtF, x)));
			break;
		default:
		{
			const bool minimum = call.operation == VectorLanewiseOp::Min;
			const RValue first = compare(
				minimum ? lowir::ComparePredicate::Lt : lowir::ComparePredicate::Gt, type, x, y);
			set_lane_bits(
				out, bytes, i, select_value(first, lane_bits(a, bytes, i), lane_bits(call.addresses[1], bytes, i)));
			break;
		}
		}
	}
	return finish_lanes(vector, slot);
}

RValue FunctionLowering::lanewise_bitwise(const LanewiseCall& call)
{
	const sema::Type* vector = call.result->unqualified;
	lowir::SlotId slot = lowir::kNoId;
	const lowir::Value out = zeroed_vector(vector, slot);
	for (uint64_t i = 0; i < 2; ++i)
	{
		RValue x = lane_bits(call.addresses[0], 8, i);
		const RValue y = lane_bits(call.addresses[1], 8, i);
		if (call.operation == VectorLanewiseOp::AndNot)
			x = unary(lowir::UnaryOp::BitNot, x);
		const lowir::BinaryOp op = call.operation == VectorLanewiseOp::Or ? lowir::BinaryOp::Or
			: call.operation == VectorLanewiseOp::Xor                     ? lowir::BinaryOp::Xor
																		  : lowir::BinaryOp::And;
		set_lane_bits(out, 8, i, binary(op, kI64, x, y));
	}
	return finish_lanes(vector, slot);
}

RValue FunctionLowering::lanewise_mask(const LanewiseCall& call)
{
	// All ones where the predicate holds, with C's unordered answers: only
	// `!=` holds for a NaN, and the `not` forms hold where theirs does not.
	const sema::Type* vector = call.result->unqualified;
	const lowir::Type type = lowering_.storage_type(call.lane());
	const uint64_t bytes = lowering_.size_of(call.lane(), 0);
	const lowir::Type bits = BitsType(bytes);
	const lowir::Value a = call.addresses[0], b = call.addresses[1];
	lowir::SlotId slot = lowir::kNoId;
	const lowir::Value out = zeroed_vector(vector, slot);
	for (uint64_t i = 0; i < call.lanes(); ++i)
	{
		if (call.scalar && i != 0)
		{
			set_lane_bits(out, bytes, i, lane_bits(a, bytes, i));
			continue;
		}
		const RValue x = vector_lane(a, vector, i), y = vector_lane(b, vector, i);
		lowir::ComparePredicate predicate = lowir::ComparePredicate::Eq;
		bool negate = false;
		switch (call.operation)
		{
		case VectorLanewiseOp::CompareLt:
			predicate = lowir::ComparePredicate::Lt;
			break;
		case VectorLanewiseOp::CompareLe:
			predicate = lowir::ComparePredicate::Le;
			break;
		case VectorLanewiseOp::CompareGt:
			predicate = lowir::ComparePredicate::Gt;
			break;
		case VectorLanewiseOp::CompareGe:
			predicate = lowir::ComparePredicate::Ge;
			break;
		case VectorLanewiseOp::CompareNe:
			predicate = lowir::ComparePredicate::Ne;
			break;
		case VectorLanewiseOp::CompareNotLt:
			predicate = lowir::ComparePredicate::Lt;
			negate = true;
			break;
		case VectorLanewiseOp::CompareNotLe:
			predicate = lowir::ComparePredicate::Le;
			negate = true;
			break;
		case VectorLanewiseOp::CompareNotGt:
			predicate = lowir::ComparePredicate::Gt;
			negate = true;
			break;
		case VectorLanewiseOp::CompareNotGe:
			predicate = lowir::ComparePredicate::Ge;
			negate = true;
			break;
		default:
			break;
		}
		RValue hit;
		if (call.operation == VectorLanewiseOp::CompareOrdered || call.operation == VectorLanewiseOp::CompareUnordered)
		{
			const RValue left = compare(lowir::ComparePredicate::Eq, type, x, x);
			const RValue right = compare(lowir::ComparePredicate::Eq, type, y, y);
			hit = binary(lowir::BinaryOp::And, left.type, left, right);
			negate = call.operation == VectorLanewiseOp::CompareUnordered;
		}
		else
		{
			hit = compare(predicate, type, x, y);
		}
		const RValue ones = integer(bits, ~uint64_t(0)), zero = integer(bits, 0);
		set_lane_bits(out, bytes, i, negate ? select_value(hit, zero, ones) : select_value(hit, ones, zero));
	}
	return finish_lanes(vector, slot);
}

RValue FunctionLowering::lanewise_ordered(const LanewiseCall& call)
{
	// comi and ucomi differ only in the exceptions they raise: lane 0 with
	// C's comparison. ptest sets ZF from a & b and CF from ~a & b.
	const lowir::Value a = call.addresses[0], b = call.addresses[1];
	if (call.operation == VectorLanewiseOp::TestZero || call.operation == VectorLanewiseOp::TestCarry ||
		call.operation == VectorLanewiseOp::TestMixed)
	{
		RValue both = integer(kI64, 0), outside = integer(kI64, 0);
		for (uint64_t i = 0; i < 2; ++i)
		{
			const RValue x = lane_bits(a, 8, i), y = lane_bits(b, 8, i);
			both = binary(lowir::BinaryOp::Or, kI64, both, binary(lowir::BinaryOp::And, kI64, x, y));
			outside = binary(lowir::BinaryOp::Or, kI64, outside,
				binary(lowir::BinaryOp::And, kI64, unary(lowir::UnaryOp::BitNot, x), y));
		}
		const RValue zero = integer(kI64, 0);
		const RValue zf = compare(lowir::ComparePredicate::Eq, kI64, both, zero);
		const RValue cf = compare(lowir::ComparePredicate::Eq, kI64, outside, zero);
		const RValue yes = integer(kI32, 1), no = integer(kI32, 0);
		if (call.operation == VectorLanewiseOp::TestZero)
			return select_value(zf, yes, no);
		if (call.operation == VectorLanewiseOp::TestCarry)
			return select_value(cf, yes, no);
		return select_value(cf, no, select_value(zf, no, yes));
	}
	lowir::ComparePredicate predicate = lowir::ComparePredicate::Eq;
	if (call.operation == VectorLanewiseOp::OrderedLt)
		predicate = lowir::ComparePredicate::Lt;
	else if (call.operation == VectorLanewiseOp::OrderedLe)
		predicate = lowir::ComparePredicate::Le;
	else if (call.operation == VectorLanewiseOp::OrderedGt)
		predicate = lowir::ComparePredicate::Gt;
	else if (call.operation == VectorLanewiseOp::OrderedGe)
		predicate = lowir::ComparePredicate::Ge;
	else if (call.operation == VectorLanewiseOp::OrderedNe)
		predicate = lowir::ComparePredicate::Ne;
	const sema::Type* vector = call.types[0];
	const lowir::Type type = lowering_.storage_type(vector->element);
	const RValue hit = compare(predicate, type, vector_lane(a, vector, 0), vector_lane(b, vector, 0));
	return select_value(hit, integer(kI32, 1), integer(kI32, 0));
}

RValue FunctionLowering::lanewise_convert(const LanewiseCall& call)
{
	const bool nearest = call.operation == VectorLanewiseOp::ConvertNearest;
	const bool zero_extend = call.operation == VectorLanewiseOp::ExtendUnsigned;
	const auto converted = [&](RValue value, const sema::Type* from, const sema::Type* to) {
		const lowir::Type target = lowering_.storage_type(to);
		if (sema::IsFloating(from) && sema::IsFloating(to))
			return convert_op(lowering_.size_of(to, 0) > lowering_.size_of(from, 0) ? lowir::ConvertOp::FPExt
																					: lowir::ConvertOp::FPTrunc,
				target, value);
		if (sema::IsFloating(to))
			return convert_op(lowir::ConvertOp::SIToFP, target, value);
		if (sema::IsFloating(from))
		{
			// Native selection gives an invalid conversion x86's
			// integer-indefinite value at the destination's width.
			if (nearest)
				value = soft_call(
					lowering_.size_of(from, 0) == 8 ? SoftFloatHelper::Rint : SoftFloatHelper::RintF, value);
			return convert_op(lowir::ConvertOp::FPToSI, target, value);
		}
		return convert_op(zero_extend ? lowir::ConvertOp::ZExt : lowir::ConvertOp::SExt, target, value);
	};
	if (call.operation == VectorLanewiseOp::ConvertLow)
	{
		// Lane 0 from the second operand, the first operand's others.
		const sema::Type* vector = call.result->unqualified;
		const uint64_t bytes = lowering_.size_of(call.lane(), 0);
		lowir::SlotId slot = lowir::kNoId;
		const lowir::Value out = zeroed_vector(vector, slot);
		const sema::Type* source = call.types[1];
		const bool lanes = source->kind == sema::TypeKind::Vector;
		const RValue low = lanes ? vector_lane(call.addresses[1], source, 0) : call.values[1];
		set_vector_lane(out, vector, 0, converted(low, lanes ? source->element->unqualified : source, call.lane()));
		for (uint64_t i = 1; i < call.lanes(); ++i)
			set_lane_bits(out, bytes, i, lane_bits(call.addresses[0], bytes, i));
		return finish_lanes(vector, slot);
	}
	const sema::Type* source = call.types[0];
	const sema::Type* from = source->element->unqualified;
	if (call.result->unqualified->kind != sema::TypeKind::Vector)
		return converted(vector_lane(call.addresses[0], source, 0), from, call.result->unqualified);
	// Fewer source lanes than result lanes leave the rest zero, as cvtpd2ps.
	const sema::Type* vector = call.result->unqualified;
	lowir::SlotId slot = lowir::kNoId;
	const lowir::Value out = zeroed_vector(vector, slot);
	const uint64_t count = call.lanes() < source->bound ? call.lanes() : source->bound;
	for (uint64_t i = 0; i < count; ++i)
		set_vector_lane(out, vector, i, converted(vector_lane(call.addresses[0], source, i), from, call.lane()));
	return finish_lanes(vector, slot);
}

RValue FunctionLowering::lanewise_move(const LanewiseCall& call)
{
	const sema::Type* vector = call.result->unqualified;
	const uint64_t bytes = lowering_.size_of(call.lane(), 0);
	const uint64_t n = call.lanes();
	const lowir::Value a = call.addresses[0], b = call.addresses[1];
	lowir::SlotId slot = lowir::kNoId;
	const lowir::Value out = zeroed_vector(vector, slot);
	for (uint64_t i = 0; i < n; ++i)
	{
		bool second = false;
		uint64_t from = i;
		switch (call.operation)
		{
		case VectorLanewiseOp::MoveScalar:
			second = i == 0;
			break;
		case VectorLanewiseOp::MoveHighToLow:
			second = i < 2;
			from = i < 2 ? i + 2 : i;
			break;
		case VectorLanewiseOp::MoveLowToHigh:
			second = i >= 2;
			from = i >= 2 ? i - 2 : i;
			break;
		case VectorLanewiseOp::UnpackLow:
		case VectorLanewiseOp::UnpackHigh:
			second = i % 2 != 0;
			from = i / 2 + (call.operation == VectorLanewiseOp::UnpackHigh ? n / 2 : 0);
			break;
		case VectorLanewiseOp::DuplicateOdd:
			from = i | 1;
			break;
		case VectorLanewiseOp::DuplicateEven:
			from = i & ~uint64_t(1);
			break;
		default:
			if (i != 0)
				continue;
			break;
		}
		set_lane_bits(out, bytes, i, lane_bits(second ? b : a, bytes, from));
	}
	return finish_lanes(vector, slot);
}

RValue FunctionLowering::lanewise_saturate(const LanewiseCall& call)
{
	const sema::Type* vector = call.result->unqualified;
	const uint64_t bytes = lowering_.size_of(call.lane(), 0);
	const lowir::Type narrow = BitsType(bytes);
	const VectorLanewiseOp op = call.operation;
	const bool average = op == VectorLanewiseOp::Average;
	const bool is_signed = op == VectorLanewiseOp::AddSaturate || op == VectorLanewiseOp::SubSaturate;
	const bool subtract = op == VectorLanewiseOp::SubSaturate || op == VectorLanewiseOp::SubSaturateUnsigned;
	const lowir::ConvertOp extend = is_signed ? lowir::ConvertOp::SExt : lowir::ConvertOp::ZExt;
	const int64_t top = int64_t(1) << (bytes * 8 - (is_signed ? 1 : 0));
	const RValue lowest = integer(kI32, is_signed ? static_cast<uint64_t>(-top) : 0);
	const RValue highest = integer(kI32, static_cast<uint64_t>(top - 1));
	lowir::SlotId slot = lowir::kNoId;
	const lowir::Value out = zeroed_vector(vector, slot);
	for (uint64_t i = 0; i < call.lanes(); ++i)
	{
		const RValue x = convert_op(extend, kI32, lane_bits(call.addresses[0], bytes, i));
		const RValue y = convert_op(extend, kI32, lane_bits(call.addresses[1], bytes, i));
		RValue wide = binary(subtract ? lowir::BinaryOp::Sub : lowir::BinaryOp::Add, kI32, x, y);
		if (average)
			wide = binary(lowir::BinaryOp::UShr, kI32, binary(lowir::BinaryOp::Add, kI32, wide, integer(kI32, 1)),
				integer(kI32, 1));
		else
		{
			wide = select_value(compare(lowir::ComparePredicate::Lt, kI32, wide, lowest), lowest, wide);
			wide = select_value(compare(lowir::ComparePredicate::Gt, kI32, wide, highest), highest, wide);
		}
		set_lane_bits(out, bytes, i, convert_op(lowir::ConvertOp::Trunc, narrow, wide));
	}
	return finish_lanes(vector, slot);
}

RValue FunctionLowering::lanewise_multiply(const LanewiseCall& call)
{
	const sema::Type* vector = call.result->unqualified;
	const VectorLanewiseOp op = call.operation;
	const lowir::Value a = call.addresses[0], b = call.addresses[1];
	lowir::SlotId slot = lowir::kNoId;
	const lowir::Value out = zeroed_vector(vector, slot);
	const RValue sixteen = integer(kI32, 16);
	if (op == VectorLanewiseOp::MulWide || op == VectorLanewiseOp::MulWideUnsigned)
	{
		const lowir::ConvertOp extend = op == VectorLanewiseOp::MulWide ? lowir::ConvertOp::SExt
																		: lowir::ConvertOp::ZExt;
		for (uint64_t i = 0; i < 2; ++i)
			set_lane_bits(out, 8, i,
				binary(lowir::BinaryOp::Mul, kI64, convert_op(extend, kI64, lane_bits(a, 4, i * 2)),
					convert_op(extend, kI64, lane_bits(b, 4, i * 2))));
		return finish_lanes(vector, slot);
	}
	const auto signed_lane = [&](lowir::Value from, uint64_t bytes, uint64_t lane) {
		return convert_op(lowir::ConvertOp::SExt, kI32, lane_bits(from, bytes, lane));
	};
	if (op == VectorLanewiseOp::MulAdd)
	{
		for (uint64_t i = 0; i < 4; ++i)
		{
			const RValue low = binary(lowir::BinaryOp::Mul, kI32, signed_lane(a, 2, i * 2), signed_lane(b, 2, i * 2));
			const RValue high = binary(
				lowir::BinaryOp::Mul, kI32, signed_lane(a, 2, i * 2 + 1), signed_lane(b, 2, i * 2 + 1));
			set_lane_bits(out, 4, i, binary(lowir::BinaryOp::Add, kI32, low, high));
		}
		return finish_lanes(vector, slot);
	}
	if (op == VectorLanewiseOp::MulAddBytes)
	{
		// Unsigned bytes of the first by signed ones of the second, each
		// pair's sum saturated to 16 bits.
		const RValue lowest = integer(kI32, static_cast<uint64_t>(int64_t(-32768)));
		const RValue highest = integer(kI32, 32767);
		for (uint64_t i = 0; i < 8; ++i)
		{
			RValue sum = integer(kI32, 0);
			for (uint64_t k = 0; k < 2; ++k)
			{
				const RValue x = convert_op(lowir::ConvertOp::ZExt, kI32, lane_bits(a, 1, i * 2 + k));
				sum = binary(lowir::BinaryOp::Add, kI32, sum,
					binary(lowir::BinaryOp::Mul, kI32, x, signed_lane(b, 1, i * 2 + k)));
			}
			sum = select_value(compare(lowir::ComparePredicate::Lt, kI32, sum, lowest), lowest, sum);
			sum = select_value(compare(lowir::ComparePredicate::Gt, kI32, sum, highest), highest, sum);
			set_lane_bits(out, 2, i, convert_op(lowir::ConvertOp::Trunc, BitsType(2), sum));
		}
		return finish_lanes(vector, slot);
	}
	const lowir::ConvertOp extend = op == VectorLanewiseOp::MulHighUnsigned ? lowir::ConvertOp::ZExt
																			: lowir::ConvertOp::SExt;
	for (uint64_t i = 0; i < 8; ++i)
	{
		const RValue product = binary(lowir::BinaryOp::Mul, kI32, convert_op(extend, kI32, lane_bits(a, 2, i)),
			convert_op(extend, kI32, lane_bits(b, 2, i)));
		// pmulhrsw: (product >> 14) + 1 >> 1, the high half rounded.
		const RValue high = op == VectorLanewiseOp::MulHighRound
			? binary(lowir::BinaryOp::Shr, kI32,
				  binary(lowir::BinaryOp::Add, kI32, binary(lowir::BinaryOp::Shr, kI32, product, integer(kI32, 14)),
					  integer(kI32, 1)),
				  integer(kI32, 1))
			: binary(lowir::BinaryOp::UShr, kI32, product, sixteen);
		set_lane_bits(out, 2, i, convert_op(lowir::ConvertOp::Trunc, BitsType(2), high));
	}
	return finish_lanes(vector, slot);
}

RValue FunctionLowering::lanewise_extreme(const LanewiseCall& call)
{
	const sema::Type* vector = call.result->unqualified;
	const uint64_t bytes = lowering_.size_of(call.lane(), 0);
	const lowir::Type type = BitsType(bytes);
	const VectorLanewiseOp op = call.operation;
	const lowir::ComparePredicate predicate = op == VectorLanewiseOp::MaxSigned ? lowir::ComparePredicate::Gt
		: op == VectorLanewiseOp::MinSigned                                     ? lowir::ComparePredicate::Lt
		: op == VectorLanewiseOp::MaxUnsigned                                   ? lowir::ComparePredicate::Ugt
																				: lowir::ComparePredicate::Ult;
	lowir::SlotId slot = lowir::kNoId;
	const lowir::Value out = zeroed_vector(vector, slot);
	for (uint64_t i = 0; i < call.lanes(); ++i)
	{
		const RValue x = lane_bits(call.addresses[0], bytes, i), y = lane_bits(call.addresses[1], bytes, i);
		set_lane_bits(out, bytes, i, select_value(compare(predicate, type, x, y), x, y));
	}
	return finish_lanes(vector, slot);
}

RValue FunctionLowering::lanewise_shift(const LanewiseCall& call)
{
	// The count is the second operand's low 64 bits, unsigned. A count of
	// the lane's width or more clears a logical shift's lane and fills an
	// arithmetic one with its sign.
	const sema::Type* vector = call.result->unqualified;
	const uint64_t bytes = lowering_.size_of(call.lane(), 0);
	const lowir::Type type = BitsType(bytes);
	const VectorLanewiseOp op = call.operation;
	const uint64_t bits = bytes * 8;
	const RValue count = lane_bits(call.addresses[1], 8, 0);
	const RValue valid = compare(lowir::ComparePredicate::Ult, kI64, count, integer(kI64, bits));
	const RValue safe = select_value(valid, count, integer(kI64, bits - 1));
	const RValue shift = bytes == 8 ? safe : convert_op(lowir::ConvertOp::Trunc, type, safe);
	lowir::SlotId slot = lowir::kNoId;
	const lowir::Value out = zeroed_vector(vector, slot);
	for (uint64_t i = 0; i < call.lanes(); ++i)
	{
		const RValue lane = binary(op == VectorLanewiseOp::ShiftLeft ? lowir::BinaryOp::Shl
				: op == VectorLanewiseOp::ShiftRightLogical          ? lowir::BinaryOp::UShr
																	 : lowir::BinaryOp::Shr,
			type, lane_bits(call.addresses[0], bytes, i), shift);
		set_lane_bits(out, bytes, i,
			op == VectorLanewiseOp::ShiftRightArithmetic ? lane : select_value(valid, lane, integer(type, 0)));
	}
	return finish_lanes(vector, slot);
}

RValue FunctionLowering::lanewise_magnitude(const LanewiseCall& call)
{
	const sema::Type* vector = call.result->unqualified;
	const lowir::Value a = call.addresses[0], b = call.addresses[1];
	lowir::SlotId slot = lowir::kNoId;
	const lowir::Value out = zeroed_vector(vector, slot);
	if (call.operation == VectorLanewiseOp::SumAbsoluteDifference)
	{
		// psadbw: each half's bytes' absolute differences, summed.
		for (uint64_t half = 0; half < 2; ++half)
		{
			RValue sum = integer(kI32, 0);
			for (uint64_t i = 0; i < 8; ++i)
			{
				const RValue x = convert_op(lowir::ConvertOp::ZExt, kI32, lane_bits(a, 1, half * 8 + i));
				const RValue y = convert_op(lowir::ConvertOp::ZExt, kI32, lane_bits(b, 1, half * 8 + i));
				const RValue difference = binary(lowir::BinaryOp::Sub, kI32, x, y);
				const RValue negated = binary(lowir::BinaryOp::Sub, kI32, integer(kI32, 0), difference);
				sum = binary(lowir::BinaryOp::Add, kI32, sum,
					select_value(
						compare(lowir::ComparePredicate::Lt, kI32, difference, integer(kI32, 0)), negated, difference));
			}
			set_lane_bits(out, 8, half, convert_op(lowir::ConvertOp::ZExt, kI64, sum));
		}
		return finish_lanes(vector, slot);
	}
	const uint64_t bytes = lowering_.size_of(call.lane(), 0);
	const lowir::Type type = BitsType(bytes);
	const RValue zero = integer(type, 0);
	for (uint64_t i = 0; i < call.lanes(); ++i)
	{
		const RValue x = lane_bits(a, bytes, i);
		const RValue negated = binary(lowir::BinaryOp::Sub, type, zero, x);
		if (call.operation == VectorLanewiseOp::Abs)
		{
			set_lane_bits(out, bytes, i, select_value(compare(lowir::ComparePredicate::Lt, type, x, zero), negated, x));
			continue;
		}
		const RValue y = lane_bits(b, bytes, i);
		const RValue kept = select_value(compare(lowir::ComparePredicate::Eq, type, y, zero), zero, x);
		set_lane_bits(out, bytes, i, select_value(compare(lowir::ComparePredicate::Lt, type, y, zero), negated, kept));
	}
	return finish_lanes(vector, slot);
}

RValue FunctionLowering::lanewise_bytes(const LanewiseCall& call)
{
	const sema::Type* vector = call.result->unqualified;
	const lowir::Type byte = BitsType(1);
	const lowir::Type u8 = lowir::Type::scalar(lowir::TypeKind::I8);
	lowir::SlotId slot = lowir::kNoId;
	const lowir::Value out = zeroed_vector(vector, slot);
	const auto byte_at = [&](lowir::Value base, const RValue& offset) {
		Place place = Place();
		place.storage = index(u8, ir_model::IPK_FIELD, base, offset).value;
		place.untyped = true;
		return load(byte, place);
	};
	if (call.operation == VectorLanewiseOp::ByteShuffle)
	{
		// pshufb: a selector's high bit gives zero, its low four a byte.
		for (uint64_t i = 0; i < 16; ++i)
		{
			const RValue selector = lane_bits(call.addresses[1], 1, i);
			const RValue wide = convert_op(lowir::ConvertOp::ZExt, kI64, selector);
			const RValue picked = byte_at(
				call.addresses[0], binary(lowir::BinaryOp::And, kI64, wide, integer(kI64, 15)));
			const RValue negative = compare(lowir::ComparePredicate::Lt, byte, selector, integer(byte, 0));
			set_lane_bits(out, 1, i, select_value(negative, integer(byte, 0), picked));
		}
		return finish_lanes(vector, slot);
	}
	// palignr: the pair's 32 bytes, the first operand high, shifted right
	// by the count, which GCC's interface states in bits on __v2di and
	// Clang's in bytes on __v16qi.
	const lowir::Type pair = lowir::Type::object(32, 4);
	Place storage = Place();
	storage.storage = lowir::Value::slot(generated_slot(lowir::SlotKind::Vector, pair));
	const lowir::Value joined = address_of(storage);
	for (uint64_t i = 0; i < 2; ++i)
	{
		set_lane_bits(joined, 8, i, lane_bits(call.addresses[1], 8, i));
		set_lane_bits(joined, 8, i + 2, lane_bits(call.addresses[0], 8, i));
	}
	const bool in_bytes = call.types[0]->bound == 16;
	const RValue shift = in_bytes ? call.values[2]
								  : binary(lowir::BinaryOp::UShr, kI32, call.values[2], integer(kI32, 3));
	const RValue start = convert_op(lowir::ConvertOp::ZExt, kI64, shift);
	for (uint64_t i = 0; i < 16; ++i)
	{
		const RValue at = binary(lowir::BinaryOp::Add, kI64, start, integer(kI64, i));
		const RValue inside = compare(lowir::ComparePredicate::Ult, kI64, at, integer(kI64, 32));
		const RValue value = byte_at(joined, select_value(inside, at, integer(kI64, 0)));
		set_lane_bits(out, 1, i, select_value(inside, value, integer(byte, 0)));
	}
	return finish_lanes(vector, slot);
}

RValue FunctionLowering::lanewise_horizontal(const LanewiseCall& call)
{
	// Adjacent lanes of the first operand fill the low half, of the second
	// the high half; the saturating forms work on 16-bit lanes.
	const sema::Type* vector = call.result->unqualified;
	const sema::Type* lane = call.lane();
	const uint64_t n = call.lanes();
	const VectorLanewiseOp op = call.operation;
	const bool subtract = op == VectorLanewiseOp::HorizontalSub || op == VectorLanewiseOp::HorizontalSubSaturate;
	const bool saturate = op == VectorLanewiseOp::HorizontalAddSaturate ||
		op == VectorLanewiseOp::HorizontalSubSaturate;
	const RValue lowest = integer(kI32, static_cast<uint64_t>(int64_t(-32768)));
	const RValue highest = integer(kI32, 32767);
	lowir::SlotId slot = lowir::kNoId;
	const lowir::Value out = zeroed_vector(vector, slot);
	for (uint64_t i = 0; i < n; ++i)
	{
		const lowir::Value source = i < n / 2 ? call.addresses[0] : call.addresses[1];
		const uint64_t first = (i % (n / 2)) * 2;
		const RValue x = vector_lane(source, vector, first), y = vector_lane(source, vector, first + 1);
		if (!saturate)
		{
			set_vector_lane(out, vector, i,
				arithmetic_op(static_cast<uint32_t>(subtract ? TokenKind::OP_MINUS : TokenKind::OP_PLUS), lane, x, y));
			continue;
		}
		RValue wide = binary(subtract ? lowir::BinaryOp::Sub : lowir::BinaryOp::Add, kI32,
			convert_op(lowir::ConvertOp::SExt, kI32, x), convert_op(lowir::ConvertOp::SExt, kI32, y));
		wide = select_value(compare(lowir::ComparePredicate::Lt, kI32, wide, lowest), lowest, wide);
		wide = select_value(compare(lowir::ComparePredicate::Gt, kI32, wide, highest), highest, wide);
		set_vector_lane(out, vector, i, convert_op(lowir::ConvertOp::Trunc, x.type, wide));
	}
	return finish_lanes(vector, slot);
}

RValue FunctionLowering::lanewise_blend(const LanewiseCall& call)
{
	// Each lane from the second operand where the immediate's bit or the
	// mask lane's sign bit is set.
	const sema::Type* vector = call.result->unqualified;
	const uint64_t bytes = lowering_.size_of(call.lane(), 0);
	const lowir::Type bits = BitsType(bytes);
	lowir::SlotId slot = lowir::kNoId;
	const lowir::Value out = zeroed_vector(vector, slot);
	for (uint64_t i = 0; i < call.lanes(); ++i)
	{
		RValue second;
		if (call.operation == VectorLanewiseOp::Blend)
			second = compare(lowir::ComparePredicate::Ne, kI32,
				binary(lowir::BinaryOp::And, kI32, call.values[2], integer(kI32, uint64_t(1) << i)), integer(kI32, 0));
		else
			second = compare(
				lowir::ComparePredicate::Lt, bits, lane_bits(call.addresses[2], bytes, i), integer(bits, 0));
		set_lane_bits(out, bytes, i,
			select_value(second, lane_bits(call.addresses[1], bytes, i), lane_bits(call.addresses[0], bytes, i)));
	}
	return finish_lanes(vector, slot);
}

RValue FunctionLowering::lanewise_memory(const LanewiseCall& call)
{
	// A non-temporal store is a store (its ordering is only weaker) and an
	// unaligned load a load; pause and clflush are hints here.
	const auto high_half = [](VectorLanewiseOp op) {
		return op == VectorLanewiseOp::LoadHigh || op == VectorLanewiseOp::StoreHigh ? uint64_t(2) : uint64_t(0);
	};
	switch (call.operation)
	{
	case VectorLanewiseOp::Fence:
	{
		lowir::Instruction fence = lowir::make_instruction(
			lowir::Opcode::AtomicThreadFence, lowir::Type::scalar(lowir::TypeKind::Void));
		fence.order = lowir::MemoryOrder::SeqCst;
		emit(fence);
		return RValue();
	}
	case VectorLanewiseOp::Hint:
		return RValue();
	case VectorLanewiseOp::Store:
	{
		Place place = Place();
		place.storage = call.values[0].value;
		place.untyped = true;
		store(lowering_.storage_type(call.types[1]), call.values[1], place);
		return RValue();
	}
	case VectorLanewiseOp::StoreValue:
	{
		Place place = Place();
		place.storage = call.values[1].value;
		place.untyped = true;
		store(lowering_.storage_type(call.types[0]), call.values[0], place);
		return RValue();
	}
	case VectorLanewiseOp::Load:
	{
		Place place = Place();
		place.storage = call.values[0].value;
		place.untyped = true;
		return load(lowering_.storage_type(call.result), place);
	}
	case VectorLanewiseOp::StoreHigh:
	case VectorLanewiseOp::StoreLow:
		for (uint64_t i = 0; i < 2; ++i)
			set_lane_bits(call.values[0].value, 4, i, lane_bits(call.addresses[1], 4, high_half(call.operation) + i));
		return RValue();
	default:
		break;
	}
	const sema::Type* vector = call.result->unqualified;
	lowir::SlotId slot = lowir::kNoId;
	const lowir::Value out = zeroed_vector(vector, slot);
	const uint64_t loaded = high_half(call.operation);
	for (uint64_t i = 0; i < 4; ++i)
	{
		const bool from_memory = i >= loaded && i < loaded + 2;
		set_lane_bits(out, 4, i,
			from_memory ? lane_bits(call.values[1].value, 4, i - loaded) : lane_bits(call.addresses[0], 4, i));
	}
	return finish_lanes(vector, slot);
}

} // namespace lower
