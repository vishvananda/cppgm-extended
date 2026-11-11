#include "lower/program_lowering.h"

#include "sema/analyzer.h"

namespace lower {

namespace {

using ir_model::AggregateElement;
using ir_model::AggregateShape;

AggregateShape shape(AggregateElement element, uint16_t count = 0)
{
	AggregateShape result;
	result.element = element;
	result.count = count;
	return result;
}

AggregateElement scalar_element(lowir::TypeKind kind)
{
	using lowir::TypeKind;
	switch (kind)
	{
	case TypeKind::I1:
		return AggregateElement::I1;
	case TypeKind::I8:
		return AggregateElement::I8;
	case TypeKind::U8:
		return AggregateElement::U8;
	case TypeKind::I16:
		return AggregateElement::I16;
	case TypeKind::U16:
		return AggregateElement::U16;
	case TypeKind::I32:
		return AggregateElement::I32;
	case TypeKind::U32:
		return AggregateElement::U32;
	case TypeKind::I64:
		return AggregateElement::I64;
	case TypeKind::I128:
		return AggregateElement::I128;
	case TypeKind::F32:
		return AggregateElement::F32;
	case TypeKind::F64:
		return AggregateElement::F64;
	case TypeKind::Ptr:
		return AggregateElement::Pointer;
	case TypeKind::V128:
		return AggregateElement::Vector128;
	default:
		return AggregateElement::Mixed;
	}
}

void merge(AggregateShape& result, AggregateShape next)
{
	if (next.element == AggregateElement::Empty)
		return;
	if (result.element == AggregateElement::Empty)
		result = next;
	else if (result.element != next.element || uint32_t(result.count) + next.count > UINT16_MAX)
		result = shape(AggregateElement::Mixed);
	else if (result.element != AggregateElement::Mixed)
		result.count += next.count;
}

} // namespace

ir_model::AggregateShape ProgramLowering::aggregate_shape(const sema::Type* type)
{
	type = type->unqualified;
	if (type->kind == sema::TypeKind::Array)
	{
		AggregateShape result = aggregate_shape(type->element);
		if (result.element > AggregateElement::Mixed)
		{
			if (type->bound > UINT16_MAX / result.count)
				return shape(AggregateElement::Mixed);
			result.count = static_cast<uint16_t>(result.count * type->bound);
			if (result.count == 0)
				return shape(AggregateElement::Empty);
		}
		return result;
	}
	if (type->kind == sema::TypeKind::Complex)
	{
		AggregateShape result = aggregate_shape(type->element);
		merge(result, result);
		return result;
	}
	if (type->kind != sema::TypeKind::Class)
	{
		if (SoftFloatOf(type) == SoftFloat::Quad ||
			(type->kind == sema::TypeKind::Fundamental &&
				FloatingRepresentation(type->fundamental, type->target) == FundamentalType::Float128))
			return shape(AggregateElement::F128, 1);
		const auto element = scalar_element(storage_type(type).kind());
		return shape(element, element == AggregateElement::Mixed ? 0 : 1);
	}
	const auto* cls = static_cast<const sema::ClassEntity*>(type->entity);
	const uint32_t known = aggregate_shapes_.find(cls);
	if (known != PointerMap::kMissing)
		return shape(static_cast<AggregateElement>(known >> 16), static_cast<uint16_t>(known));
	AggregateShape result = shape(cls->dynamic ? AggregateElement::Mixed : AggregateElement::Empty);
	for (const sema::BaseClass* base = cls->bases; base != nullptr; base = base->next)
		merge(result, aggregate_shape(base->base->type));
	for (const sema::DataMember* member = cls->members; member != nullptr; member = member->next)
	{
		if (member->bit_field && (member->width == 0 || member->variable->name == nullptr))
			continue;
		merge(result, aggregate_shape(member->variable->type));
	}
	aggregate_shapes_.set(cls, (static_cast<uint32_t>(result.element) << 16) | result.count);
	return result;
}

} // namespace lower
