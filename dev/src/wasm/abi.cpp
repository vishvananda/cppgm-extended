#include "wasm/abi.h"

namespace wasm {

namespace {

lowir::Type scalar_type(ir_model::AggregateElement element, lowir::Type pointer)
{
	using ir_model::AggregateElement;
	using lowir::Type;
	using lowir::TypeKind;
	switch (element)
	{
	case AggregateElement::I1:
		return Type::scalar(TypeKind::I1);
	case AggregateElement::I8:
		return Type::scalar(TypeKind::I8);
	case AggregateElement::U8:
		return Type::scalar(TypeKind::U8);
	case AggregateElement::I16:
		return Type::scalar(TypeKind::I16);
	case AggregateElement::U16:
		return Type::scalar(TypeKind::U16);
	case AggregateElement::I32:
		return Type::scalar(TypeKind::I32);
	case AggregateElement::U32:
		return Type::scalar(TypeKind::U32);
	case AggregateElement::I64:
		return Type::scalar(TypeKind::I64);
	case AggregateElement::F32:
		return Type::scalar(TypeKind::F32);
	case AggregateElement::F64:
		return Type::scalar(TypeKind::F64);
	case AggregateElement::Pointer:
		return pointer;
	case AggregateElement::Vector128:
		return Type::scalar(TypeKind::V128);
	default:
		throw WasmError("aggregate scalar requires wide-value ABI legalization");
	}
}

} // namespace

AbiValue classify_value(lowir::Type type, ir_model::AggregateShape shape, lowir::Type pointer)
{
	using ir_model::AggregateElement;
	if (!type.is_object())
		return AbiValue{ Passing::Direct, type };
	if (shape.element == AggregateElement::None)
		throw WasmError("Wasm object boundary requires an explicit aggregate shape");
	if (shape.element == AggregateElement::Empty)
		return AbiValue{ Passing::Ignore, lowir::Type::scalar(lowir::TypeKind::Void) };
	if (shape.count == 1)
	{
		if ((shape.element == AggregateElement::F128 || shape.element == AggregateElement::I128) && type.bytes == 16 &&
			lowir::data_alignment(type) <= 16)
			return AbiValue{ Passing::Expand, lowir::Type::scalar(lowir::TypeKind::I64) };
		const lowir::Type scalar = scalar_type(shape.element, pointer);
		if (lowir::storage_bytes(scalar) == type.bytes && lowir::data_alignment(scalar) >= lowir::data_alignment(type))
			return AbiValue{ Passing::Coerce, scalar };
	}
	return AbiValue{ Passing::Indirect, pointer };
}

} // namespace wasm
