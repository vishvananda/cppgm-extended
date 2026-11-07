// Construct narrow integer vectors in registers, preserving list evaluation
// order and each lane's bit pattern. Scalar stores followed by a vector load
// otherwise require costly store forwarding even for a constant splat.
#include "lower/function_lowering.h"

namespace lower {

RValue FunctionLowering::vector_scalar_cast(const sema::Type* vector, const sema::Type* from, const RValue& given)
{
	const sema::Type* lane = vector->element;
	const RValue value = convert(given, from, lane, ConversionContext::Cast);
	lowir::VectorOp form;
	if (vector_scalar_form(vector, form))
		return spread_vector(vector, form, value);
	const uint64_t bytes = lowering_.size_of(lane, 0);
	if (lowering_.register_vector(vector) && value.type.is_integer() && (bytes == 1 || bytes == 2))
	{
		const lowir::Type word = lowir::Type::scalar(lowir::TypeKind::I64);
		const RValue bits = convert_op(lowir::ConvertOp::ZExt, word, value);
		const RValue repeated = binary(lowir::BinaryOp::Mul, word, bits,
			integer(word, bytes == 1 ? 0x0101010101010101ull : 0x0001000100010001ull));
		const RValue half = vector_instruction(lowir::VectorOp::Fromi64, &repeated);
		return vector_instruction(lowir::VectorOp::Punpcklqdq, &half, &half);
	}
	lowir::SlotId slot;
	const lowir::Value out = zeroed_vector(vector, slot);
	for (uint64_t i = 0; i < vector->bound; ++i)
		set_vector_lane(out, vector, i, value);
	return finish_lanes(vector, slot);
}

bool FunctionLowering::direct_small_vector_list(
	const sema::Type* vector, const syntax::BracedInitList& list, RValue& result)
{
	if (!lowering_.register_vector(vector) || list.elements.size != vector->unqualified->bound)
		return false;
	const sema::Type* lane = vector->unqualified->element;
	const lowir::Type scalar = lowering_.storage_type(lane);
	const uint64_t bytes = lowering_.size_of(lane, 0);
	if (!scalar.is_integer() || (bytes != 1 && bytes != 2))
		return false;
	const lowir::Type word = lowir::Type::scalar(lowir::TypeKind::I64);
	RValue halves[2] = { integer(word, 0), integer(word, 0) };
	for (uint32_t i = 0; i < list.elements.size; ++i)
	{
		const RValue value = fit(operand(*list.elements[i], ConversionContext::Initialization), lane);
		RValue bits = convert_op(lowir::ConvertOp::ZExt, word, value);
		const uint32_t shift = static_cast<uint32_t>(i * bytes % 8) * 8;
		if (shift != 0)
			bits = binary(lowir::BinaryOp::Shl, word, bits, integer(word, shift));
		const uint32_t half = static_cast<uint32_t>(i * bytes / 8);
		halves[half] = binary(lowir::BinaryOp::Or, word, halves[half], bits);
	}
	const RValue low = vector_instruction(lowir::VectorOp::Fromi64, &halves[0]);
	const RValue high = vector_instruction(lowir::VectorOp::Fromi64, &halves[1]);
	result = vector_instruction(lowir::VectorOp::Punpcklqdq, &low, &high);
	return true;
}

} // namespace lower
