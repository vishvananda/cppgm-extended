#include "semantic/analysis/analyzer.h"
#include "support/exceptions.h"

#include <algorithm>
#include <limits>

namespace cppgm
{
namespace semantic
{

namespace
{

std::uint64_t AlignLayoutOffset(std::uint64_t value, std::uint64_t alignment)
{
	if (alignment == 0 || (alignment & (alignment - 1)) != 0)
		ThrowInternalCompilerError("invalid class member alignment");
	const std::uint64_t remainder = value & (alignment - 1);
	if (remainder == 0) return value;
	if (value > std::numeric_limits<std::uint64_t>::max() - alignment)
		ThrowSemanticResourceLimit("class layout is too large");
	return value + alignment - remainder;
}

}

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

// Whether the class or one of its subobjects is an empty class.
bool Analyzer::ClassHasEmptySubobject(EntityId entity) const
{
	const EntityRecord& owner = program_->entities[entity];
	if (owner.empty_class) return true;
	for (std::size_t i = 0; i < owner.direct_base_count; ++i)
		if (program_->entities[
			program_->DirectBase(entity, i).entity].has_empty_subobject)
			return true;
	if (entity >= entity_layout_members_.size()) return false;
	const std::vector<ClassLayoutMember>& members = entity_layout_members_[entity];
	for (std::size_t i = 0; i < members.size(); ++i)
	{
		std::uint64_t count = 0;
		const EntityId element = members[i].bit_field ? kNoEntity :
			LayoutElementClass(*program_, members[i].type, &count);
		if (element != kNoEntity && count != 0 &&
			program_->entities[element].has_empty_subobject) return true;
	}
	return false;
}

// The largest empty component a layout tries at offset zero: offsets below
// it are where earlier components' empty subobjects can still conflict.
std::uint64_t Analyzer::EmptyComponentRecordLimit(EntityId entity)
{
	const EntityRecord& owner = program_->entities[entity];
	std::uint64_t limit = 0;
	for (std::size_t i = 0; i < owner.direct_base_count; ++i)
	{
		const DirectBaseEdge& edge = program_->DirectBase(entity, i);
		if (!edge.virtual_base && program_->entities[edge.entity].empty_class)
			limit = std::max(limit, program_->entities[edge.entity].object_size);
	}
	std::vector<VirtualBaseLayout> layouts;
	CollectVirtualBaseLayouts(entity, &layouts);
	for (std::size_t i = 0; i < layouts.size(); ++i)
		if (program_->entities[layouts[i].entity].empty_class)
			limit = std::max(limit,
				program_->entities[layouts[i].entity].object_size);
	const std::vector<ClassLayoutMember>& members = entity_layout_members_[entity];
	for (std::size_t i = 0; i < members.size(); ++i)
	{
		if (members[i].bit_field || members[i].binding == kNoBinding ||
			!program_->bindings[members[i].binding].potentially_overlapping_member)
			continue;
		std::uint64_t count = 0;
		const EntityId element = LayoutElementClass(
			*program_, members[i].type, &count);
		if (element != kNoEntity && count == 1 &&
			program_->entities[element].layout_complete &&
			program_->entities[element].empty_class)
			limit = std::max(limit, program_->entities[element].object_size);
	}
	return limit;
}

// Itanium C++ ABI 2.2: a POD for the purpose of layout -- an aggregate
// without user-provided copy assignment or destructor whose members are
// such PODs or scalars -- keeps its tail padding.  The member conditions
// are checked as members are laid out.
bool Analyzer::ClassSpecialMembersKeepPodLayout(EntityId entity)
{
	const EntityRecord& owner = program_->entities[entity];
	if (owner.has_user_provided_constructor || owner.has_direct_base ||
		owner.dynamic_class) return false;
	if (owner.has_user_declared_destructor)
	{
		const BindingId destructor = DestructorForType(owner.type);
		if (destructor == kNoBinding) return false;
		const FunctionInfo& info = GetFunction(destructor);
		if (!info.defaulted_destructor && !info.deleted_destructor &&
			!info.deleted_function) return false;
	}
	if (entity >= class_special_members_.size()) return true;
	const ClassSpecialMemberFacts& facts = class_special_members_[entity];
	if (!facts.user_copy_assignment || facts.copy_assignment == kNoBinding)
		return true;
	const FunctionInfo& assignment = GetFunction(facts.copy_assignment);
	return assignment.deleted_special_member || assignment.deleted_function ||
		(assignment.defaulted_special_member &&
		 !assignment.user_provided_special_member);
}

bool Analyzer::MemberKeepsPodLayout(const BindingRecord& member) const
{
	if (member.access != ACCESS_PUBLIC || member.has_default_member_initializer)
		return false;
	const TypeRecord& top = program_->types.Get(member.type);
	if (top.kind == TYPE_LVALUE_REFERENCE || top.kind == TYPE_RVALUE_REFERENCE)
		return false;
	std::uint64_t count = 0;
	const EntityId element = LayoutElementClass(*program_, member.type, &count);
	return element == kNoEntity || program_->entities[element].pod_for_layout;
}

// A bit-field (2.4 II.1 with the SysV rules it defers to): at the next free
// bit, moved to the next unit of its declared type rather than straddle one
// (under #pragma pack or GNU packed it is not moved); a zero-width bit-field
// moves to its type's alignment and ends the run.  One wider than its type
// starts at the alignment of the largest integral type that fits in its
// width.  A named field aligns the class by its type, an unnamed narrow one
// does not.  Storage units are assigned when the class's size is known.
void Analyzer::LayoutClassBitField(const ClassLayoutMember& layout,
	std::uint64_t member_size, std::uint64_t type_alignment,
	std::size_t packing_alignment, bool is_union, ClassLayoutCursor* cursor)
{
	const std::uint64_t unit_bits = member_size * 8;
	const std::uint64_t width = layout.bit_width;
	const std::uint64_t pack = packing_alignment;
	if (is_union)
	{
		if (width == 0) return;
		cursor->natural_alignment = std::max(cursor->natural_alignment,
			type_alignment);
		cursor->alignment = std::max(cursor->alignment,
			pack == 0 ? type_alignment : std::min(type_alignment, pack));
		if (layout.binding != kNoBinding)
		{
			BindingLayoutFact& fact = program_->MutableBindingLayout(
				program_->bindings[layout.binding]);
			fact.member_offset = 0;
			fact.bit_offset = 0;
			fact.bit_storage_bits = static_cast<std::uint32_t>(unit_bits);
		}
		cursor->Place(std::max<std::uint64_t>(cursor->size,
			width <= unit_bits ? member_size : (width + 7) / 8), true);
		return;
	}
	std::uint64_t bit = cursor->open_bit_byte ? cursor->bit_end :
		cursor->data_size * 8;
	if (width == 0)
	{
		bit = AlignLayoutOffset(bit, 8 * type_alignment);
		cursor->Place(bit / 8, true);
		bit_field_layout_.EndRun(cursor->data_size);
		return;
	}
	bit_field_layout_.BeginRun(cursor->data_size);
	std::uint64_t alignment = layout.binding != kNoBinding ? type_alignment : 1;
	if (width > unit_bits)
	{
		std::uint64_t wide = 1;
		while (wide < 8 && wide * 16 <= width) wide *= 2;
		bit = AlignLayoutOffset(bit, 8 * (pack == 0 ? wide : std::min(wide, pack)));
		alignment = std::max(alignment, wide);
	}
	else if (pack == 0 && bit / unit_bits != (bit + width - 1) / unit_bits)
		bit = AlignLayoutOffset(bit, unit_bits);
	if (bit > std::numeric_limits<std::uint64_t>::max() - width)
		ThrowSemanticResourceLimit("class layout is too large");
	bit_field_layout_.Add(layout.binding, bit, std::min(width, unit_bits),
		member_size, pack != 0);
	cursor->Place((bit + width + 7) / 8, true);
	cursor->bit_end = bit + width;
	cursor->open_bit_byte = cursor->bit_end % 8 != 0;
	cursor->natural_alignment = std::max(cursor->natural_alignment, alignment);
	cursor->alignment = std::max(cursor->alignment,
		pack == 0 ? alignment : std::min(alignment, pack));
}

// A non-static data member at the data size rounded up to its alignment,
// moved on by its alignment while one of its empty subobjects would share
// its type and offset with one placed before (2.4 II.2).  A
// potentially-overlapping member is placed as a base is: an empty one at
// offset zero first, a non-empty one claiming only its data size.
std::uint64_t Analyzer::LayoutClassDataMember(const ClassLayoutMember& layout,
	const BindingRecord& member, std::uint64_t member_size,
	std::uint64_t member_alignment, bool is_union, ClassLayoutCursor* cursor,
	bool* empty_class)
{
	if (is_union)
	{
		*empty_class = false;
		cursor->Place(std::max(cursor->size, member_size), true);
		return 0;
	}
	std::uint64_t count = 0;
	const EntityId element = LayoutElementClass(*program_, layout.type, &count);
	const bool overlapping = member.potentially_overlapping_member &&
		element != kNoEntity && count == 1;
	const bool empty = overlapping && program_->entities[element].empty_class;
	std::uint64_t offset = 0;
	if (!empty || empty_subobjects_.Conflict(element, 0, true))
	{
		offset = AlignLayoutOffset(cursor->data_size, member_alignment);
		while (empty_subobjects_.MemberConflict(layout.type, offset))
			offset += member_alignment;
	}
	if (offset > std::numeric_limits<std::uint64_t>::max() - member_size)
		ThrowSemanticResourceLimit("class layout is too large");
	if (empty)
	{
		empty_subobjects_.Record(element, offset, true, true);
		cursor->Place(offset + member_size, false);
		return offset;
	}
	*empty_class = false;
	bit_field_layout_.EndRun(offset);
	empty_subobjects_.RecordMember(layout.type, offset);
	cursor->Place(offset + member_size, true);
	if (overlapping)
		cursor->data_size = offset + LayoutDataSize(program_->entities[element]);
	return offset;
}

void Analyzer::AccumulateImplicitDefaultConstructorMember(EntityId entity,
	const BindingRecord& member)
{
	EntityRecord& owner = program_->entities[entity];
	if (member.has_default_member_initializer)
	{
		owner.trivial_default_constructor = false;
		return;
	}
	TypeId member_type = member.type;
	const TypeRecord* member_record = &program_->types.Get(member_type);
	while (member_record->kind == TYPE_ARRAY)
	{
		member_type = member_record->child;
		member_record = &program_->types.Get(member_type);
	}
	if (member_record->kind == TYPE_LVALUE_REFERENCE ||
		member_record->kind == TYPE_RVALUE_REFERENCE)
	{
		owner.default_constructible = false;
		return;
	}
	const bool const_member = member_record->kind == TYPE_QUALIFIED &&
		(member_record->cv & CV_CONST) != 0;
	if (const_member)
	{
		member_type = member_record->child;
		member_record = &program_->types.Get(member_type);
	}
	if (member_record->kind != TYPE_NAMED)
	{
		if (const_member) owner.default_constructible = false;
		return;
	}
	const EntityRecord& subobject = program_->entities[member_record->entity];
	if (subobject.flavor == NAMED_ENUM || subobject.flavor == NAMED_ENUM_CLASS)
	{
		if (const_member) owner.default_constructible = false;
		return;
	}
	if (!subobject.default_constructible)
		owner.default_constructible = false;
	if (!subobject.trivial_default_constructor)
		owner.trivial_default_constructor = false;
}

void Analyzer::CompleteClassLayout(EntityId entity)
{
	if (program_->entities[entity].layout_complete) return;
	++class_layouts_;
	for (std::size_t i = 0; i < entity_layout_members_[entity].size(); ++i)
		EnsureClassDefinition(entity_layout_members_[entity][i].type);
	EntityRecord& owner = program_->entities[entity];
	InitializeClassZeroSpanFacts(entity);
	const std::size_t packing_alignment =
		static_cast<std::size_t>(owner.packing_alignment);
	const bool is_union = owner.flavor == NAMED_UNION;
	empty_subobjects_.Begin(program_, &entity_layout_members_,
		is_union ? 0 : EmptyComponentRecordLimit(entity));
	bit_field_layout_.Clear();
	ClassLayoutCursor cursor;
	(void)InitializeClassBaseLayout(entity, packing_alignment, &cursor);
	bool empty_class = ClassBasesAreEmpty(entity) && !owner.dynamic_class;
	const bool defaulted_destructor = ClassDestructorIsDefaulted(owner);
	const bool implicit_default_constructor =
		!owner.has_user_declared_constructor;
	owner.is_aggregate = !owner.has_user_provided_constructor &&
		!owner.has_direct_base && !owner.polymorphic_class;
	bool pod_for_layout = ClassSpecialMembersKeepPodLayout(entity);
	if (implicit_default_constructor)
		InitializeImplicitBaseConstructorFacts(entity);
	for (std::size_t i = 0;
		i < entity_layout_members_[entity].size(); ++i)
	{
		++class_layout_member_visits_;
		const ClassLayoutMember layout = entity_layout_members_[entity][i];
		EntityRecord& current_owner = program_->entities[entity];
		AccumulateClassZeroSpanFacts(entity, layout.type);
		BindingRecord* member = layout.binding == kNoBinding ? 0 :
			&program_->bindings[layout.binding];
		if (is_union && member && member->has_default_member_initializer)
		{
			if (current_owner.union_default_member != kNoBinding &&
				current_owner.union_default_member != layout.binding)
				ThrowSemanticError(
					"union has multiple default member initializers");
			current_owner.union_default_member = layout.binding;
		}
		if (member && (member->has_default_member_initializer ||
			member->access != ACCESS_PUBLIC))
			current_owner.is_aggregate = false;
		if (member && !MemberKeepsPodLayout(*member)) pod_for_layout = false;
		// A shape-only completion lays out nothing: its members' types are the
		// template's own parameters standing in, and a stand-in has no size.
		// Asking for one turns a specialization that only had to exist into an
		// incomplete-type error.  Every concrete specialization is completed
		// separately and is laid out for real.
		if (dependent_shape_completion_depth_ != 0 &&
			!TypeHasKnownSize(layout.type))
			continue;
		const std::uint64_t member_size = program_->SizeOf(layout.type);
		const std::uint64_t alias_alignment = member ? DeclaredTypeAlignment(
			layout.type, BindingDeclaredTypeLayout(layout.binding)) : 0;
		const std::uint64_t type_alignment = alias_alignment != 0 ?
			alias_alignment : program_->AlignOf(layout.type);
		if (layout.bit_field)
		{
			if (layout.bit_width != 0) empty_class = false;
			LayoutClassBitField(layout, member_size, type_alignment,
				packing_alignment, is_union, &cursor);
			continue;
		}
		if (!member)
			ThrowInternalCompilerError("ordinary layout member has no binding");
		BindingLayoutFact& member_layout = program_->MutableBindingLayout(*member);
		const std::uint64_t requested_member_alignment =
			member_layout.requested_alignment;
		std::uint64_t member_alignment = packing_alignment == 0 ? type_alignment :
			std::min<std::uint64_t>(type_alignment, packing_alignment);
		member_alignment = std::max(member_alignment, requested_member_alignment);
		cursor.natural_alignment = std::max(cursor.natural_alignment,
			std::max(type_alignment, requested_member_alignment));
		cursor.alignment = std::max(cursor.alignment, member_alignment);
		const std::uint64_t offset = LayoutClassDataMember(layout, *member,
			member_size, member_alignment, is_union, &cursor, &empty_class);
		program_->MutableBindingLayout(program_->bindings[layout.binding]).
			member_offset = offset;
		if (implicit_default_constructor)
			AccumulateImplicitDefaultConstructorMember(entity,
				program_->bindings[layout.binding]);
	}
	program_->entities[entity].pod_for_layout = pod_for_layout &&
		program_->entities[entity].is_aggregate;
	CompleteClassMemberDestructionFacts(entity, is_union,
		defaulted_destructor);
	FinalizeClassVirtualBaseLayout(entity, packing_alignment, &cursor,
		&empty_class);
}

}
}
