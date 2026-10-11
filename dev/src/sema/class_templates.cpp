#include "sema/template_packs.h"

#include <cstring>
#include <string>

#include "lex/identifier_table.h"
#include "sema/constant_evaluator.h"
#include "syntax/ast_clone.h"

// Class templates (clause 14): template heads with merged default
// arguments, template-ids naming specializations, dependent member names,
// and the instantiation of a specialization's definition (14.7.1) when it
// is first required to be complete. A specialization's members are
// declared by analyzing a copy of its template's member-specification
// (syntax/ast_clone.h) in a frame binding the definition's parameter names
// to its arguments; member function bodies, named member classes and
// out-of-class member definitions wait for their own demand.
namespace sema {

size_t PlaceholderHash::operator()(const PlaceholderKey& key) const
{
	return (reinterpret_cast<uintptr_t>(key.qualifier) * 0x9E3779B97F4A7C15ull) ^ reinterpret_cast<uintptr_t>(key.name);
}

Analyzer::~Analyzer()
{
	for (InstantiationScratch* scratch : scratch_pool_)
		delete scratch;
}

Analyzer::InstantiationContext::InstantiationContext(
	Analyzer& analyzer, uint32_t depth, bool substitution, uint32_t limit)
	: a_(analyzer), limit_(analyzer.names_.limit()), dependent_uses_(analyzer.names_.dependent_uses()),
	  instantiation_depth_(analyzer.instantiation_depth_), body_(analyzer.body_), class_depth_(analyzer.class_depth_),
	  template_depth_(analyzer.template_depth_), template_list_(analyzer.template_list_),
	  template_scope_(analyzer.template_scope_), template_owner_(analyzer.template_owner_),
	  owner_specifiers_(analyzer.owner_specifiers_), placeholder_home_(analyzer.placeholder_home_),
	  namespace_friend_class_(analyzer.namespace_friend_class_), access_point_(analyzer.access_point_),
	  access_scope_(analyzer.access_scope_), suppress_implicit_(analyzer.suppress_implicit_),
	  current_access_(analyzer.current_access_), member_scope_(analyzer.member_scope_),
	  expression_scope_(analyzer.expression_scope_), naming_class_(analyzer.naming_class_),
	  last_prefix_(analyzer.last_prefix_), declaration_class_(analyzer.declaration_class_),
	  access_deferral_(analyzer.access_deferral_), base_object_(analyzer.base_object_),
	  c_language_(analyzer.c_language_), linkage_extern_(analyzer.linkage_extern_),
	  linkage_specified_(analyzer.linkage_specified_), pending_prototype_(analyzer.pending_prototype_),
	  demanding_(analyzer.demanding_), current_epoch_(analyzer.current_epoch_), initializing_(analyzer.initializing_),
	  unevaluated_(analyzer.unevaluated_), deferring_(analyzer.deferring_),
	  deferring_reserved_(analyzer.deferring_reserved_), template_arguments_(analyzer.template_arguments_),
	  callee_name_(analyzer.callee_name_), soft_substitution_(analyzer.soft_substitution_), failure_(analyzer.failure_),
	  substitution_(substitution), direct_class_(analyzer.direct_class_), standard_only_(analyzer.standard_only_),
	  explicit_specialization_(analyzer.explicit_specialization_), lexical_limit_(analyzer.lexical_limit_)
{
	a_.instantiation_depth_ = depth;
	a_.body_ = nullptr;
	a_.class_depth_ = 0;
	a_.template_depth_ = 0;
	a_.template_list_ = nullptr;
	a_.template_scope_ = nullptr;
	a_.template_owner_ = nullptr;
	a_.owner_specifiers_ = false;
	a_.placeholder_home_ = nullptr;
	a_.namespace_friend_class_ = nullptr;
	a_.access_point_ = nullptr;
	a_.access_scope_ = nullptr;
	a_.suppress_implicit_ = false;
	a_.access_deferral_ = nullptr;
	a_.declaration_class_ = nullptr;
	a_.base_object_ = nullptr;
	a_.c_language_ = false;
	a_.linkage_extern_ = false;
	a_.linkage_specified_ = false;
	a_.pending_prototype_ = PendingPrototype{ nullptr, nullptr };
	a_.demanding_ = nullptr;
	a_.current_epoch_ = a_.root_epoch_;
	a_.initializing_ = nullptr;
	a_.unevaluated_ = 0;
	a_.deferring_ = 0;
	// An instantiation is no hosted wrapper's own statement: a reserved
	// builtin it calls is an error where it is instantiated (see
	// Analyzer::define_function).
	a_.deferring_reserved_ = 0;
	a_.template_arguments_ = 0;
	a_.callee_name_ = false;
	a_.soft_substitution_ = substitution ? soft_substitution_ : 0;
	a_.failure_ = SubstitutionFailure();
	a_.direct_class_ = nullptr;
	a_.standard_only_ = false;
	a_.explicit_specialization_ = false;
	a_.lexical_limit_ = false;
	// Its lookups see the definition context's namespace-scope names
	// (14.6.4.2), or all (a definition point not recorded: none).
	a_.names_.set_limit(limit != 0 ? limit : NameLookup::kNoLimit);
	if (a_.scratch_pool_.size() <= a_.scratch_level_)
		a_.scratch_pool_.push_back(new InstantiationScratch());
	scratch_ = a_.scratch_pool_[a_.scratch_level_++];
	swap_scratch();
}

Analyzer::InstantiationContext::~InstantiationContext()
{
	swap_scratch();
	--a_.scratch_level_;
	a_.instantiation_depth_ = instantiation_depth_;
	a_.body_ = body_;
	a_.class_depth_ = class_depth_;
	a_.template_depth_ = template_depth_;
	a_.template_list_ = template_list_;
	a_.template_scope_ = template_scope_;
	a_.template_owner_ = template_owner_;
	a_.owner_specifiers_ = owner_specifiers_;
	a_.placeholder_home_ = placeholder_home_;
	a_.namespace_friend_class_ = namespace_friend_class_;
	a_.access_point_ = access_point_;
	a_.access_scope_ = access_scope_;
	a_.suppress_implicit_ = suppress_implicit_;
	a_.current_access_ = current_access_;
	a_.member_scope_ = member_scope_;
	a_.expression_scope_ = expression_scope_;
	a_.naming_class_ = naming_class_;
	a_.last_prefix_ = last_prefix_;
	a_.declaration_class_ = declaration_class_;
	a_.access_deferral_ = access_deferral_;
	a_.base_object_ = base_object_;
	a_.c_language_ = c_language_;
	a_.linkage_extern_ = linkage_extern_;
	a_.linkage_specified_ = linkage_specified_;
	a_.pending_prototype_ = pending_prototype_;
	a_.demanding_ = demanding_;
	a_.current_epoch_ = current_epoch_;
	a_.initializing_ = initializing_;
	a_.unevaluated_ = unevaluated_;
	a_.deferring_ = deferring_;
	a_.deferring_reserved_ = deferring_reserved_;
	a_.template_arguments_ = template_arguments_;
	a_.callee_name_ = callee_name_;
	a_.soft_substitution_ = soft_substitution_;
	// A substitution's failure inside is the substitution's.
	if (!substitution_ || !a_.failed())
		a_.failure_ = failure_;
	a_.direct_class_ = direct_class_;
	a_.standard_only_ = standard_only_;
	a_.explicit_specialization_ = explicit_specialization_;
	a_.lexical_limit_ = lexical_limit_;
	a_.names_.set_limit(limit_);
	a_.names_.set_dependent_uses(dependent_uses_);
}

void Analyzer::InstantiationContext::swap_scratch()
{
	scratch_->candidates.swap(a_.candidates_);
	scratch_->overload_set.swap(a_.overload_set_);
	scratch_->sequences.swap(a_.sequences_);
	scratch_->argument_scratch.swap(a_.argument_scratch_);
	scratch_->operator_set.swap(a_.operator_set_);
	scratch_->operand_options.swap(a_.operand_options_);
	scratch_->converter_scratch.swap(a_.converter_scratch_);
	scratch_->adl_functions.swap(a_.adl_functions_);
	scratch_->adl_namespaces.swap(a_.adl_namespaces_);
	scratch_->adl_enclosing.swap(a_.adl_enclosing_);
	scratch_->adl_pending.swap(a_.adl_pending_);
	scratch_->adl_classes.swap(a_.adl_classes_);
	scratch_->binary_spine.swap(a_.binary_spine_);
}

uint32_t Analyzer::enter_instantiation(uint32_t depth, uint32_t location) const
{
	// Annex B: at least 1024 recursively nested instantiations (a class's
	// and its members' bodies count one level each here).
	if (depth > kInstantiationDepthLimit)
		error(location, "template instantiation nested too deeply");
	return depth;
}

// ------------------------------------------------------------ parameters

TemplateParameterEntity* Analyzer::positional_parameter(uint32_t depth, uint32_t index, bool value, bool pack)
{
	// One per (depth, index, kind): a type and a non-type parameter at a
	// position are different parameters (14.5.6.1p6 compares kinds).
	const uint32_t row = depth * 4 + (value ? 1 : 0) + (pack ? 2 : 0);
	if (positional_.size() <= row)
		positional_.resize(row + 1);
	std::vector<TemplateParameterEntity*>& level = positional_[row];
	while (level.size() <= index)
	{
		TemplateParameterEntity* entity = new_entity<TemplateParameterEntity>(
			EntityKind::TemplateParameter, nullptr, nullptr, nullptr);
		entity->depth = static_cast<uint16_t>(depth);
		entity->index = static_cast<uint16_t>(level.size());
		entity->non_type = value;
		entity->pack = pack;
		entity->canonical = entity;
		entity->type = types_.named(TypeKind::TemplateParameter, entity);
		level.push_back(entity);
	}
	return level[index];
}

ClassTemplateFacts* Analyzer::class_template_facts(ClassEntity* entity, TemplateRole role)
{
	if (entity->templ != nullptr)
		return entity->templ;
	ClassTemplateFacts* facts = static_cast<ClassTemplateFacts*>(
		arena_.allocate(sizeof(ClassTemplateFacts), alignof(ClassTemplateFacts)));
	std::memset(static_cast<void*>(facts), 0, sizeof(ClassTemplateFacts));
	facts->role = role;
	facts->depth = instantiation_depth_;
	entity->templ = facts;
	return facts;
}

TemplateHead* Analyzer::template_head(
	const syntax::TemplateParameterList& list, Scope* scope, const std::vector<TemplateParameterEntity*>& parameters)
{
	TemplateHead* head = static_cast<TemplateHead*>(arena_.allocate(sizeof(TemplateHead), alignof(TemplateHead)));
	head->count = static_cast<uint32_t>(parameters.size());
	head->parameters = arena_.copy_array(parameters.data(), parameters.size());
	head->defaults = arena_.allocate_array<const syntax::TypeId*>(parameters.size());
	head->default_values = arena_.allocate_array<const syntax::Expr*>(parameters.size());
	head->default_scopes = arena_.allocate_array<Scope*>(parameters.size());
	head->argument_defaults = nullptr;
	head->shape = nullptr;
	head->trailing_pack = head->count != 0 && parameters.back()->pack ? head->count - 1 : head->count;
	head->ordinal = definition_point();
	// The first declaration's defaults come with the template wherever it
	// is found (a member of a dependent type's class or by argument-
	// dependent lookup, declared after a template's definition context);
	// only a redeclaration's are limited to where it is (14.1p10, see
	// merge_template_head).
	for (uint32_t i = 0; i < head->count; ++i)
	{
		head->defaults[i] = list.parameters[i]->default_type;
		head->default_values[i] = list.parameters[i]->default_value;
		head->default_scopes[i] = HasDefault(head, i) ? scope : nullptr;
	}
	return head;
}

void Analyzer::default_ordinal(const TemplateHead* head, uint32_t index, uint32_t ordinal)
{
	// The heads with a redeclaration's defaults have their parameters'
	// ordinals in default_ordinals_ (0 for none or the first
	// declaration's).
	uint32_t first = default_ordinal_index_.find(head);
	if (first == PointerMap::kMissing)
	{
		first = static_cast<uint32_t>(default_ordinals_.size());
		default_ordinal_index_.set(head, first);
		default_ordinals_.resize(default_ordinals_.size() + head->count, 0);
	}
	default_ordinals_[first + index] = ordinal;
}

void Analyzer::merge_template_head(
	TemplateHead* head, const syntax::TemplateParameterList& list, Scope* scope, uint32_t location)
{
	// 14.1p10: a redeclaration's defaults join the earlier ones; p12: a
	// parameter gets a default once. 14.5.6.1: the lists agree in length
	// and in each parameter's kind.
	if (list.parameters.size != head->count)
		error(location, "template redeclared with a different template parameter list");
	for (uint32_t i = 0; i < head->count; ++i)
	{
		const syntax::TemplateParameter* parameter = list.parameters[i];
		const TemplateParameterEntity* earlier = head->parameters[i];
		const bool non_type = parameter->kind == syntax::TemplateParameterKind::NonType;
		if (non_type != earlier->non_type || TemplateParameterPack(*parameter) != earlier->pack ||
			(parameter->kind == syntax::TemplateParameterKind::Template) != earlier->template_template)
			error(parameter->location, "template redeclared with a different template parameter list");
		// Template template-parameters of equivalent lists and non-type
		// parameters of equivalent types (14.5.6.1p6, 14.1), whatever
		// depth each declaration has (a friend's, 14.5.4).
		const TemplateParameterEntity* again = template_entities_.size() == head->count ? template_entities_[i]
																						: nullptr;
		if (again != nullptr &&
			(!template_parameters_match(again, earlier) || !template_parameters_match(earlier, again)))
			error(parameter->location, "template redeclared with a different template parameter list");
		if (parameter->default_type == nullptr && parameter->default_value == nullptr)
			continue;
		if (HasDefault(head, i))
			error(parameter->location, "default template argument redefined");
		head->defaults[i] = parameter->default_type;
		head->default_values[i] = parameter->default_value;
		head->default_scopes[i] = scope;
		default_ordinal(head, i, definition_point());
	}
}

void Analyzer::check_template_defaults(const TemplateHead* head, uint32_t location)
{
	// 14.1p11: a class template parameter with a default is followed only
	// by parameters with defaults, and a parameter pack only ends the
	// list.
	bool defaulted = false;
	for (uint32_t i = 0; i < head->count; ++i)
	{
		if (head->parameters[i]->pack && i + 1 != head->count)
			error(location, "a class template parameter pack is not last");
		if (HasDefault(head, i))
			defaulted = true;
		else if (defaulted && !head->parameters[i]->pack)
			error(location, "a template parameter without a default follows one with a default");
	}
}

void Analyzer::declare_class_template(ClassEntity* entity, bool existed, uint32_t location)
{
	// The class is the templated entity of the template declaration being
	// analyzed (declared with its parameter scope).
	ClassTemplateFacts* facts = entity->templ;
	if (existed && (facts == nullptr || facts->role != TemplateRole::Template))
		error(location, "a class template redeclares a class that is not a template");
	facts = class_template_facts(entity, TemplateRole::Template);
	if (facts->head == nullptr)
		facts->head = template_head(*template_list_, template_scope_, template_entities_);
	else
	{
		// A friend declaration in a templated class that declared it first
		// (14.5.4p1) gave parameters a level deeper than its own: a
		// declaration of its namespace's level supplies them (its own
		// parameters as arguments name it, 14.6.2.1p1).
		TemplateHead* head = facts->head;
		merge_template_head(head, *template_list_, template_scope_, location);
		if (head->count != 0 && head->count == template_entities_.size() &&
			template_entities_[0]->depth < head->parameters[0]->depth)
			head->parameters = arena_.copy_array(template_entities_.data(), template_entities_.size());
	}
	check_template_defaults(facts->head, location);
}

bool Analyzer::templated_class(const Scope* scope)
{
	// A class scope whose class is a template or a member of one (its
	// members are patterns too).
	if (scope == nullptr || scope->kind != ScopeKind::Class || scope->entity == nullptr)
		return false;
	const ClassTemplateFacts* facts = static_cast<const ClassEntity*>(scope->entity)->templ;
	return facts != nullptr &&
		(facts->role == TemplateRole::Template || facts->role == TemplateRole::Pattern ||
			facts->role == TemplateRole::Partial);
}

bool Analyzer::has_dependent_bases(const Scope* scope)
{
	// A templated class whose definition names a dependent base.
	if (!templated_class(scope))
		return false;
	const syntax::ClassSpecifier* definition = static_cast<const ClassEntity*>(scope->entity)->templ->definition;
	for (uint32_t i = 0; definition != nullptr && i < definition->bases.size; ++i)
	{
		if (definition->bases[i].dependent)
			return true;
	}
	return false;
}

bool Analyzer::instance_class(const Scope* scope)
{
	// A class whose members are instantiated from a pattern's: a
	// specialization (not an explicit one, whose members are its own) or
	// a member class of one.
	if (scope == nullptr || scope->kind != ScopeKind::Class || scope->entity == nullptr)
		return false;
	const ClassTemplateFacts* facts = static_cast<const ClassEntity*>(scope->entity)->templ;
	return facts != nullptr && !facts->explicit_specialization &&
		(facts->role == TemplateRole::Specialization || facts->role == TemplateRole::MemberInstance);
}

bool Analyzer::WithinInstance(const Scope* scope)
{
	// A class scope that is, or is nested in, an instance class (a member
	// class template of one, say): its members were copied from a complete
	// template definition. An explicit specialization's members, and those
	// of classes nested in it, are its own (14.7.3p5).
	for (; scope != nullptr && scope->kind == ScopeKind::Class && scope->entity != nullptr;
		scope = static_cast<const ClassEntity*>(scope->entity)->owner)
	{
		const ClassTemplateFacts* facts = static_cast<const ClassEntity*>(scope->entity)->templ;
		if (facts != nullptr && facts->explicit_specialization)
			return false;
		if (instance_class(scope))
			return true;
	}
	return false;
}

bool Analyzer::InstanceMemberTemplate(const Scope* scope)
{
	// A scope of a member template's declaration (its template parameters,
	// its prototype) in an instance class: the instantiation substituted
	// the enclosing classes' arguments, so a name that depends on none of
	// the member template's own parameters is no longer dependent.
	for (; scope != nullptr && scope->kind != ScopeKind::Class; scope = scope->parent)
	{
		if (scope->kind != ScopeKind::TemplateParameters && scope->kind != ScopeKind::Function)
			return false;
	}
	return scope != nullptr && WithinInstance(scope);
}

void Analyzer::define_pattern(ClassEntity* entity, const syntax::ClassSpecifier& specifier, Scope* parameters)
{
	// A templated class's definition, which its instances' definitions
	// copy: `parameters` is the template parameter scope whose names it
	// uses (null for a member class defined in its templated class).
	ClassTemplateFacts* facts = class_template_facts(entity, TemplateRole::Pattern);
	facts->definition = &specifier;
	facts->definition_parameters = parameters;
}

ClassEntity* Analyzer::class_template_of(const Entity* entity)
{
	// The template a class name denotes when a template argument list
	// follows it: a class template's name, or the injected-class-name of
	// the template or one of its specializations (14.6.1p1).
	if (entity == nullptr || entity->kind != EntityKind::Class)
		return nullptr;
	const ClassEntity* cls = static_cast<const ClassEntity*>(entity);
	if (cls->templ == nullptr)
		return nullptr;
	if (cls->templ->role == TemplateRole::Template)
		return const_cast<ClassEntity*>(cls);
	if (cls->templ->role == TemplateRole::Specialization || cls->templ->role == TemplateRole::Partial)
		return cls->templ->pattern;
	return nullptr;
}

// ------------------------------------------------------------ arguments

bool Analyzer::template_arguments(
	const syntax::NameComponent& component, Scope* scope, TypeList& out, const TemplateHead* head)
{
	// 14.3: each argument a type (14.3.1) or an integral constant value
	// (14.3.2), dependent ones kept as such. With the template's head, a
	// value argument knows its parameter's type when that depends on no
	// parameter or only on earlier ones given (a class object's converted
	// constant, 5.19p3; a reference parameter's object, a pointer
	// parameter's overload set, 14.3.2p1, p5).
	out.clear();
	for (const syntax::TemplateArgument& argument : component.arguments)
	{
		// A type-id's pack declarator (`T...`) makes its type an expansion.
		const uint32_t index = static_cast<uint32_t>(out.size());
		const TemplateParameterEntity* parameter = head == nullptr || head->count == 0
			? nullptr
			: head->parameters[index < head->count ? index : head->count - 1];
		if (parameter != nullptr && index >= head->count && !parameter->pack)
			parameter = nullptr;
		const Type* target = parameter == nullptr || parameter->value_type == nullptr ? nullptr
			: index < head->count && !parameter->pack ? argument_target(head, index, out.data())
			: !dependent_type(parameter->value_type)  ? parameter->value_type
													  : nullptr;
		// A value's parameter type unknown (several function templates, a
		// type naming a pack) until its conversion (see template_value).
		const bool unknown = target == nullptr && (parameter == nullptr || parameter->value_type != nullptr);
		// A template template-argument names a template (14.3.3p1); for an
		// unknown parameter, a name that denotes only a template (no
		// injected-class-name) is one too.
		bool expansion = false;
		const syntax::Name* name = parameter != nullptr && !parameter->template_template
			? nullptr
			: PlainTemplateName(argument, &expansion);
		const Type* templ = name != nullptr ? template_name_argument(*name, scope, parameter != nullptr) : nullptr;
		if (failed())
			return false;
		if (templ != nullptr && expansion)
			out.push_back(pack_expansion_type(templ, component.location));
		else if (templ != nullptr)
			out.push_back(templ);
		else if (parameter != nullptr && parameter->template_template && !expansion)
			error(component.location, "a template template-argument that names no template");
		else if (argument.pack)
		{
			expanded_argument(argument, scope, component.location, out);
			if (failed())
				return false;
		}
		else
		{
			// A type-id's pack expansion of a pack a frame binds is its
			// elements (see pack_expansion_type); a pack named without one
			// is an outer expansion's pattern, expanded with it.
			const Type* type = template_argument(argument, scope, component.location, target, unknown);
			if (type == nullptr)
				return false;
			const syntax::Declarator* declarator = argument.type != nullptr ? argument.type->declarator : nullptr;
			if (type->kind == TypeKind::Pack && declarator != nullptr && declarator->pack != syntax::PackPosition::None)
				flattened_arguments(&type, 1, out);
			else if (type->kind == TypeKind::Expansion && declarator != nullptr &&
				declarator->pack != syntax::PackPosition::None)
				bound_expansion(type, out);
			else
				out.push_back(type);
		}
	}
	return true;
}

const Type* Analyzer::template_argument(
	const syntax::TemplateArgument& argument, Scope* scope, uint32_t location, const Type* target, bool unknown)
{
	// A type-id (14.3p2 resolved an ambiguous argument to one), an
	// id-expression naming a type, or else an expression: a value. 14.6p2:
	// a qualified name that depends on a template parameter names a type
	// only after `typename` (or after `template` a template, 14.2p4); so
	// written, it is a value. Null for a substitution's failure.
	const Type* type = nullptr;
	if (argument.type != nullptr)
	{
		type = type_id(*argument.type, scope);
		if (type == nullptr)
			return nullptr;
	}
	else if (argument.expression->kind == syntax::ExprKind::Id)
	{
		const syntax::Name& name = *static_cast<const syntax::IdExpr*>(argument.expression)->name;
		const LookupResult result = lookup_name(name, scope, LookupFilter::Ordinary);
		if (failed())
			return nullptr;
		const BindingKind kind = result.binding->kind;
		// A non-type template parameter of the definition: its dependent
		// value.
		if (kind == BindingKind::TemplateValue)
			return result.binding->entity->type;
		const bool dependent = name.qualified() && !name.last().template_keyword &&
			(last_prefix_.dependent != nullptr || IsDependentMember(result.binding));
		// 14.6p6: a name the definition assumed to name no type (a copy of
		// the name keeps the fact) names none in its instantiations.
		if (dependent)
			name.dependent_qualifier = true;
		else if (name.dependent_qualifier && (kind == BindingKind::Type || kind == BindingKind::TypeAlias))
			error(name.location, "a dependent qualified name names a type only with typename");
		if ((kind != BindingKind::Type && kind != BindingKind::TypeAlias) || dependent)
			return template_value(*argument.expression, scope, target, unknown);
		type = binding_type(result.binding, name.location);
	}
	else
		return template_value(*argument.expression, scope, target, unknown);
	if (type == auto_placeholder_)
		error(location, "auto as a template argument");
	return type;
}

const Type* Analyzer::template_value(const syntax::Expr& expr, Scope* scope, const Type* target, bool unknown)
{
	// 14.3.2p1: an integral constant expression, its value converted to its
	// parameter's type where the specialization is formed (p5); a template
	// definition's value-dependent one (14.6.2.3) is a placeholder until
	// substituted. Semantics mode types it first (overloaded operators and
	// calls in unevaluated operands, constexpr calls); an object of class
	// type converts to the parameter's type `target`, when known, by its
	// constexpr conversion function (5.19p3).
	if (semantic() && template_depth_ == 0)
	{
		// A pointer or reference parameter's argument (14.3.2p1), a
		// set of overloaded functions resolved by its type; or one of
		// neither integral nor class type before its parameter is known (an
		// integral lvalue is then read as its value).
		const bool address = target != nullptr && AddressParameterType(target);
		const uint64_t values = names_.value_parameter_uses();
		++template_arguments_;
		constant_operand(expr, scope, !address);
		--template_arguments_;
		if (failed())
			return nullptr;
		// A value naming a template parameter (an instantiation's frame
		// binds it): the template-id is a dependent type in the template
		// (14.6.2.1p7), so a call it is an argument of is dependent there
		// (14.6.2p1, its argument-dependent lookup seeing the
		// instantiation's context, 14.6.4.2).
		if (names_.value_parameter_uses() != values)
			names_.note_dependent_use();
		const ExprFacts* facts = expr.facts;
		if (address ||
			(target == nullptr && facts != nullptr && facts->type != nullptr && !IsIntegralType(facts->type) &&
				facts->type->unqualified->kind != TypeKind::Enum && ClassOf(facts->type) == nullptr))
			return address_argument(expr, target);
		// An explicit argument whose parameter is not known yet (one of
		// several function templates') naming an integral or enumeration
		// object with linkage designates it: a reference parameter binds
		// it, an integral one reads its value when the argument is
		// converted (converted_value, 14.3.2p1, p5).
		if (unknown && NamesLinkedObject(expr))
			return address_argument(expr, nullptr);
		// Evaluated before its parameter is known (a value-dependent
		// argument's substitution): a class object converts by its one
		// conversion function to an integral or enumeration type, the
		// result converted to the parameter where the specialization is
		// formed (14.3.2p5).
		const ClassEntity* cls = expr.facts != nullptr && expr.facts->type != nullptr ? ClassOf(expr.facts->type)
																					  : nullptr;
		if (target == nullptr && cls != nullptr)
		{
			const FunctionEntity* conversion = unique_conversion(cls, IsIntegralOrUnscopedEnum);
			const Type* result = conversion != nullptr ? conversion->type->canonical->element : nullptr;
			if (result != nullptr)
				target = result->is_reference() ? result->element->unqualified : result->unqualified;
		}
		if (target != nullptr && cls != nullptr)
		{
			BodyContext* saved = body_;
			BodyContext context = BodyContext();
			if (body_ == nullptr)
				body_ = &context;
			convert(expr, target, false);
			body_ = saved;
			Constant converted;
			if (!converted_constant(expr, converted, soft_substitution_ == 0))
			{
				substitution_error(expr.location, "a template argument that is not a constant expression");
				return nullptr;
			}
			return types_.value(converted.type->unqualified, converted.bits);
		}
	}
	// In a substitution, an argument that is no constant expression is its
	// failure (14.8.2p8).
	Constant value;
	if (constant_value(expr, scope, value, template_depth_ == 0 && soft_substitution_ == 0))
	{
		IntegralKind kind;
		if (!IntegralKindOf(value.type, kind))
			error(expr.location, "a non-type template argument of non-integral type is not supported here");
		return types_.value(value.type->unqualified, value.bits);
	}
	if (template_depth_ == 0)
	{
		substitution_error(expr.location, "a template argument that is not a constant expression");
		return nullptr;
	}
	// 14.6p8, p10: an argument that names nothing dependent and is not
	// type-dependent is evaluated where it appears (see
	// dependent_expression).
	const Type* type = dependent_expression(
		expr, scope, true, target != nullptr && AddressParameterType(target) ? target : nullptr, semantic());
	if (type == nullptr || type->kind != TypeKind::TemplateParameter)
		return type;
	TemplateParameterEntity* placeholder = static_cast<TemplateParameterEntity*>(const_cast<Entity*>(type->entity));
	if (!placeholder->fixed)
		return type;
	if (placeholder->fixed_value == nullptr || placeholder->fixed_target != target || unknown)
	{
		const uint32_t depth = template_depth_;
		template_depth_ = 0;
		const Type* value = template_value(expr, scope, target, unknown);
		template_depth_ = depth;
		if (value == nullptr)
			return nullptr;
		placeholder->fixed_value = value;
		placeholder->fixed_target = target;
	}
	return placeholder->fixed_value;
}

const Type* Analyzer::argument_target(const TemplateHead* head, uint32_t index, const Type* const* earlier)
{
	// The type non-type parameter `index`'s argument converts to (14.3.2p5)
	// where it is known: its declared type, or one naming earlier
	// parameters (`template<class T, T& R>`) substituted by their
	// arguments `earlier` when none follows a pack or is an expansion or
	// dependent; else null.
	const TemplateParameterEntity* parameter = head->parameters[index];
	const Type* type = parameter->value_type;
	if (type == nullptr || !dependent_type(type))
		return type;
	for (uint32_t i = 0; i < index; ++i)
	{
		if (head->parameters[i]->pack || earlier[i]->kind == TypeKind::Expansion || dependent_type(earlier[i]))
			return nullptr;
	}
	type = substitute(type, Substitution{ parameter->depth, earlier, index });
	return type != nullptr && !dependent_type(type) ? type : nullptr;
}

bool Analyzer::value_argument(const Type* argument)
{
	return argument->kind == TypeKind::Value ||
		((argument->kind == TypeKind::TemplateParameter || argument->kind == TypeKind::BoundPack) &&
			static_cast<const TemplateParameterEntity*>(argument->entity)->non_type);
}

bool Analyzer::match_template_arguments(const TemplateHead* head, const Type* const* given, uint32_t count,
	TypeList& out, bool fatal, uint32_t location, bool partial)
{
	// 14.3p1: each argument of its parameter's kind (14.3.1, 14.3.2), a
	// value converted to its parameter's type (14.3.2p5; a type naming an
	// earlier parameter, `template<class T, T v>`, substituted with the
	// arguments before it); a parameter pack takes an argument pack of the
	// arguments left (14.5.3p1, 14.8.1p3), or one a deduction
	// formed. The result is canonical, one argument per parameter. A pack
	// expansion given for a parameter that is no pack leaves the positions
	// unknown until substituted: the list stays as given. `partial`: an
	// explicit argument list, whose pack given no argument is left out
	// for deduction (14.8.1p9). False (or an error when `fatal`) for a
	// mismatch.
	out.clear();
	uint32_t next = 0;
	for (uint32_t i = 0; i < head->count && (next < count || head->parameters[i]->pack); ++i)
	{
		const TemplateParameterEntity* parameter = head->parameters[i];
		if (parameter->pack)
		{
			if (partial && next == count)
				break;
			TypeList elements;
			const bool formed = next < count && given[next]->kind == TypeKind::Pack;
			const Type* const* from = formed ? given[next]->params : given + next;
			const uint32_t size = formed ? given[next]->param_count : count - next;
			next = formed ? next + 1 : count;
			// An expanded pack (14.1p15: a non-type pack whose type names
			// packs an enclosing instance binds, `Ts... Vs`): each value
			// converts to its position's type (14.3.2p5).
			const Type* pattern = parameter->value_type != nullptr && dependent_type(parameter->value_type)
				? parameter->value_type
				: nullptr;
			if (pattern != nullptr && pattern->kind == TypeKind::Expansion)
				pattern = pattern->element;
			for (uint32_t j = 0; j < size; ++j)
			{
				const Type* target = pattern != nullptr
					? substitute(pattern,
						  Substitution{ parameter->depth, out.data(), static_cast<uint32_t>(out.size()), nullptr, &j })
					: nullptr;
				if (pattern != nullptr && target == nullptr)
					return false;
				const Type* element = matched_element(parameter, from[j], out, fatal, location, target);
				if (element == nullptr)
					return false;
				elements.push_back(element);
			}
			out.push_back(types_.pack(elements.data(), static_cast<uint32_t>(elements.size())));
			continue;
		}
		if (given[next]->kind == TypeKind::Expansion)
		{
			// The expansion's pattern is of the parameter's kind.
			if (value_argument(given[next]->element) != (parameter->value_type != nullptr))
			{
				if (fatal)
					error(location, "a pack expansion of the wrong kind for its template parameter");
				return false;
			}
			out.clear();
			for (uint32_t j = 0; j < count; ++j)
				out.push_back(given[j]->canonical);
			return true;
		}
		const Type* argument = matched_element(parameter, given[next++], out, fatal, location);
		if (argument == nullptr)
			return false;
		out.push_back(argument);
	}
	if (next < count)
	{
		if (fatal)
			error(location, "too many template arguments");
		return false;
	}
	return true;
}

const Type* Analyzer::matched_element(const TemplateParameterEntity* parameter, const Type* argument,
	const TypeList& earlier, bool fatal, uint32_t location, const Type* element_type)
{
	// One argument of its parameter's kind, a value converted; an expansion
	// of a dependent pack as it is (its pattern of the parameter's kind).
	argument = argument->canonical;
	const Type* element = argument->kind == TypeKind::Expansion ? argument->element : argument;
	const bool value = value_argument(element);
	const bool templ = element->kind == TypeKind::Template || element->kind == TypeKind::TemplateTemplateParameter ||
		(element->kind == TypeKind::BoundPack &&
			static_cast<const TemplateParameterEntity*>(element->entity)->template_template);
	if (value != (parameter->value_type != nullptr) || templ != parameter->template_template)
	{
		if (fatal)
			error(location,
				templ ? "a template given for a template parameter that is no template template-parameter"
					: parameter->template_template ? "a template template-parameter's argument that is no template"
					: value                        ? "a value given for a type template parameter"
												   : "a type given for a non-type template parameter");
		return nullptr;
	}
	if (argument->kind == TypeKind::Expansion)
		return argument;
	// 14.3.3p3: the argument template's parameters match the template
	// template-parameter's.
	const TemplateHead* own = templ ? TemplateHeadOf(argument) : nullptr;
	if (own != nullptr && parameter->template_head != nullptr && !template_heads_match(parameter->template_head, own))
	{
		if (fatal)
			error(location, "a template template-argument whose template parameters do not match");
		return nullptr;
	}
	return argument->kind == TypeKind::Value
		? converted_value(parameter, argument, earlier, fatal, location, element_type)
		: argument;
}

const Type* Analyzer::converted_value(const TemplateParameterEntity* parameter, const Type* argument,
	const TypeList& earlier, bool fatal, uint32_t location, const Type* element)
{
	// 14.3.2p5: the conversions of a converted constant expression (5.19p3):
	// integral promotions and integral conversions that do not narrow, or
	// the same enumeration.
	const Type* target = element != nullptr ? element : parameter->value_type;
	if (dependent_type(target))
	{
		target = substitute(
			target, Substitution{ parameter->depth, earlier.data(), static_cast<uint32_t>(earlier.size()) });
		if (target == nullptr)
			return nullptr;
		if (dependent_type(target))
			return argument;
	}
	target = target->unqualified;
	// An object designated before its parameter was known (see
	// template_value), for an integral or enumeration parameter: its value
	// when it is usable in constant expressions (5.19p2-p3).
	if (AddressArgument(argument) && argument->element->is_reference() &&
		argument->entity->kind == EntityKind::Variable && (IsIntegralType(target) || target->kind == TypeKind::Enum))
	{
		Constant value;
		if (!variable_constant(static_cast<const VariableEntity*>(argument->entity), value))
		{
			if (fatal)
				error(location, "a template argument that is not a constant expression");
			return nullptr;
		}
		argument = types_.value(value.type->unqualified, value.bits);
	}
	if (AddressParameterType(target) || AddressArgument(argument))
	{
		if (IsIntegralType(target) || target->kind == TypeKind::Enum || AddressParameterType(target))
			return converted_address(target, argument, fatal, location);
		if (fatal)
			error(location, "a non-type template parameter of a type 14.1p4 does not allow");
		return nullptr;
	}
	const Type* source = argument->element;
	const char* failure = nullptr;
	IntegralKind kind;
	if (target->kind == TypeKind::Enum && source != target)
		failure = "a template argument does not convert to the parameter's enumeration type";
	else if (IsScopedEnumeration(source) && source != target)
		failure = "a scoped enumeration template argument does not convert to an integer";
	else if (!IntegralKindOf(target, kind))
		failure = "a non-type template parameter of incomplete enumeration type";
	else if (!Representable(Constant{ argument->bound, source }, kind))
		failure = "a narrowing conversion of a template argument";
	else if (!HeldAsWideConstant(Constant{ argument->bound, source }, kind))
		failure = "a 128-bit template argument beyond the 64-bit constants evaluated";
	if (failure != nullptr)
	{
		if (fatal)
			error(location, failure);
		return nullptr;
	}
	return types_.value(target, ConvertIntegral(argument->bound, kind));
}

const Type* Analyzer::template_default(
	const TemplateHead* head, const Type* const* arguments, uint32_t index, bool substitution, Scope* named_in)
{
	// 14.1p9: parameter `index`'s default argument, analyzed with the
	// earlier parameters bound to their arguments (14.1p14), in the context
	// of the template declaration that gave it. For arguments that do not
	// depend on a template parameter that is an instantiation: the analysis
	// it interrupts (a template-id, an overload resolution deducing a
	// candidate, 14.8.2p5) keeps its state and scratch from the default's
	// own calls, and the default's expressions are copied (their facts are
	// the instantiation's). A dependent one (in a template definition) stays
	// dependent. `substitution`: a deduction's, whose failure is a result
	// (14.8.2p5): null then. `named_in`: a template template-parameter's own
	// list's, the scope where the parameter's name was found (its template's
	// parameter scope, or an instantiation's frame binding that template's
	// parameters), which a template definition's makes dependent.
	Scope* parameters = head->default_scopes[index];
	// 14.1p10: the defaults available are those of the declarations in
	// scope; an instantiation's template-id sees its definition's context
	// (14.6.4.2), without the defaults a later declaration gave. The first
	// declaration's are available wherever the template is found (see
	// template_head). Either way the default's names are looked up in the
	// context of the declaration that gave it: `ordinal`.
	const uint32_t first = default_ordinal_index_.find(head);
	const uint32_t added = first != PointerMap::kMissing ? default_ordinals_[first + index] : 0;
	const uint32_t ordinal = added != 0 ? added : head->ordinal;
	if (names_.limit() != NameLookup::kNoLimit && added > names_.limit())
	{
		substitution_error(
			head->defaults[index] != nullptr ? head->defaults[index]->location : head->default_values[index]->location,
			"a default template argument declared after the template's context");
		return nullptr;
	}
	// A template template-parameter's default names a template (14.1p9),
	// looked up in the template's context (14.6.4.2), not the one of the
	// instantiation that interrupts: libc++'s tuple constructors default
	// `template <class...> class _And = _And`, met while is_constructible
	// (declared before _And) is instantiated.
	if (head->parameters[index]->template_template)
	{
		Scope* frame = instantiation_frame(
			parameters, arguments, index, named_in != nullptr ? named_in : parameters->parent);
		const syntax::TemplateArgument argument =
			syntax::TemplateArgument{ const_cast<syntax::TypeId*>(head->defaults[index]), nullptr, false };
		const syntax::Name* name = PlainTemplateName(argument);
		const Type* templ = nullptr;
		if (name != nullptr)
		{
			InstantiationContext context(
				*this, enter_instantiation(instantiation_depth_ + 1, name->location), substitution, ordinal);
			templ = template_name_argument(*name, frame, true);
		}
		if (templ == nullptr)
			error(head->defaults[index]->location, "a template template-parameter's default names no template");
		return templ;
	}
	// A member template's non-dependent default: the type it named where it
	// appeared (in the scope of the declaration that gave it).
	if (const Type** bound = head->defaults[index] != nullptr
			? bound_defaults_.find(DefaultKey{ head->defaults[index], parameters })
			: nullptr)
		return *bound;
	bool dependent = named_in != nullptr && template_depth_ != 0;
	for (uint32_t i = 0; i < index && template_depth_ != 0 && !dependent; ++i)
		dependent = dependent_type(arguments[i]);
	// A member template of a templated class, named in a template
	// definition (its class's own, say `Q<>` in it): the default can name
	// the enclosing templates' parameters, still unbound there (14.6.2.3).
	for (const Scope* s = parameters->parent;
		template_depth_ != 0 && !dependent && s != nullptr && s->kind != ScopeKind::Namespace; s = s->parent)
		dependent = templated_class(s);
	const syntax::TypeId* type = head->defaults[index];
	const syntax::Expr* value = head->default_values[index];
	// A value is its parameter's argument, as one written is (a reference
	// parameter's object, 14.3.2p1).
	const Type* target = value != nullptr ? argument_target(head, index, arguments) : nullptr;
	if (dependent && named_in == nullptr)
	{
		// Analyzed in the template's own parameter scope, then substituted: a
		// placeholder it makes (a decltype, a value-dependent expression) is
		// the template's, whose scope a later substitution rebuilds (in a
		// frame, its names would stand for dependent arguments that no
		// substitution sees, see substituted_scope).
		const Type* result = type != nullptr ? type_id(*type, parameters) : template_value(*value, parameters, target);
		if (result == nullptr || index == 0)
			return result;
		// At the depth of the declaration that gave it (a friend's may differ).
		uint32_t depth = head->parameters[0]->depth;
		for (const Binding* binding = parameters->bindings; binding != nullptr; binding = binding->next)
		{
			if (DeclaresParameter(binding))
			{
				depth = static_cast<const TemplateParameterEntity*>(binding->entity)->depth;
				break;
			}
		}
		return substitute(result, Substitution{ depth, arguments, index, nullptr });
	}
	Scope* frame = instantiation_frame(
		parameters, arguments, index, named_in != nullptr ? named_in : parameters->parent);
	if (dependent)
		return type != nullptr ? type_id(*type, frame) : template_value(*value, frame, target);
	// Its names are looked up in the context of the declaration that gave
	// it (14.6.4.2).
	InstantiationContext context(*this,
		enter_instantiation(instantiation_depth_ + 1, type != nullptr ? type->location : value->location), substitution,
		ordinal);
	// Copies expand the packs the frame binds.
	// A pack expansion of packs of different lengths (14.5.3p5) fails a
	// deduction's substitution (14.8.2p8).
	FrameExpander expander(*this, frame);
	syntax::SyntaxCloner cloner(arena_, true, expander.get());
	if (type != nullptr)
	{
		// A type's expressions (a template argument's value, a decltype
		// operand) are copied too: the default is shared by every instance
		// of a member template's class (a class's copy keeps its members'
		// template-heads), whose own analysis of it must not read this
		// one's facts (`enable_if_t<!is_same<U, It>::value>` evaluated
		// with U = It is no value in rev<rev<It>>'s copy, where U is
		// unknown). One without them is its own copy, known from its first.
		const bool expands = expander.get() != nullptr;
		const bool fact_free = !expands && fact_free_defaults_.find(type) != PointerMap::kMissing;
		const syntax::TypeId* copy = fact_free ? type : cloner.type_id(type);
		if (!expands && !fact_free && copy == type)
			fact_free_defaults_.set(type, 1);
		return failed() ? nullptr : type_id(*copy, frame);
	}
	const syntax::Expr* copy = cloner.expr(value);
	return failed() ? nullptr : template_value(*copy, frame, target);
}

uint32_t Analyzer::qualifier_arguments(const TemplateHead* head, const Type* const* arguments, uint32_t count)
{
	// A dependent specialization known by its type alone (an alias
	// template's parameter's argument) as an unresolved name's qualifier
	// level: Itanium 5.1.6 spells a dependent name's template arguments as
	// written, and the type keeps its canonical arguments, defaults
	// included; the written list most nearly is those but the trailing ones
	// equal to their defaults (`Requires<_Callable<F>, R>` of std::function
	// is sr 10_CallableIT_EE, as Clang mangles it).
	while (count != 0)
	{
		const uint32_t index = count - 1;
		if (index >= head->count || head->parameters[index]->pack || head->parameters[index]->template_template ||
			!HasDefault(head, index))
			break;
		Scope* parameters = head->default_scopes[index];
		const void* syntax = head->defaults[index] != nullptr ? static_cast<const void*>(head->defaults[index])
															  : static_cast<const void*>(head->default_values[index]);
		const DefaultKey key = DefaultKey{ syntax, parameters };
		const Type* unbound = nullptr;
		if (const Type** known = unbound_defaults_.find(key))
			unbound = *known;
		else
		{
			// As a template definition's default is (template_default).
			++template_depth_;
			unbound = head->defaults[index] != nullptr
				? type_id(*head->defaults[index], parameters)
				: template_value(*head->default_values[index], parameters, argument_target(head, index, arguments));
			--template_depth_;
			unbound_defaults_.insert(key, unbound);
		}
		if (unbound == nullptr)
			break;
		uint32_t depth = head->parameters[0]->depth;
		for (const Binding* binding = parameters->bindings; binding != nullptr; binding = binding->next)
		{
			if (DeclaresParameter(binding))
			{
				depth = static_cast<const TemplateParameterEntity*>(binding->entity)->depth;
				break;
			}
		}
		const Type* bound = index == 0 || !dependent_type(unbound)
			? unbound
			: substitute(unbound, Substitution{ depth, arguments, index, nullptr });
		if (bound == nullptr || bound->canonical != arguments[index]->canonical)
			break;
		--count;
	}
	return count;
}

void Analyzer::default_template_arguments(
	const TemplateHead* head, TypeList& arguments, uint32_t location, Scope* named_in)
{
	// 14.1p9: a missing argument is its parameter's default; a parameter
	// pack takes the arguments left, none for an empty pack. A pack
	// expansion among the arguments leaves their positions unknown until
	// substituted. `named_in`: see template_default.
	for (const Type* argument : arguments)
	{
		if (argument->kind == TypeKind::Expansion)
			return;
	}
	// A substitution's template-id with too many or too few is its failure
	// (14.8.2p8).
	if (arguments.size() > head->count && head->trailing_pack == head->count)
	{
		substitution_error(location, "too many template arguments");
		return;
	}
	for (size_t i = arguments.size(); i < head->count; ++i)
	{
		if (head->parameters[i]->pack)
			break;
		if (!HasDefault(head, i))
		{
			substitution_error(location, "too few template arguments");
			return;
		}
		// Formed in a deduction's substitution, an invalid default is its
		// failure (14.8.2p8; std::function's _Callable<F> of the function
		// itself, as the hosts take it).
		const Type* argument = template_default(
			head, arguments.data(), static_cast<uint32_t>(i), soft_substitution_ != 0, named_in);
		if (argument == nullptr)
			return;
		arguments.push_back(argument);
	}
}

bool Analyzer::own_arguments(const TemplateHead* head, const Type* const* canonical, uint32_t count)
{
	// Whether canonical arguments are a template's own parameters (a pack's
	// expansion for a parameter pack), each by its position.
	if (count != head->count)
		return false;
	for (uint32_t i = 0; i < count; ++i)
	{
		const Type* parameter = head->parameters[i]->canonical->type;
		if (head->parameters[i]->pack)
		{
			const Type* expansion = types_.expansion(parameter);
			parameter = types_.pack(&expansion, 1);
		}
		if (canonical[i] != parameter)
			return false;
	}
	return true;
}

ClassEntity* Analyzer::class_specialization(
	ClassEntity* templ, const TypeList& arguments, uint32_t location, bool fatal)
{
	// One entity per (template, canonical arguments), whatever the context:
	// X<T> for a T at X's own parameter's position is a dependent
	// specialization like any other (members unknown until instantiated);
	// the callers decide where it names X itself (14.6.2.1p1).
	// A dependent template-name's stand-in takes the arguments as written.
	// Arguments that do not match are an error, or (not `fatal`) null.
	const TemplateHead* head = templ->templ->head;
	const bool stand_in = IsStandIn(templ) || templ->templ->alias_stand_in;
	TypeList canonical;
	if (stand_in)
	{
		for (const Type* argument : arguments)
			canonical.push_back(argument->canonical);
	}
	else if (!match_template_arguments(
				 head, arguments.data(), static_cast<uint32_t>(arguments.size()), canonical, fatal, location))
	{
		if (fatal)
			error(location, "template arguments that do not match their template's parameters");
		return nullptr;
	}
	const bool own = !stand_in && own_arguments(head, canonical.data(), static_cast<uint32_t>(canonical.size()));
	const SpecializationKey probe = SpecializationKey{ templ, canonical.data(),
		static_cast<uint32_t>(canonical.size()) };
	if (Entity** found = specializations_.find(probe))
		return static_cast<ClassEntity*>(*found);
	ClassEntity* entity = new_class(templ->key, templ->name, templ->owner);
	ClassTemplateFacts* facts = class_template_facts(entity, TemplateRole::Specialization);
	facts->pattern = templ;
	facts->arguments = arena_.copy_array(canonical.data(), canonical.size());
	facts->argument_count = static_cast<uint32_t>(canonical.size());
	// A member template of a templated class: a nested class of a dependent
	// type, whatever its arguments (14.6.2.1p7), instantiated with its
	// class's instances (`M<char>::w` in the class's own definition).
	facts->dependent = stand_in || (templ->owner != nullptr && templated_class(templ->owner));
	for (const Type* argument : canonical)
		facts->dependent = facts->dependent || dependent_type(argument);
	facts->primary_arguments = own;
	// Its presentation spelling (`box<int>`) is rendered where a symbol
	// needs it (lower/symbol_lowering.cpp).
	entity->label = templ->label;
	Binding* binding = new_binding(BindingKind::Type, templ->name, entity, entity->type);
	binding->hidden = true;
	binding->key = templ->key;
	facts->binding = binding;
	SpecializationKey key = probe;
	key.arguments = facts->arguments;
	specializations_.insert(key, entity);
	++graph_.specializations;
	return entity;
}

namespace {

// Whether a template-id's arguments as written include a pack expansion
// (an instantiation's copy, its elements).
bool WrittenExpansion(const syntax::NameComponent& component)
{
	if (component.expanded)
		return true;
	for (const syntax::TemplateArgument& argument : component.arguments)
	{
		const syntax::Declarator* declarator = argument.type != nullptr ? argument.type->declarator : nullptr;
		if (argument.pack || (declarator != nullptr && declarator->pack != syntax::PackPosition::None))
			return true;
	}
	return false;
}

} // namespace

Binding* Analyzer::template_id_binding(const Binding* found, const syntax::NameComponent& component, Scope* scope)
{
	// A template-id naming a class template specialization (14.2): the
	// binding its lookups return. An alias template's names the type it
	// aliases (14.5.7p2); a template template-parameter's, a dependent
	// specialization of its stand-in.
	const Type* name = template_name_type(found, true);
	if (name == nullptr)
		return nullptr;
	// A template template-parameter TT (or an instantiation's binding of it
	// to its argument): TT<A...> is formed with TT's own template parameter
	// list, its defaults supplied (14.1p9, 14.3.3p1), and not the argument
	// template's. They are the declared parameter's, which the name found
	// (its canonical positional one has the first list of its shape). A
	// pack expansion written among the arguments leaves the positions
	// unknown where the template-id is defined: substituted, it is the
	// argument template's template-id (as GCC and Clang form it).
	const TemplateParameterEntity* parameter = found->entity != nullptr &&
			found->entity->kind == EntityKind::TemplateParameter
		? static_cast<const TemplateParameterEntity*>(found->entity)
		: nullptr;
	const TemplateHead* own = parameter != nullptr && parameter->template_template && !WrittenExpansion(component)
		? parameter->template_head
		: nullptr;
	if (name->kind != TypeKind::Template || name->entity->kind != EntityKind::Class)
	{
		TypeList given;
		if (!template_arguments(component, scope, given, own != nullptr ? own : TemplateHeadOf(name)))
			return nullptr;
		if (own != nullptr)
			default_template_arguments(own, given, component.location, const_cast<Scope*>(found->declared_in));
		if (failed())
			return nullptr;
		const Type* specialization = specialize_template(name, given, component.location);
		return specialization != nullptr ? specialization_binding(specialization, name->entity) : nullptr;
	}
	ClassEntity* templ = static_cast<ClassEntity*>(name->entity);
	if (templ->templ->head == nullptr)
		return nullptr;
	TypeList arguments;
	if (!template_arguments(component, scope, arguments, templ->templ->head))
		return nullptr;
	if (own != nullptr)
		default_template_arguments(own, arguments, component.location, const_cast<Scope*>(found->declared_in));
	ClassEntity* specialization = nullptr;
	if (arguments.size() < templ->templ->head->count)
	{
		// Defaults depend only on the template and the arguments given
		// (14.1p9, analyzed in the template's context): a template-id
		// omitting some finds the specialization it named before under the
		// arguments it gives, keyed apart from full argument lists by their
		// count.
		for (const Type*& argument : arguments)
			argument = argument->canonical;
		const SpecializationKey given = SpecializationKey{ templ, arguments.data(),
			static_cast<uint32_t>(arguments.size()) };
		if (Entity** found_given = specializations_.find(given))
			specialization = static_cast<ClassEntity*>(*found_given);
		else
		{
			SpecializationKey key = given;
			key.arguments = arena_.copy_array(arguments.data(), arguments.size());
			default_template_arguments(templ->templ->head, arguments, component.location);
			specialization = failed()
				? nullptr
				: class_specialization(templ, arguments, component.location, soft_substitution_ == 0);
			if (specialization != nullptr)
				specializations_.insert(key, specialization);
		}
	}
	else
	{
		default_template_arguments(templ->templ->head, arguments, component.location);
		specialization = failed() ? nullptr
								  : class_specialization(templ, arguments, component.location, soft_substitution_ == 0);
	}
	// Arguments of a substitution's template-id that do not fit its
	// template are its failure (14.8.2p8).
	if (specialization == nullptr)
	{
		substitution_error(component.location, "template arguments that do not match their template's parameters");
		return nullptr;
	}
	// A partial specialization's argument list names it where it is the
	// current instantiation (14.6.2.1p1).
	ClassEntity* partial = specialization->templ->dependent
		? partial_of_arguments(templ, specialization->templ->arguments, specialization->templ->argument_count)
		: nullptr;
	if (partial != nullptr && partial->templ->binding != nullptr && current_instantiation(partial, scope))
		return partial->templ->binding;
	if (!specialization->templ->primary_arguments || !current_instantiation(templ, scope))
		return specialization->templ->binding;
	// The current instantiation: a binding of the template's own name.
	if (templ->templ->binding == nullptr)
	{
		Binding* binding = new_binding(BindingKind::Type, templ->name, templ, templ->type);
		binding->hidden = true;
		binding->key = templ->key;
		templ->templ->binding = binding;
	}
	return templ->templ->binding;
}

bool Analyzer::current_instantiation(const ClassEntity* templ, const Scope* scope) const
{
	// 14.6.2.1p1: X's own parameters as its arguments name X itself in the
	// definition of X or of a member of X: where X's scope encloses the
	// name, or in an out-of-class member declaration of X (qualified by
	// X<T...>, 14.5.1p3), its decl-specifiers included.
	if (template_owner_ != nullptr &&
		(template_owner_ == templ || (templ->scope != nullptr && encloses(templ->scope, template_owner_->owner))))
		return true;
	for (; scope != nullptr && templ->scope != nullptr; scope = scope->parent)
	{
		if (scope == templ->scope)
			return true;
	}
	return false;
}

namespace {

const syntax::Name* DeclaratorName(const syntax::Declarator* declarator)
{
	for (; declarator != nullptr; declarator = declarator->nested)
	{
		if (declarator->name != nullptr)
			return declarator->name;
	}
	return nullptr;
}

// The name a declaration declares: its declarator-id or class-head name.
const syntax::Name* NameDeclared(const syntax::Decl& decl)
{
	switch (decl.kind)
	{
	case syntax::DeclKind::Function:
		return DeclaratorName(static_cast<const syntax::FunctionDecl&>(decl).declarator);
	case syntax::DeclKind::SpecialMember:
		return static_cast<const syntax::SpecialMemberDecl&>(decl).name;
	case syntax::DeclKind::Simple:
	{
		const syntax::SimpleDecl& simple = static_cast<const syntax::SimpleDecl&>(decl);
		if (simple.declarators.size != 0)
			return DeclaratorName(simple.declarators[0].declarator);
		for (const syntax::DeclSpecifier& specifier : simple.specifiers)
		{
			if (specifier.kind == syntax::DeclSpecKind::Class)
				return specifier.class_specifier->name;
		}
		return nullptr;
	}
	case syntax::DeclKind::Class:
	{
		const syntax::DeclSpecifier& specifier = static_cast<const syntax::ClassDecl&>(decl).specifier;
		return specifier.kind == syntax::DeclSpecKind::Class ? specifier.class_specifier->name : specifier.name;
	}
	case syntax::DeclKind::Enum:
		return static_cast<const syntax::EnumDecl&>(decl).enum_specifier->name;
	case syntax::DeclKind::Template:
		return NameDeclared(*static_cast<const syntax::TemplateDecl&>(decl).declaration);
	default:
		return nullptr;
	}
}

} // namespace

ClassEntity* Analyzer::qualifying_template(const syntax::Decl& decl, Scope* scope, Scope** named)
{
	// The templated class whose member a template declaration declares out
	// of class: the innermost class template among its declarator-id's (or
	// class-head's) qualifiers, or the partial specialization whose
	// argument list its template-id gives (14.5.5.3p1). Its template-ids
	// (and those of the classes enclosing it) with their own parameters as
	// arguments name them (14.6.2.1p1, see current_instantiation). With
	// `named`, also the class or namespace the whole nested-name-specifier
	// names so (null where lookup alone does not tell).
	if (named != nullptr)
		*named = nullptr;
	const syntax::Name* name = NameDeclared(decl);
	if (name == nullptr || !name->qualified())
		return nullptr;
	NamePrefix prefix = name->global ? NamePrefix{ graph_.global, graph_.global->entity, nullptr }
									 : NamePrefix{ nullptr, nullptr, nullptr };
	ClassEntity* owner = nullptr;
	// A template-head enclosing another (`template<class T> template<class
	// U> ... outer<T>::in<U*>::member`): the qualifier's template-ids after
	// the ones the declared heads give belong to the heads still to come
	// (one per enclosing class template or partial specialization, outermost
	// first, 14.5.2p1, 14.5.5.3p1), whose parameters they may name.
	uint32_t heads = ~0u;
	if (decl.kind == syntax::DeclKind::Template)
	{
		heads = 0;
		for (const Scope* current = scope; current != nullptr && current->kind == ScopeKind::TemplateParameters;
			current = current->parent)
			++heads;
	}
	for (uint32_t i = 0; i + 1 < name->components.size; ++i)
	{
		const syntax::NameComponent& component = name->components[i];
		if (component.kind != syntax::NameComponentKind::Identifier)
			return owner;
		const LookupResult result = prefix.entity == nullptr
			? names_.unqualified(scope, component.identifier, LookupFilter::Prefix)
			: lookup_in(prefix, component.identifier, LookupFilter::Prefix);
		if (!result.found() || result.ambiguous)
			return owner;
		if (result.binding->kind == BindingKind::Namespace || result.binding->kind == BindingKind::NamespaceAlias)
		{
			prefix = scope_of_binding(result.binding, component.location);
			continue;
		}
		ClassEntity* cls = result.binding->kind == BindingKind::Type && result.binding->entity != nullptr &&
				result.binding->entity->kind == EntityKind::Class
			? static_cast<ClassEntity*>(result.binding->entity)
			: nullptr;
		// A typedef-name for a class names that class (7.1.3p8).
		if (result.binding->kind == BindingKind::TypeAlias && !component.has_arguments &&
			result.binding->type != nullptr && result.binding->type->unqualified->kind == TypeKind::Class)
			cls = static_cast<ClassEntity*>(result.binding->type->unqualified->entity);
		if (cls != nullptr && component.has_arguments)
		{
			cls = class_template_of(cls);
			if (cls == nullptr || cls->templ->head == nullptr)
				return owner;
			// Under `template<>`, a template-id whose arguments depend on no
			// template parameter names a class template specialization, whose
			// explicitly specialized member class template the declaration's
			// template-head belongs to (14.7.3p5, p16).
			if (ClassEntity* specialization = explicit_specialization_ ? named_specialization(cls, component, scope)
																	   : nullptr)
				cls = specialization;
			else
			{
				if (heads == 0)
					return owner;
				--heads;
				owner = cls;
				if (ClassEntity* partial = named_partial(cls, component, scope))
					owner = cls = partial;
			}
		}
		if (cls == nullptr || cls->scope == nullptr)
			return owner;
		prefix = NamePrefix{ cls->scope, cls, nullptr };
	}
	if (named != nullptr)
		*named = prefix.scope;
	return owner;
}

ClassEntity* Analyzer::named_specialization(ClassEntity* templ, const syntax::NameComponent& component, Scope* scope)
{
	// The class template specialization a template-id in a declaration's
	// qualifier names when its arguments depend on no template parameter
	// (complete, its members declared), or null.
	TypeList given;
	++template_depth_;
	template_arguments(component, scope, given, templ->templ->head);
	default_template_arguments(templ->templ->head, given, component.location);
	--template_depth_;
	for (const Type* argument : given)
	{
		if (dependent_type(argument))
			return nullptr;
	}
	TypeList canonical;
	if (!match_template_arguments(templ->templ->head, given.data(), static_cast<uint32_t>(given.size()), canonical,
			false, component.location))
		return nullptr;
	ClassEntity* specialization = class_specialization(templ, canonical, component.location, false);
	return specialization != nullptr && require_complete(specialization) ? specialization : nullptr;
}

Scope* Analyzer::member_template_home(const syntax::TemplateDecl& decl, Scope* scope)
{
	// An out-of-class declaration's innermost template-head is the member
	// template's own when the declaration has more template-heads than its
	// qualifier has template-ids (one per enclosing class template, 14.5.2p1,
	// 14.5.5.3p1; under `template<>`, each of those names an explicitly
	// specialized enclosing class template instead, 14.7.3p15): it
	// redeclares the head its class (or namespace) declared, so its
	// placeholders are keyed there (14.5.6.1p5, see dependent_expression).
	// That class or namespace, else null.
	if (decl.declaration->kind == syntax::DeclKind::Template)
		return nullptr;
	const syntax::Name* name = NameDeclared(*decl.declaration);
	if (name == nullptr || !name->qualified())
		return nullptr;
	uint32_t heads = 1;
	for (const Scope* current = scope; current->kind == ScopeKind::TemplateParameters; current = current->parent)
		++heads;
	uint32_t ids = 0;
	for (uint32_t i = 0; i + 1 < name->components.size; ++i)
		ids += name->components[i].has_arguments ? 1 : 0;
	const uint32_t specialized = explicit_specialization_ ? specialization_levels_ : 0;
	if (heads + specialized <= ids)
		return nullptr;
	Scope* home = nullptr;
	qualifying_template(*decl.declaration, scope, &home);
	return home;
}

ClassEntity* Analyzer::named_partial(ClassEntity* templ, const syntax::NameComponent& component, Scope* scope)
{
	// The partial specialization of a class template whose argument list a
	// template-id in a template declaration's qualifier gives (14.5.5.3p1),
	// or null.
	if (templ->templ->partials == nullptr && templ->templ->partial_positions == 0)
		return nullptr;
	// Analyzed as the template declaration's names (before its declaration
	// counts as one).
	TypeList given;
	++template_depth_;
	template_arguments(component, scope, given, templ->templ->head);
	default_template_arguments(templ->templ->head, given, component.location);
	--template_depth_;
	TypeList canonical;
	if (!match_template_arguments(templ->templ->head, given.data(), static_cast<uint32_t>(given.size()), canonical,
			false, component.location))
		return nullptr;
	return partial_of_arguments(templ, canonical.data(), static_cast<uint32_t>(canonical.size()));
}

bool Analyzer::dependent_type(const Type* type) const
{
	// 14.6.2.1p7: a template parameter, a dependent name, a specialization
	// with a dependent argument, the current instantiation or a member of
	// it, or a compound type built from one. A compound type keeps its
	// answer (its components never change); a chain of pointers, references
	// and arrays is walked once, iteratively, and each link learns it.
	const Type* chain = type;
	bool result = false;
	for (;;)
	{
		if (type->dependence != 0)
		{
			result = type->dependence == 2;
			break;
		}
		// An array of a dependent bound is dependent (14.6.2.1p7), as is a
		// vector of a dependent lane type or size.
		if ((type->kind == TypeKind::Array || type->kind == TypeKind::Vector) && type->owner != nullptr)
		{
			result = true;
			type->dependence = 2;
			break;
		}
		if (type->kind == TypeKind::Pointer || type->kind == TypeKind::LValueReference ||
			type->kind == TypeKind::RValueReference || type->kind == TypeKind::Array)
		{
			type = type->element;
			continue;
		}
		result = leaf_dependent(type);
		if (type->kind == TypeKind::Function || type->kind == TypeKind::MemberPointer || type->kind == TypeKind::Pack)
			type->dependence = result ? 2 : 1;
		break;
	}
	for (; chain != type; chain = chain->element)
		chain->dependence = result ? 2 : 1;
	return result;
}

bool Analyzer::leaf_dependent(const Type* type) const
{
	switch (type->kind)
	{
	case TypeKind::TemplateParameter:
		return type->unqualified != auto_placeholder_;
	case TypeKind::TemplateTemplateParameter:
		return true;
	case TypeKind::Class:
	{
		const ClassTemplateFacts* facts = static_cast<const ClassEntity*>(type->entity)->templ;
		if (facts != nullptr)
			return facts->role == TemplateRole::Template || facts->role == TemplateRole::Pattern ||
				facts->role == TemplateRole::Partial || facts->dependent;
		const Scope* owner = type->entity->owner;
		return owner != nullptr && templated_class(owner);
	}
	case TypeKind::Enum:
		return type->entity->owner != nullptr && templated_class(type->entity->owner);
	case TypeKind::MemberPointer:
		return dependent_type(type->owner) || dependent_type(type->element);
	case TypeKind::Function:
		for (uint32_t i = 0; i < type->param_count; ++i)
		{
			if (dependent_type(type->params[i]))
				return true;
		}
		return dependent_type(type->element);
	case TypeKind::Pack:
		for (uint32_t i = 0; i < type->param_count; ++i)
		{
			if (dependent_type(type->params[i]))
				return true;
		}
		return false;
	case TypeKind::Expansion:
	case TypeKind::BoundPack:
		return true;
	default:
		return false;
	}
}

Binding* Analyzer::dependent_member_binding(const Type* qualifier, const Identifier* name)
{
	// A member `name` of a dependent qualifier (14.6p2-p3): one placeholder
	// type per (qualifier, name), canonically that of the canonical
	// qualifier, and the binding lookups through the qualifier return.
	const PlaceholderKey key = PlaceholderKey{ qualifier, name };
	if (Binding** found = placeholders_.find(key))
		return *found;
	TemplateParameterEntity* canonical = nullptr;
	if (qualifier->canonical != qualifier)
		canonical = static_cast<TemplateParameterEntity*>(dependent_member_binding(qualifier->canonical, name)->entity);
	TemplateParameterEntity* entity = new_entity<TemplateParameterEntity>(
		EntityKind::TemplateParameter, name, nullptr, nullptr);
	entity->qualifier = qualifier;
	entity->canonical = canonical != nullptr ? canonical : entity;
	entity->type = types_.named(TypeKind::TemplateParameter, entity);
	Binding* binding = new_binding(BindingKind::TypeAlias, name, entity, entity->type);
	binding->hidden = true;
	placeholders_.insert(key, binding);
	return binding;
}

const Type* Analyzer::dependent_member(const Type* qualifier, const Identifier* name)
{
	return dependent_member_binding(qualifier, name)->type;
}

Scope* Analyzer::substituted_scope(
	Scope* scope, const Substitution& substitution, const TemplateParameterEntity* operand)
{
	// A template definition's scope chain as its specialization sees it: the
	// substituted template parameters' scope becomes their frame, and a
	// prototype scope below it declares its parameters with substituted
	// types; scopes outside the template stay. For a placeholder's
	// `operand` whose named parameters are known, a parameter of dependent
	// type it does not name is declared without a type: the scope may be
	// another template's (an equivalent operand's first declaration, see
	// TemplateParameterEntity::named_parameters), whose other parameters
	// the arguments need not fit (another kind of template parameter at a
	// position, an invalid type outside the immediate context).
	if (scope == nullptr)
		return nullptr;
	if (scope->kind == ScopeKind::TemplateParameters)
	{
		for (const Binding* binding = scope->bindings; binding != nullptr; binding = binding->next)
		{
			if (binding->entity != nullptr && binding->entity->kind == EntityKind::TemplateParameter &&
				static_cast<const TemplateParameterEntity*>(binding->entity)->depth == substitution.depth)
				return instantiation_frame(
					scope, substitution.arguments, substitution.count, scope->parent, nullptr, substitution.elements);
		}
	}
	if (scope->kind != ScopeKind::Function || scope->entity != nullptr)
		return enclosing_frame(scope, substitution);
	Scope* result = new_scope(
		ScopeKind::Function, substituted_scope(scope->parent, substitution, operand), nullptr, nullptr, nullptr);
	result->dumped = false;
	result->guarded = scope->guarded;
	// A function parameter pack whose length the substitution knows is its
	// elements' parameters, its name bound to their types' pack in a scope
	// below (a copy's expansions expand it, see function_pack_scope).
	// A parameter whose type fails to substitute is declared without a
	// type, its failure not the substitution's: a placeholder's operand
	// need not name it (one shared by equivalent operands, 14.5.6.1p5, has
	// its first declaration's scope, whose other parameters are not in its
	// key); naming it fails (see binding_expression).
	Scope* packs = nullptr;
	const SubstitutionFailure failure = failure_;
	for (const Binding* binding = scope->bindings; binding != nullptr; binding = binding->next, failure_ = failure)
	{
		if (binding->kind != BindingKind::Parameter)
			continue;
		const bool skipped = binding->type == nullptr || unnamed_parameter(operand, binding);
		if (!skipped && binding->type->kind == TypeKind::Expansion && binding->name != nullptr)
		{
			TypeList elements;
			substitute_list(&binding->type, 1, substitution, elements);
			bool known = !AnyNull(elements);
			for (const Type* element : elements)
				known = known && element->kind != TypeKind::Expansion;
			if (known)
			{
				for (uint32_t i = 0; i < elements.size(); ++i)
				{
					const Identifier* name = pack_element_name(binding->name, i);
					Entity* parameter = new_entity<Entity>(EntityKind::Parameter, name, result, elements[i]);
					Binding* declared = new_binding(BindingKind::Parameter, name, parameter, elements[i]);
					declared->dependent = true;
					print_in(result, declared);
					declare_in(result, declared);
				}
				if (packs == nullptr)
				{
					packs = new_scope(ScopeKind::TemplateParameters, result, nullptr, nullptr, nullptr);
					packs->dumped = false;
				}
				Binding* pack = new_binding(BindingKind::TypeAlias, binding->name, nullptr,
					types_.pack(elements.data(), static_cast<uint32_t>(elements.size())));
				pack->hidden = true;
				print_in(packs, pack);
				declare_in(packs, pack);
				continue;
			}
		}
		const Type* type = skipped ? nullptr : substitute(binding->type, substitution);
		Entity* parameter = new_entity<Entity>(EntityKind::Parameter, binding->name, result, type);
		Binding* declared = new_binding(BindingKind::Parameter, binding->name, parameter, type);
		declared->dependent = binding->type == nullptr || dependent_type(binding->type);
		print_in(result, declared);
		if (binding->name != nullptr)
			declare_in(result, declared);
	}
	return packs != nullptr ? packs : result;
}

Scope* Analyzer::enclosing_frame(Scope* scope, const Substitution& substitution)
{
	// The parameters an enclosing template head declares at the
	// substitution's depth (a member template's expression naming its
	// class template's): a frame inside `scope` binds the names that find
	// them there (a member may hide one, 14.6.1p7, p9).
	const Scope* parameters = scope;
	while (parameters != nullptr && !DeclaresDepth(parameters, substitution.depth))
		parameters = parameters->parent;
	if (parameters == nullptr)
		return scope;
	Scope* frame = new_scope(ScopeKind::TemplateParameters, scope, nullptr, nullptr, nullptr);
	frame->dumped = false;
	frame->lists = 0;
	for (Binding* binding = parameters->bindings; binding != nullptr; binding = binding->next)
	{
		if (!DeclaresParameter(binding))
			continue;
		const TemplateParameterEntity* parameter = static_cast<const TemplateParameterEntity*>(binding->entity);
		if (parameter->depth != substitution.depth || parameter->index >= substitution.count ||
			parameter->name == nullptr ||
			names_.unqualified(scope, parameter->name, LookupFilter::Ordinary).binding != binding)
			continue;
		bind_argument(frame, binding, substitution.arguments[parameter->index],
			substitution.elements != nullptr ? substitution.elements[parameter->index] : -1);
	}
	return frame;
}

const Type* Analyzer::substitute_bound(const Type* array, const Substitution& given)
{
	TypeList scratch;
	const Substitution substitution = Substitution{ given.depth, effective_arguments(given, scratch), given.count,
		nullptr };
	// An array bound that depends on template parameters: evaluated (8.3.4p1)
	// once every argument is known and none is dependent (partial ordering's
	// unique types leave it a non-deduced bound, 14.8.2.5p5).
	const Type* element = substitute(array->element, substitution);
	if (element == nullptr)
		return nullptr;
	for (uint32_t i = 0; i < substitution.count; ++i)
	{
		if (substitution.arguments[i] == nullptr || dependent_type(substitution.arguments[i]))
			return types_.array(element, 0, array->owner);
	}
	const TemplateParameterEntity* placeholder = static_cast<const TemplateParameterEntity*>(array->owner->entity);
	// A bound that is just a parameter of another depth stays that
	// parameter: an instance's member template's partial specialization
	// `D<Y[N], U>` with the enclosing class template's arguments substituted
	// (libc++'s shared_ptr::__shared_ptr_default_delete).
	if (placeholder->named != nullptr && placeholder->named->depth != given.depth)
		return types_.array(element, 0, array->owner);
	Scope* scope = substituted_scope(placeholder->expression_scope, substitution, placeholder);
	FrameExpander expander(*this, scope);
	syntax::SyntaxCloner cloner(arena_, true, expander.get());
	const syntax::Expr* copy = cloner.expr(placeholder->expression);
	if (failed())
		return nullptr;
	// An invalid or non-positive bound is the substitution's failure
	// (14.8.2p8).
	uint64_t bound = 0;
	{
		InstantiationContext context(
			*this, enter_instantiation(instantiation_depth_ + 1, copy->location), true, placeholder->ordinal);
		bound = array_bound(*copy, scope);
		if (failed())
			return nullptr;
	}
	return checked_array(element, bound, copy->location);
}

const Type* Analyzer::substitute_vector_size(const Type* vector, const Substitution& given)
{
	// A vector whose lane type or size depends on template parameters: its
	// size evaluated (as an array bound is) once every argument is known and
	// none is dependent, then checked as the vector's.
	TypeList scratch;
	const Substitution substitution = Substitution{ given.depth, effective_arguments(given, scratch), given.count,
		nullptr };
	const Type* lane = substitute(vector->element, substitution);
	if (lane == nullptr)
		return nullptr;
	for (uint32_t i = 0; i < substitution.count; ++i)
	{
		if (substitution.arguments[i] == nullptr || dependent_type(substitution.arguments[i]))
			return types_.vector(lane, 0, vector->owner, vector->variadic);
	}
	const TemplateParameterEntity* placeholder = static_cast<const TemplateParameterEntity*>(vector->owner->entity);
	Scope* scope = substituted_scope(placeholder->expression_scope, substitution, placeholder);
	FrameExpander expander(*this, scope);
	syntax::SyntaxCloner cloner(arena_, true, expander.get());
	const syntax::Expr* copy = cloner.expr(placeholder->expression);
	if (failed())
		return nullptr;
	uint64_t size = 0;
	{
		InstantiationContext context(
			*this, enter_instantiation(instantiation_depth_ + 1, copy->location), true, placeholder->ordinal);
		size = array_bound(*copy, scope);
		if (failed())
			return nullptr;
	}
	return checked_vector(lane, size, vector->variadic, copy->location);
}

// -------------------------------------------------------- instantiation

Scope* Analyzer::instantiation_frame(Scope* parameters, const Type* const* arguments, uint32_t count, Scope* parent,
	const Scope* bridge, const int32_t* elements)
{
	// The names of a template parameter scope bound to arguments (by
	// position): an immutable frame parented at the template's context. An
	// out-of-class member definition's frame binds only the names its
	// definition's bridge kept (a member hides the others, 14.6.1p7).
	// `elements`: one element of an expansion is substituted (see
	// Substitution), whose packs bind that element's name alone.
	// It stands for no template parameter list: templates declared in an
	// instance (member templates) are positioned as in a non-template
	// class (and as a `template<> template<...>` definition of one is).
	Scope* frame = new_scope(ScopeKind::TemplateParameters, parent, nullptr, nullptr, nullptr);
	frame->dumped = false;
	frame->lists = 0;
	for (Binding* binding = parameters->bindings; binding != nullptr; binding = binding->next)
	{
		if (binding->entity == nullptr || binding->entity->kind != EntityKind::TemplateParameter)
			continue;
		const TemplateParameterEntity* parameter = static_cast<const TemplateParameterEntity*>(binding->entity);
		if (parameter->index >= count || parameter->name == nullptr)
			continue;
		const NameEntry* bridged = bridge != nullptr ? names_.find(bridge, parameter->name) : nullptr;
		if (bridge != nullptr && (bridged == nullptr || (bridged->type != binding && bridged->value != binding)))
			continue;
		bind_argument(
			frame, binding, arguments[parameter->index], elements != nullptr ? elements[parameter->index] : -1);
	}
	return frame;
}

void Analyzer::bind_argument(Scope* frame, Binding* binding, const Type* argument, int32_t element)
{
	const TemplateParameterEntity* parameter = static_cast<const TemplateParameterEntity*>(binding->entity);
	if (parameter->pack && argument != nullptr && argument->kind == TypeKind::Pack)
	{
		bind_pack(frame, parameter, argument, element);
		return;
	}
	Binding* alias = parameter->value_type != nullptr
		? value_binding(parameter, argument, frame)
		: new_binding(BindingKind::TypeAlias, parameter->name, binding->entity, argument);
	alias->hidden = true;
	declare_in(frame, alias);
}

Binding* Analyzer::value_binding(
	const TemplateParameterEntity* parameter, const Type* argument, Scope* frame, const Identifier* name)
{
	// A non-type parameter's name (or a pack element's) bound to its
	// argument (14.1p6: a prvalue of its value), a constant as an
	// enumerator is; a dependent argument (a template parameter's, a
	// placeholder, one not yet deduced) keeps the name value-dependent.
	if (name == nullptr)
		name = parameter->name;
	if (argument != nullptr && argument->kind == TypeKind::Value)
	{
		EnumeratorEntity* constant = new_entity<EnumeratorEntity>(
			EntityKind::Enumerator, name, frame, argument->element);
		constant->enumeration = nullptr;
		constant->value = Constant{ argument->bound, argument->element };
		if (AddressArgument(argument))
			constant->argument = argument;
		// Its name is type-dependent if its declared type is (14.6.2.2p3).
		Binding* binding = new_binding(BindingKind::Enumerator, name, constant, argument->element);
		binding->dependent = parameter->value_type == nullptr || dependent_type(parameter->value_type);
		return binding;
	}
	Entity* entity = argument != nullptr && argument->kind == TypeKind::TemplateParameter
		? argument->entity
		: const_cast<TemplateParameterEntity*>(parameter);
	return new_binding(BindingKind::TemplateValue, name, entity, parameter->value_type);
}

bool Analyzer::complete_class(const ClassEntity* entity)
{
	// (require_complete's case of an incomplete class.)
	const ClassTemplateFacts* facts = entity->templ;
	if (facts != nullptr && !facts->dependent &&
		(facts->role == TemplateRole::Specialization || facts->role == TemplateRole::MemberInstance))
		instantiate_class(const_cast<ClassEntity*>(entity));
	if (!entity->complete)
		++incomplete_classes_seen_;
	return entity->complete;
}

void Analyzer::instantiate_class(ClassEntity* entity)
{
	// 14.7.1p1: the definition of the template (or pattern member class) is
	// copied and analyzed in its frame; a recursive demand while it is under
	// way sees the class still incomplete.
	ClassTemplateFacts* facts = entity->templ;
	if (facts->definition_state != FactState::NotStarted)
		return;
	const ClassTemplateFacts* pattern = facts->pattern->templ;
	if (pattern == nullptr)
		return;
	// 14.5.5.1: a specialization a partial specialization matches is
	// instantiated from that one's definition, its parameters bound to the
	// arguments deduced. Selecting it may demand the class again (a
	// partial's argument naming it): in progress already.
	facts->definition_state = FactState::InProgress;
	TypeList deduced;
	ClassEntity* partial = facts->role == TemplateRole::Specialization &&
			(pattern->partials != nullptr || pattern->partial_positions != 0)
		? matching_partial(facts->pattern, facts->arguments, facts->argument_count, deduced)
		: nullptr;
	// An instance's member template's partial specializations declared
	// outside the class template are the class template's member
	// template's, their enclosing parameters the instance's arguments
	// (14.5.5.3p2).
	const ClassEntity* instance = facts->role == TemplateRole::Specialization && facts->pattern->owner != nullptr &&
			facts->pattern->owner->kind == ScopeKind::Class
		? static_cast<const ClassEntity*>(facts->pattern->owner->entity)
		: nullptr;
	ClassEntity* member = partial == nullptr && instance != nullptr ? pattern_member_template(facts->pattern) : nullptr;
	// A member template of an instance's member template: the class
	// template's member template's (original_member_template), where
	// partial specializations declared outside the class templates are.
	for (uint32_t level = 0;
		member != nullptr && member->templ->partials == nullptr && member->templ->partial_positions == 0 && level < 64;
		++level)
	{
		ClassEntity* outer = const_cast<ClassEntity*>(original_member_template(member));
		member = outer != member ? outer : nullptr;
	}
	const bool outside_partial = member != nullptr &&
		(member->templ->partials != nullptr || member->templ->partial_positions != 0) &&
		(partial = matching_partial(member, facts->arguments, facts->argument_count, deduced, instance)) != nullptr;
	if (partial != nullptr)
		pattern = partial->templ;
	// An instance's member template defined outside the class template:
	// the class template's member template's definition.
	ClassEntity* outside = pattern->definition == nullptr && partial == nullptr &&
			facts->role == TemplateRole::Specialization
		? outside_member_template(facts->pattern)
		: nullptr;
	// A member class of an instance's member template defined outside the
	// class templates (`template<class A> template<class B> struct
	// X<A>::Y<B>::Z { ... }`, 14.5.2p1, 14.5.1.2): the class template's
	// member template's member's definition.
	ClassEntity* outside_member = outside == nullptr && pattern->definition == nullptr &&
			facts->role == TemplateRole::MemberInstance
		? outside_member_class(facts->pattern)
		: nullptr;
	if (outside != nullptr)
		pattern = outside->templ;
	else if (outside_member != nullptr)
		pattern = outside_member->templ;
	if (pattern->definition == nullptr)
	{
		facts->definition_state = FactState::NotStarted;
		// The program's std::initializer_list declared only: the course's
		// builtin layout completes its specializations.
		if (partial == nullptr && outside == nullptr && facts->role == TemplateRole::Specialization &&
			is_initializer_list_template(facts->pattern))
		{
			facts->definition_state = FactState::Done;
			builtin_initializer_list(entity);
		}
		return;
	}
	facts->partial = partial != nullptr ? partial : outside != nullptr ? outside : outside_member;
	if (partial != nullptr)
	{
		partial_deduced_index_.set(facts, static_cast<uint32_t>(partial_deduced_.size()));
		partial_deduced_.push_back(arena_.copy_array(deduced.data(), deduced.size()));
	}
	Scope* parent = nullptr;
	if (outside != nullptr)
		parent = outside_frames(pattern->definition_parameters, facts->arguments, facts->argument_count, instance,
			facts->pattern->owner, outside->scope->parent);
	else if (outside_partial)
		parent = outside_frames(pattern->definition_parameters, deduced.data(), static_cast<uint32_t>(deduced.size()),
			instance, facts->pattern->owner, partial->scope != nullptr ? partial->scope->parent : nullptr);
	else if (partial != nullptr)
	{
		parent = instantiation_frame(pattern->definition_parameters, deduced.data(),
			static_cast<uint32_t>(deduced.size()), facts->pattern->owner);
		// In place of a qualified definition's bridge, it is one.
		parent->bridge = partial->scope != nullptr && partial->scope->parent != nullptr &&
			partial->scope->parent->bridge;
	}
	else if (facts->role == TemplateRole::Specialization)
	{
		parent = instantiation_frame(
			pattern->definition_parameters, facts->arguments, facts->argument_count, facts->pattern->owner);
		const Scope* defined = facts->pattern->scope;
		parent->bridge = defined != nullptr && defined->parent != nullptr && defined->parent->bridge;
	}
	else if (pattern->definition_parameters == nullptr)
		parent = facts->enclosing->scope;
	else
	{
		// A member class defined out of its class template: the frame of
		// the parameter names its definition's bridge kept.
		parent = outside_frames(pattern->definition_parameters, nullptr, 0, facts->enclosing, facts->enclosing->scope,
			(outside_member != nullptr ? outside_member : facts->pattern)->scope->parent);
	}
	FrameExpander expander(*this, parent);
	syntax::SyntaxCloner cloner(arena_, false, expander.get());
	const syntax::ClassSpecifier* copy = cloner.class_specifier(pattern->definition);
	check_stack(pattern->definition->location);
	{
		InstantiationContext context(*this, enter_instantiation(facts->depth + 1, pattern->definition->location), false,
			pattern->definition_ordinal);
		const size_t first_enum = pending_member_enums_.size();
		class_instantiations_.push_back(ClassInstantiation{ entity, pattern->definition, copy, 0, 0 });
		class_body(entity, *copy, parent, nullptr);
		class_instantiations_.pop_back();
		link_member_patterns(*pattern->definition, *copy);
		// Out-of-class member enumeration definitions see the whole class
		// (their names are looked up as a member's, 9.2p2).
		for (size_t i = first_enum; i < pending_member_enums_.size(); ++i)
			instantiate_member_enum(pending_member_enums_[i]);
		pending_member_enums_.resize(first_enum);
	}
	facts->definition_state = FactState::Done;
	++graph_.instantiated_classes;
}

bool Analyzer::declare_member_instance(
	const syntax::ClassSpecifier* specifier, const syntax::Name& name, TokenKind key, Scope* where, Specifiers& out)
{
	// A named member class of an instance: declared now, its definition
	// instantiated from the pattern member class when required (14.7.1p1).
	(void)specifier;
	const syntax::NameComponent& last = name.last();
	if (name.qualified() || last.has_arguments || last.kind != syntax::NameComponentKind::Identifier)
		return false;
	ClassEntity* owner = static_cast<ClassEntity*>(where->entity);
	const ClassEntity* owner_pattern = owner->templ->partial != nullptr ? owner->templ->partial : owner->templ->pattern;
	if (owner_pattern == nullptr || owner_pattern->scope == nullptr)
		return false;
	const LookupResult found = names_.own(owner_pattern->scope, last.identifier, LookupFilter::Elaborated);
	if (!found.found() || found.binding->entity == nullptr || found.binding->entity->kind != EntityKind::Class)
		return false;
	ClassEntity* pattern = static_cast<ClassEntity*>(found.binding->entity);
	if (pattern->templ == nullptr || pattern->templ->role != TemplateRole::Pattern)
		return false;
	ClassEntity* entity = nullptr;
	const NameEntry* entry = names_.find(where, last.identifier);
	if (entry != nullptr && entry->tag != nullptr && entry->tag->entity->kind == EntityKind::Class)
		entity = static_cast<ClassEntity*>(entry->tag->entity);
	else
	{
		check_type_name_conflict(where, last.identifier, nullptr, name.location);
		entity = new_class(key, last.identifier, where);
		ClassTemplateFacts* facts = class_template_facts(entity, TemplateRole::MemberInstance);
		facts->pattern = pattern;
		facts->enclosing = owner;
	}
	Binding* binding = new_binding(BindingKind::Type, last.identifier, entity, entity->type);
	binding->key = key;
	print_in(where, binding);
	declare_in(where, binding);
	out.declared_binding = binding;
	out.declared = entity;
	return true;
}

void Analyzer::declare_member_enum_instance(EnumEntity* entity, ClassEntity* owner)
{
	// An instance's member enumeration declared opaquely: the pattern's
	// enumeration of its name, when that is defined out of class (14.5.1.4),
	// defines it with its class (14.7.1p1; a scoped one too), or once the
	// definition is analyzed.
	const ClassEntity* pattern_class = owner->templ->partial != nullptr ? owner->templ->partial : owner->templ->pattern;
	const NameEntry* found = pattern_class != nullptr && pattern_class->scope != nullptr
		? names_.find(pattern_class->scope, entity->name)
		: nullptr;
	if (found == nullptr || found->tag == nullptr || found->tag->entity->kind != EntityKind::Enum)
		return;
	EnumEntity* pattern = static_cast<EnumEntity*>(found->tag->entity);
	const bool defined_out_of_class = pattern->templ != nullptr && pattern->templ->definition != nullptr;
	// Defined in the class template, the instance's own copy defines it.
	if (pattern->defined && !defined_out_of_class)
		return;
	EnumTemplateFacts* facts = enum_template_facts(entity);
	facts->pattern = pattern;
	facts->enclosing = owner;
	if (defined_out_of_class)
	{
		if (owner->complete)
			instantiate_member_enum(entity);
		else
			pending_member_enums_.push_back(entity);
		return;
	}
	EnumTemplateFacts* pattern_facts = enum_template_facts(pattern);
	facts->next = pattern_facts->waiting;
	pattern_facts->waiting = entity;
}

void Analyzer::instantiate_member_enum(EnumEntity* entity)
{
	// The pattern's out-of-class definition copied and analyzed for the
	// instance: its enumerators join the instance class (a scoped
	// enumeration's own scope), their values analyzed in the frame of the
	// definition's parameter names over it.
	const EnumTemplateFacts* facts = entity->templ;
	const EnumTemplateFacts* pattern = facts->pattern->templ;
	ClassEntity* owner = facts->enclosing;
	if (entity->defined)
		return;
	Scope* lookup = entity->scoped ? entity->scope : owner->scope;
	const Binding* declaration = names_.find(owner->scope, entity->name)->tag;
	Scope* values = lookup;
	if (pattern->parameters != nullptr)
	{
		values = outside_frames(pattern->parameters, nullptr, 0, owner, lookup, nullptr);
	}
	FrameExpander expander(*this, values);
	syntax::SyntaxCloner cloner(arena_, true, expander.get());
	const syntax::EnumSpecifier* copy = cloner.enum_specifier(pattern->definition);
	{
		// Its names are looked up in its definition's context (14.6.4.2).
		InstantiationContext context(
			*this, enter_instantiation(owner->templ->depth + 1, copy->location), false, pattern->definition_ordinal);
		enumerators(entity, *copy, declaration, lookup, lookup, values);
	}
}

void Analyzer::link_member_patterns(
	const syntax::ClassSpecifier& pattern, const syntax::ClassSpecifier& instance, uint32_t first, uint32_t end)
{
	// The copy's member declarations are the pattern's in order: each
	// instance member function learns the pattern member whose definition
	// (in the class or out of it) it instantiates; those in [first, end).
	for (uint32_t i = first; i < pattern.members.size && i < end; ++i)
	{
		const syntax::Decl* from = pattern.members[i];
		const syntax::Decl* to = instance.members[i];
		// A member template's declaration (14.5.2).
		while (from->kind == syntax::DeclKind::Template && to->kind == syntax::DeclKind::Template)
		{
			from = static_cast<const syntax::TemplateDecl*>(from)->declaration;
			to = static_cast<const syntax::TemplateDecl*>(to)->declaration;
		}
		const Binding* source[1] = { nullptr };
		const Binding* target[1] = { nullptr };
		switch (from->kind)
		{
		case syntax::DeclKind::Function:
			source[0] = static_cast<const syntax::FunctionDecl*>(from)->declared;
			target[0] = static_cast<const syntax::FunctionDecl*>(to)->declared;
			break;
		case syntax::DeclKind::SpecialMember:
			source[0] = static_cast<const syntax::SpecialMemberDecl*>(from)->declared;
			target[0] = static_cast<const syntax::SpecialMemberDecl*>(to)->declared;
			break;
		case syntax::DeclKind::Simple:
		{
			const syntax::SimpleDecl* a = static_cast<const syntax::SimpleDecl*>(from);
			const syntax::SimpleDecl* b = static_cast<const syntax::SimpleDecl*>(to);
			for (uint32_t j = 0; j < a->declarators.size; ++j)
				link_member(a->declarators[j].declared, b->declarators[j].declared);
			continue;
		}
		default:
			continue;
		}
		link_member(source[0], target[0]);
	}
}

void Analyzer::link_member(const Binding* pattern, const Binding* instance)
{
	if (pattern == nullptr || instance == nullptr || pattern->entity == nullptr || instance->entity == nullptr ||
		pattern->entity->kind != instance->entity->kind)
		return;
	if (instance->entity->kind == EntityKind::Function)
	{
		// A friend only declared in the template names a function defined
		// elsewhere (not an instantiated one). An instance's member template
		// learns the pattern's, whose definition outside the class template
		// its specializations instantiate.
		FunctionEntity* function = static_cast<FunctionEntity*>(instance->entity);
		// Clang declares an instance's member from the pattern member's
		// latest declaration, which its definition outside the class is
		// (one that may not add ABI tags): defined there by now, the
		// member's in-class ABI tags are not the instance's (the hosts
		// differ, GCC keeps them; the fixtures pin Clang's names).
		const FunctionEntity* source = static_cast<const FunctionEntity*>(pattern->entity);
		if (graph_.has_abi_tags() && source->defined && !source->inline_member && graph_.abi_tags(function) != nullptr)
			graph_.set_abi_tags(function, SemanticGraph::AbiTags{ nullptr, 0 });
		if (function->templated && PatternOf(function) == nullptr && function->owner->kind == ScopeKind::Class &&
			static_cast<const FunctionEntity*>(pattern->entity)->templated)
		{
			function_template_facts(function)->pattern = static_cast<FunctionEntity*>(pattern->entity);
			return;
		}
		if (PatternOf(function) != nullptr || function->templated || function->primary != nullptr ||
			(function->owner->kind != ScopeKind::Class && !function->instantiated))
			return;
		function_template_facts(function)->pattern = static_cast<FunctionEntity*>(pattern->entity);
		function->instantiated = true;
	}
	else if (instance->entity->kind == EntityKind::Variable)
	{
		VariableEntity* variable = static_cast<VariableEntity*>(instance->entity);
		if (variable->member || variable->templ != nullptr)
			return;
		variable_template_facts(variable)->pattern = static_cast<const VariableEntity*>(pattern->entity);
	}
}

const ClassEntity* Analyzer::original_member_template(const ClassEntity* copy)
{
	// The member class template an instance's copy of a member-specification
	// declared (see pattern_member_template), or a member class template of
	// such a copy, stands for: the class template's own (14.5.2p1).
	if (const ClassEntity* pattern = pattern_member_template(copy))
		return pattern;
	const Scope* owner = copy->owner;
	if (owner == nullptr || owner->kind != ScopeKind::Class || owner->entity == nullptr)
		return nullptr;
	const ClassEntity* enclosing = static_cast<const ClassEntity*>(owner->entity);
	if (enclosing->templ == nullptr || enclosing->templ->role != TemplateRole::Template)
		return nullptr;
	const ClassEntity* outer = original_member_template(enclosing);
	if (outer == nullptr || outer->scope == nullptr)
		return nullptr;
	const LookupResult found = names_.own(outer->scope, copy->name, LookupFilter::Elaborated);
	const ClassEntity* pattern = found.found() && found.binding->kind == BindingKind::Type
		? class_template_of(found.binding->entity)
		: nullptr;
	return pattern != nullptr && pattern != copy && pattern->templ->role == TemplateRole::Template ? pattern : nullptr;
}

ClassEntity* Analyzer::outside_member_class(const ClassEntity* member)
{
	// A member class of an instance's member class template (which the
	// instance's copy of the member-specification declared) defined outside
	// the class templates: the class template's member template's member
	// class of its name, when that one has the definition, else null.
	const Scope* owner = member->owner;
	if (owner == nullptr || owner->kind != ScopeKind::Class || owner->entity == nullptr)
		return nullptr;
	const ClassEntity* enclosing = static_cast<const ClassEntity*>(owner->entity);
	if (enclosing->templ == nullptr || enclosing->templ->role != TemplateRole::Template)
		return nullptr;
	const ClassEntity* original = original_member_template(enclosing);
	if (original == nullptr || original->scope == nullptr)
		return nullptr;
	const LookupResult found = names_.own(original->scope, member->name, LookupFilter::Elaborated);
	if (!found.found() || found.binding->kind != BindingKind::Type || found.binding->entity == member ||
		found.binding->entity->kind != EntityKind::Class)
		return nullptr;
	ClassEntity* pattern = static_cast<ClassEntity*>(found.binding->entity);
	return pattern->templ != nullptr && pattern->templ->role == TemplateRole::Pattern &&
			pattern->templ->definition != nullptr && pattern->scope != nullptr
		? pattern
		: nullptr;
}

const FunctionEntity* Analyzer::outside_pattern_member(const FunctionEntity* member)
{
	// A member of an instance's member class template, which the instance's
	// copy of the class template's member-specification declared, has its
	// definition outside the class templates on the class template's member
	// template's member of the same position (14.5.2p1), and shares an
	// in-class one's body with it: that member, else null. A complete
	// copy's members are paired once, the class at a time, so later queries
	// are lookups (instantiating each of n members would scan O(n) each);
	// an incomplete copy's, whose members are still being declared, by a
	// scan.
	const Scope* owner = member->owner;
	if (owner == nullptr || owner->kind != ScopeKind::Class || owner->entity == nullptr)
		return nullptr;
	const ClassEntity* copy = static_cast<const ClassEntity*>(owner->entity);
	if (copy->complete && outside_paired_classes_.find(copy) != PointerMap::kMissing)
	{
		const uint32_t found = outside_member_index_.find(member);
		return found != PointerMap::kMissing ? outside_members_[found] : nullptr;
	}
	const bool pair_all = copy->complete;
	if (pair_all)
		outside_paired_classes_.set(copy, 1);
	const ClassEntity* pattern = copy->templ != nullptr && copy->templ->definition != nullptr
		? original_member_template(copy)
		: nullptr;
	if (pattern == nullptr || pattern->templ->definition == nullptr)
		return nullptr;
	const FunctionEntity* result = nullptr;
	const auto pair = [&](const Binding* x, const Binding* y) {
		if (x == nullptr || y == nullptr || x->entity == nullptr || y->entity == nullptr ||
			x->entity->kind != EntityKind::Function || y->entity->kind != EntityKind::Function)
			return;
		const FunctionEntity* found = static_cast<const FunctionEntity*>(y->entity);
		if (x->entity == member)
			result = found;
		if (pair_all)
		{
			outside_member_index_.set(x->entity, static_cast<uint32_t>(outside_members_.size()));
			outside_members_.push_back(found);
		}
	};
	const syntax::ClassSpecifier& from = *copy->templ->definition;
	const syntax::ClassSpecifier& to = *pattern->templ->definition;
	for (uint32_t i = 0; i < from.members.size && i < to.members.size && (pair_all || result == nullptr); ++i)
	{
		const syntax::Decl* a = from.members[i];
		const syntax::Decl* b = to.members[i];
		while (a->kind == syntax::DeclKind::Template && b->kind == syntax::DeclKind::Template)
		{
			a = static_cast<const syntax::TemplateDecl*>(a)->declaration;
			b = static_cast<const syntax::TemplateDecl*>(b)->declaration;
		}
		if (a->kind != b->kind)
			continue;
		if (a->kind == syntax::DeclKind::Function)
			pair(static_cast<const syntax::FunctionDecl*>(a)->declared,
				static_cast<const syntax::FunctionDecl*>(b)->declared);
		else if (a->kind == syntax::DeclKind::SpecialMember)
			pair(static_cast<const syntax::SpecialMemberDecl*>(a)->declared,
				static_cast<const syntax::SpecialMemberDecl*>(b)->declared);
		else if (a->kind == syntax::DeclKind::Simple)
		{
			const syntax::SimpleDecl* first = static_cast<const syntax::SimpleDecl*>(a);
			const syntax::SimpleDecl* second = static_cast<const syntax::SimpleDecl*>(b);
			for (uint32_t j = 0; j < first->declarators.size && j < second->declarators.size; ++j)
				pair(first->declarators[j].declared, second->declarators[j].declared);
		}
	}
	return result;
}

const VariableEntity* Analyzer::outside_pattern_variable(const VariableEntity* member)
{
	// A static data member (or member variable template) of an instance, or
	// of an instance's member class template, that the copy of a
	// member-specification declared: the class template's member of its
	// name, which a definition outside the class templates defines
	// (14.5.2p1, 14.5.1.3p1), else null.
	const Scope* owner = member->owner;
	if (owner == nullptr || owner->kind != ScopeKind::Class || owner->entity == nullptr)
		return nullptr;
	const ClassEntity* copy = static_cast<const ClassEntity*>(owner->entity);
	const ClassEntity* pattern = nullptr;
	if (instance_class(owner))
		pattern = copy->templ->partial != nullptr ? copy->templ->partial : copy->templ->pattern;
	else if (copy->templ != nullptr && copy->templ->role == TemplateRole::Template)
		pattern = original_member_template(copy);
	if (pattern == nullptr || pattern->scope == nullptr || pattern == copy)
		return nullptr;
	const LookupResult found = names_.own(pattern->scope, member->name, LookupFilter::Ordinary);
	if (!found.found() || found.binding->kind != BindingKind::Variable || found.binding->entity == member)
		return nullptr;
	return static_cast<const VariableEntity*>(found.binding->entity);
}

FunctionTemplateFacts* Analyzer::function_template_facts(FunctionEntity* function)
{
	if (function->templ == nullptr)
	{
		function->templ = static_cast<FunctionTemplateFacts*>(
			arena_.allocate(sizeof(FunctionTemplateFacts), alignof(FunctionTemplateFacts)));
		std::memset(static_cast<void*>(function->templ), 0, sizeof(FunctionTemplateFacts));
		function->templ->depth = instantiation_depth_;
	}
	return function->templ;
}

VariableTemplateFacts* Analyzer::variable_template_facts(VariableEntity* variable)
{
	if (variable->templ == nullptr)
	{
		variable->templ = static_cast<VariableTemplateFacts*>(
			arena_.allocate(sizeof(VariableTemplateFacts), alignof(VariableTemplateFacts)));
		std::memset(static_cast<void*>(variable->templ), 0, sizeof(VariableTemplateFacts));
	}
	return variable->templ;
}

EnumTemplateFacts* Analyzer::enum_template_facts(EnumEntity* entity)
{
	if (entity->templ == nullptr)
	{
		entity->templ = static_cast<EnumTemplateFacts*>(
			arena_.allocate(sizeof(EnumTemplateFacts), alignof(EnumTemplateFacts)));
		std::memset(static_cast<void*>(entity->templ), 0, sizeof(EnumTemplateFacts));
	}
	return entity->templ;
}

bool Analyzer::instantiate_variable(const VariableEntity* entity)
{
	// An instance's static data member defined where it is needed: the
	// pattern member's out-of-class definition copied and analyzed once
	// (in-class initializers were the instance's declaration's).
	VariableEntity* variable = const_cast<VariableEntity*>(entity);
	VariableTemplateFacts* facts = variable->templ;
	if (facts == nullptr || facts->pattern == nullptr)
		return variable->defined;
	if (facts->arguments != nullptr)
		return instantiate_variable_specialization(variable);
	if (facts->state != FactState::NotStarted)
		return facts->state == FactState::Done;
	// A static data member of an instance's member class template defined
	// outside the enclosing class templates: the class template's member
	// template's member's definition (see outside_pattern_variable).
	const VariableEntity* defined = facts->pattern;
	while (defined != nullptr && !defined->defined)
		defined = outside_pattern_variable(defined);
	const VariableTemplateFacts* pattern = defined != nullptr ? defined->templ : nullptr;
	if (pattern == nullptr)
		return false;
	facts->state = FactState::InProgress;
	Scope* owner = variable->owner;
	Scope* scope = owner;
	if (pattern->parameters != nullptr)
	{
		scope = outside_frames(
			pattern->parameters, nullptr, 0, static_cast<ClassEntity*>(owner->entity), owner, pattern->scope);
	}
	FrameExpander expander(*this, scope);
	syntax::SyntaxCloner cloner(arena_, true, expander.get());
	const syntax::Initializer* initializer = cloner.initializer(pattern->initializer);
	{
		// Its names are looked up in its definition's context (14.6.4.2):
		// an out-of-class one's, or its class template's.
		const ClassEntity* instance = static_cast<const ClassEntity*>(owner->entity);
		uint32_t limit = pattern->definition_ordinal;
		const ClassTemplateFacts* templ = instance->templ;
		const ClassEntity* source = templ->partial != nullptr ? templ->partial : templ->pattern;
		if (limit == 0 && source != nullptr && source->templ != nullptr)
			limit = source->templ->definition_ordinal;
		InstantiationContext context(
			*this, enter_instantiation(instance->templ->depth + 1, pattern->location), false, limit);
		variable->defined = true;
		expression_scope_ = scope;
		// 3.9p6, 8.3.4p3: a member declared with an omitted bound has the
		// bound its definition gives (a dependent one evaluated in the copy).
		const Type* declared = defined->type;
		if (variable->type->kind == TypeKind::Array && variable->type->bound == 0 && variable->type->owner == nullptr &&
			declared->kind == TypeKind::Array && (declared->bound != 0 || declared->owner != nullptr))
		{
			uint64_t bound = declared->bound;
			if (bound == 0 && pattern->bound != nullptr)
				bound = array_bound(*cloner.expr(pattern->bound), scope);
			if (bound != 0)
				variable->type = types_.array(variable->type->element, bound);
		}
		if (initializer != nullptr)
		{
			// 8.3.4p3, 8.5.1p4: the definition's initializer completes an
			// omitted bound (a braced list's by its typed initialization).
			const Type* type = complete_array_from_initializer(variable->type, initializer, true);
			initialize_variable(variable, type, initializer, scope, pattern->location);
			if (type->kind == TypeKind::Array && type->bound == 0 && type->owner == nullptr &&
				(initializer->kind == syntax::InitializerKind::Assign ||
					initializer->kind == syntax::InitializerKind::Braced) &&
				initializer->expression->kind == syntax::ExprKind::BracedInit)
				type = initializer->expression->facts->type;
			variable->type = type;
			record_constant_initializer(variable, initializer, scope);
		}
		// A member defined inline in its class template has the instance's
		// in-class initializer, typed where the instance declared it.
		else if (variable->constant != ConstantState::Value && !variable->is_constexpr &&
			variable->member_initializer == nullptr)
			default_initialize(variable, variable->type, pattern->location);
		check_destruction(variable->type, scope, pattern->location);
	}
	facts->initializer = initializer;
	facts->scope = scope;
	facts->state = FactState::Done;
	++graph_.instantiated_variables;
	return true;
}

void Analyzer::explicit_instantiation(const syntax::ExplicitInstantiationDecl& decl, Scope* scope)
{
	// 14.7.2: `template class X<A>;` instantiates the specialization's
	// definition and (a definition, not `extern`) its members' definitions
	// (p8), as `template class X<A>::M;` does a member class's (p1); the
	// class-key agrees with the template's (7.1.6.3p3), and the declaration
	// is in the template's namespace (an unqualified name) or one enclosing
	// it (p5).
	const syntax::Decl& inner = *decl.declaration;
	// 14p3: an explicit instantiation has at most one declarator.
	if (inner.kind == syntax::DeclKind::Simple && static_cast<const syntax::SimpleDecl&>(inner).declarators.size > 1)
		error(decl.location, "an explicit instantiation must declare a single entity");
	if (inner.kind != syntax::DeclKind::Class)
		return explicit_function_instantiation(decl, scope);
	const syntax::DeclSpecifier& specifier = static_cast<const syntax::ClassDecl&>(inner).specifier;
	if (specifier.kind != syntax::DeclSpecKind::Elaborated ||
		(!specifier.name->last().has_arguments && !specifier.name->qualified()))
		error(inner.location, "an explicit instantiation names a class template specialization");
	const syntax::Name& name = *specifier.name;
	if (alias_template_id(name, scope))
		error(name.location, "an explicit instantiation names an alias template specialization");
	// p12: access is not checked for the names it uses (their checks are
	// deferred and dropped).
	Scope* saved_deferral = access_deferral_;
	const size_t pending = pending_access_.size();
	access_deferral_ = semantic() ? scope : nullptr;
	const LookupResult result = lookup_name(name, scope, LookupFilter::Elaborated);
	pending_access_.resize(pending);
	access_deferral_ = saved_deferral;
	if (result.binding->kind != BindingKind::Type || result.binding->entity->kind != EntityKind::Class)
		error(name.location, "an explicit instantiation names a class template specialization");
	ClassEntity* entity = static_cast<ClassEntity*>(result.binding->entity);
	const TemplateRole role = name.last().has_arguments ? TemplateRole::Specialization : TemplateRole::MemberInstance;
	if (entity->templ == nullptr || entity->templ->role != role)
		error(
			name.location, "an explicit instantiation names a class template specialization or a member class of one");
	if ((specifier.keyword == TokenKind::KW_UNION) != (entity->key == TokenKind::KW_UNION))
		error(name.location, "class-key does not agree with the template's");
	Scope* here = declaration_scope(scope);
	const Scope* home = entity->templ->pattern->owner;
	bool placed = name.qualified() ? encloses(here, home) : here == home;
	for (const Scope* s = home; !placed && !name.qualified() && s != nullptr && s->kind == ScopeKind::Namespace &&
		static_cast<const NamespaceEntity*>(s->entity)->is_inline;
		s = s->parent)
		placed = s->parent == here;
	if (!placed)
		error(name.location, "an explicit instantiation outside its template's namespace");
	if (!require_complete(entity))
		error(name.location, "explicit instantiation of an undefined class template");
	// p11: a declaration follows no definition of the specialization; p5:
	// one naming an explicit specialization has no effect.
	if (decl.is_extern)
	{
		if (entity->templ->explicit_instantiation)
			error(name.location, "an explicit instantiation declaration after the definition");
		if (!entity->templ->explicit_specialization)
			entity->templ->extern_instantiation = true;
		return;
	}
	// 14.7p5: at most one explicit instantiation definition (no diagnostic
	// required; the hosts diagnose one unit's).
	if (entity->templ->explicit_instantiation)
		error(name.location, "a specialization explicitly instantiated twice");
	entity->templ->explicit_instantiation = true;
	explicitly_instantiate_members(entity);
}

void Analyzer::explicitly_instantiate_members(ClassEntity* cls)
{
	// 14.7.2p8-p9: an explicit instantiation definition of a class template
	// specialization is one of each of its members (not inherited ones)
	// defined at its point: member functions (their definitions
	// instantiated where the unit is lowered, as object roots), static data
	// members (their definitions instantiated now, their initializers
	// checked, and emitted), and member classes (their definitions, and in
	// turn their members). A member defined later is not instantiated. An
	// explicitly specialized class's are its own (p5: no effect): its
	// non-inline member functions' definitions are object roots, as their
	// linkage requires anyway (see explicit_function_instantiation).
	if (!cls->templ->explicit_specialization)
		explicit_class_instantiations_.push_back(cls);
	for (Binding* binding = cls->scope->bindings; binding != nullptr; binding = binding->next)
	{
		Entity* member = binding->entity;
		if (member == nullptr || member == cls || member->owner != cls->scope)
			continue;
		if (binding->kind == BindingKind::Function)
		{
			FunctionEntity* function = static_cast<FunctionEntity*>(member);
			if (cls->templ->explicit_specialization && !function->instantiated)
			{
				function->explicit_instantiation = true;
				continue;
			}
			const FunctionEntity* pattern = PatternOf(function);
			if (!function->instantiated || function->explicit_instantiation || pattern == nullptr ||
				!pattern->defined || Excluded(function))
				continue;
			function->explicit_instantiation = true;
			explicit_instantiations_.push_back(function);
		}
		else if (binding->kind == BindingKind::Variable)
		{
			VariableEntity* variable = static_cast<VariableEntity*>(member);
			VariableTemplateFacts* facts = variable->templ;
			if (facts == nullptr || facts->pattern == nullptr || !facts->pattern->defined ||
				!instantiate_variable(variable))
				continue;
			facts->explicit_definition = true;
			if (!facts->demanded)
			{
				facts->demanded = true;
				demanded_variables_.push_back(variable);
			}
		}
		else if (binding->kind == BindingKind::Type && member->kind == EntityKind::Class)
		{
			ClassEntity* nested = static_cast<ClassEntity*>(member);
			if (nested->templ != nullptr && nested->templ->role == TemplateRole::MemberInstance &&
				require_complete(nested))
				explicitly_instantiate_members(nested);
		}
	}
}

const syntax::Expr* Analyzer::default_argument(const FunctionEntity* function, uint32_t index)
{
	instantiate_default_argument(const_cast<FunctionEntity*>(function), index);
	return function->default_arguments[index];
}

void Analyzer::instantiate_default_argument(FunctionEntity* function, uint32_t index)
{
	// A default argument is instantiated where a call uses it (14.7.1p1,
	// p12): a specialization's is its template's, copied and analyzed in
	// the frame of the declaring parameter scope; a class template
	// instance's member's, copied with its class, is analyzed in the class
	// (a complete-class context, 9.2p2).
	// Its names are looked up in the context of the declaration that gave
	// it (14.6.4.2): the function template's first (8.3.6p4), or the
	// class template's member-specification (8.3.6p6), complete at its end
	// (9.2p2).
	const FunctionEntity* primary = function->primary;
	const syntax::Expr* pattern = function->default_arguments[index];
	if (pattern == nullptr || pattern->facts != nullptr)
		return;
	Scope* frame = nullptr;
	const syntax::Expr* copy = pattern;
	uint32_t limit = 0;
	if (primary != nullptr && ArgumentsOf(function) != nullptr && HeadOf(primary) != nullptr &&
		HeadOf(primary)->argument_defaults != nullptr)
	{
		Scope* parameters = HeadOf(primary)->argument_defaults;
		frame = instantiation_frame(parameters, ArgumentsOf(function), HeadOf(primary)->count, parameters->parent);
		FrameExpander expander(*this, frame);
		syntax::SyntaxCloner cloner(arena_, true, expander.get());
		copy = cloner.expr(pattern);
		limit = HeadOf(primary)->ordinal;
	}
	else if (instance_class(function->owner))
	{
		frame = function->owner;
		if (PatternOf(function) != nullptr)
			limit = class_definition_context(PatternOf(function)->owner);
	}
	else
		return;
	const Type* parameter = function->type->canonical->params[index];
	{
		InstantiationContext context(
			*this, enter_instantiation(instantiation_depth_ + 1, copy->location), false, limit);
		BodyContext body = BodyContext();
		// A member function's closures' context (see number_closure): a
		// class template instance's member, or a member template's
		// specialization, whose closures are numbered within it (Itanium
		// 5.1.8), not among its class's in the unit's instantiation order.
		if (frame == function->owner || (function->owner != nullptr && function->owner->kind == ScopeKind::Class))
		{
			body.closure_context = function;
			body.default_argument = index;
		}
		body_ = &body;
		expression_scope_ = frame;
		if (copy->kind == syntax::ExprKind::BracedInit)
			list_argument(static_cast<const syntax::BracedInitList&>(*copy), parameter, frame, copy->location);
		else
		{
			typed(*copy, frame);
			convert(*copy, parameter, false);
			ClassEntity* cls = parameter->is_reference() ? nullptr : const_cast<ClassEntity*>(ClassOf(parameter));
			if (cls != nullptr && same_class_source(cls, *copy))
				record_copy(cls, parameter, *copy, copy->location);
		}
	}
	function->default_arguments[index] = copy;
	++graph_.instantiated_arguments;
}

// ------------------------------------------------------ function bodies

uint32_t Analyzer::definition_context(const FunctionEntity* definition)
{
	// A templated function's definition context (14.6.4.2): where it is
	// defined, and for a member defined in its class's member-
	// specification where the outermost enclosing templated class ends
	// (its body sees the whole class, 9.2p2).
	const uint32_t limit = definition->definition_ordinal;
	const uint32_t enclosing = class_definition_context(definition->owner);
	return limit != 0 && enclosing > limit ? enclosing : limit;
}

uint32_t Analyzer::class_definition_context(const Scope* scope)
{
	// Where the outermost templated class enclosing `scope` ends (0 when
	// none does).
	uint32_t limit = 0;
	for (; scope != nullptr && scope->kind == ScopeKind::Class; scope = scope->parent)
	{
		const ClassEntity* cls = static_cast<const ClassEntity*>(scope->entity);
		if (cls != nullptr && cls->templ != nullptr && cls->templ->definition_ordinal > limit)
			limit = cls->templ->definition_ordinal;
	}
	return limit;
}

bool Analyzer::instantiate_definition(const FunctionEntity* entity)
{
	FunctionEntity* function = const_cast<FunctionEntity*>(entity);
	if (!function->instantiated)
		return function->defined && function->body != nullptr;
	if (function->instantiation == FactState::Done)
		return true;
	if (function->instantiation == FactState::InProgress)
		return true;
	// The template's definition: the pattern member's (in its class or out
	// of it), or the function template's.
	const FunctionEntity* definition = PatternOf(function) != nullptr ? PatternOf(function) : function->primary;
	// A member explicitly defaulted out of its class template (8.4.2p5):
	// the implicit definition's, synthesized for the instance's class.
	if (definition != nullptr && definition->synthesized && definition->body == nullptr &&
		PatternOf(function) != nullptr)
	{
		function->instantiation = FactState::InProgress;
		{
			InstantiationContext context(*this, enter_instantiation(function->templ->depth + 1, 0));
			expression_scope_ = function->owner;
			default_definition(function, false, 0);
		}
		function->instantiation = FactState::Done;
		++graph_.instantiated_functions;
		return true;
	}
	// A member of an instance's member class template defined outside the
	// enclosing class templates: the class template's member template's
	// member's definition (see outside_pattern_member).
	if (definition != nullptr && PatternOf(function) != nullptr)
	{
		const FunctionEntity* outer = definition;
		while (outer != nullptr && outer->body == nullptr && !outer->synthesized)
			outer = outside_pattern_member(outer);
		if (outer != nullptr && outer->body != nullptr)
			definition = outer;
	}
	// A specialization of an instance's member template defined outside the
	// class template: the pattern member template's definition (through an
	// instance's member class templates, the outermost class template's).
	bool outside = false;
	if (definition != nullptr && PatternOf(function) == nullptr && definition == function->primary &&
		definition->body == nullptr && PatternOf(definition) != nullptr)
	{
		const FunctionEntity* outer = PatternOf(definition);
		while (outer != nullptr && outer->body == nullptr)
			outer = outside_pattern_member(outer);
		if (outer != nullptr)
		{
			definition = outer;
			outside = true;
		}
	}
	// An in-class definition in an instance whose completion is under way
	// (a member class template's member, or a member template's
	// specialization, a constant expression of the instance calls) has its
	// body only as the copy of the template's: it is instantiated from that
	// (14.7.1p3), its names found in its class (the member template's in
	// its parameter scope there).
	const bool copied = definition != nullptr && definition->scope == nullptr && WithinInstance(definition->owner) &&
		(PatternOf(function) != nullptr
				? definition->template_parameters == nullptr
				: definition == function->primary && definition->template_parameters != nullptr);
	if (definition == nullptr || !definition->defined || definition->body == nullptr ||
		(definition->scope == nullptr && !copied))
		return false;
	function->instantiation = FactState::InProgress;
	Scope* parent = nullptr;
	if (outside)
		parent = member_template_frame(function, definition);
	else if (PatternOf(function) != nullptr)
	{
		// A member of an instance (or a friend defined in it, 3.4.1p9): its
		// class, or the frame of an out-of-class definition's parameter
		// names.
		Scope* owner = function->owner;
		if (owner->kind != ScopeKind::Class && function->befriending != nullptr)
			owner = function->befriending->entity->scope;
		if (definition->template_parameters == nullptr)
			parent = owner;
		else
		{
			parent = outside_frames(definition->template_parameters, nullptr, 0,
				static_cast<ClassEntity*>(owner->entity), owner, definition->scope->parent);
		}
	}
	else
	{
		// A function template specialization: the frame of the definition's
		// parameter names, in the scope the definition's names continue in.
		Scope* after = definition->scope != nullptr ? definition->scope->parent : definition->template_parameters;
		parent = instantiation_frame(definition->template_parameters, ArgumentsOf(function),
			HeadOf(definition) != nullptr ? HeadOf(definition)->count : 0, after->parent);
		// In place of an out-of-class definition's bridge, it is one.
		parent->bridge = after->bridge;
	}
	// Copies expand the frame's packs; the parameter clause's function
	// parameter packs expand with them (see function_pack_scope).
	// An instance's member template defined in its class, or a member of
	// its member class template (at any depth), shares the class template's
	// body, whose names are those of the class template's parameter clause
	// (a class parameter pack's function parameter pack unexpanded): that
	// clause is copied with it.
	FrameExpander expander(*this, parent);
	syntax::SyntaxCloner cloner(arena_, true, expander.get());
	const syntax::ParameterClause* source = definition->parameters;
	for (const FunctionEntity* inner = definition; inner != nullptr && definition->body != nullptr;)
	{
		const FunctionEntity* outer = PatternOf(inner) != nullptr ? PatternOf(inner)
			: expander.get() != nullptr                           ? outside_pattern_member(inner)
																  : nullptr;
		if (outer == nullptr || outer == inner || outer->body == nullptr || outer->body->body != definition->body->body)
			break;
		if (outer->parameters != nullptr)
			source = outer->parameters;
		inner = outer;
	}
	const syntax::ParameterClause* parameters = source;
	if (expander.get() != nullptr && parameters != nullptr)
	{
		parameters = cloner.parameter_clause(parameters);
		parent = function_pack_scope(
			parent, *source, cloner.parameter_packs().data(), cloner.parameter_packs().size(), function->type);
	}
	const syntax::FunctionBody* body = cloner.body(*definition->body);
	check_stack(definition->body->body != nullptr ? definition->body->body->location : 0);
	{
		InstantiationContext context(*this,
			enter_instantiation(function->templ != nullptr ? function->templ->depth + 1 : instantiation_depth_ + 1,
				definition->body->body != nullptr ? definition->body->body->location : 0),
			false, definition_context(definition));
		function->defined = true;
		function->inline_member = true;
		function->definition_location = definition->definition_location;
		if (PatternOf(function) == nullptr && function->primary != nullptr)
			substitute_throw_specification(function);
		// A templated lambda's call operator specialization is its
		// closure's lambda body (sealed_frame).
		LambdaFrame frame;
		const bool closure_body = function->owner != nullptr && function->owner->kind == ScopeKind::Class &&
			sealed_frame(static_cast<const ClassEntity*>(function->owner->entity), frame);
		LambdaFrame* saved_lambda = pending_lambda_;
		if (closure_body)
		{
			pending_lambda_ = &frame;
			++lambda_bodies_;
		}
		function_body(function, function->type, parameters, *body, nullptr, parent);
		if (closure_body)
		{
			pending_lambda_ = saved_lambda;
			if (--lambda_bodies_ == 0)
				captured_.clear();
		}
		// A virtual destructor's deallocation function is looked up at its
		// definition (12.4p12): an instance's out-of-class one's here, at
		// the instantiation (its class's completion found it undefined).
		virtual_definition(function, definition->body->body != nullptr ? definition->body->body->location : 0);
	}
	function->instantiation = FactState::Done;
	++graph_.instantiated_functions;
	return true;
}

} // namespace sema
