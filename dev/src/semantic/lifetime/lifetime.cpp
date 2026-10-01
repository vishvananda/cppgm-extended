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

std::uint32_t Analyzer::CurrentLexicalCleanupRoot(ScopeId scope)
{
	const std::uint32_t depth = program_->KindOfScope(scope) == SCOPE_FUNCTION ||
		exception_control_contexts_.empty() ? 0 :
		exception_control_contexts_[current_exception_control_context_].depth;
	std::uint32_t tail = depth == 0 ? 0 :
		exception_control_contexts_[current_exception_control_context_].cleanup_root;
	std::vector<std::pair<ScopeId, std::size_t> > pending;
	ScopeId current = scope < nearest_lifetime_scopes_.size() ?
		nearest_lifetime_scopes_[scope] : kNoScope;
	while (current != kNoScope)
	{
		const std::size_t count = current < scope_lifetimes_.size() ?
			scope_lifetimes_[current].size() : 0;
		for (std::size_t i = count; i != 0; --i)
		{
			const std::uint32_t known = scope_lifetimes_[current][i - 1].cleanup_plan;
			if (known != 0)
			{
				if (dump_.lexical_cleanup_plans[known - 1].depth == depth) tail = known;
				current = kNoScope;
				break;
			}
			pending.push_back(std::make_pair(current, i - 1));
		}
		if (current == kNoScope || program_->KindOfScope(current) == SCOPE_FUNCTION) break;
		const ScopeId parent = scope_parents_[current];
		current = parent != kNoScope && parent < nearest_lifetime_scopes_.size() ?
			nearest_lifetime_scopes_[parent] : kNoScope;
	}
	for (std::size_t i = pending.size(); i != 0; --i)
	{
		const ScopeId owner = pending[i - 1].first;
		const std::size_t index = pending[i - 1].second;
		const LifetimeObligation obligation = scope_lifetimes_[owner][index];
		const std::uint32_t action = obligation.temporary == kNoDumpEdge ?
			MakeDestructorAction(obligation.type, obligation.destructor,
				obligation.object, 0, false) :
			MakeTemporaryDestructorAction(obligation.temporary, obligation.destructor);
		if (action == kNoDumpEdge) continue;
		tail = dump_.AddLexicalCleanupPlan(LEXICAL_CLEANUP_OBJECT, action, tail, depth,
			!program_->bindings[obligation.destructor].nonthrowing);
		scope_lifetimes_[owner][index].cleanup_plan = tail;
	}
	return tail;
}

std::uint32_t Analyzer::CreateLifetimeCleanupPlan(ScopeId scope,
	const LifetimeObligation& obligation)
{
	const bool function_parameter = program_->KindOfScope(scope) == SCOPE_FUNCTION;
	const std::uint32_t depth = function_parameter || exception_control_contexts_.empty() ? 0 :
		exception_control_contexts_[current_exception_control_context_].depth;
	if (function_parameter || (depth == 0 &&
		program_->bindings[obligation.destructor].nonthrowing)) return 0;
	const std::uint32_t root = CurrentLexicalCleanupRoot(scope);
	const std::uint32_t action = obligation.temporary == kNoDumpEdge ?
		MakeDestructorAction(obligation.type, obligation.destructor,
			obligation.object, 0, false) :
		MakeTemporaryDestructorAction(obligation.temporary, obligation.destructor);
	return action == kNoDumpEdge ? root :
		dump_.AddLexicalCleanupPlan(LEXICAL_CLEANUP_OBJECT, action, root, depth,
			!program_->bindings[obligation.destructor].nonthrowing);
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
	LifetimeObligation obligation(object, destructor, type);
	obligation.cleanup_plan = CreateLifetimeCleanupPlan(scope, obligation);
	PrepareLifetimeScope(scope, &scope_lifetimes_, &nearest_lifetime_scopes_);
	scope_lifetimes_[scope].push_back(obligation);
}

void Analyzer::AddTemporaryLifetimeObligation(ScopeId scope,
	std::uint32_t temporary)
{
	const std::uint32_t action = MakeTemporaryDestructorAction(temporary);
	if (action == kNoDumpEdge) return;
	const DumpNode& cleanup = dump_.nodes[action];
	LifetimeObligation obligation(kNoBinding, cleanup.binding, cleanup.operand_type, temporary);
	obligation.cleanup_plan = CreateLifetimeCleanupPlan(scope, obligation);
	PrepareLifetimeScope(scope, &scope_lifetimes_, &nearest_lifetime_scopes_);
	scope_lifetimes_[scope].push_back(obligation);
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
