#include "lower/function_lowering.h"

namespace lower {

uint32_t AbiTableHeaderBytes(target_model::Target target)
{
	return 2 * target_model::cxx_abi(target).word_bytes;
}

lowir::Type AbiWordType(target_model::Target target)
{
	return lowir::Type::scalar(
		target_model::cxx_abi(target).word_bytes == 4 ? lowir::TypeKind::I32 : lowir::TypeKind::I64);
}

lowir::Type MemberFunctionBitsType(target_model::Target target)
{
	return lowir::Type::scalar(
		target_model::cxx_abi(target).word_bytes == 4 ? lowir::TypeKind::I64 : lowir::TypeKind::I128);
}

lowir::Type MemberFunctionStorageType(target_model::Target target)
{
	const auto abi = target_model::cxx_abi(target);
	return abi.indirect_member_function ? lowir::Type::object(2 * abi.word_bytes, abi.word_bytes == 4 ? 2 : 3)
										: MemberFunctionBitsType(target);
}

RValue FunctionLowering::member_function_bits(const RValue& value)
{
	if (!value.type.is_object())
		return value;
	Place representation = Place();
	representation.storage = value.value;
	return load(MemberFunctionBitsType(program_.target), representation);
}

RValue FunctionLowering::member_function_object(const RValue& bits)
{
	if (!target_model::cxx_abi(program_.target).indirect_member_function)
		return bits;
	const lowir::Type type = MemberFunctionStorageType(program_.target);
	Place object = Place();
	object.storage = lowir::Value::slot(generated_slot(lowir::SlotKind::Bits, type));
	store(bits.type, bits, object);
	return load(type, object);
}

} // namespace lower
