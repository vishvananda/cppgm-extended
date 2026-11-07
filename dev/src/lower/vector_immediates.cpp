// Fixed-width integer vector operations with immediate selectors. Wide
// permutes retain the ordinary vector-object ABI and need no AVX registers.
#include "lower/function_lowering.h"

#include "sema/expression_facts.h"

namespace lower {

RValue FunctionLowering::binary_immediate_vector(const syntax::CallExpr& expr)
{
	const bool multiply = static_cast<sema::Intrinsic>(expr.facts->value & 0xff) ==
		sema::Intrinsic::VectorCarrylessMultiply;
	const RValue first = operand(*expr.arguments[0], ConversionContext::Argument);
	const RValue second = operand(*expr.arguments[1], ConversionContext::Argument);
	const lowir::Type i64 = lowir::Type::scalar(lowir::TypeKind::I64);
	const lowir::Type ptr = lowir::Type::scalar(lowir::TypeKind::Ptr);
	const bool deferred = (expr.facts->value >> 16 & 1) != 0;
	const uint32_t immediate = static_cast<uint32_t>(expr.facts->value >> 8 & 255);
	RValue count = integer(i64, immediate);
	if (deferred)
	{
		const RValue given = operand(*expr.arguments[2], ConversionContext::Argument);
		builder_.call(lowir::Type::scalar(lowir::TypeKind::Void),
			lowir::Value::symbol(lowering_.source_immediate_guard()), &given.value, 1, lowir::kNoId, 0);
		count = convert_op(lowir::ConvertOp::ZExt, i64, given);
	}
	else
		discard(*expr.arguments[2]);
	if (multiply && !deferred)
		return vector_instruction(lowir::VectorOp::Pclmulqdq, &first, &second, nullptr, immediate);
	const lowir::Value a = vector_storage(first);
	const lowir::Value b = vector_storage(second);
	const auto read_word = [&](lowir::Value base, const RValue& offset) {
		Place place = Place();
		place.untyped = true;
		place.storage = index(lowir::Type::scalar(lowir::TypeKind::I8), ir_model::IPK_FIELD, base, offset).value;
		return load(i64, place);
	};
	if (multiply)
	{
		const RValue low = binary(
			lowir::BinaryOp::Mul, i64, binary(lowir::BinaryOp::And, i64, count, integer(i64, 1)), integer(i64, 8));
		const RValue high = binary(
			lowir::BinaryOp::UShr, i64, binary(lowir::BinaryOp::And, i64, count, integer(i64, 16)), integer(i64, 1));
		const RValue av = read_word(a, low);
		const RValue bv = read_word(b, high);
		const RValue packed_a = vector_instruction(lowir::VectorOp::Fromi64, &av);
		const RValue packed_b = vector_instruction(lowir::VectorOp::Fromi64, &bv);
		return vector_instruction(lowir::VectorOp::Pclmulqdq, &packed_a, &packed_b, nullptr, 0);
	}
	lowir::SlotId slot = lowir::kNoId;
	const lowir::Value out = zeroed_vector(expr.facts->type, slot);
	RValue first_address;
	first_address.type = ptr;
	first_address.value = a;
	RValue second_address = first_address;
	second_address.value = b;
	for (uint32_t half = 0; half < 2; ++half)
	{
		const RValue selector = binary(lowir::BinaryOp::UShr, i64, count, integer(i64, half * 4));
		const RValue choose_second = compare(lowir::ComparePredicate::Ne, i64,
			binary(lowir::BinaryOp::And, i64, selector, integer(i64, 2)), integer(i64, 0));
		const RValue base = select_value(choose_second, second_address, first_address);
		const RValue offset = binary(
			lowir::BinaryOp::Mul, i64, binary(lowir::BinaryOp::And, i64, selector, integer(i64, 1)), integer(i64, 16));
		const RValue zero = compare(lowir::ComparePredicate::Ne, i64,
			binary(lowir::BinaryOp::And, i64, selector, integer(i64, 8)), integer(i64, 0));
		for (uint32_t lane = 0; lane < 2; ++lane)
		{
			const RValue at = binary(lowir::BinaryOp::Add, i64, offset, integer(i64, lane * 8));
			const RValue value = select_value(zero, integer(i64, 0), read_word(base.value, at));
			set_vector_lane(out, expr.facts->type, half * 2 + lane, value);
		}
	}
	Place result = Place();
	result.storage = lowir::Value::slot(slot);
	return load(lowering_.storage_type(expr.facts->type), result);
}

} // namespace lower
