#include "semantic/analysis/analyzer.h"

namespace cppgm
{
namespace semantic
{

bool Analyzer::TypeHasKnownSize(TypeId type) const
{
	const TypeRecord* record = &program_->types.Get(
		program_->types.RemoveTopCv(type));
	while (record->kind == TYPE_ARRAY)
		record = &program_->types.Get(
			program_->types.RemoveTopCv(record->child));
	if (record->kind != TYPE_NAMED) return true;
	return record->entity < program_->entities.size() &&
		program_->entities[record->entity].complete;
}

// N3485 3.9/10 disqualifies a class with a volatile non-static data member
// from being a literal type.  `_Atomic` is a separate extension and carries no
// such rule -- clang accepts a constexpr constructor on a class with an
// _Atomic member, and libc++'s atomic_flag depends on that -- but the layout
// fact below deliberately conflates the two, because both inhibit whole-object
// zero initialization.  Ask the narrower question the literal-type rule
// actually asks, walking the members rather than trusting the conflated flag.
// True when the type is one of the stand-ins a template body is analysed with
// before its arguments are known.  Nothing about such a type is decidable yet.
bool Analyzer::IsDependentParameterShapeType(TypeId type) const
{
	const TypeRecord* record = &program_->types.Get(
		program_->types.RemoveTopCv(type));
	if (record->kind == TYPE_LVALUE_REFERENCE ||
		record->kind == TYPE_RVALUE_REFERENCE)
		record = &program_->types.Get(
			program_->types.RemoveTopCv(record->child));
	return record->kind == TYPE_NAMED &&
		record->entity < program_->entities.size() &&
		program_->entities[record->entity].flavor == NAMED_TYPENAME_PARAMETER;
}

bool Analyzer::IsVolatileOnlySubobjectType(TypeId type) const
{
	const TypeRecord& record = program_->types.Get(type);
	if (record.kind == TYPE_QUALIFIED)
		return (record.cv & CV_VOLATILE) != 0 ||
			IsVolatileOnlySubobjectType(record.child);
	if (record.kind == TYPE_ARRAY)
		return IsVolatileOnlySubobjectType(record.child);
	if (record.kind != TYPE_NAMED || record.entity >= program_->entities.size())
		return false;
	const EntityRecord& entity = program_->entities[record.entity];
	if (IsEnumNamedFlavor(entity.flavor) || !entity.has_volatile_subobject)
		return false;
	for (std::size_t i = 0; i < entity.direct_base_count; ++i)
		if (IsVolatileOnlySubobjectType(program_->entities[
			program_->DirectBase(record.entity, i).entity].type)) return true;
	if (record.entity < entity_data_members_.size())
		for (std::size_t i = 0;
			i < entity_data_members_[record.entity].size(); ++i)
			if (IsVolatileOnlySubobjectType(program_->bindings[
				entity_data_members_[record.entity][i]].type)) return true;
	return false;
}

bool Analyzer::IsVolatileSubobjectType(TypeId type) const
{
	const TypeRecord& record = program_->types.Get(type);
	if (record.kind == TYPE_QUALIFIED)
		return (record.cv & (CV_VOLATILE | CV_ATOMIC)) != 0 ||
			IsVolatileSubobjectType(record.child);
	if (record.kind == TYPE_ARRAY)
		return IsVolatileSubobjectType(record.child);
	if (record.kind != TYPE_NAMED || record.entity >= program_->entities.size())
		return false;
	const EntityRecord& entity = program_->entities[record.entity];
	return !IsEnumNamedFlavor(entity.flavor) &&
		entity.has_volatile_subobject;
}

bool Analyzer::TypeContainsUnionSubobject(TypeId type) const
{
	const TypeRecord& record = program_->types.Get(type);
	if (record.kind == TYPE_QUALIFIED || record.kind == TYPE_ARRAY)
		return TypeContainsUnionSubobject(record.child);
	if (record.kind != TYPE_NAMED || record.entity >= program_->entities.size())
		return false;
	const EntityRecord& entity = program_->entities[record.entity];
	return entity.flavor == NAMED_UNION || entity.has_union_subobject;
}

void Analyzer::InitializeClassZeroSpanFacts(EntityId entity)
{
	EntityRecord& owner = program_->entities[entity];
	owner.has_volatile_subobject = false;
	owner.has_union_subobject = owner.flavor == NAMED_UNION;
	for (std::size_t base_index = 0;
		base_index < owner.direct_base_count; ++base_index)
	{
		const EntityRecord& base = program_->entities[
			program_->DirectBase(entity, base_index).entity];
		owner.has_volatile_subobject = owner.has_volatile_subobject ||
			base.has_volatile_subobject;
		owner.has_union_subobject = owner.has_union_subobject ||
			base.has_union_subobject || base.flavor == NAMED_UNION;
	}
}

void Analyzer::AccumulateClassZeroSpanFacts(
	EntityId entity, TypeId type)
{
	EntityRecord& owner = program_->entities[entity];
	owner.has_volatile_subobject = owner.has_volatile_subobject ||
		IsVolatileSubobjectType(type);
	owner.has_union_subobject = owner.has_union_subobject ||
		TypeContainsUnionSubobject(type);
}

bool Analyzer::ClassBasesAreEmpty(EntityId entity) const
{
	const EntityRecord& owner = program_->entities[entity];
	for (std::size_t base_index = 0;
		base_index < owner.direct_base_count; ++base_index)
		if (!program_->entities[
			program_->DirectBase(entity, base_index).entity].empty_class)
			return false;
	return true;
}

void Analyzer::SetBindingRequestedAlignment(
	BindingRecord& binding, std::size_t alignment)
{
	if (alignment != 0)
		program_->MutableBindingLayout(binding).requested_alignment = alignment;
}

}
}
