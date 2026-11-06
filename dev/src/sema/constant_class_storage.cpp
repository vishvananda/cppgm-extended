#include "sema/constant_interpreter.h"

#include "sema/analyzer.h"
#include "sema/constant_storage.h"

namespace sema {

ConstValue ConstantInterpreter::prefix_skeleton(const Type* type, uint32_t depth)
{
	return prefix_value(type, depth, false);
}

ConstValue ConstantInterpreter::prefix_value(const Type* type, uint32_t depth, bool zero)
{
	if (subobjects_too_deep(depth) || steps_ >= kStepBudget)
	{
		steps_ = kStepBudget;
		return ConstValue();
	}
	if (type->kind == TypeKind::Array)
	{
		++steps_;
		ConstValue value = array_prefix(type);
		if (zero)
			value.flags |= kArrayTailZero;
		return value;
	}
	const ClassEntity* cls = type->is_reference() ? nullptr : ClassOf(type);
	if (cls == nullptr)
	{
		++steps_;
		ConstValue value = ConstValue();
		value.type = type->unqualified;
		if (IsComplex(type) || LaneVector(type))
		{
			PointerMap& parts = class_storage().classes[zero ? 1 : 0];
			const uint32_t known = parts.find(type->unqualified);
			if (known != PointerMap::kMissing)
				return class_storage().values[known];
			value.kind = ValueKind::Array;
			value.count = IsComplex(type) ? 2 : static_cast<uint32_t>(type->unqualified->bound);
			value.elements = class_storage().immutable_arena.allocate_array<ConstValue>(value.count);
			const ConstValue element = prefix_value(type->unqualified->element, depth + 1, zero);
			for (uint32_t i = 0; i < value.count; ++i)
				value.elements[i] = element;
			value.flags = kSharedElements;
			parts.set(type->unqualified, static_cast<uint32_t>(class_storage().values.size()));
			class_storage().values.push_back(value);
		}
		else if (zero)
			zero_into(value);
		return value;
	}
	PointerMap& classes = class_storage().classes[zero ? 1 : 0];
	const uint32_t known = classes.find(cls);
	if (known != PointerMap::kMissing)
		return class_storage().values[known];
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
	value.elements = class_storage().immutable_arena.allocate_array<ConstValue>(value.count);
	value.flags = kSharedElements;
	bool zero_class = zero && !cls->polymorphic;
	auto zero_child = [&](const ConstValue& child) {
		if (child.kind == ValueKind::Class)
			return (child.flags & kClassZero) != 0;
		if (child.kind == ValueKind::Array && child.type->kind == TypeKind::Array)
			return scalar_array(child.type);
		return child.kind != ValueKind::Indeterminate;
	};
	uint32_t index = 0;
	for (const BaseClass* base = cls->bases; base != nullptr; base = base->next)
	{
		const ConstValue child = prefix_value(base->base->type, depth + 1, zero);
		value.elements[index++] = child;
		zero_class = zero_class && zero_child(child);
	}
	const bool is_union = cls->key == TokenKind::KW_UNION;
	const DataMember* first = zero && is_union ? first_zero_member(cls) : nullptr;
	if (first != nullptr)
		value.extra = first->ordinal + 1;
	for (const DataMember* member = cls->members; member != nullptr; member = member->next)
	{
		const ConstValue child = prefix_value(
			member->variable->type, depth + 1, zero && (!is_union || member == first));
		value.elements[index + member->ordinal] = child;
		if ((!is_union || member == first) && !(member->bit_field && member->variable->name == nullptr))
			zero_class = zero_class && zero_child(child);
	}
	if (zero_class)
		value.flags |= kClassZero;
	classes.set(cls, static_cast<uint32_t>(class_storage().values.size()));
	class_storage().values.push_back(value);
	return value;
}

void ConstantInterpreter::detach_elements(ConstValue& value)
{
	// A constructor may write an object belonging to its caller. The
	// detached payload must survive that constructor's scratch rewind.
	retain_mutable_allocation();
	value.elements = class_storage().mutable_arena.copy_array(value.elements, value.count);
	value.flags &= ~(kSharedElements | kClassZero);
}

void ConstantInterpreter::retain_mutable_allocation()
{
	ConstantStorage& storage = class_storage();
	// Keep the first block reusable even when all objects of a call end.
	if (storage.mutable_arena.bytes_reserved() == 0)
		storage.mutable_arena.allocate(1, 1);
	storage.allocations.push_back(
		ConstantStorage::Allocation{ mutable_owner_, mutable_generation_, storage.mutable_arena.mark() });
}

ConstantStorage& ConstantInterpreter::class_storage() const
{
	if (storage_ == nullptr)
		storage_.reset(new ConstantStorage);
	return *storage_;
}

void ConstantInterpreter::release_mutable(const Region& region)
{
	ConstantStorage& storage = *storage_;
	if (storage.allocations.size() == region.mutable_allocations)
		return;
	// A callee's writes to an older object survive its return. Once all
	// newly allocated paths belong to objects ending here, their first
	// allocation records exactly where reclamation starts.
	for (size_t i = region.mutable_allocations; i < storage.allocations.size(); ++i)
	{
		const ConstantStorage::Allocation& allocation = storage.allocations[i];
		if (allocation.object == PointerMap::kMissing ||
			(allocation.object < region.objects && objects_[allocation.object].generation == allocation.generation))
			return;
	}
	storage.mutable_arena.rewind(storage.allocations[region.mutable_allocations].mark);
	storage.allocations.resize(region.mutable_allocations);
}

} // namespace sema
