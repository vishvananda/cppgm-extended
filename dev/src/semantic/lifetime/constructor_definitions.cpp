#include "semantic/analysis/analyzer.h"
#include "support/exceptions.h"
#include "support/scoped_state.h"

#include <algorithm>

namespace cppgm
{
namespace semantic
{

void Analyzer::ValidateConstexprConstructorDefinition(
	const FunctionInfo& constructor)
{
	if (!constructor.constructor || !constructor.constexpr_function ||
		constructor.definition_body == kNoNode ||
		constructor.defaulted_constructor || constructor.implicit_constructor)
		return;
	const EntityId entity =
		program_->bindings[constructor.binding].member_owner;
	if (entity == kNoEntity || entity >= entity_data_members_.size())
		ThrowInternalCompilerError(
			"constexpr constructor is missing its member index");
	if (IsClassTemplateSpecializationContext(entity)) return;
	const std::vector<BindingId>& members = entity_data_members_[entity];
	std::vector<std::uint8_t> initialized(members.size(), 0);
	for (std::size_t i = 0; i < members.size(); ++i)
		if (program_->bindings[members[i]].has_default_member_initializer)
			initialized[i] = 1;
	if (constructor.constructor_initializer != kNoNode)
		for (std::uint32_t edge = arena_->FirstEdge(
			constructor.constructor_initializer); edge != kNoEdge;
			edge = arena_->NextEdge(edge))
		{
			const NodeId syntax = arena_->EdgeChild(edge);
			if (!arena_->IsTag(syntax, ::cppgm::syntax::STAG_MEM_INITIALIZER)) continue;
			const NodeId id = FindChild(syntax, ::cppgm::syntax::STAG_MEM_INITIALIZER_ID);
			if (id == kNoNode) continue;
			if (arena_->Payload(id) == program_->names.Get(
				program_->entities[entity].identity_name)) return;
			const LookupResult found = program_->LookupDirect(
				program_->entities[entity].member_scope,
				program_->names.UseInterned(arena_->PayloadId(id)), LOOKUP_ORDINARY);
			if (found.ordinary == kNoBinding ||
				!program_->bindings[found.ordinary].non_static_data_member ||
				program_->bindings[found.ordinary].member_owner != entity)
				continue;
			const std::size_t ordinal =
				program_->BindingLayout(
					program_->bindings[found.ordinary]).member_ordinal;
			if (ordinal < members.size() && members[ordinal] == found.ordinary)
				initialized[ordinal] = 1;
		}
	for (std::size_t i = 0; i < members.size(); ++i)
	{
		if (initialized[i]) continue;
		TypeId type = program_->bindings[members[i]].type;
		TypeRecord record = program_->types.Get(type);
		while (record.kind == TYPE_ARRAY || record.kind == TYPE_QUALIFIED)
		{
			type = record.child;
			record = program_->types.Get(type);
		}
		const NamedFlavor flavor = record.kind == TYPE_NAMED ?
			program_->entities[record.entity].flavor : NAMED_NONE;
		if (record.kind != TYPE_NAMED || !IsClassNamedFlavor(flavor))
			ThrowSemanticError(
				"constexpr constructor leaves a scalar member uninitialized");
	}
}

void Analyzer::CompleteOutOfClassDefaultedConstructor(EntityId entity,
	BindingId constructor)
{
	FunctionInfo& info = GetMutableFunction(constructor);
	info.user_provided_special_member = true;
	const std::size_t required_parameters =
		info.special_member == SPECIAL_MEMBER_NONE ? 0 : 1;
	if (info.parameters.size() != required_parameters)
		ThrowSemanticError(
			"explicitly defaulted constructor has the wrong type");
	for (std::size_t i = 0; i < info.parameters.size(); ++i)
		if (info.parameters[i].default_argument != kNoNode)
			ThrowSemanticError(
				"explicitly defaulted function has a default argument");
	if (info.special_member != SPECIAL_MEMBER_NONE)
	{
		bool deleted = false;
		bool trivial = false;
		bool nonthrowing = false;
		EvaluateSynthesizedConstructor(entity, info.special_member,
			&deleted, &trivial, &nonthrowing);
		info.deleted_constructor = deleted;
		info.deleted_special_member = deleted;
		const bool retained_specialization =
			IsClassTemplateSpecializationContext(entity);
		info.trivial_special_member = retained_specialization && trivial;
		info.synthesized_storage_copy = !retained_specialization && trivial;
		program_->bindings[constructor].nonthrowing = nonthrowing;
	}
	else CompleteDefaultedDefaultConstructor(entity, constructor);
	if (info.deleted_constructor)
		ThrowSemanticError(
			"out-of-class defaulted constructor is deleted");
}

void Analyzer::CompleteDefaultedDefaultConstructor(EntityId entity,
	BindingId constructor)
{
	bool deleted = false;
	bool trivial = true;
	const EntityRecord& owner = program_->entities[entity];
	for (std::size_t base_ordinal = 0;
		base_ordinal < owner.direct_base_count; ++base_ordinal)
	{
		const EntityRecord& base = program_->entities[
			program_->DirectBase(entity, base_ordinal).entity];
		deleted = deleted || !base.default_constructible;
		trivial = trivial && base.trivial_default_constructor;
	}
	if (entity < entity_data_members_.size())
		for (std::size_t i = 0; i < entity_data_members_[entity].size(); ++i)
		{
			const BindingRecord& member =
				program_->bindings[entity_data_members_[entity][i]];
			if (member.has_default_member_initializer)
			{
				trivial = false;
				continue;
			}
			TypeId member_type = member.type;
			const TypeRecord* record = &program_->types.Get(member_type);
			bool const_member = false;
			while (record->kind == TYPE_ARRAY ||
				record->kind == TYPE_QUALIFIED)
			{
				if (record->kind == TYPE_QUALIFIED &&
					(record->cv & CV_CONST) != 0)
					const_member = true;
				member_type = record->child;
				record = &program_->types.Get(member_type);
			}
			if (record->kind == TYPE_LVALUE_REFERENCE ||
				record->kind == TYPE_RVALUE_REFERENCE)
			{
				deleted = true;
				trivial = false;
			}
			else if (record->kind == TYPE_NAMED)
			{
				const EntityRecord& subobject =
					program_->entities[record->entity];
				if (subobject.flavor == NAMED_STRUCT ||
					subobject.flavor == NAMED_CLASS ||
					subobject.flavor == NAMED_UNION)
				{
					deleted = deleted || !subobject.default_constructible;
					trivial = trivial &&
						subobject.trivial_default_constructor;
				}
				else if (const_member) deleted = true;
			}
			else if (const_member) deleted = true;
		}
	FunctionInfo& info = GetMutableFunction(constructor);
	info.deleted_constructor = deleted;
	info.deleted_special_member = deleted;
	program_->entities[entity].default_constructible = !deleted;
	program_->entities[entity].trivial_default_constructor =
		!deleted && trivial;
	program_->bindings[constructor].nonthrowing = !deleted && trivial;
}

bool Analyzer::InheritedConstructorSubobjectsAreConstructible(
	BindingId constructor)
{
	const BindingId source = GetFunction(constructor).inherited_constructor_source;
	if (GetFunction(source).inherited_constructor_source != kNoBinding &&
		!InheritedConstructorSubobjectsAreConstructible(source)) return false;
	const EntityId entity = program_->bindings[constructor].member_owner;
	const EntityId inherited_base = program_->bindings[source].member_owner;
	ScopedValueRestore<EntityId> class_context(&current_class_context_, entity);
	const std::vector<NodeId> syntax;
	const std::vector<ExpressionInfo> arguments;
	const auto default_constructible = [this, entity, &syntax, &arguments](TypeId type)
	{
		bool is_const = false;
		TypeRecord shape = program_->types.Get(type);
		while (shape.kind == TYPE_ARRAY || shape.kind == TYPE_QUALIFIED)
		{
			if (shape.kind == TYPE_QUALIFIED && (shape.cv & CV_CONST) != 0)
				is_const = true;
			type = shape.child;
			shape = program_->types.Get(type);
		}
		if (shape.kind == TYPE_LVALUE_REFERENCE ||
			shape.kind == TYPE_RVALUE_REFERENCE) return false;
		if (shape.kind != TYPE_NAMED ||
			!IsClassNamedFlavor(program_->entities[shape.entity].flavor))
			return !is_const;
		const BindingId selected = SelectConstructor(program_->entities[entity].member_scope,
			syntax, arguments, ConstructorCandidates(shape.entity),
			false, false, 0, true, kNoNode, type);
		return selected != kNoBinding &&
			(GetFunction(selected).inherited_constructor_source == kNoBinding ||
			 InheritedConstructorSubobjectsAreConstructible(selected));
	};
	const std::size_t base_count = program_->entities[entity].direct_base_count;
	for (std::size_t i = 0; i < base_count; ++i)
	{
		const EntityId base = program_->DirectBase(entity, i).entity;
		if (base != inherited_base &&
			!default_constructible(program_->entities[base].type)) return false;
	}
	if (entity < entity_data_members_.size())
	{
		const std::vector<BindingId> members = entity_data_members_[entity];
		for (std::size_t i = 0; i < members.size(); ++i)
		{
			const BindingRecord member = program_->bindings[members[i]];
			if (!member.has_default_member_initializer &&
				!default_constructible(member.type)) return false;
		}
	}
	return true;
}

bool Analyzer::EvaluateDestructorSubobjects(EntityId entity,
	bool defaulted_destructor, bool* deleted,
	std::vector<TypeId>* allowed, bool* unrestricted)
{
	if (deleted) *deleted = false;
	if (allowed) allowed->clear();
	if (unrestricted) *unrestricted = false;
	bool nonthrowing = true;
	const EntityRecord& owner = program_->entities[entity];
	const auto visit = [this, entity, deleted, allowed, unrestricted, &nonthrowing](
		TypeId type, bool variant)
	{
		const EntityId subobject = DestructedEntity(type);
		if (subobject == kNoEntity) return;
		const BindingId selected = DestructorForType(type);
		if (deleted && (selected == kNoBinding ||
			!program_->entities[subobject].destructible ||
			GetFunction(selected).deleted_destructor ||
			!CanAccessMember(selected, subobject, entity)))
			*deleted = true;
		if (selected == kNoBinding ||
			!FunctionIsNonthrowing(selected))
			nonthrowing = false;
		if (allowed && unrestricted)
		{
			if (selected == kNoBinding ||
				program_->bindings[selected].exception_boundary ==
					FUNCTION_EXCEPTION_BOUNDARY_NONE)
				*unrestricted = true;
			else if (!*unrestricted)
			{
				const BindingRecord& binding = program_->bindings[selected];
				allowed->insert(allowed->end(),
					program_->function_exception_types.begin() + binding.exception_type_begin,
					program_->function_exception_types.begin() + binding.exception_type_begin +
						binding.exception_type_count);
			}
		}
		if (deleted && variant &&
			!program_->entities[subobject].trivial_destructor)
			*deleted = true;
	};
	for (std::size_t base_ordinal = 0;
		base_ordinal < owner.direct_base_count; ++base_ordinal)
	{
		const DirectBaseEdge& edge = program_->DirectBase(
			entity, base_ordinal);
		if (!edge.virtual_base)
			visit(program_->entities[edge.entity].type, false);
	}
	for (std::size_t virtual_ordinal = 0;
		virtual_ordinal < owner.virtual_base_count; ++virtual_ordinal)
		visit(program_->entities[program_->VirtualBase(
			entity, virtual_ordinal).entity].type, false);
	if (entity < entity_data_members_.size())
		for (std::size_t i = 0; i < entity_data_members_[entity].size(); ++i)
		{
			const BindingRecord& member = program_->bindings[
				entity_data_members_[entity][i]];
			if (member.anonymous_union_storage && !defaulted_destructor)
				continue;
			visit(member.type, owner.flavor == NAMED_UNION);
		}
	if (allowed && unrestricted)
	{
		if (*unrestricted) allowed->clear();
		else
		{
			std::sort(allowed->begin(), allowed->end());
			allowed->erase(std::unique(allowed->begin(), allowed->end()), allowed->end());
		}
	}
	return nonthrowing;
}

void Analyzer::CompleteDefaultedDestructor(EntityId entity,
	BindingId destructor, std::vector<TypeId>* allowed, bool* unrestricted)
{
	bool deleted = false;
	const bool nonthrowing = EvaluateDestructorSubobjects(
		entity, true, &deleted, allowed, unrestricted);
	FunctionInfo& info = GetMutableFunction(destructor);
	info.deleted_destructor = deleted;
	program_->entities[entity].destructible = !deleted;
	program_->entities[entity].trivial_destructor = false;
	program_->bindings[destructor].nonthrowing = !deleted && nonthrowing;
}

}
}
