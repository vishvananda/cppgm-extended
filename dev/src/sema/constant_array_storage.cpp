#include "sema/constant_interpreter.h"

#include <algorithm>

#include "sema/aggregates.h"

namespace sema {

bool ConstantInterpreter::scalar_array(const Type* type)
{
	if (type->kind != TypeKind::Array)
		return false;
	while (type->kind == TypeKind::Array)
		type = type->element;
	return IsScalar(type) || IsComplex(type) || LaneVector(type);
}

ConstValue ConstantInterpreter::array_prefix(const Type* type)
{
	ConstValue value = ConstValue();
	value.kind = ValueKind::Array;
	value.type = type;
	value.flags = kArrayPrefix;
	return value;
}

bool ConstantInterpreter::zero_array_tail(const Type* type, uint32_t depth)
{
	while (type->kind == TypeKind::Array)
		type = type->element;
	if (type->is_reference() || depth > 64)
		return false;
	const ClassEntity* cls = ClassOf(type);
	if (cls == nullptr)
		return true;
	if (!IsAggregateClass(cls))
		return cls->default_constructor != nullptr && cls->default_constructor->trivial &&
			!cls->default_constructor->deleted;
	if (zero_array_classes_.find(cls) != PointerMap::kMissing)
		return true;
	for (const DataMember* member = cls->members; member != nullptr; member = member->next)
	{
		if (member->variable->kind != EntityKind::Variable || (member->bit_field && member->variable->name == nullptr))
			continue;
		const bool zero = zero_array_tail(member->variable->type, depth + 1);
		if (!zero)
			return false;
		if (cls->key == TokenKind::KW_UNION)
			break;
	}
	zero_array_classes_.set(cls, 1);
	return true;
}

const DataMember* ConstantInterpreter::first_zero_member(const ClassEntity* cls)
{
	for (const DataMember* member = cls->members; member != nullptr; member = member->next)
		if (member->variable->kind == EntityKind::Variable && (!member->bit_field || member->variable->name != nullptr))
			return member;
	return nullptr;
}

ConstValue ConstantInterpreter::prefix_skeleton(const Type* type, uint32_t depth)
{
	if (subobjects_too_deep(depth))
	{
		steps_ = kStepBudget;
		return ConstValue();
	}
	if (type->kind == TypeKind::Array)
	{
		++steps_;
		return array_prefix(type);
	}
	const ClassEntity* cls = type->is_reference() ? nullptr : ClassOf(type);
	if (cls == nullptr)
		return skeleton(type);
	ConstValue value = ConstValue();
	value.kind = ValueKind::Class;
	value.type = type->unqualified;
	value.count = cls->base_count + cls->member_count;
	if (value.count > kValueBudget || steps_ + value.count >= kStepBudget)
	{
		steps_ = kStepBudget;
		return ConstValue();
	}
	++steps_;
	value.elements = arena_.allocate_array<ConstValue>(value.count);
	uint32_t next = 0;
	for (const BaseClass* base = cls->bases; base != nullptr; base = base->next)
		value.elements[next++] = prefix_skeleton(base->base->type, depth + 1);
	for (const DataMember* member = cls->members; member != nullptr; member = member->next)
		value.elements[next++] = prefix_skeleton(member->variable->type, depth + 1);
	return value;
}

bool ConstantInterpreter::expand_array(ConstValue& array, const Type* type, uint32_t location)
{
	// Writes and representation queries may need actual storage. Keep the
	// existing evaluation budget when turning a prefix into a dense value.
	if (leaves(type, 0) > kArrayExpansionBytes / sizeof(ConstValue))
		return fail(location, "constant evaluation exceeds its storage budget");
	ConstValue dense = skeleton(type);
	if (steps_ >= kStepBudget)
		return fail(location, "constant evaluation exceeds its storage budget");
	for (uint32_t i = 0; i < dense.count; ++i)
	{
		if (i < array.count)
			copy_into(dense.elements[i], array.elements[i]);
		else if ((array.flags & kArrayTailZero) != 0)
			zero_into(dense.elements[i]);
	}
	array = dense;
	return true;
}

ConstValue* ConstantInterpreter::zero_storage(const Type* type)
{
	// A read creates only its value. Its address remains the original
	// place's root/offset, retaining distinct object and element identities.
	if (!step(0))
		return nullptr;
	ConstValue* zero = static_cast<ConstValue*>(arena_.allocate(sizeof(ConstValue), alignof(ConstValue)));
	*zero = prefix_skeleton(type, 0);
	if (steps_ >= kStepBudget)
		return nullptr;
	zero_into(*zero);
	return zero;
}

bool ConstantInterpreter::aggregate_array(
	const Type* type, const ConstPlace& target, const syntax::BracedInitList& list, uint32_t& next, uint32_t location)
{
	ConstValue* value = nullptr;
	if (!locate(target, true, value, location))
		return false;
	const bool prefix = (value->flags & kArrayPrefix) != 0;
	uint64_t count = type->bound;
	if (prefix)
	{
		const uint64_t remaining = next < list.elements.size ? list.elements.size - next : 0;
		count = std::min(count, remaining);
		const uint64_t capacity = std::min(type->bound, remaining + 1);
		if (capacity > kValueBudget || steps_ + capacity > kStepBudget)
			return fail(location, "constant evaluation exceeds its storage budget");
		value->count = 0;
		value->elements = arena_.allocate_array<ConstValue>(static_cast<uint32_t>(capacity));
	}
	const uint64_t size = size_of(type->element, location);
	auto initialize = [&](uint64_t index) {
		if (prefix && index >= value->count)
		{
			value->elements[index] = prefix_skeleton(type->element, 0);
			value->count = static_cast<uint32_t>(index + 1);
			if (steps_ >= kStepBudget)
				return fail(location, "constant evaluation exceeds its storage budget");
		}
		ConstPlace place = target;
		place.offset += static_cast<int64_t>(index * size);
		place.type = type->element;
		return aggregate_element(type->element, place, list, next, location);
	};
	uint64_t made = 0;
	while (made < count)
	{
		if (!initialize(made))
			return false;
		++made;
		if (prefix && next >= list.elements.size)
			break;
	}
	if (prefix)
	{
		if (made < type->bound && !zero_array_tail(type->element, 0))
		{
			// Try one omitted object before allocating the remaining extent.
			// A runtime default/argument fails softly in this small prefix.
			if (!initialize(made++))
				return false;
			value->count = static_cast<uint32_t>(made);
			if (!expand_array(*value, type, location))
				return false;
			while (made < type->bound)
				if (!initialize(made++))
					return false;
			return true;
		}
		value->count = static_cast<uint32_t>(made);
		value->flags |= kArrayTailZero;
	}
	return true;
}

} // namespace sema
