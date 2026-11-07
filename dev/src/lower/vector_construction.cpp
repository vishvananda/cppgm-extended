// Construct narrow integer vectors in registers, preserving list evaluation
// order and each lane's bit pattern. Scalar stores followed by a vector load
// otherwise require costly store forwarding even for a constant splat.
#include "lower/function_lowering.h"

namespace lower {

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
