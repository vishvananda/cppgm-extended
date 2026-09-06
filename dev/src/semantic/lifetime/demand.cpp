#include "semantic/analysis/diagnostic_location.h"
#include "semantic/analysis/analyzer.h"

namespace cppgm
{
namespace semantic
{

void Analyzer::CompleteTranslationUnitDemand()
{
	if (source_type_view_)
		for (std::size_t i = 0; i < functions_.size(); ++i)
		{
			const FunctionInfo& function = functions_[i];
			if (function.definition_body != kNoNode &&
				!program_->bindings[function.binding].compiler_generated)
				DemandRuntimeDefinition(function.binding);
		}
	DemandMaterializedConstructorActions(root_);
	if (function_templates_.empty() && class_templates_.empty())
		for (std::size_t i = 0; i < hidden_friend_anchor_by_entity_.size(); ++i)
			if (hidden_friend_anchor_by_entity_[i] != kNoBinding &&
				!GetFunction(hidden_friend_anchor_by_entity_[i]).constexpr_function)
				DemandFunction(hidden_friend_anchor_by_entity_[i]);
	ReplayRequiredFunctionDemandEdges();
	std::size_t default_demand = 0;
	std::size_t function_demand = 0;
	std::size_t member_definition_demand = 0;
	while (member_definition_demand <
			demanded_class_template_member_definitions_.size() ||
		default_demand < demanded_default_constructor_entities_.size() ||
		function_demand < demanded_functions_.size())
	{
		while (member_definition_demand <
			demanded_class_template_member_definitions_.size())
			ApplyDemandedClassTemplateMemberDefinitions(
				demanded_class_template_member_definitions_[
					member_definition_demand++]);
		while (default_demand < demanded_default_constructor_entities_.size())
			EmitDefaultConstructor(
				demanded_default_constructor_entities_[default_demand++]);
		while (function_demand < demanded_functions_.size())
			EmitDemandedFunction(demanded_functions_[function_demand++]);
	}
}

void Analyzer::MarkFunctionObjectOutputRoot(BindingId binding)
{
	if (binding == kNoBinding) return;
	binding = program_->bindings[binding].canonical;
	program_->bindings[binding].object_output_root = true;
	const BindingId lifecycle_base =
		program_->bindings[binding].lifecycle_base_entry;
	if (lifecycle_base != kNoBinding &&
		lifecycle_base < program_->bindings.size())
		program_->bindings[lifecycle_base].object_output_root = true;
	if (binding < constructor_base_entry_by_binding_.size())
	{
		const BindingId base = constructor_base_entry_by_binding_[binding];
		if (base != kNoBinding && base < program_->bindings.size())
			program_->bindings[base].object_output_root = true;
	}
	if (binding < destructor_base_entry_by_binding_.size())
	{
		const BindingId base = destructor_base_entry_by_binding_[binding];
		if (base != kNoBinding && base < program_->bindings.size())
			program_->bindings[base].object_output_root = true;
	}
}

void Analyzer::DemandRuntimeFunction(BindingId binding,
	FunctionDemandReason reason)
{
	if (binding == kNoBinding) return;
	binding = program_->bindings[binding].canonical;
	RecordFunctionDemand(binding, reason);
	EnsureFunctionExceptionSpecification(binding);
	if (current_function_context_ != kNoBinding &&
		!FunctionObjectDefinitionRequired(current_function_context_)) return;
	DemandRuntimeDefinition(binding);
}

// N3485 14.7.2/10: an explicit instantiation declaration suppresses the
// implicit instantiation of what it names except an inline function, whose
// definition stays the using translation unit's to produce -- and to inline.
// libc++ names basic_string's inline members (`compare`, the destructor, the
// copy constructor) in its extern template list, and every hosted compiler
// inlines them; without this exemption each became a call into the shared
// library.
bool Analyzer::ExplicitInstantiationSuppressesDefinition(
	BindingId binding) const
{
	binding = program_->bindings[binding].canonical;
	const BindingRecord& record = program_->bindings[binding];
	if (!record.explicit_instantiation_suppressed) return false;
	if (binding >= function_fact_by_binding_.size() ||
		function_fact_by_binding_[binding] == kNoDumpEdge) return true;
	// An inline function in the standard's sense (7.1.2): declared inline,
	// defined in its class, or constexpr.  The binding's inline_function
	// flag is wider -- a constructor or destructor carries it for having a
	// definition anywhere -- and would exempt out-of-class definitions that
	// the instantiation owns.
	const FunctionInfo& function = GetFunction(binding);
	return !function.inline_specified && !function.definition_in_class &&
		!function.constexpr_function;
}

bool Analyzer::FunctionObjectDefinitionRequired(
	BindingId binding) const
{
	if (binding == kNoBinding) return false;
	binding = program_->bindings[binding].canonical;
	const BindingRecord& record = program_->bindings[binding];
	return !ExplicitInstantiationSuppressesDefinition(binding) &&
		(!record.inline_function || record.emission_demanded ||
		 record.object_output_root);
}

void Analyzer::ReplayFunctionDemandEdges(BindingId binding)
{
	binding = program_->bindings[binding].canonical;
	FunctionInfo& function = GetMutableFunction(binding);
	if (function.emission_dependencies_replayed) return;
	function.emission_dependencies_replayed = true;
	if (stats_) ++stats_->demand_replayed_functions;
	if (binding >= function_demand_head_by_binding_.size()) return;
	for (std::uint32_t edge = function_demand_head_by_binding_[binding];
		edge != kNoDumpEdge; edge = function_demand_edges_[edge].next)
	{
		if (stats_) ++stats_->demand_replayed_edges;
		DemandRuntimeDefinition(function_demand_edges_[edge].callee);
	}
}

void Analyzer::ReplayRequiredFunctionDemandEdges()
{
	for (std::size_t i = 0; i < functions_with_demand_edges_.size(); ++i)
		if (FunctionObjectDefinitionRequired(functions_with_demand_edges_[i]))
			ReplayFunctionDemandEdges(functions_with_demand_edges_[i]);
}

void Analyzer::DemandRuntimeDefinition(BindingId binding)
{
	if (binding == kNoBinding) return;
	binding = program_->bindings[binding].canonical;
	DemandClassTemplateMemberDefinitions(program_->bindings[binding].member_owner);
	program_->bindings[binding].emission_demanded |=
		program_->bindings[binding].inline_function;
	if (FunctionObjectDefinitionRequired(binding))
		ReplayFunctionDemandEdges(binding);
	if ((program_->bindings[binding].constructor ||
		 program_->bindings[binding].destructor) &&
		program_->bindings[binding].member_owner != kNoEntity &&
		program_->entities[program_->bindings[binding].member_owner].
			polymorphic_class)
		MarkVtableDemand(program_->bindings[binding].member_owner);
	if (binding < constructor_base_entry_by_binding_.size())
	{
		const BindingId base_entry =
			constructor_base_entry_by_binding_[binding];
		if (base_entry != kNoBinding && base_entry != binding)
			DemandRuntimeDefinition(base_entry);
	}
	if (binding < destructor_base_entry_by_binding_.size())
	{
		const BindingId base_entry =
			destructor_base_entry_by_binding_[binding];
		if (base_entry != kNoBinding && base_entry != binding)
			DemandRuntimeDefinition(base_entry);
	}
	QueueDeferredFunctionDefinition(binding);
}

void Analyzer::QueueDeferredFunctionDefinition(BindingId binding)
{
	if (binding >= function_fact_by_binding_.size() ||
		function_fact_by_binding_[binding] == kNoDumpEdge) return;
	FunctionInfo& function = GetMutableFunction(binding);
	if (!function.deferred ||
		function.definition_state != FUNCTION_DEFINITION_NOT_STARTED) return;
	function.definition_state = FUNCTION_DEFINITION_QUEUED;
	demanded_functions_.push_back(binding);
	++demand_worklist_pushes_;
}

void Analyzer::CompleteFunctionDefinition(BindingId binding)
{
	binding = program_->bindings[binding].canonical;
	GetMutableFunction(binding).definition_state =
		FUNCTION_DEFINITION_COMPLETE;
	++demanded_function_emissions_;
	if (!stats_) return;
	if (FunctionObjectDefinitionRequired(binding))
		++stats_->definition_emission_required_completions;
	else ++stats_->definition_validation_only_completions;
}

void Analyzer::EmitDemandedFunction(BindingId binding)
{
	binding = program_->bindings[binding].canonical;
	if (binding >= function_fact_by_binding_.size() ||
		function_fact_by_binding_[binding] == kNoDumpEdge) return;
	FunctionInfo& state = GetMutableFunction(binding);
	if (state.definition_state >= FUNCTION_DEFINITION_IN_PROGRESS) return;
	state.definition_state = FUNCTION_DEFINITION_IN_PROGRESS;
	// A specialization's body is analysed here, long after the instantiation
	// that demanded it; an error inside points at the pattern's source, so
	// the diagnostic names the specialization that reached it.
	const ScopedInstantiation instantiation_note(
		DescribeFunctionSpecialization(binding));
	const FunctionInfo& initial = GetFunction(binding);
	if (!program_->bindings[binding].explicit_instantiation_suppressed &&
		EnclosingExplicitInstantiationSuppressed(binding) &&
		ExplicitInstantiationCoversMember(binding))
		program_->bindings[binding].explicit_instantiation_suppressed = true;
	// An inline function the declaration named is exempt (N3485 14.7.2/10):
	// it is defined here after all, and as the weak definition every user
	// produces, so the flag that would make the lowering bind it strongly
	// comes off, from its lifecycle peer too.
	if (program_->bindings[binding].explicit_instantiation_suppressed &&
		!ExplicitInstantiationSuppressesDefinition(binding))
	{
		program_->bindings[binding].explicit_instantiation_suppressed = false;
		const BindingId peer =
			program_->bindings[binding].lifecycle_base_entry;
		if (peer != kNoBinding && peer < program_->bindings.size() &&
			(program_->bindings[peer].constructor ||
			 program_->bindings[peer].destructor))
			program_->bindings[peer].explicit_instantiation_suppressed = false;
	}
	const bool emit_definition = initial.defined &&
		!program_->bindings[binding].explicit_instantiation_suppressed;
	const bool member = initial.member_owner != kNoType;
	const TypeId output_type = member ?
		AdaptMemberFunctionType(initial.binding) : initial.type;
	const std::uint32_t function = MakeDump(emit_definition ?
		DUMP_FUNCTION_DEFINITION : DUMP_FUNCTION_DECLARATION,
		output_type, VALUE_NONE, 0, initial.binding);
	dump_.Add(root_, function);
	if (!emit_definition && (retain_lowering_facts_ || member ||
		program_->bindings[binding].explicit_instantiation_suppressed))
	{
		CompleteFunctionDefinition(binding);
		return;
	}
	if (emit_definition &&
		initial.retained_definition_semantics != kNoDumpEdge)
	{
		for (std::uint32_t edge = dump_.nodes[
			initial.retained_definition_semantics].first_edge;
			edge != kNoDumpEdge; edge = dump_.edges[edge].next)
			dump_.Add(function, dump_.edges[edge].child);
		FinalizeStaticallyUnreachableBranchCleanup(function);
		DemandMaterializedConstructorActions(function, true);
		CompleteFunctionDefinition(binding);
		return;
	}
	const FunctionInfo info = GetFunction(binding);
	const ScopeId function_scope = NewScope(info.lexical_scope, SCOPE_FUNCTION,
		program_->bindings[info.binding].name, ScopePrefixId(info.owner));
	std::vector<BindingId> parameter_bindings; BindingId this_binding = kNoBinding;
	BindFunctionParameterPackElement(function_scope, info.parameter_pack_name, kNoBinding);
	if (member)
	{
		const TypeId this_type = program_->types.Parameters(output_type)[0];
		const NameId this_name = program_->names.Intern("this");
		this_binding = program_->AddBinding(function_scope,
			BIND_PARAMETER, this_name, this_type);
		program_->bindings[this_binding].compiler_generated = true;
		dump_.Add(function, MakeDump(DUMP_PARAMETER, this_type,
			VALUE_NONE, this_name, this_binding));
	}
	for (std::size_t i = 0; i < info.parameters.size(); ++i)
	{
		const ParameterInfo& parameter = info.parameters[i];
		const BindingId parameter_binding = program_->AddBinding(function_scope,
			BIND_PARAMETER, parameter.name, ParameterBindingType(parameter));
		RecordSourceTypeOverride(parameter_binding, parameter.declared_type);
		parameter_bindings.push_back(parameter_binding);
		BindFunctionParameterPackElement(
			function_scope, parameter.pack_name, parameter_binding);
		dump_.Add(function, MakeDump(DUMP_PARAMETER, parameter.function_type,
			VALUE_NONE, parameter.name, parameter_binding));
		AddLifetimeObligation(function_scope, parameter_binding,
			parameter.function_type, false);
	}
	InstallLambdaCaptureBindings(function_scope, this_binding, info);
	if (!emit_definition)
	{
		CompleteFunctionDefinition(binding);
		return;
	}
	if (emit_definition)
	{
		const TypeId previous_return = current_return_type_;
		const EntityId previous_class = current_class_context_;
		const BindingId previous_function = current_function_context_;
		current_return_type_ = program_->types.Get(info.type).child;
		current_class_context_ = info.friend_of != kNoEntity ? info.friend_of :
			program_->bindings[info.binding].member_owner;
		current_function_context_ =
			program_->bindings[info.binding].canonical;
		BeginFunctionControlFlowFacts();
		if (info.constructor)
		{
			std::uint32_t function_try;
			const std::uint32_t constructor_parent = BeginFunctionTryRegion(
				function, info.function_try_block, &function_try);
			const std::uint32_t constructor_body =
				MakeDump(DUMP_COMPOUND_STATEMENT);
			dump_.Add(constructor_parent, constructor_body);
			if ((info.special_member == SPECIAL_MEMBER_COPY_CONSTRUCTOR ||
				 info.special_member == SPECIAL_MEMBER_MOVE_CONSTRUCTOR) &&
				(info.implicit_special_member || info.defaulted_special_member))
				AddSynthesizedConstructorBody(info, parameter_bindings,
					constructor_body);
			else AddConstructorMemberActions(info, function_scope,
				parameter_bindings, constructor_body);
			if (info.special_member == SPECIAL_MEMBER_NONE &&
				(info.implicit_constructor || info.defaulted_constructor) &&
				InitializationActionsAreNonthrowing(constructor_body))
				program_->bindings[info.binding].nonthrowing = true;
			if (info.definition_body != kNoNode)
				AnalyzeCompound(info.definition_body, function_scope,
					constructor_body);
			DemandConstructorUnwindDestructors(constructor_body);
			if (function_try != kNoDumpEdge)
			{
				AnalyzeFunctionTryHandlers(info.function_try_block,
					function_scope, function_try,
					FUNCTION_TRY_BODY_CONSTRUCTOR);
			}
		}
		else if ((info.special_member == SPECIAL_MEMBER_COPY_ASSIGNMENT ||
			info.special_member == SPECIAL_MEMBER_MOVE_ASSIGNMENT) &&
			(info.implicit_special_member || info.defaulted_special_member))
		{
			const std::uint32_t assignment_body =
				MakeDump(DUMP_COMPOUND_STATEMENT);
			dump_.Add(function, assignment_body);
			AddSynthesizedAssignmentBody(info, parameter_bindings,
				assignment_body);
		}
		else if (info.destructor)
		{
			std::uint32_t function_try;
			const std::uint32_t destructor_parent = BeginFunctionTryRegion(
				function, info.function_try_block, &function_try);
			const std::uint32_t destructor_body =
				MakeDump(DUMP_COMPOUND_STATEMENT);
			dump_.Add(destructor_parent, destructor_body);
			const EntityId entity =
				program_->bindings[info.binding].member_owner;
			if (entity != kNoEntity &&
				program_->entities[entity].polymorphic_class)
				dump_.Add(destructor_body,
					MakeDump(DUMP_VPTR_INITIALIZATION_ACTION,
						program_->entities[entity].type));
			if (info.definition_body != kNoNode)
				AnalyzeCompound(info.definition_body, function_scope,
					destructor_body);
			AddDestructorSubobjectActions(
				program_->bindings[info.binding].member_owner,
				info.binding, destructor_body);
			if (function_try != kNoDumpEdge)
				AnalyzeFunctionTryHandlers(info.function_try_block,
					function_scope, function_try,
					FUNCTION_TRY_BODY_DESTRUCTOR);
		}
		else if (info.definition_body != kNoNode)
			AnalyzeCompound(info.definition_body, function_scope, function);
		else dump_.Add(function, MakeDump(DUMP_COMPOUND_STATEMENT));
		FinishFunctionControlFlowFacts();
		FinalizeNamedReturnSlot(function);
		current_return_type_ = previous_return;
		current_class_context_ = previous_class;
		current_function_context_ = previous_function;
		DemandMaterializedConstructorActions(function, true);
	}
	CompleteFunctionDefinition(binding);
}

}
}
