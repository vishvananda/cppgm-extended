#include "semantic/analysis/analyzer.h"
#include "support/exceptions.h"

namespace cppgm
{
namespace semantic
{

namespace
{

void PrepareLifetimeScope(ScopeId scope,
	std::vector<std::vector<LifetimeObligation> >* lifetimes,
	std::vector<ScopeId>* nearest)
{
	if (lifetimes->size() <= scope)
		lifetimes->resize(static_cast<std::size_t>(scope) + 1);
	if (nearest->size() <= scope)
		nearest->resize(
			static_cast<std::size_t>(scope) + 1, kNoScope);
	(*nearest)[scope] = scope;
}

}

void Analyzer::AddLifetimeObligation(ScopeId scope,
	BindingId object, TypeId type, bool allow_elision)
{
	if (IsInitializerListType(type)) return;
	const EntityId entity = DestructedEntity(type);
	if (entity == kNoEntity) return;
	EnsureClassDefinition(type);
	if (!program_->entities[entity].destructible)
		ThrowSemanticError("object type is not destructible");
	const BindingId destructor = DestructorForType(type);
	if (destructor == kNoBinding)
		ThrowInternalCompilerError("class has no destructor identity");
	if (!CanAccessMember(destructor, entity))
		ThrowSemanticError("inaccessible destructor");
	const TypeKind object_kind = program_->types.Get(
		program_->types.RemoveTopCv(type)).kind;
	if (program_->entities[entity].trivial_destructor) return;
	// Odr-use owns host emission even when an empty call can be elided.
	if (host_object_emission_)
	{
		const BindingId canonical = program_->bindings[destructor].canonical;
		const BindingId base_entry = EnsureDestructorBaseEntry(canonical);
		DemandFunction(canonical);
		if (base_entry != canonical)
			MarkFunctionObjectOutputRoot(base_entry);
	}
	if (allow_elision && object_kind != TYPE_ARRAY &&
		IsElidableAutomaticDestructor(destructor))
	{
		if (host_object_emission_)
			MarkFunctionObjectOutputRoot(destructor);
		return;
	}
	PrepareLifetimeScope(scope, &scope_lifetimes_, &nearest_lifetime_scopes_);
	scope_lifetimes_[scope].push_back(
		LifetimeObligation(object, destructor, type));
}

void Analyzer::AddTemporaryLifetimeObligation(ScopeId scope,
	std::uint32_t temporary)
{
	const std::uint32_t action = MakeTemporaryDestructorAction(temporary);
	if (action == kNoDumpEdge) return;
	const DumpNode& cleanup = dump_.nodes[action];
	PrepareLifetimeScope(scope, &scope_lifetimes_, &nearest_lifetime_scopes_);
	scope_lifetimes_[scope].push_back(LifetimeObligation(kNoBinding,
		cleanup.binding, cleanup.operand_type, temporary));
	MarkInitializerListLifetimeScope(scope, temporary);
}

void Analyzer::CollectReferenceLifetimeObjects(std::uint32_t node,
	std::vector<std::pair<std::uint32_t, std::uint32_t> >* objects)
{
	if (node == kNoDumpEdge) return;
	const DumpNode& value = dump_.nodes[node];
	if (value.kind == DUMP_TEMPORARY_OBJECT)
	{
		const std::uint32_t destructor =
			MakeTemporaryDestructorAction(node, kNoBinding, true);
		objects->push_back(std::make_pair(node, destructor));
		return;
	}
	if (value.category == VALUE_PRVALUE && !IsClassObjectType(value.type) &&
		!program_->types.IsReference(value.type) &&
		program_->types.Get(program_->types.RemoveTopCv(value.type)).kind != TYPE_ARRAY)
	{
		objects->push_back(std::make_pair(node, kNoDumpEdge));
		return;
	}
	if (value.first_edge == kNoDumpEdge) return;
	const std::uint32_t first = dump_.edges[value.first_edge].child;
	if ((value.kind == DUMP_MEMBER_EXPRESSION && value.binding != kNoBinding &&
		 program_->bindings[value.binding].non_static_data_member &&
		 !program_->types.IsReference(program_->bindings[value.binding].type)) ||
		(value.kind == DUMP_SUBSCRIPT_EXPRESSION &&
		 program_->types.Get(program_->types.RemoveTopCv(
			EffectiveType(dump_.nodes[first].type))).kind == TYPE_ARRAY) ||
		(value.kind == DUMP_CAST_EXPRESSION && value.category != VALUE_PRVALUE))
		CollectReferenceLifetimeObjects(first, objects);
	else if (value.kind == DUMP_BINARY_EXPRESSION && value.OperationIs(OP_COMMA))
	{
		const std::uint32_t second = dump_.edges[value.first_edge].next;
		if (second != kNoDumpEdge)
			CollectReferenceLifetimeObjects(dump_.edges[second].child, objects);
	}
	else if (value.kind == DUMP_CONDITIONAL_EXPRESSION)
	{
		for (std::uint32_t edge = dump_.edges[value.first_edge].next;
			edge != kNoDumpEdge; edge = dump_.edges[edge].next)
			CollectReferenceLifetimeObjects(dump_.edges[edge].child, objects);
	}
}

}  // namespace semantic
}  // namespace cppgm
