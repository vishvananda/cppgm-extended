#include "sema/template_packs.h"

// Variable templates (which the PA15 fixtures use although the README
// leaves them to later stages): a namespace-scope variable declared under a
// template-head is a template (its VariableTemplateFacts' head), and each
// template-id names a specialization, one VariableEntity per (template,
// canonical arguments), defined where needed from the template's
// definition or that of the partial specialization its arguments match.
// A specialization whose value is a constant is read as that value (no
// odr-use, the course's form); one odr-used is emitted, weak, from its
// instantiated or explicitly specialized definition.
namespace sema {

VariableEntity* Analyzer::variable_template_of(const Binding* binding)
{
	if (binding == nullptr || binding->kind != BindingKind::Variable || binding->entity == nullptr)
		return nullptr;
	VariableEntity* variable = static_cast<VariableEntity*>(binding->entity);
	return variable->templ != nullptr && variable->templ->head != nullptr ? variable : nullptr;
}

void Analyzer::declare_variable_template(
	VariableEntity* variable, const syntax::Initializer* initializer, Scope* scope, Scope* lookup, uint32_t location)
{
	// The templated entity of a variable template declaration, whose
	// definition (its initializer and parameter scope) its specializations
	// instantiate.
	VariableTemplateFacts* facts = variable_template_facts(variable);
	if (facts->head == nullptr)
		facts->head = template_head(*template_list_, template_scope_, template_entities_);
	else
		merge_template_head(facts->head, *template_list_, template_scope_, location);
	if (initializer != nullptr || facts->parameters == nullptr)
	{
		facts->initializer = initializer;
		facts->parameters = scope;
		facts->scope = lookup;
		facts->location = location;
	}
}

VariableEntity* Analyzer::named_variable_template(const syntax::Name& name, Scope* scope)
{
	// The variable template a declarator-id's template-id names.
	syntax::NameComponent last = name.last();
	last.has_arguments = false;
	last.arguments = syntax::Slice<syntax::TemplateArgument>{ nullptr, 0 };
	SmallList<syntax::NameComponent, 4> components;
	for (uint32_t i = 0; i + 1 < name.components.size; ++i)
		components.push_back(name.components[i]);
	components.push_back(last);
	syntax::Name bare = syntax::Name();
	bare.global = name.global;
	bare.location = name.location;
	bare.components = syntax::Slice<syntax::NameComponent>{ components.data(),
		static_cast<uint32_t>(components.size()) };
	VariableEntity* templ = variable_template_of(lookup_name(bare, scope, LookupFilter::Ordinary).binding);
	if (templ == nullptr)
		error(name.location, "a template-id declarator of no variable template");
	return templ;
}

Binding* Analyzer::variable_template_binding(const Binding* found, const syntax::NameComponent& component, Scope* scope)
{
	// A template-id naming a variable template's specialization: the
	// specialization's binding (null when `found` is no variable template).
	VariableEntity* templ = variable_template_of(found);
	if (templ == nullptr)
		return nullptr;
	const TemplateHead* head = templ->templ->head;
	TypeList arguments;
	template_arguments(component, scope, arguments, head);
	default_template_arguments(head, arguments, component.location);
	// A member template's specialization has its declaration's access
	// (14.5.2p2, 11p4), as its binding tells unqualified uses.
	Binding* binding = variable_specialization(templ, arguments, component.location)->templ->binding;
	if (found->declared_in == binding->declared_in)
		binding->access = found->access;
	return binding;
}

VariableEntity* Analyzer::variable_specialization(
	VariableEntity* templ, const TypeList& given, uint32_t location, bool declared)
{
	// One entity per (template, canonical arguments): its type the
	// declared type of the definition it instantiates (the template's, or
	// that of the partial specialization its arguments match, 14.5.5.1)
	// substituted, its linkage and specifiers the template's. One an
	// explicit specialization `declared` first has that declaration's type
	// (14.7.3), which its caller gives it.
	const TemplateHead* head = templ->templ->head;
	TypeList arguments;
	match_template_arguments(head, given.data(), static_cast<uint32_t>(given.size()), arguments, true, location);
	const SpecializationKey probe = SpecializationKey{ templ, arguments.data(),
		static_cast<uint32_t>(arguments.size()) };
	if (Entity** found = specializations_.find(probe))
	{
		if ((*found)->type == nullptr && !declared)
			error(location, "an auto variable template's specialization named in its own initializer");
		return static_cast<VariableEntity*>(*found);
	}
	bool known = !declared;
	for (const Type* argument : arguments)
		known = known && !dependent_type(argument);
	TypeList deduced;
	const VariableEntity* partial = known && (templ->templ->partials != nullptr || templ->templ->partial_positions != 0)
		? matched_partial(templ, arguments.data(), static_cast<uint32_t>(arguments.size()), deduced, location)
		: nullptr;
	// An auto declaration's specialization has the type its instantiated
	// initializer deduces (7.1.6.4p6-p7; the initializer is no unevaluated
	// operand, a lambda-expression may be it, 5.1.2p2): instantiated where
	// the specialization is first named, before which it has no type.
	const bool automatic = known && (partial != nullptr ? partial : templ)->templ->auto_type != nullptr;
	const Type* type = declared || automatic ? nullptr
		: partial != nullptr ? substitute(partial->type, substitution_of(partial->templ->head, deduced.data()))
							 : substitute(templ->type, substitution_of(head, arguments.data()));
	VariableEntity* variable = new_entity<VariableEntity>(EntityKind::Variable, templ->name, templ->owner, type);
	variable->is_static = templ->is_static;
	// constexpr is the declaration's own (7.1.5p9): the partial
	// specialization's, an explicit specialization's (set by its caller).
	variable->is_constexpr = (partial != nullptr ? partial : templ)->is_constexpr;
	variable->is_thread_local = templ->is_thread_local;
	variable->internal_linkage = templ->internal_linkage;
	VariableTemplateFacts* facts = variable_template_facts(variable);
	facts->pattern = templ;
	facts->arguments = arena_.copy_array(arguments.data(), arguments.size());
	facts->argument_count = static_cast<uint32_t>(arguments.size());
	Binding* binding = new_binding(BindingKind::Variable, templ->name, variable, type);
	binding->hidden = true;
	binding->declared_in = templ->owner;
	facts->binding = binding;
	SpecializationKey key = probe;
	key.arguments = facts->arguments;
	specializations_.insert(key, variable);
	++graph_.specializations;
	if (automatic && (!instantiate_variable_specialization(variable) || variable->type == nullptr))
		error(location, "an auto variable template's specialization without an initializer to deduce from");
	return variable;
}

const VariableEntity* Analyzer::matched_partial(
	const VariableEntity* templ, const Type* const* arguments, uint32_t count, TypeList& deduced, uint32_t location)
{
	// The partial specialization the arguments match that is more
	// specialized than each other match (14.5.5.1, 14.5.5.2, as for class
	// templates), with its deduced arguments; null for none: the last
	// winner in turn, checked against the others, of the unindexed partial
	// specializations and those of the index lists the arguments select.
	const VariableTemplateFacts* source = templ->templ;
	const VariableEntity* best = nullptr;
	SmallList<const VariableEntity*, 4> matches;
	SmallList<Entity*, 4> lists;
	lists.push_back(source->partials);
	indexed_partials(templ, source->partial_positions, arguments, count, lists);
	for (Entity* first : lists)
	{
		for (const VariableEntity* partial = static_cast<VariableEntity*>(first); partial != nullptr;
			partial = partial->templ->next_partial)
		{
			TypeList trial;
			const VariableTemplateFacts* candidate = partial->templ;
			if (!deduce_partial_arguments(
					candidate->head, candidate->arguments, candidate->argument_count, arguments, count, trial))
				continue;
			matches.push_back(partial);
			if (best == nullptr ||
				(variable_partial_ordered(partial, best) && !variable_partial_ordered(best, partial)))
			{
				best = partial;
				deduced = trial;
			}
		}
	}
	for (const VariableEntity* other : matches)
	{
		if (other != best && (!variable_partial_ordered(best, other) || variable_partial_ordered(other, best)))
			error(location, "a specialization matches several partial specializations, none more specialized");
	}
	return best;
}

const Type* Analyzer::auto_variable_type(const VariableEntity* variable, const Specifiers& specifiers,
	const Type* declared, const syntax::Initializer* initializer, Scope* scope, uint32_t location)
{
	// A variable template specialization's type deduced from its
	// initializer (7.1.6.4p6), const for a constexpr one (7.1.5p9).
	const Type* saved_auto = auto_deduced_;
	auto_deduced_ = nullptr;
	const Type* type = deduced_type(specifiers, declared, initializer, scope, location);
	auto_deduced_ = saved_auto;
	if (type != nullptr && (specifiers.is_constexpr || variable->is_constexpr))
		type = types_.qualified(type, kConst);
	if (type != nullptr)
		variable->templ->binding->type = type;
	return type;
}

void Analyzer::variable_partial_specialization(const Specifiers& specifiers, const DeclaratorInfo& info,
	const syntax::Initializer* initializer, Scope* scope, uint32_t location)
{
	// 14.5.5 (as for class templates): under a template-head, a
	// template-id declarator-id declares a partial specialization of the
	// variable template, a pattern with its own head, argument list (so
	// restricted, p8) and definition.
	VariableEntity* templ = named_variable_template(*info.name, scope);
	const TemplateHead* head = templ->templ->head;
	TypeList given;
	template_arguments(info.name->last(), scope, given, head);
	default_template_arguments(head, given, location);
	TypeList arguments;
	match_template_arguments(head, given.data(), static_cast<uint32_t>(given.size()), arguments, true, location);
	const Type* type = specifiers.is_constexpr ? types_.qualified(info.type, kConst) : info.type;
	VariableEntity* partial = new_entity<VariableEntity>(EntityKind::Variable, templ->name, templ->owner, type);
	partial->defined = initializer != nullptr;
	partial->is_constexpr = specifiers.is_constexpr;
	VariableTemplateFacts* facts = variable_template_facts(partial);
	facts->head = template_head(*template_list_, template_scope_, template_entities_);
	check_partial_arguments(head, facts->head, arguments.data(), static_cast<uint32_t>(arguments.size()), location);
	facts->arguments = arena_.copy_array(arguments.data(), arguments.size());
	facts->argument_count = static_cast<uint32_t>(arguments.size());
	facts->initializer = initializer;
	facts->parameters = scope;
	facts->scope = scope;
	facts->location = location;
	if (holds_auto(info.type))
	{
		facts->auto_type = info.type;
		facts->auto_cv = specifiers.cv;
	}
	// Indexed as a class template's partial specialization is (see
	// partial_index_key), else the last of the unindexed ones.
	VariableTemplateFacts* source = templ->templ;
	const Type* key[2];
	const bool indexed = partial_index_key(head, facts->arguments, facts->argument_count, key);
	VariableEntity* former = static_cast<VariableEntity*>(
		index_partial(templ, partial, indexed ? key : nullptr, source->partials, source->partial_positions));
	if (indexed)
		facts->next_partial = former;
	else if (former != nullptr)
		former->templ->next_partial = partial;
	else
		source->partials = partial;
	if (initializer != nullptr)
		initializer_names(initializer, scope, false);
}

void Analyzer::variable_explicit_specialization(const Specifiers& specifiers, const DeclaratorInfo& info,
	const syntax::Initializer* initializer, Scope* scope, uint32_t location)
{
	// 14.7.3: `template<>` with a template-id declarator-id defines the
	// variable template's specialization from its own initializer, before
	// any use would instantiate it (p6).
	// A static data member template's declarator-id is checked as the
	// member's declaration is (11p6).
	explicit_specialization_ = false;
	Scope* saved_class = declaration_class_;
	if (info.qualifier != nullptr && info.qualifier->kind == ScopeKind::Class && scope->kind == ScopeKind::Namespace)
		declaration_class_ = info.qualifier;
	VariableEntity* templ = named_variable_template(*info.name, scope);
	const TemplateHead* head = templ->templ->head;
	TypeList given;
	template_arguments(info.name->last(), scope, given, head);
	declaration_class_ = saved_class;
	default_template_arguments(head, given, location);
	VariableEntity* variable = variable_specialization(templ, given, location, true);
	VariableTemplateFacts* facts = variable->templ;
	// Its first declaration gives its type: declared, or deduced from its
	// own initializer (7.1.6.4p6; a constexpr object is const, 7.1.5p9),
	// and whether it is constexpr (not the template's).
	if (variable->type == nullptr)
		variable->is_constexpr = specifiers.is_constexpr;
	if (variable->type == nullptr && holds_auto(info.type))
		variable->type = auto_variable_type(variable, specifiers, info.type, initializer, scope, location);
	else if (variable->type == nullptr)
		facts->binding->type = variable->type = specifiers.is_constexpr ? types_.qualified(info.type, kConst)
																		: info.type;
	// 14.7.3p13: a static data member template's explicit specialization
	// without an initializer is a declaration (a later one may define it);
	// a namespace-scope variable template's is a definition.
	const bool member = templ->owner != nullptr && templ->owner->kind == ScopeKind::Class;
	const bool declared = member && facts->state == FactState::Done && !variable->defined;
	if (facts->state != FactState::NotStarted && !declared)
		error(location, "an explicit specialization after the specialization's instantiation or definition");
	if (declared && initializer == nullptr)
		return;
	facts->state = FactState::Done;
	facts->initializer = initializer;
	facts->scope = scope;
	facts->location = location;
	variable->defined = initializer != nullptr || !member;
	if (initializer != nullptr && semantic())
	{
		expression_scope_ = scope;
		initialize_specialization(variable, initializer, scope, location);
	}
}

void Analyzer::initialize_specialization(
	VariableEntity* variable, const syntax::Initializer* initializer, Scope* scope, uint32_t location)
{
	// A variable template specialization's initialization; one declared
	// with an omitted bound takes its initializer's (8.3.4p3, 8.5.1p4, a
	// braced list's by its typed initialization, as a variable's).
	const Type* type = complete_array_from_initializer(variable->type, initializer, true);
	initialize_variable(variable, type, initializer, scope, location);
	if (type->kind == TypeKind::Array && type->bound == 0 && type->owner == nullptr &&
		(initializer->kind == syntax::InitializerKind::Assign ||
			initializer->kind == syntax::InitializerKind::Braced) &&
		initializer->expression->kind == syntax::ExprKind::BracedInit && initializer->expression->facts != nullptr)
		type = initializer->expression->facts->type;
	if (type != variable->type)
		variable->templ->binding->type = variable->type = type;
	record_constant_initializer(variable, initializer, scope);
}

bool Analyzer::variable_partial_ordered(const VariableEntity* first, const VariableEntity* second)
{
	// 14.5.5.2: the first partial specialization at least as specialized as
	// the second.
	return partial_ordered(first, first->templ->head, first->templ->arguments, first->templ->argument_count, second,
		second->templ->head, second->templ->arguments, second->templ->argument_count);
}

bool Analyzer::instantiate_variable_specialization(VariableEntity* variable)
{
	// 14.7.1p1: a variable template's specialization defined where it is
	// needed: the matched partial specialization's definition (14.5.5.1)
	// or the template's, copied and analyzed once in the frame of its
	// parameters. A dependent one is never instantiated.
	VariableTemplateFacts* facts = variable->templ;
	if (facts->state != FactState::NotStarted)
		return facts->state == FactState::Done && variable->defined;
	for (uint32_t i = 0; i < facts->argument_count; ++i)
	{
		if (dependent_type(facts->arguments[i]))
			return false;
	}
	const VariableTemplateFacts* source = facts->pattern->templ;
	const Type* const* arguments = facts->arguments;
	uint32_t count = facts->argument_count;
	TypeList deduced;
	const VariableEntity* best = source->partials != nullptr || source->partial_positions != 0
		? matched_partial(facts->pattern, facts->arguments, facts->argument_count, deduced, facts->location)
		: nullptr;
	if (best != nullptr)
	{
		source = best->templ;
		arguments = deduced.data();
		count = static_cast<uint32_t>(deduced.size());
	}
	// An instance's static data member template defined outside the class
	// template: the class template's member template's definition, its
	// enclosing template-heads bound to the instance's enclosing
	// specializations' arguments (14.5.2p1, see outside_pattern_variable).
	const VariableEntity* outside = nullptr;
	if (best == nullptr && source->initializer == nullptr && !facts->pattern->defined)
	{
		outside = outside_pattern_variable(facts->pattern);
		while (outside != nullptr && !outside->defined)
			outside = outside_pattern_variable(outside);
		if (outside != nullptr && outside->templ != nullptr && outside->templ->head != nullptr)
			source = outside->templ;
		else
			outside = nullptr;
	}
	if (source->parameters == nullptr ||
		(source->initializer == nullptr &&
			!(best != nullptr            ? best
					: outside != nullptr ? outside
										 : facts->pattern)
				->defined))
		return false;
	facts->state = FactState::InProgress;
	// A class's static data member template defined outside it: its
	// initializer is in the class's scope (9.4.2p2), the template
	// parameters its definition's bridge kept binding over it (14.6.1p7).
	const bool qualified = outside == nullptr && best == nullptr && facts->pattern->owner != nullptr &&
		facts->pattern->owner->kind == ScopeKind::Class && source->scope != nullptr &&
		source->scope != source->parameters;
	Scope* frame = outside != nullptr
		? outside_frames(source->parameters, arguments, count,
			  static_cast<const ClassEntity*>(facts->pattern->owner->entity), facts->pattern->owner, source->scope)
		: qualified ? instantiation_frame(source->parameters, arguments, count, facts->pattern->owner, source->scope)
					: instantiation_frame(source->parameters, arguments, count, source->parameters->parent);
	FrameExpander expander(*this, frame);
	syntax::SyntaxCloner cloner(arena_, true, expander.get());
	const syntax::Initializer* initializer = cloner.initializer(source->initializer);
	{
		InstantiationContext context(*this, enter_instantiation(instantiation_depth_ + 1, source->location));
		// An auto declaration's specialization: its type deduced from the
		// copy (see variable_specialization).
		if (variable->type == nullptr)
		{
			if (source->auto_type != nullptr && initializer != nullptr)
			{
				Specifiers specifiers = Specifiers();
				specifiers.cv = source->auto_cv;
				specifiers.is_static = true;
				variable->type = auto_variable_type(
					variable, specifiers, source->auto_type, initializer, frame, source->location);
			}
			if (variable->type == nullptr)
				return false;
		}
		variable->defined = true;
		expression_scope_ = frame;
		if (initializer != nullptr)
			initialize_specialization(variable, initializer, frame, source->location);
		else
			default_initialize(variable, variable->type, source->location);
		check_destruction(variable->type, frame, source->location);
	}
	facts->initializer = initializer;
	facts->scope = frame;
	facts->location = source->location;
	facts->state = FactState::Done;
	++graph_.instantiated_variables;
	return true;
}

} // namespace sema
