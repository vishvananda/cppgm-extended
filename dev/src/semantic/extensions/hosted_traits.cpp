#include "semantic/analysis/analyzer.h"
#include "support/exceptions.h"

#include <string>
#include <vector>

namespace cppgm
{
namespace semantic
{
namespace
{

bool IsClassEntity(const EntityRecord& entity)
{
	return IsClassNamedFlavor(entity.flavor);
}

BindingId SelectedConversionFunction(const CallConversionFact& conversion)
{
	if (conversion.conversion_function != kNoBinding)
		return conversion.conversion_function;
	if (conversion.constructor != kNoBinding) return conversion.constructor;
	return conversion.constructor_argument_conversion_function;
}

}

ExpressionInfo Analyzer::MakeBuiltinTraitOperand(TypeId type) const
{
	ExpressionInfo result;
	const TypeRecord top = program_->types.Get(type);
	result.type = EffectiveType(type);
	if (program_->types.Get(result.type).kind == TYPE_FUNCTION)
		result.category = VALUE_LVALUE;
	else if (top.kind == TYPE_LVALUE_REFERENCE)
		result.category = VALUE_LVALUE;
	else result.category = VALUE_XVALUE;
	return result;
}

bool Analyzer::BuiltinConversionIsUsable(
	const CallConversionFact& conversion) const
{
	if (conversion.rank == CONVERSION_INVALID) return false;
	const BindingId bindings[] = {
		conversion.conversion_function,
		conversion.constructor,
		conversion.constructor_argument_conversion_function
	};
	for (std::size_t i = 0; i < sizeof(bindings) / sizeof(bindings[0]); ++i)
	{
		if (bindings[i] == kNoBinding) continue;
		const FunctionInfo& function = GetFunction(bindings[i]);
		if (function.deleted_function || function.deleted_constructor ||
			function.deleted_special_member || !CanAccessMember(bindings[i]))
			return false;
	}
	return true;
}

bool Analyzer::BuiltinConversionIsNonthrowing(
	const CallConversionFact& conversion)
{
	if (!BuiltinConversionIsUsable(conversion)) return false;
	const BindingId bindings[] = {
		conversion.conversion_function,
		conversion.constructor,
		conversion.constructor_argument_conversion_function
	};
	for (std::size_t i = 0; i < sizeof(bindings) / sizeof(bindings[0]); ++i)
		if (bindings[i] != kNoBinding &&
			!FunctionIsNonthrowing(bindings[i])) return false;
	return true;
}

bool Analyzer::EvaluateBuiltinConstructibility(
	const std::vector<TypeId>& operands, BindingId* selected,
	std::vector<CallConversionFact>* argument_conversions)
{
	*selected = kNoBinding;
	argument_conversions->clear();
	if (operands.empty()) return false;
	TypeId target = operands[0];
	const TypeRecord target_top = program_->types.Get(target);
	if (target_top.kind == TYPE_LVALUE_REFERENCE ||
		target_top.kind == TYPE_RVALUE_REFERENCE)
	{
		if (operands.size() != 2) return false;
		const ExpressionInfo source = MakeBuiltinTraitOperand(operands[1]);
		CallConversionFact conversion = CallConversion(source, target, 0, 0);
		if (!BuiltinConversionIsUsable(conversion))
			conversion = ConvertingFunction(source, target, true);
		if (!BuiltinConversionIsUsable(conversion)) return false;
		argument_conversions->push_back(conversion);
		*selected = SelectedConversionFunction(conversion);
		return true;
	}

	target = program_->types.RemoveTopCv(target);
	const TypeRecord shape = program_->types.Get(target);
	if (shape.kind == TYPE_ARRAY)
	{
		if (shape.bound == 0 || operands.size() != 1) return false;
		std::vector<TypeId> element(1, shape.child);
		return EvaluateBuiltinConstructibility(
			element, selected, argument_conversions);
	}
	if (shape.kind == TYPE_FUNCTION || IsVoid(target)) return false;

	const EntityId entity = EntityOf(target);
	if (entity != kNoEntity && IsClassEntity(program_->entities[entity]))
	{
		EnsureClassDefinition(target);
		if (!program_->entities[entity].complete ||
			program_->entities[entity].abstract_class) return false;
		std::vector<ExpressionInfo> arguments;
		arguments.reserve(operands.size() - 1);
		for (std::size_t i = 1; i < operands.size(); ++i)
			arguments.push_back(MakeBuiltinTraitOperand(operands[i]));
		const std::vector<NodeId> syntax(arguments.size(), kNoNode);
		*selected = SelectConstructor(kNoScope, syntax, arguments,
			ConstructorCandidates(entity), false, false,
			argument_conversions, true, kNoNode, target);
		if (*selected != kNoBinding) return true;
		if (arguments.size() != 1) return false;
		const CallConversionFact conversion =
			ConvertingFunction(arguments[0], target, true);
		if (!BuiltinConversionIsUsable(conversion)) return false;
		argument_conversions->assign(1, conversion);
		*selected = conversion.conversion_function;
		return true;
	}

	if (operands.size() == 1) return true;
	if (operands.size() != 2) return false;
	const ExpressionInfo source = MakeBuiltinTraitOperand(operands[1]);
	CallConversionFact conversion = CallConversion(source, target, 0, 0);
	if (!BuiltinConversionIsUsable(conversion))
		conversion = ConvertingFunction(source, target, true);
	if (!BuiltinConversionIsUsable(conversion)) return false;
	argument_conversions->push_back(conversion);
	*selected = SelectedConversionFunction(conversion);
	return true;
}

bool Analyzer::EvaluateBuiltinConvertibility(
	TypeId source_type, TypeId target)
{
	const bool source_void = IsVoid(source_type);
	const bool target_void = IsVoid(target);
	if (source_void || target_void) return source_void && target_void;
	const TypeId source_object = program_->types.RemoveTopCv(
		EffectiveType(source_type));
	const TypeId target_object_type = program_->types.RemoveTopCv(
		EffectiveType(target));
	const TypeRecord source_shape = program_->types.Get(source_object);
	const TypeRecord target_shape = program_->types.Get(target_object_type);
	if (source_shape.kind == TYPE_POINTER)
		EnsureClassDefinition(source_shape.child);
	if (target_shape.kind == TYPE_POINTER)
		EnsureClassDefinition(target_shape.child);
	ExpressionInfo source = MakeBuiltinTraitOperand(source_type);
	const TypeRecord target_top = program_->types.Get(target);
	if (program_->types.Get(source.type).kind == TYPE_FUNCTION &&
		target_top.kind == TYPE_RVALUE_REFERENCE)
		source.category = VALUE_XVALUE;
	const CallConversionFact conversion = CallConversion(source, target, 0, 0);
	if (!BuiltinConversionIsUsable(conversion)) return false;
	if (conversion.conversion_function == kNoBinding) return true;
	const TypeId converted =
		GetFunction(conversion.conversion_function).conversion_target;
	const TypeRecord converted_top = program_->types.Get(converted);
	const TypeId target_object = program_->types.RemoveTopCv(
		EffectiveType(target));
	if ((converted_top.kind != TYPE_LVALUE_REFERENCE &&
		 converted_top.kind != TYPE_RVALUE_REFERENCE) ||
		program_->types.RemoveTopCv(EffectiveType(converted)) != target_object ||
		EntityOf(target_object) == kNoEntity) return true;
	std::vector<TypeId> construction;
	construction.push_back(target);
	construction.push_back(converted);
	BindingId selected = kNoBinding;
	std::vector<CallConversionFact> argument_conversions;
	return EvaluateBuiltinConstructibility(
		construction, &selected, &argument_conversions);
}

bool Analyzer::BuiltinDefaultConstructionIsNonthrowing(EntityId entity)
{
	const std::vector<BindingId>& candidates = ConstructorCandidates(entity);
	BindingId selected = kNoBinding;
	for (std::size_t i = 0; i < candidates.size(); ++i)
	{
		const FunctionInfo& function = GetFunction(candidates[i]);
		if (!function.constructor || function.deleted_function ||
			function.deleted_constructor || function.deleted_special_member)
			continue;
		std::size_t required = function.parameters.size();
		while (required != 0 &&
			function.parameters[required - 1].default_argument != kNoNode)
			--required;
		if (required != 0) continue;
		if (selected != kNoBinding) return false;
		selected = candidates[i];
	}
	if (selected == kNoBinding) return false;
	const FunctionInfo& constructor = GetFunction(selected);
	if (!constructor.implicit_constructor && !constructor.defaulted_constructor)
		return FunctionIsNonthrowing(selected);
	const EntityRecord& owner = program_->entities[entity];
	for (std::size_t i = 0; i < owner.direct_base_count; ++i)
		if (!BuiltinDefaultConstructionIsNonthrowing(
			program_->DirectBase(entity, i).entity)) return false;
	if (entity >= entity_data_members_.size()) return true;
	for (std::size_t i = 0; i < entity_data_members_[entity].size(); ++i)
	{
		TypeId member =
			program_->bindings[entity_data_members_[entity][i]].type;
		TypeRecord shape = program_->types.Get(member);
		while (shape.kind == TYPE_ARRAY || shape.kind == TYPE_QUALIFIED)
		{
			member = shape.child;
			shape = program_->types.Get(member);
		}
		if (shape.kind == TYPE_LVALUE_REFERENCE ||
			shape.kind == TYPE_RVALUE_REFERENCE) return false;
		if (shape.kind == TYPE_NAMED &&
			IsClassEntity(program_->entities[shape.entity]) &&
			!BuiltinDefaultConstructionIsNonthrowing(shape.entity)) return false;
	}
	return true;
}

bool Analyzer::BuiltinConstructionIsNonthrowing(TypeId target,
	BindingId selected,
	const std::vector<CallConversionFact>& argument_conversions)
{
	for (std::size_t i = 0; i < argument_conversions.size(); ++i)
		if (!BuiltinConversionIsNonthrowing(argument_conversions[i])) return false;
	if (selected == kNoBinding) return true;
	const FunctionInfo& function = GetFunction(selected);
	if (!function.constructor || !function.parameters.empty())
		return FunctionIsNonthrowing(selected);
	while (program_->types.Get(target).kind == TYPE_ARRAY)
		target = program_->types.Get(target).child;
	const EntityId entity = EntityOf(target);
	return entity != kNoEntity &&
		BuiltinDefaultConstructionIsNonthrowing(entity);
}

bool Analyzer::BuiltinConstructionIsTrivial(TypeId target,
	BindingId selected,
	const std::vector<CallConversionFact>& argument_conversions) const
{
	for (std::size_t i = 0; i < argument_conversions.size(); ++i)
		if (argument_conversions[i].rank == CONVERSION_USER_DEFINED)
			return false;
	if (selected == kNoBinding) return true;
	const FunctionInfo& function = GetFunction(selected);
	if (!function.constructor) return false;
	while (program_->types.Get(target).kind == TYPE_ARRAY)
		target = program_->types.Get(target).child;
	const EntityId entity = EntityOf(target);
	if (entity == kNoEntity || !program_->entities[entity].trivial_destructor)
		return false;
	if (function.special_member != SPECIAL_MEMBER_NONE)
		return function.trivial_special_member;
	return (function.implicit_constructor || function.defaulted_constructor) &&
		program_->entities[entity].trivial_default_constructor;
}

bool Analyzer::EvaluateBuiltinAssignability(TypeId target,
	TypeId source_type, ScopeId scope, BindingId* selected,
	std::vector<CallConversionFact>* argument_conversions)
{
	*selected = kNoBinding;
	argument_conversions->clear();
	ExpressionInfo left = MakeBuiltinTraitOperand(target);
	ExpressionInfo right = MakeBuiltinTraitOperand(source_type);
	const EntityId entity = EntityOf(left.type);
	if (entity != kNoEntity && IsClassEntity(program_->entities[entity]))
	{
		EnsureClassDefinition(left.type);
		const NameId name = program_->names.Intern("operator=");
		BeginCandidateCollection();
		std::vector<BindingId> candidates;
		EntityId naming_class = kNoEntity;
		const LookupResult member = program_->LookupMember(
			entity, name, LOOKUP_ORDINARY);
		if (member.ordinary != kNoBinding &&
			program_->bindings[member.ordinary].kind == BIND_FUNCTION)
		{
			naming_class = member.naming_class;
			const std::vector<BindingId> functions =
				FunctionSet(member.ordinary);
			for (std::size_t i = 0; i < functions.size(); ++i)
				if (GetFunction(functions[i]).member_owner != kNoType)
					AddCandidate(functions[i], &candidates);
		}
		const LookupResult templates = program_->LookupMember(
			entity, name, LOOKUP_FUNCTION_TEMPLATE);
		std::vector<std::size_t> patterns;
		for (std::size_t owner = 0;
			owner < templates.FunctionTemplateOwnerCount(); ++owner)
		{
			const ScopeId template_owner =
				templates.FunctionTemplateOwnerAt(owner);
			const std::uint64_t key =
				(static_cast<std::uint64_t>(template_owner) << 32) | name;
			const CompactIndexSequence* indexed =
				template_function_sets_.Find(key);
			if (!indexed) continue;
			for (std::size_t i = 0; i < indexed->Size(); ++i)
				patterns.push_back((*indexed)[i]);
		}
		if (!patterns.empty())
		{
			associated_declaration_visits_ += patterns.size();
			const std::vector<ExpressionInfo> arguments(1, right);
			std::vector<BindingId> specializations;
			DeduceFunctionTemplatePatterns(
				patterns, arguments, &specializations);
			for (std::size_t i = 0; i < specializations.size(); ++i)
				if (GetFunction(specializations[i]).member_owner != kNoType)
					AddCandidate(specializations[i], &candidates);
			if (naming_class == kNoEntity)
				naming_class = templates.naming_class;
		}
		if (candidates.empty()) return false;
		ExpressionInfo object;
		object.type = program_->types.Pointer(EffectiveType(left.type));
		object.category = left.category;
		std::vector<NodeId> syntax(2, kNoNode);
		std::vector<ExpressionInfo> operands;
		operands.push_back(left);
		operands.push_back(right);
		bool selected_member = false;
		ObjectConversionFact object_conversion;
		*selected = SelectOperatorOverload(scope, syntax, operands,
			candidates, object, &selected_member, &object_conversion,
			argument_conversions, true);
		if (*selected == kNoBinding || !selected_member) return false;
		const FunctionInfo& function = GetFunction(*selected);
		if (function.deleted_function || function.deleted_special_member ||
			!CanAccessMember(*selected, naming_class, entity)) return false;
		for (std::size_t i = 0; i < argument_conversions->size(); ++i)
			if (!BuiltinConversionIsUsable((*argument_conversions)[i]))
				return false;
		return true;
	}

	if (!IsModifiableLvalue(left)) return false;
	const TypeRecord shape = program_->types.Get(
		program_->types.RemoveTopCv(EffectiveType(left.type)));
	if (shape.kind == TYPE_ARRAY) return false;
	const CallConversionFact conversion = CallConversion(
		right, EffectiveType(left.type), 0, 0);
	if (!BuiltinConversionIsUsable(conversion)) return false;
	argument_conversions->push_back(conversion);
	return true;
}

bool Analyzer::BuiltinAssignmentIsNonthrowing(BindingId selected,
	const std::vector<CallConversionFact>& argument_conversions)
{
	if (selected != kNoBinding && !FunctionIsNonthrowing(selected)) return false;
	for (std::size_t i = 0; i < argument_conversions.size(); ++i)
		if (!BuiltinConversionIsNonthrowing(argument_conversions[i])) return false;
	return true;
}

bool Analyzer::BuiltinAssignmentIsTrivial(BindingId selected,
	const std::vector<CallConversionFact>& argument_conversions) const
{
	for (std::size_t i = 0; i < argument_conversions.size(); ++i)
		if (argument_conversions[i].rank == CONVERSION_USER_DEFINED)
			return false;
	if (selected == kNoBinding) return true;
	const FunctionInfo& function = GetFunction(selected);
	return (function.special_member == SPECIAL_MEMBER_COPY_ASSIGNMENT ||
		function.special_member == SPECIAL_MEMBER_MOVE_ASSIGNMENT) &&
		function.trivial_special_member;
}

bool Analyzer::TrivialSpecialMemberDeclaration(BindingId binding,
	FlatBindingIdSet* visited) const
{
	const FunctionInfo& function = GetFunction(binding);
	if (function.user_provided_special_member ||
		(!function.implicit_special_member &&
		 !function.defaulted_special_member && !function.deleted_special_member &&
		 !function.deleted_constructor && !function.deleted_function)) return false;
	if (function.trivial_special_member) return true;
	// Every consumer is a conjunction that stops at the first false result.
	// Repeated nodes of the acyclic subobject graph need no second visit.
	if (!visited->Insert(binding)) return true;
	const EntityId entity = program_->bindings[binding].member_owner;
	const EntityRecord& owner = program_->entities[entity];
	if (owner.polymorphic_class || owner.virtual_base_count != 0) return false;
	const SpecialMemberKind kind = function.special_member;
	const bool assignment = kind == SPECIAL_MEMBER_COPY_ASSIGNMENT ||
		kind == SPECIAL_MEMBER_MOVE_ASSIGNMENT;
	const auto visit = [this, kind, assignment, visited](TypeId type)
	{
		TypeRecord shape = program_->types.Get(type);
		while (shape.kind == TYPE_ARRAY || shape.kind == TYPE_QUALIFIED)
		{
			type = shape.child;
			shape = program_->types.Get(type);
		}
		if (shape.kind != TYPE_NAMED ||
			!IsClassEntity(program_->entities[shape.entity])) return true;
		const BindingId selected = assignment ? AssignmentForSubobject(type, kind) :
			ConstructorForSubobject(type, kind);
		return selected != kNoBinding &&
			TrivialSpecialMemberDeclaration(selected, visited);
	};
	for (std::size_t i = 0; i < owner.direct_base_count; ++i)
		if (!visit(program_->entities[program_->DirectBase(entity, i).entity].type))
			return false;
	if (entity < entity_data_members_.size())
		for (std::size_t i = 0; i < entity_data_members_[entity].size(); ++i)
			if (!visit(program_->bindings[entity_data_members_[entity][i]].type))
				return false;
	return true;
}

bool Analyzer::TrivialDestructorDeclaration(EntityId entity,
	FlatBindingIdSet* visited) const
{
	const EntityRecord& owner = program_->entities[entity];
	if (owner.trivial_destructor) return true;
	if (entity >= entity_destructor_by_entity_.size() ||
		entity_destructor_by_entity_[entity] == kNoBinding) return false;
	const BindingId binding = entity_destructor_by_entity_[entity];
	if (!visited->Insert(binding)) return true;
	const FunctionInfo& function = GetFunction(binding);
	if (program_->bindings[binding].virtual_function ||
		function.user_provided_special_member ||
		(!function.implicit_destructor && !function.defaulted_destructor &&
		 !function.deleted_destructor)) return false;
	const auto visit = [this, visited](TypeId type)
	{
		const EntityId subobject = DestructedEntity(type);
		return subobject == kNoEntity ||
			TrivialDestructorDeclaration(subobject, visited);
	};
	for (std::size_t i = 0; i < owner.direct_base_count; ++i)
		if (!visit(program_->entities[program_->DirectBase(entity, i).entity].type))
			return false;
	if (entity < entity_data_members_.size())
		for (std::size_t i = 0; i < entity_data_members_[entity].size(); ++i)
			if (!visit(program_->bindings[entity_data_members_[entity][i]].type))
				return false;
	return true;
}

bool Analyzer::EvaluateBuiltinTriviallyCopyable(TypeId type) const
{
	type = program_->types.RemoveTopCv(EffectiveType(type));
	const EntityId entity = EntityOf(type);
	if (entity == kNoEntity) return true;
	if (!IsClassEntity(program_->entities[entity]) ||
		entity >= class_special_members_.size()) return false;
	const ClassSpecialMemberFacts& facts = class_special_members_[entity];
	const bool completed = program_->entities[entity].complete;
	if (completed && facts.copyable_declaration != DECLARATION_TRIVIALITY_UNKNOWN)
		return facts.copyable_declaration == DECLARATION_TRIVIALITY_TRIVIAL;
	const auto finish = [&facts, completed](bool value)
	{
		if (completed) facts.copyable_declaration = value ?
			DECLARATION_TRIVIALITY_TRIVIAL : DECLARATION_TRIVIALITY_NONTRIVIAL;
		return value;
	};
	FlatBindingIdSet destructors;
	if (!TrivialDestructorDeclaration(entity, &destructors)) return finish(false);
	const BindingId members[] = {
		facts.copy_constructor, facts.move_constructor,
		facts.copy_assignment, facts.move_assignment
	};
	FlatBindingIdSet transfers;
	for (std::size_t i = 0; i < sizeof(members) / sizeof(members[0]); ++i)
	{
		if (members[i] == kNoBinding) continue;
		if (!TrivialSpecialMemberDeclaration(members[i], &transfers))
			return finish(false);
	}
	const auto check = [this, &transfers](const std::vector<BindingId>& functions)
	{
		for (std::size_t i = 0; i < functions.size(); ++i)
			if (GetFunction(functions[i]).special_member != SPECIAL_MEMBER_NONE &&
				!TrivialSpecialMemberDeclaration(functions[i], &transfers)) return false;
		return true;
	};
	return finish((entity >= entity_constructors_.size() ||
		check(entity_constructors_[entity])) &&
		(entity >= entity_member_functions_.size() ||
		 check(entity_member_functions_[entity])));
}

bool Analyzer::EvaluateBuiltinStandardLayout(TypeId type) const
{
	type = program_->types.RemoveTopCv(EffectiveType(type));
	const EntityId entity = EntityOf(type);
	if (entity == kNoEntity) return true;
	const EntityRecord& owner = program_->entities[entity];
	if (!IsClassEntity(owner) || !owner.complete || owner.polymorphic_class ||
		owner.virtual_base_count != 0 ||
		entity >= entity_data_members_.size()) return false;
	bool has_access = false;
	AccessKind member_access = ACCESS_PUBLIC;
	for (std::size_t i = 0; i < entity_data_members_[entity].size(); ++i)
	{
		const BindingRecord& member =
			program_->bindings[entity_data_members_[entity][i]];
		if (!has_access)
		{
			member_access = member.access;
			has_access = true;
		}
		else if (member.access != member_access) return false;
		TypeId member_type = member.type;
		TypeRecord shape = program_->types.Get(member_type);
		while (shape.kind == TYPE_ARRAY || shape.kind == TYPE_QUALIFIED)
		{
			member_type = shape.child;
			shape = program_->types.Get(member_type);
		}
		if (shape.kind == TYPE_LVALUE_REFERENCE ||
			shape.kind == TYPE_RVALUE_REFERENCE) return false;
		if (shape.kind == TYPE_NAMED &&
			IsClassEntity(program_->entities[shape.entity]) &&
			!EvaluateBuiltinStandardLayout(member_type)) return false;
	}
	bool base_has_members = false;
	for (std::size_t i = 0; i < owner.direct_base_count; ++i)
	{
		const DirectBaseEdge& edge = program_->DirectBase(entity, i);
		if (edge.virtual_base ||
			!EvaluateBuiltinStandardLayout(program_->entities[edge.entity].type))
			return false;
		const bool has_members = edge.entity < entity_data_members_.size() &&
			!entity_data_members_[edge.entity].empty();
		if (has_members && (base_has_members || has_access)) return false;
		base_has_members = base_has_members || has_members;
	}
	return true;
}

bool Analyzer::EvaluateBuiltinTrivialLayoutTrait(
	hosted_builtin::TypeTraitKind trait, TypeId type,
	const TypeRecord& shape, const EntityRecord* named) const
{
	bool value = IsIntegral(type, true) || IsFloating(type) ||
		shape.kind == TYPE_COMPLEX || shape.kind == TYPE_POINTER ||
		shape.kind == TYPE_MEMBER_POINTER;
	if (!named || !IsClassEntity(*named)) return value;
	const bool copyable = EvaluateBuiltinTriviallyCopyable(type);
	if (trait == hosted_builtin::TYPE_TRAIT_IS_STANDARD_LAYOUT)
		return EvaluateBuiltinStandardLayout(type);
	if (trait == hosted_builtin::TYPE_TRAIT_IS_POD)
		return copyable && named->trivial_default_constructor &&
			EvaluateBuiltinStandardLayout(type);
	return copyable &&
		(trait == hosted_builtin::TYPE_TRAIT_IS_LITERAL_TYPE ||
		 named->trivial_default_constructor);
}

bool Analyzer::EvaluateBuiltinNothrowCopy(TypeId type)
{
	type = program_->types.RemoveTopCv(EffectiveType(type));
	const EntityId entity = EntityOf(type);
	if (entity == kNoEntity) return true;
	if (!IsClassEntity(program_->entities[entity]) ||
		entity >= class_special_members_.size()) return false;
	const BindingId copy = class_special_members_[entity].copy_constructor;
	if (copy == kNoBinding) return false;
	const FunctionInfo& function = GetFunction(copy);
	return !function.deleted_function && !function.deleted_constructor &&
		!function.deleted_special_member && CanAccessMember(copy) &&
		FunctionIsNonthrowing(copy);
}

}
}
