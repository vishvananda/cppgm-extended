#include "sema/analyzer.h"
#include "sema/instantiation_context.h"
#include "sema/template_packs.h"

// Explicit specializations (14.7.3): `template<>` declares a class
// template's specialization, a function template's, or a member of a
// class template specialization as an ordinary class, function or
// variable of its own, which the specialization's key then names instead
// of an instantiation of the template. A specialization is declared before
// the uses that would instantiate it (14.7.3p6): an entity already
// instantiated cannot be specialized.
namespace sema {

namespace {

// A partial specialization's argument that depends on no parameter and
// that a specialization it matches has canonically the same: a type, or a
// value of its parameter's (non-dependent) type.
bool IndexableArgument(const TemplateHead* head, const Type* const* arguments, uint32_t position)
{
	const Type* argument = arguments[position];
	if (argument->kind == TypeKind::Pack || argument->kind == TypeKind::Expansion || position >= head->count ||
		head->parameters[position]->pack)
		return false;
	if (argument->kind == TypeKind::Value)
		return head->parameters[position]->value_type != nullptr &&
			argument->element == head->parameters[position]->value_type->unqualified->canonical;
	return argument->kind != TypeKind::TemplateParameter;
}

// The class template whose specialization a partial specialization's
// dependent argument is (a class type, no cv-qualifier), of which the
// argument there of each specialization it matches is a specialization
// too: deduction and substitution give that argument itself (14.5.5.1p2,
// no base class of it, 14.8.2.5). Null for another argument, for a
// template template-parameter's specialization (its argument any
// template), and for a member template of a templated class (an
// instance's own member template stands for it when the partial
// specialization of an instance's member template is matched,
// 14.5.5.3p2).
const ClassEntity* ArgumentTemplate(const TemplateHead* head, const Type* argument, uint32_t position)
{
	if (argument->kind != TypeKind::Class || argument->cv != 0 || position >= head->count ||
		head->parameters[position]->pack)
		return nullptr;
	const ClassTemplateFacts* facts = static_cast<const ClassEntity*>(argument->entity)->templ;
	if (facts == nullptr || facts->role != TemplateRole::Specialization || !facts->dependent ||
		facts->pattern->templ->template_name != nullptr)
		return nullptr;
	const Scope* owner = facts->pattern->owner;
	const ClassTemplateFacts* enclosing = owner != nullptr && owner->kind == ScopeKind::Class &&
			owner->entity != nullptr
		? static_cast<const ClassEntity*>(owner->entity)->templ
		: nullptr;
	if (enclosing != nullptr &&
		(enclosing->role == TemplateRole::Template || enclosing->role == TemplateRole::Pattern ||
			enclosing->role == TemplateRole::Partial))
		return nullptr;
	return facts->pattern;
}

// A template parameter of `depth` marked deduced (by index).
void MarkDeduced(const TemplateParameterEntity* parameter, uint32_t depth, SmallList<uint8_t, 8>& deduced)
{
	if (parameter != nullptr && parameter->depth == depth && parameter->index < deduced.size())
		deduced[parameter->index] = 1;
}

// A partial_index_ key's position tag: the position, apart for its two
// kinds of key (see Analyzer::index_partial).
const Type* PositionTag(uint32_t position, bool by_template)
{
	return reinterpret_cast<const Type*>(static_cast<uintptr_t>(position) + (by_template ? 65 : 1));
}

// The lookup key's spelling of a declarator-id without its template
// arguments.
syntax::Name WithoutArguments(const syntax::Name& name, syntax::NameComponent& last)
{
	last = name.last();
	last.has_arguments = false;
	last.arguments = syntax::Slice<syntax::TemplateArgument>{ nullptr, 0 };
	syntax::Name bare = syntax::Name();
	bare.components = syntax::Slice<syntax::NameComponent>{ &last, 1 };
	bare.location = name.location;
	return bare;
}

} // namespace

void Analyzer::explicit_specialization(const syntax::TemplateDecl& decl, Scope* scope)
{
	// 14.7.3p2: at namespace scope (in C++11, not in a class), not in a
	// template (a member template's specialization is outside PA15). The
	// declaration it wraps is analyzed as a non-template one; the first
	// class head or declarator that names a specialization takes the flag.
	if (declaration_scope(scope)->kind != ScopeKind::Namespace)
		error(decl.location, "an explicit specialization outside namespace scope");
	if (template_depth_ != 0)
		error(decl.location, "an explicit specialization in a template is not supported");
	const bool saved = explicit_specialization_;
	const uint32_t saved_levels = specialization_levels_;
	explicit_specialization_ = true;
	++specialization_levels_;
	declaration(*decl.declaration, scope);
	explicit_specialization_ = saved;
	specialization_levels_ = saved_levels;
}

uint32_t Analyzer::SpecializedEnclosing(const Scope* qualifier)
{
	// The implicitly instantiated class template specializations a member's
	// qualifier is or is nested in: one `template<>` each introduces its
	// explicit specialization (14.7.3p15; an explicitly specialized one's
	// members are ordinary, p5).
	if (qualifier == nullptr || qualifier->kind != ScopeKind::Class || qualifier->entity == nullptr)
		return 0;
	EnclosingSpecializations enclosing;
	enclosing_specializations(static_cast<const ClassEntity*>(qualifier->entity), enclosing);
	uint32_t count = 0;
	for (const ClassTemplateFacts* facts : enclosing)
		count += facts->explicit_specialization ? 0 : 1;
	return count;
}

ClassEntity* Analyzer::specialized_class(const syntax::Name& name, TokenKind key, Scope* scope)
{
	// The specialization a `template<>` class head names: a class template
	// specialization whose arguments depend on no template parameter
	// (a dependent one would be a partial specialization), not yet
	// instantiated, declared in the template's namespace or one enclosing
	// it (14.7.3p2). Access is not checked for the names of its head (a
	// private member template, a private type as an argument: P0692's
	// [temp.spec]p6, as 14.7.2p12 has it for explicit instantiations;
	// GCC and Clang in C++11 mode): their checks are deferred and dropped.
	Scope* saved_deferral = access_deferral_;
	const size_t pending = pending_access_.size();
	access_deferral_ = semantic() ? scope : nullptr;
	const LookupResult result = lookup_name(name, scope, LookupFilter::Elaborated);
	pending_access_.resize(pending);
	access_deferral_ = saved_deferral;
	if (result.binding->kind != BindingKind::Type || result.binding->entity->kind != EntityKind::Class)
		error(name.location, "an explicit specialization names no class template specialization");
	ClassEntity* entity = static_cast<ClassEntity*>(result.binding->entity);
	ClassTemplateFacts* facts = entity->templ;
	if (facts == nullptr || facts->role != TemplateRole::Specialization || facts->dependent)
		error(name.location, "an explicit specialization names no class template specialization");
	if (!facts->explicit_specialization && facts->definition_state != FactState::NotStarted)
		error(name.location, "an explicit specialization after the specialization's instantiation");
	if ((key == TokenKind::KW_UNION) != (entity->key == TokenKind::KW_UNION))
		error(name.location, "class-key does not agree with the template's");
	if (!encloses(declaration_scope(scope), facts->pattern->owner))
		error(name.location, "an explicit specialization outside its template's namespace");
	facts->explicit_specialization = true;
	facts->definition_state = FactState::Done;
	return entity;
}

Entity* Analyzer::class_explicit_specialization(const syntax::ClassSpecifier& specifier, Scope* scope, Specifiers& out)
{
	// `template<> class-head { ... }`: the specialization's own definition,
	// an ordinary class whose members are its own (14.7.3p5), in the
	// template's scope.
	explicit_specialization_ = false;
	ClassEntity* entity = specialized_class(*specifier.name, specifier.key, scope);
	if (entity->scope != nullptr)
		error(specifier.name->location, "class redefinition");
	out.declared_binding = entity->templ->binding;
	class_body(entity, specifier, entity->owner, scope);
	return entity;
}

Binding* Analyzer::explicit_function_specialization(const Specifiers& specifiers, const DeclaratorInfo& info,
	Scope* scope, bool definition, uint32_t location, bool deleted)
{
	// A function template's specialization (its arguments explicit, or
	// deduced from the declaration's type, 14.8.2.6) or a member function
	// of a class template specialization (14.7.3p16): the function the key
	// names is defined here, not instantiated. It is inline or constexpr
	// only as its own declarations say (14.7.3p12); it has no default
	// arguments (p21).
	explicit_specialization_ = false;
	const syntax::NameComponent& last = info.name->last();
	FunctionEntity* function = nullptr;
	const Identifier* key = nullptr;
	if (info.qualifier != nullptr && instance_class(info.qualifier) && !last.has_arguments)
	{
		Scope* where = declarator_target(*info.name, info.qualifier, scope, location);
		const DeclaredName name = declared_name(*info.name, where);
		key = name.key;
		implicit_members_for(where, key);
		const Type* signature = name.kind == syntax::NameComponentKind::Conversion ? info.type->canonical
																				   : signature_of(info.type);
		const FunctionList* same = names_.overload(where, key, signature);
		if (same != nullptr && same->binding->entity->owner == where)
		{
			// 14.7.3p15: one template<> per explicitly specialized enclosing
			// class template.
			if (specialization_levels_ != SpecializedEnclosing(where))
				error(location, "an explicit specialization of a member with the wrong number of template<>");
			function = static_cast<FunctionEntity*>(same->binding->entity);
			if (!function->explicit_specialization &&
				(!function->instantiated || function->instantiation != FactState::NotStarted))
				error(location, "an explicit specialization after the member's instantiation");
		}
	}
	// Else a function template's specialization, or a specialization of a
	// member template of the class template specialization, its arguments
	// deduced (14.7.3p14: `template<> template<> void A<int>::g1(int,
	// char)`, X1 deduced as char).
	const bool of_template = function == nullptr;
	if (function == nullptr)
	{
		key = specialization_key(*info.name);
		function = declared_specialization(info, scope, location);
		if (function == nullptr)
			error(location, "an explicit specialization matches no function template");
		// 14.7.3p15: one template<> per explicitly specialized enclosing
		// class template, and the template's own.
		if (specialization_levels_ != SpecializedEnclosing(info.qualifier) + 1)
			error(location, "an explicit specialization of a function template with the wrong number of template<>");
		if (function->explicit_specialization ? definition && function->defined
											  : function->instantiation != FactState::NotStarted)
			error(location, "an explicit specialization after the specialization's instantiation or definition");
	}
	if (definition && function->defined && function->explicit_specialization)
		error(location, "redefinition of a function");
	// 15.4p4: its exception-specification is compatible with the template's
	// for its arguments.
	if (semantic() && template_depth_ == 0)
		check_redeclared_exceptions(function, info, info.after != nullptr ? info.after : scope, location);
	for (uint32_t i = 0; info.parameters != nullptr && i < info.parameters->parameters.size; ++i)
	{
		if (info.parameters->parameters[i]->default_argument != nullptr)
			error(location, "a default argument in an explicit specialization");
	}
	// Deleted as its own declarations say (8.4.3p4), not as the template
	// or the member it replaces is.
	function->deleted = (function->explicit_specialization && function->deleted) || deleted;
	// A function template's explicit specialization is a function of its
	// own: its attributes are its declarations' (as GCC reads them, warning
	// of one missing its template's), not the template's its entity was
	// made with (a member's redeclares the member).
	if (of_template && !function->explicit_specialization)
	{
		function->noreturn = false;
		function->effects = FunctionEffect::Default;
		function->no_inline = false;
	}
	// Retain inline from an earlier declaration of this specialization,
	// while discarding the primary template's independent inline state.
	function->is_inline = (function->explicit_specialization && function->is_inline) || specifiers.is_inline ||
		specifiers.is_constexpr;
	function->explicit_specialization = true;
	function->instantiated = false;
	function->is_constexpr = specifiers.is_constexpr;
	function->defined = function->defined || definition || deleted;
	if (info.parameters != nullptr)
		function->parameters = info.parameters;
	if (semantic())
		check_operator_declaration(function, location);
	Binding* binding = new_binding(BindingKind::Function, key, function, info.type);
	binding->hidden = true;
	binding->declared_in = function->owner;
	return binding;
}

FunctionEntity* Analyzer::declared_specialization(const DeclaratorInfo& info, Scope* scope, uint32_t location)
{
	// The function template specialization a declaration names (14.7.3,
	// 14.7.2p3): of the templates of its name, those whose type deduction
	// from the declared type (after its explicit arguments, 14.8.1)
	// succeeds; of several, the more specialized one's (14.5.6.2p1). Null
	// for none.
	const syntax::NameComponent& last = info.name->last();
	const Identifier* key = specialization_key(*info.name);
	// A constructor template's (`template W<int>::W(double);`, 12.1p1).
	if (info.qualifier != nullptr && info.qualifier->kind == ScopeKind::Class && info.qualifier->entity != nullptr &&
		last.kind == syntax::NameComponentKind::Identifier && info.qualifier->entity->name == key)
		key = constructor_key_;
	// The explicit arguments of a member's declarator-id are checked as the
	// member's declaration is (11p6).
	TypeList explicit_arguments;
	Scope* saved_class = declaration_class_;
	if (info.qualifier != nullptr && info.qualifier->kind == ScopeKind::Class && scope->kind == ScopeKind::Namespace)
		declaration_class_ = info.qualifier;
	if (last.has_arguments)
		template_arguments(last, scope, explicit_arguments);
	declaration_class_ = saved_class;
	const LookupResult found = info.qualifier != nullptr ? names_.qualified(info.qualifier, key, LookupFilter::Ordinary)
														 : names_.unqualified(scope, key, LookupFilter::Ordinary);
	FunctionEntity* function = nullptr;
	for (const FunctionList* node = found.functions; node != nullptr; node = node->next)
	{
		FunctionEntity* primary = static_cast<FunctionEntity*>(node->binding->entity);
		if (!primary->templated)
			continue;
		FunctionEntity* candidate = deduce_for_declaration(primary, explicit_arguments, info.type);
		if (candidate == nullptr || candidate == function)
			continue;
		if (function != nullptr)
		{
			const bool forward = at_least_as_specialized(candidate->primary, function->primary, info.type->param_count);
			const bool backward = at_least_as_specialized(
				function->primary, candidate->primary, info.type->param_count);
			if (forward == backward)
				error(location, "a declaration matches several function templates");
			if (!forward)
				continue;
		}
		function = candidate;
	}
	return function;
}

void Analyzer::explicit_function_instantiation(const syntax::ExplicitInstantiationDecl& decl, Scope* scope)
{
	// 14.7.2p1, p3: an explicit instantiation of a function template's
	// specialization or of a member function of a class template
	// specialization. `extern`: a declaration, which suppresses the
	// function's implicit instantiation unless it is inline (p10) and
	// follows no definition (p11); else a definition, which the unit emits
	// (an object root). p12: access is not checked for the names it uses.
	// p4: in the template's namespace or one enclosing it.
	const syntax::Decl& inner = *decl.declaration;
	Specifiers specifiers = Specifiers();
	DeclaratorInfo info = DeclaratorInfo();
	Scope* saved_deferral = access_deferral_;
	const size_t pending = pending_access_.size();
	access_deferral_ = semantic() ? scope : nullptr;
	if (inner.kind == syntax::DeclKind::Simple)
	{
		const syntax::SimpleDecl& simple = static_cast<const syntax::SimpleDecl&>(inner);
		if (simple.declarators.size != 1 || simple.declarators[0].initializer != nullptr)
			error(inner.location, "an explicit instantiation declares one function or variable");
		this->specifiers(simple.specifiers, scope, specifiers, false);
		if (specifiers.type == nullptr)
			error(inner.location, "an explicit instantiation without a type");
		info = declarator(simple.declarators[0].declarator, specifiers.type, scope, inner.location, &specifiers);
	}
	else if (inner.kind == syntax::DeclKind::SpecialMember)
	{
		const syntax::SpecialMemberDecl& member = static_cast<const syntax::SpecialMemberDecl&>(inner);
		if (member.definition || member.initializer != nullptr)
			error(inner.location, "an explicit instantiation with a definition");
		const syntax::NameComponent& last = member.name->last();
		const Type* result = types_.fundamental(FundamentalType::Void);
		if (last.kind == syntax::NameComponentKind::Conversion)
		{
			const NamePrefix prefix = member.name->qualified()
				? name_prefix(*member.name, scope, true)
				: NamePrefix{ declaration_scope(scope), nullptr, nullptr };
			result = type_id(*last.conversion, prefix.scope != nullptr ? prefix.scope : scope);
		}
		info = declarator(member.declarator, result, scope, inner.location);
	}
	else
		error(inner.location, "an explicit instantiation of no class, function or variable");
	if (info.name == nullptr)
		error(inner.location, "an explicit instantiation of no function or variable");
	// The static data member, variable template specialization or function
	// it names, looked up while access is not checked either (its explicit
	// template arguments' names too).
	const Binding* variable = info.type->kind != TypeKind::Function
		? lookup_name(*info.name, scope, LookupFilter::Ordinary).binding
		: nullptr;
	FunctionEntity* function = variable == nullptr ? instantiated_function(info, scope, inner.location) : nullptr;
	pending_access_.resize(pending);
	access_deferral_ = saved_deferral;
	if (variable != nullptr)
		return explicit_variable_instantiation(decl, info, specifiers, variable, scope);
	if (specifiers.is_inline || specifiers.is_constexpr || specifiers.is_static || specifiers.is_extern)
		error(inner.location, "a specifier an explicit instantiation may not have");
	if (!encloses(declaration_scope(scope), enclosing_namespace(function->owner)))
		error(inner.location, "an explicit instantiation outside its template's namespace");
	// 13.5p6 for the specialization's parameters.
	if (semantic())
		check_operator_declaration(function, inner.location);
	// 14.7.2p5: an explicit instantiation after an explicit specialization
	// has no effect, including no additional emission demand.
	if (function->explicit_specialization)
		return;
	if (decl.is_extern)
	{
		if (function->explicit_instantiation)
			error(inner.location, "an explicit instantiation declaration after the definition");
		function->extern_instantiation = true;
		return;
	}
	if (function->explicit_instantiation)
		error(inner.location, "a specialization explicitly instantiated twice");
	function->explicit_instantiation = true;
	explicit_instantiations_.push_back(function);
	named_instantiations_.push_back(NamedInstantiation{ function, inner.location });
}

void Analyzer::explicit_variable_instantiation(const syntax::ExplicitInstantiationDecl& decl,
	const DeclaratorInfo& info, const Specifiers& specifiers, const Binding* found, Scope* scope)
{
	// 14.7.2p1, p3: an explicit instantiation of a static data member of a
	// class template specialization (or of a variable template's
	// specialization, the course's extension), of the variable's type. A
	// definition instantiates the variable's definition, which the unit
	// emits; a declaration (`extern`) names one defined elsewhere, whose
	// definition this unit still emits where it uses it (the same
	// definition, 3.2p6: suppressing it is an optimization).
	const uint32_t location = decl.declaration->location;
	VariableEntity* variable = found->kind == BindingKind::Variable && found->entity != nullptr &&
			found->entity->kind == EntityKind::Variable
		? static_cast<VariableEntity*>(found->entity)
		: nullptr;
	if (variable == nullptr || variable->templ == nullptr || variable->templ->pattern == nullptr)
		error(location, "an explicit instantiation names no static data member or variable template specialization");
	if (variable->type->canonical != info.type->canonical)
		error(location, "an explicit instantiation's type differs from the variable's");
	if (specifiers.is_inline || specifiers.is_constexpr || specifiers.is_static || specifiers.is_extern ||
		specifiers.is_thread_local)
		error(location, "a specifier an explicit instantiation may not have");
	if (!encloses(declaration_scope(scope), enclosing_namespace(variable->owner)))
		error(location, "an explicit instantiation outside its template's namespace");
	if (decl.is_extern)
		return;
	if (!instantiate_variable(variable))
		error(location, "an explicit instantiation of a variable whose template's definition is not available");
	VariableTemplateFacts* facts = variable->templ;
	facts->explicit_definition = true;
	if (!facts->demanded)
	{
		facts->demanded = true;
		demanded_variables_.push_back(variable);
	}
}

FunctionEntity* Analyzer::instantiated_function(const DeclaratorInfo& info, Scope* scope, uint32_t location)
{
	// The function an explicit instantiation names (14.7.2p3): a member
	// function of a class template specialization of the declared type,
	// else a function template's (or member template's) specialization.
	const syntax::NameComponent& last = info.name->last();
	if (info.qualifier != nullptr && instance_class(info.qualifier) && !last.has_arguments)
	{
		const DeclaredName name = declared_name(*info.name, info.qualifier);
		implicit_members_for(info.qualifier, name.key);
		const Type* signature = name.kind == syntax::NameComponentKind::Conversion ? info.type->canonical
																				   : signature_of(info.type);
		const FunctionList* same = names_.overload(info.qualifier, name.key, signature);
		if (same != nullptr && same->binding->entity->owner == info.qualifier)
		{
			FunctionEntity* function = static_cast<FunctionEntity*>(same->binding->entity);
			if (function->type->canonical->element != info.type->canonical->element)
				error(location, "an explicit instantiation's return type differs from the member's");
			if (!function->instantiated && !function->explicit_specialization)
				error(location, "an explicit instantiation of a member that is not instantiated");
			return function;
		}
	}
	FunctionEntity* function = declared_specialization(info, scope, location);
	if (function == nullptr)
		error(location, "an explicit instantiation matches no function template specialization");
	return function;
}

bool Analyzer::Excluded(const FunctionEntity* function)
{
	// Clang's exclude_from_explicit_instantiation on the member or its
	// pattern's declarations.
	const FunctionEntity* pattern = PatternOf(function);
	return function->excluded_instantiation || (pattern != nullptr && pattern->excluded_instantiation);
}

bool Analyzer::InstantiationSuppressed(const FunctionEntity* function)
{
	// 14.7.2p10: a specialization, or a member of a class template
	// specialization, that an explicit instantiation declaration names and
	// no definition does, and that is not inline. GNU always_inline's calls
	// run the body the unit sees (see ProgramLowering::inline_copy), so it
	// is instantiated here too: libc++ marks the members its library does
	// not provide always_inline where Clang's
	// exclude_from_explicit_instantiation is not reported (__has_attribute),
	// also members defined outside their class without inline.
	if (!function->instantiated || function->explicit_instantiation || function->is_inline || Excluded(function))
		return false;
	const FunctionEntity* pattern = PatternOf(function);
	if (function->always_inline || (pattern != nullptr && pattern->always_inline) ||
		(function->primary != nullptr && function->primary->always_inline))
		return false;
	if (function->extern_instantiation)
		return true;
	if (pattern == nullptr || function->owner == nullptr || function->owner->kind != ScopeKind::Class)
		return false;
	const ClassTemplateFacts* facts = static_cast<const ClassEntity*>(function->owner->entity)->templ;
	return facts != nullptr && facts->extern_instantiation && !facts->explicit_instantiation;
}

bool Analyzer::InstantiationSuppressed(const VariableEntity* variable)
{
	// 14.7.2p10: a static data member of a class template specialization an
	// explicit instantiation declaration names (anywhere in the unit: the
	// unit's end is a point of instantiation, 14.6.4.1p8) and no definition
	// does is another unit's to define.
	const VariableTemplateFacts* facts = variable->templ;
	if (facts == nullptr || facts->pattern == nullptr || facts->arguments != nullptr || variable->owner == nullptr ||
		variable->owner->kind != ScopeKind::Class)
		return false;
	const ClassTemplateFacts* owner = static_cast<const ClassEntity*>(variable->owner->entity)->templ;
	return owner != nullptr && owner->extern_instantiation && !owner->explicit_instantiation;
}

Binding* Analyzer::member_template_specialization(
	const DeclaratorInfo& info, Scope* scope, bool definition, uint32_t location)
{
	// 14.7.3p16-p18: `template<> template<...>` declares a member template of
	// a class template specialization explicitly specialized: the
	// specialization's member template of an equivalent head (14.5.6.1)
	// takes this definition instead of the class template's, defined once
	// and before a use instantiates it (p6).
	explicit_specialization_ = false;
	if (info.qualifier == nullptr || !instance_class(info.qualifier))
		error(location, "an explicit specialization of a member template outside a class template specialization");
	const DeclaredName name = declared_name(*info.name, info.qualifier);
	const FunctionList* same = names_.overload(info.qualifier, name.key, declared_type(info.type),
		template_shape(template_entities_.data(), static_cast<uint32_t>(template_entities_.size())));
	if (same == nullptr || same->binding->entity->owner != info.qualifier)
		error(location, "an explicit specialization matches no member template of the class template specialization");
	FunctionEntity* function = static_cast<FunctionEntity*>(same->binding->entity);
	if (definition && function->explicit_specialization && function->defined)
		error(location, "redefinition of a function");
	if (function->explicit_specialization && function->defined && !definition)
		return same->binding;
	function->explicit_specialization = true;
	if (definition)
	{
		// Its own definition replaces the one its class copied.
		function->defined = true;
		function->body = nullptr;
		function->template_parameters = scope;
		if (info.parameters != nullptr)
			function->parameters = info.parameters;
		function_template_facts(function)->pattern = nullptr;
	}
	Binding* binding = new_binding(BindingKind::Function, name.key, function, info.type);
	binding->hidden = true;
	binding->declared_in = function->owner;
	return binding;
}

FunctionEntity* Analyzer::deduce_for_declaration(
	FunctionEntity* primary, const TypeList& explicit_arguments, const Type* type)
{
	// 14.8.2.6: the template's function type against the declaration's,
	// after its explicit arguments (14.8.1); the specialization's type is
	// the declared one.
	const TemplateHead* head = HeadOf(primary);
	if (head == nullptr)
		return nullptr;
	TypeList arguments(head->count, nullptr);
	TypeList converted;
	// A trailing parameter pack given no explicit argument is deduced
	// (14.8.1p9), not taken as empty.
	if (!match_template_arguments(head, explicit_arguments.data(), static_cast<uint32_t>(explicit_arguments.size()),
			converted, false, 0, true))
		return nullptr;
	for (size_t i = 0; i < converted.size(); ++i)
		arguments[i] = converted[i];
	const uint32_t depth = head->count != 0 ? head->parameters[0]->depth : 0;
	const Type* declared = type->canonical;
	const Type* pattern = declaration_pattern(primary->type->canonical, declared, depth);
	// Deduction may extend a pack given explicit arguments (14.8.1p9): it is
	// deduced whole, and must begin with them.
	for (size_t i = 0; i < converted.size(); ++i)
	{
		if (head->parameters[i]->pack && converted[i] != nullptr && converted[i]->kind == TypeKind::Pack)
			arguments[i] = nullptr;
	}
	bool deduced = deduce_type(pattern, declared, depth, arguments, false);
	for (size_t i = 0; i < converted.size() && deduced; ++i)
	{
		const Type* given = converted[i];
		if (arguments[i] == given || !head->parameters[i]->pack || given == nullptr || given->kind != TypeKind::Pack)
			continue;
		const Type* found = arguments[i];
		deduced = found != nullptr && found->kind == TypeKind::Pack && found->param_count >= given->param_count;
		for (uint32_t k = 0; k < given->param_count && deduced; ++k)
			deduced = found->params[k]->canonical == given->params[k]->canonical;
	}
	if (!deduced)
	{
		// 14.8.2p3, 14.8.1p2: the explicit arguments are substituted into the
		// function type first (a parameter T&& given T = int& is int&,
		// 8.3.2p6), the rest deduced from what remains.
		if (explicit_arguments.empty())
			return nullptr;
		for (size_t i = 0; i < arguments.size(); ++i)
			arguments[i] = i < converted.size() ? converted[i] : nullptr;
		const Entity* saved = substitution_context_;
		substitution_context_ = nullptr;
		SoftSubstitution soft(*this);
		const Type* substituted = substitute(
			primary->type->canonical, Substitution{ depth, arguments.data(), head->count, nullptr });
		substitution_context_ = saved;
		if (soft.failed() || substituted == nullptr ||
			!deduce_type(
				declaration_pattern(substituted->canonical, declared, depth), declared, depth, arguments, false))
			return nullptr;
	}
	if (!complete_deduction(primary, arguments))
		return nullptr;
	FunctionEntity* function = specialize(primary, arguments);
	return function != nullptr && function->type->canonical == declared ? function : nullptr;
}

const Type* Analyzer::declaration_pattern(const Type* pattern, const Type* declared, uint32_t depth)
{
	// 14.8.2.5p10: deducing from a function declaration (14.8.2.6), a
	// top-level parameter that is an rvalue reference to a cv-unqualified
	// template parameter, against an lvalue reference, is that template
	// parameter (T&& against X& deduces T = X&); a trailing function
	// parameter pack T&&... is T... against lvalue references alone.
	if (pattern->kind != TypeKind::Function || declared->kind != TypeKind::Function)
		return pattern;
	const auto forwarding = [depth](const Type* parameter) {
		if (parameter->kind != TypeKind::RValueReference || parameter->element->kind != TypeKind::TemplateParameter ||
			parameter->element->cv != 0)
			return false;
		const TemplateParameterEntity* entity = static_cast<const TemplateParameterEntity*>(parameter->element->entity);
		return entity->depth == depth && !IsPlaceholder(entity);
	};
	TypeList parameters;
	bool adjusted = false;
	for (uint32_t i = 0; i < pattern->param_count; ++i)
	{
		const Type* parameter = pattern->params[i];
		if (parameter->kind == TypeKind::Expansion && i + 1 == pattern->param_count && forwarding(parameter->element))
		{
			bool lvalues = declared->param_count > i;
			for (uint32_t j = i; j < declared->param_count && lvalues; ++j)
				lvalues = declared->params[j]->kind == TypeKind::LValueReference;
			if (lvalues)
			{
				parameter = types_.expansion(parameter->element->element);
				adjusted = true;
			}
		}
		else if (i < declared->param_count && forwarding(parameter) &&
			declared->params[i]->kind == TypeKind::LValueReference)
		{
			parameter = parameter->element;
			adjusted = true;
		}
		parameters.push_back(parameter);
	}
	if (!adjusted)
		return pattern;
	return types_
		.function(pattern->element, parameters.data(), static_cast<uint32_t>(parameters.size()), pattern->variadic,
			pattern->function_cv, pattern->ref)
		->canonical;
}

const Identifier* Analyzer::specialization_key(const syntax::Name& name)
{
	// The lookup key of a declarator-id naming a function template's
	// specialization: its name without its template arguments.
	syntax::NameComponent last;
	const syntax::Name bare = WithoutArguments(name, last);
	return declared_name(bare, graph_.global).key;
}

ClassEntity* Analyzer::partial_specialization(const syntax::Name& name, TokenKind key, Scope* scope)
{
	// 14.5.5: under a template-head, a class head naming a template-id
	// declares a partial specialization of the named template: a pattern of
	// its own, with its template head and its argument list in terms of
	// its parameters, which specializations it matches deduce (14.5.5.1).
	// Its argument list is restricted (14.5.5p8), and it is declared in
	// its template's scope.
	const syntax::NameComponent& last = name.last();
	syntax::NameComponent bare_last;
	syntax::Name bare = syntax::Name();
	bare.global = name.global;
	bare.location = name.location;
	SmallList<syntax::NameComponent, 4> components;
	for (uint32_t i = 0; i + 1 < name.components.size; ++i)
		components.push_back(name.components[i]);
	bare_last = last;
	bare_last.has_arguments = false;
	bare_last.arguments = syntax::Slice<syntax::TemplateArgument>{ nullptr, 0 };
	components.push_back(bare_last);
	bare.components = syntax::Slice<syntax::NameComponent>{ components.data(),
		static_cast<uint32_t>(components.size()) };
	const LookupResult result = lookup_name(bare, scope, LookupFilter::Elaborated);
	ClassEntity* templ = result.binding->kind == BindingKind::Type ? class_template_of(result.binding->entity)
																   : nullptr;
	if (templ == nullptr || templ->templ->role != TemplateRole::Template || templ->templ->head == nullptr)
		error(name.location, "a partial specialization of no class template");
	if ((key == TokenKind::KW_UNION) != (templ->key == TokenKind::KW_UNION))
		error(name.location, "class-key does not agree with the template's");
	if (!encloses(declaration_scope(scope->parent), templ->owner))
		error(name.location, "a partial specialization outside its template's scope");
	// Its argument list is complete, omitted defaults supplied (14.1p9).
	TypeList given;
	template_arguments(last, scope, given, templ->templ->head);
	default_template_arguments(templ->templ->head, given, name.location);
	TypeList arguments;
	match_template_arguments(
		templ->templ->head, given.data(), static_cast<uint32_t>(given.size()), arguments, true, name.location);
	// A redeclaration (its definition after a declaration) declares the
	// partial specialization of its argument list again.
	const uint32_t count = static_cast<uint32_t>(arguments.size());
	if (ClassEntity* earlier = partial_of_arguments(templ, arguments.data(), count))
	{
		merge_template_head(earlier->templ->head, *template_list_, template_scope_, name.location);
		check_partial_arguments(templ->templ->head, earlier->templ->head, arguments.data(), count, name.location);
		return earlier;
	}
	const TemplateHead* head = template_head(*template_list_, template_scope_, template_entities_);
	check_partial_arguments(templ->templ->head, head, arguments.data(), count, name.location);
	ClassEntity* entity = new_class(templ->key, templ->name, templ->owner);
	ClassTemplateFacts* facts = class_template_facts(entity, TemplateRole::Partial);
	facts->pattern = templ;
	facts->head = const_cast<TemplateHead*>(head);
	facts->arguments = arena_.copy_array(arguments.data(), arguments.size());
	facts->argument_count = static_cast<uint32_t>(arguments.size());
	facts->definition_parameters = scope;
	facts->dependent = true;
	entity->label = templ->label;
	partial_arguments_.insert(SpecializationKey{ templ, facts->arguments, facts->argument_count }, entity);
	// Indexed (see partial_index_key), else the last of the unindexed ones.
	const Type* index_key[2];
	const bool indexed = partial_index_key(templ->templ->head, facts->arguments, facts->argument_count, index_key);
	ClassEntity* former = static_cast<ClassEntity*>(index_partial(
		templ, entity, indexed ? index_key : nullptr, templ->templ->partials, templ->templ->partial_positions));
	if (indexed)
		facts->next_partial = former;
	else if (former != nullptr)
		former->templ->next_partial = entity;
	else
		templ->templ->partials = entity;
	return entity;
}

void Analyzer::check_partial_arguments(const TemplateHead* primary, const TemplateHead* head,
	const Type* const* arguments, uint32_t count, uint32_t location)
{
	// 14.5.5p8, for a partial specialization of the template of head
	// `primary`, its own head `head` and template parameter list the
	// declaration's: no default template argument; not the primary
	// template's implicit argument list (its parameters in order); no
	// unexpanded parameter pack in an argument, and a pack expansion only
	// as the last argument; no specialized non-type argument for a
	// parameter whose type (substituted) depends on its parameters. And
	// each of its parameters deducible from its arguments, appearing in a
	// deduced context (14.8.2.5p8, none of p5's non-deduced ones): the rule
	// of CWG 1315, which replaced p8's "a non-type argument expression
	// involving a parameter is just its name" and which the hosts apply in
	// C++11 (a PA15 fixture's `v<T, sizeof(T) * 0>` is valid by it).
	for (uint32_t i = 0; template_list_ != nullptr && i < template_list_->parameters.size; ++i)
	{
		const syntax::TemplateParameter* parameter = template_list_->parameters[i];
		if (parameter->default_type != nullptr || parameter->default_value != nullptr)
			error(location, "a default template argument in a partial specialization's template parameter list");
	}
	bool own = count == head->count;
	for (uint32_t i = 0; i < count && own; ++i)
	{
		const Type* parameter = head->parameters[i]->canonical->type;
		if (head->parameters[i]->pack)
		{
			const Type* expansion = types_.expansion(parameter);
			parameter = types_.pack(&expansion, 1);
		}
		own = arguments[i] == parameter;
	}
	if (own)
		error(location, "a partial specialization whose arguments are its template's own parameters");
	// A parameter pack's argument is a pack of the arguments given for it,
	// each an argument.
	for (uint32_t i = 0; i < count; ++i)
	{
		const Type* argument = arguments[i];
		const uint32_t elements = argument->kind == TypeKind::Pack ? argument->param_count : 1;
		for (uint32_t j = 0; j < elements; ++j)
		{
			const Type* element = argument->kind == TypeKind::Pack ? argument->params[j] : argument;
			PackList packs;
			if (element->kind != TypeKind::Expansion)
				unexpanded_packs(element, kAnyDepth, packs);
			else if (i + 1 != count || j + 1 != elements)
				error(location, "a pack expansion other than a partial specialization's last argument");
			if (!packs.empty())
				error(location, "a partial specialization's argument names an unexpanded parameter pack");
		}
	}
	const uint32_t depth = head->count != 0 ? head->parameters[0]->depth : 0;
	SmallList<uint8_t, 8> deduced(head->count, 0);
	deduced_parameters(arguments, count, true, depth, deduced);
	for (uint32_t i = 0; i < head->count; ++i)
	{
		if (deduced[i] == 0)
			error(location, "a partial specialization's template parameter that its arguments do not deduce");
	}
	for (uint32_t i = 0; i < count && i < primary->count; ++i)
	{
		// A specialized non-type argument: other than the name of one of
		// its parameters (which a converted one names). Whether the type
		// depends on its parameters: at depth 0 any dependence (no others
		// are in scope); deeper, those a deduced context names.
		const TemplateParameterEntity* parameter = primary->parameters[i];
		if (parameter->pack || parameter->value_type == nullptr || !dependent_type(parameter->value_type))
			continue;
		const TemplateParameterEntity* named = arguments[i]->kind == TypeKind::TemplateParameter
			? static_cast<const TemplateParameterEntity*>(arguments[i]->entity)
			: nullptr;
		if (named != nullptr && named->named != nullptr)
			named = named->named;
		if (named != nullptr && !IsPlaceholder(named) && named->depth == depth)
			continue;
		const Type* type = substitute(parameter->value_type, substitution_of(primary, arguments));
		bool dependent = depth == 0 && dependent_type(type);
		if (depth != 0)
		{
			SmallList<uint8_t, 8> named_parameters(head->count, 0);
			deduced_parameters(&type, 1, false, depth, named_parameters);
			for (uint8_t mark : named_parameters)
				dependent = dependent || mark != 0;
		}
		if (dependent)
			error(location,
				"a specialized non-type argument for a parameter whose type depends on the partial "
				"specialization's parameters");
	}
}

void Analyzer::deduced_parameters(
	const Type* const* types, uint32_t count, bool template_arguments, uint32_t depth, SmallList<uint8_t, 8>& deduced)
{
	// The template parameters of `depth` a list of types names in deduced
	// contexts (14.8.2.5p8), marked by index: a template argument list's
	// (argument packs flattened), wholly non-deduced when a pack expansion
	// is not its last argument (p9), or a function parameter list's, where
	// such an expansion alone is non-deduced (p5). Not a nested-name-
	// specifier's, a decltype's, nor a non-type argument's or array bound's
	// expression but a parameter's name alone (p5).
	for (uint32_t pass = template_arguments ? 0 : 1; pass < 2; ++pass)
	{
		for (uint32_t i = 0; i < count; ++i)
		{
			const bool pack = types[i]->kind == TypeKind::Pack;
			const uint32_t elements = pack ? types[i]->param_count : 1;
			for (uint32_t j = 0; j < elements; ++j)
			{
				const Type* type = pack ? types[i]->params[j] : types[i];
				const bool trailing = i + 1 == count && j + 1 == elements;
				if (pass == 0 && type->kind == TypeKind::Expansion && !trailing)
					return;
				if (pass == 1 && (type->kind != TypeKind::Expansion || trailing))
					deduced_parameters(type, depth, deduced);
			}
		}
	}
}

void Analyzer::deduced_parameters(const Type* type, uint32_t depth, SmallList<uint8_t, 8>& deduced)
{
	for (;;)
	{
		switch (type->kind)
		{
		case TypeKind::TemplateParameter:
		case TypeKind::TemplateTemplateParameter:
		{
			// A value placeholder: the parameter it names alone.
			const TemplateParameterEntity* parameter = static_cast<const TemplateParameterEntity*>(type->entity);
			if (parameter->qualifier == nullptr && parameter->expression != nullptr && parameter->non_type)
				MarkDeduced(parameter->named, depth, deduced);
			else if (!IsPlaceholder(parameter))
				MarkDeduced(parameter, depth, deduced);
			return;
		}
		case TypeKind::Class:
		{
			// A template template-parameter's specialization deduces it too
			// (TT<T>); a dependent member template-name's is non-deduced.
			const ClassTemplateFacts* facts = static_cast<const ClassEntity*>(type->entity)->templ;
			// An alias template-id dropping dependent arguments deduces as
			// the type it aliases (CWG 1558).
			if (const Type* aliased = aliased_of(type))
			{
				type = aliased;
				continue;
			}
			if (facts == nullptr || facts->role != TemplateRole::Specialization || !facts->dependent)
				return;
			if (const TemplateParameterEntity* name = facts->pattern->templ->template_name)
			{
				if (name->qualifier != nullptr)
					return;
				deduced_parameters(name->type, depth, deduced);
			}
			deduced_parameters(facts->arguments, facts->argument_count, true, depth, deduced);
			return;
		}
		case TypeKind::Array:
		case TypeKind::Vector:
			// A dependent bound's (or vector size's) placeholder: the
			// parameter it names alone.
			if (type->owner != nullptr)
				MarkDeduced(static_cast<const TemplateParameterEntity*>(type->owner->entity)->named, depth, deduced);
			type = type->element;
			continue;
		case TypeKind::MemberPointer:
			deduced_parameters(type->owner, depth, deduced);
			type = type->element;
			continue;
		case TypeKind::Pointer:
		case TypeKind::LValueReference:
		case TypeKind::RValueReference:
		case TypeKind::Expansion:
		case TypeKind::Mode:
			type = type->element;
			continue;
		case TypeKind::Function:
			deduced_parameters(type->params, type->param_count, false, depth, deduced);
			type = type->element;
			continue;
		case TypeKind::Pack:
			deduced_parameters(type->params, type->param_count, true, depth, deduced);
			return;
		default:
			return;
		}
	}
}

bool Analyzer::partial_index_key(
	const TemplateHead* head, const Type* const* arguments, uint32_t count, const Type** key)
{
	// The partial_index_ key of a partial specialization's argument list
	// (`head` its template's): its first argument that depends on no
	// parameter (a value of its parameter's own type), which each
	// specialization it matches has there too; else its first that is a
	// class template's specialization (see ArgumentTemplate), by that
	// template; false for neither.
	for (uint32_t position = 0; position < count && position < 64; ++position)
	{
		if (IndexableArgument(head, arguments, position) && !dependent_type(arguments[position]))
		{
			key[0] = PositionTag(position, false);
			key[1] = arguments[position];
			return true;
		}
	}
	for (uint32_t position = 0; position < count && position < 64; ++position)
	{
		// An alias template-id dropping dependent arguments: the type it
		// aliases.
		const Type* argument = arguments[position];
		for (const Type* aliased = argument->cv == 0 ? aliased_of(argument) : nullptr; aliased != nullptr;
			aliased = argument->cv == 0 ? aliased_of(argument) : nullptr)
			argument = aliased;
		if (const ClassEntity* templ = ArgumentTemplate(head, argument, position))
		{
			key[0] = PositionTag(position, true);
			key[1] = templ->type;
			return true;
		}
	}
	return false;
}

Entity* Analyzer::index_partial(
	Entity* templ, Entity* partial, const Type* const* key, Entity* first, uint64_t& positions)
{
	// A new partial specialization made the first of its index list (the
	// former first returned, which it links next; its position marked in
	// the template's `positions`), or without a key the last of its
	// template's unindexed ones, of which `first` is the first (the former
	// last returned, which links it; null for none). The last is kept once
	// there are two.
	if (key != nullptr)
		positions |= uint64_t(1) << ((reinterpret_cast<uintptr_t>(key[0]) - 1) & 63);
	else if (first == nullptr)
		return nullptr;
	SpecializationKey probe = SpecializationKey{ templ, key, key != nullptr ? 2u : 0u };
	if (Entity** slot = partial_index_.find(probe))
	{
		Entity* former = *slot;
		*slot = partial;
		return former;
	}
	if (key != nullptr)
		probe.arguments = arena_.copy_array(key, 2);
	partial_index_.insert(probe, partial);
	return key != nullptr ? nullptr : first;
}

void Analyzer::indexed_partials(
	const Entity* templ, uint64_t positions, const Type* const* arguments, uint32_t count, SmallList<Entity*, 4>& lists)
{
	// The index lists of the partial specializations a specialization's
	// arguments may match: at each indexed position, those of the argument
	// there and those of its class template, if it is a specialization.
	for (uint32_t position = 0; position < count && position < 64; ++position)
	{
		if ((positions >> position & 1) == 0)
			continue;
		const Type* argument = arguments[position]->canonical;
		const Type* key[2] = { PositionTag(position, false), argument };
		if (Entity** first = partial_index_.find(SpecializationKey{ templ, key, 2 }))
			lists.push_back(*first);
		if (argument->kind != TypeKind::Class || argument->cv != 0)
			continue;
		const ClassTemplateFacts* facts = static_cast<const ClassEntity*>(argument->entity)->templ;
		if (facts == nullptr || facts->role != TemplateRole::Specialization)
			continue;
		key[0] = PositionTag(position, true);
		key[1] = facts->pattern->type;
		if (Entity** first = partial_index_.find(SpecializationKey{ templ, key, 2 }))
			lists.push_back(*first);
	}
}

ClassEntity* Analyzer::partial_of_arguments(ClassEntity* templ, const Type* const* arguments, uint32_t count)
{
	// The partial specialization declared with this canonical argument list
	// (a redeclaration's; the current instantiation a dependent template-id
	// of that list names).
	if (templ->templ->partials == nullptr && templ->templ->partial_positions == 0)
		return nullptr;
	ClassEntity** found = partial_arguments_.find(SpecializationKey{ templ, arguments, count });
	return found != nullptr ? *found : nullptr;
}

Entity* Analyzer::class_partial_specialization(const syntax::ClassSpecifier& specifier, Scope* scope, Specifiers& out)
{
	// Under `template<>` too: `template<> template<class U> struct
	// O<int>::I<U*>` partially specializes the member template of a class
	// template specialization (14.7.3p16, 14.5.5.3), one template<> per
	// explicitly specialized enclosing class template (p15).
	if (explicit_specialization_)
	{
		explicit_specialization_ = false;
		Scope* saved_deferral = access_deferral_;
		const size_t pending = pending_access_.size();
		access_deferral_ = semantic() ? scope : nullptr;
		Scope* qualifier = specifier.name->qualified() ? name_prefix(*specifier.name, scope, true).scope : nullptr;
		pending_access_.resize(pending);
		access_deferral_ = saved_deferral;
		if (qualifier == nullptr || !instance_class(qualifier))
			error(specifier.name->location,
				"a partial specialization under template<> of no member template of a class template "
				"specialization");
		if (specialization_levels_ != SpecializedEnclosing(qualifier))
			error(specifier.name->location,
				"a member template's partial specialization with the wrong number of template<>");
	}
	ClassEntity* entity = partial_specialization(*specifier.name, specifier.key, scope);
	if (entity->templ->definition != nullptr)
		error(specifier.name->location, "class redefinition");
	entity->templ->definition = &specifier;
	entity->templ->definition_parameters = scope;
	Binding* binding = new_binding(BindingKind::Type, entity->name, entity, entity->type);
	binding->hidden = true;
	binding->key = specifier.key;
	entity->templ->binding = binding;
	out.declared_binding = binding;
	// A qualified one (a member template's, 14.5.5.3) looks names up in the
	// class or namespace it names, its template parameters bridged over it
	// (3.4.3p3).
	const Scope* owner = entity->templ->pattern->owner;
	// Keep the selected declaration's analyzed definition origin. Matching
	// it again in the original class would lose instance-local aliases.
	if (!specifier.name->qualified() && WithinInstance(owner))
	{
		entity->templ->partial = member_partial_pattern(specifier);
		entity->templ->definition = nullptr;
		return entity;
	}
	if (owner != nullptr && owner->kind == ScopeKind::Class)
	{
		member_partial_definitions_.set(&specifier, static_cast<uint32_t>(member_partial_patterns_.size()));
		member_partial_patterns_.push_back(entity);
	}
	Scope* parent = specifier.name->qualified() ? qualified_lookup_scope(const_cast<Scope*>(owner), scope) : scope;
	class_body(entity, specifier, parent, scope);
	// Its definition context ends with it, as a primary template's (14.6.4.2).
	entity->templ->definition_ordinal = definition_point();
	return entity;
}

ClassEntity* Analyzer::member_partial_pattern(const syntax::ClassSpecifier& specifier)
{
	// A class instantiation pairs source and copy members by declaration
	// position (as link_member_patterns does), never by names or locations.
	if (class_instantiations_.empty())
		return nullptr;
	const ClassInstantiation& active = class_instantiations_.back();
	if (active.declared >= active.pattern->members.size)
		return nullptr;
	const syntax::Decl* declaration = active.pattern->members[active.declared];
	const syntax::Decl* copy = active.copy->members[active.declared];
	while (declaration->kind == syntax::DeclKind::Template && copy->kind == syntax::DeclKind::Template)
	{
		declaration = static_cast<const syntax::TemplateDecl*>(declaration)->declaration;
		copy = static_cast<const syntax::TemplateDecl*>(copy)->declaration;
	}
	if (declaration->kind != syntax::DeclKind::Class || copy->kind != syntax::DeclKind::Class ||
		static_cast<const syntax::ClassDecl*>(copy)->specifier.class_specifier != &specifier)
		return nullptr;
	const syntax::ClassSpecifier* source =
		static_cast<const syntax::ClassDecl*>(declaration)->specifier.class_specifier;
	const uint32_t index = member_partial_definitions_.find(source);
	if (index == PointerMap::kMissing)
		return nullptr;
	member_partial_definitions_.set(&specifier, index);
	return member_partial_patterns_[index];
}

ClassEntity* Analyzer::matching_partial(
	ClassEntity* templ, const Type* const* arguments, uint32_t count, TypeList& deduced, const ClassEntity* instance)
{
	// 14.5.5.1: the partial specializations whose argument lists deduce
	// from the specialization's arguments (every parameter deduced, a pack
	// to an empty one at worst, and the substituted list the arguments
	// themselves); of several, the one more specialized than the others
	// (14.5.5.2), none being an error. An indexed partial specialization
	// can match only a specialization with its indexing argument (or a
	// specialization of its indexing template) there: those are found by
	// the specialization's arguments at the indexed positions, the others
	// all tried.
	SmallList<Entity*, 4> lists;
	lists.push_back(templ->templ->partials);
	indexed_partials(templ, templ->templ->partial_positions, arguments, count, lists);
	// The one more specialized than each other match (14.5.5.2): the last
	// winner of the matches in turn, checked against those before it.
	SmallList<ClassEntity*, 4> matches;
	ClassEntity* best = nullptr;
	for (Entity* first : lists)
	{
		for (ClassEntity* partial = static_cast<ClassEntity*>(first); partial != nullptr;
			partial = partial->templ->next_partial)
		{
			// An instance already owns the declarations of inline member
			// partials. This fallback searches only declarations outside the
			// class; their instantiated counterparts were already considered.
			if (instance != nullptr && partial->templ->definition_parameters != nullptr &&
				declaration_scope(partial->templ->definition_parameters)->kind == ScopeKind::Class)
				continue;
			TypeList trial;
			if (!deduce_partial(partial, arguments, count, trial, instance))
				continue;
			matches.push_back(partial);
			if (best == nullptr ||
				(partial_at_least_as_specialized(partial, best) && !partial_at_least_as_specialized(best, partial)))
			{
				best = partial;
				deduced = trial;
			}
		}
	}
	for (ClassEntity* other : matches)
	{
		if (other != best &&
			(!partial_at_least_as_specialized(best, other) || partial_at_least_as_specialized(other, best)))
			error(0, "a specialization matches several partial specializations, none more specialized");
	}
	return best;
}

bool Analyzer::deduce_partial(const ClassEntity* partial, const Type* const* arguments, uint32_t count,
	TypeList& deduced, const ClassEntity* instance)
{
	// A class template's member template's partial specialization matched
	// for an instance's member template: its enclosing parameters are the
	// instance's arguments (14.5.5.3p2).
	// Its substitution checks access as the class template's (14.8.2p8, a
	// friend of the template befriends its partial specializations,
	// 14.5.4p1).
	++graph_.partial_candidates;
	const ClassTemplateFacts* facts = partial->templ;
	const Entity* saved = substitution_context_;
	substitution_context_ = facts->pattern;
	bool matched;
	if (instance == nullptr)
		matched = deduce_partial_arguments(
			facts->head, facts->arguments, facts->argument_count, arguments, count, deduced);
	else
	{
		TypeList pattern;
		enclosing_substitution(facts->arguments, facts->argument_count, instance, pattern);
		matched = deduce_partial_arguments(
			facts->head, pattern.data(), static_cast<uint32_t>(pattern.size()), arguments, count, deduced);
	}
	substitution_context_ = saved;
	return matched;
}

bool Analyzer::deduce_partial_arguments(const TemplateHead* head, const Type* const* pattern, uint32_t pattern_count,
	const Type* const* arguments, uint32_t count, TypeList& deduced)
{
	// 14.5.5.1p2: a partial specialization's argument list against a
	// specialization's (14.8.2.5): every parameter deduced (a pack to an
	// empty one at worst) and the substituted list the arguments
	// themselves (a non-deduced context or a value placeholder decides only
	// once substituted).
	if (pattern_count != count)
		return false;
	deduced.clear();
	for (uint32_t i = 0; i < head->count; ++i)
		deduced.push_back(nullptr);
	const uint32_t depth = head->count != 0 ? head->parameters[0]->depth : 0;
	for (uint32_t i = 0; i < count; ++i)
	{
		if (!deduce_type(pattern[i], arguments[i], depth, deduced, false))
			return false;
	}
	for (uint32_t i = 0; i < head->count; ++i)
	{
		if (deduced[i] == nullptr && head->parameters[i]->pack)
			deduced[i] = types_.pack(nullptr, 0);
		if (deduced[i] == nullptr)
			return false;
	}
	// Deduced values converted to their parameters' types (an array bound's
	// is a std::size_t, 14.8.2.5p17).
	TypeList converted;
	if (!match_template_arguments(head, deduced.data(), static_cast<uint32_t>(deduced.size()), converted, false, 0))
		return false;
	deduced = converted;
	// A substitution failure means no match (14.5.5.1p2, 14.8.2p8).
	const Substitution substitution = substitution_of(head, deduced.data());
	for (uint32_t i = 0; i < count; ++i)
	{
		SoftSubstitution soft(*this);
		const Type* substituted = substitute(pattern[i], substitution);
		if (soft.failed() || substituted == nullptr || substituted->canonical != arguments[i]->canonical)
			return false;
	}
	return true;
}

bool Analyzer::partial_at_least_as_specialized(const ClassEntity* first, const ClassEntity* second)
{
	return partial_ordered(first, first->templ->head, first->templ->arguments, first->templ->argument_count, second,
		second->templ->head, second->templ->arguments, second->templ->argument_count);
}

bool Analyzer::partial_ordered(const Entity* first, const TemplateHead* head, const Type* const* pattern,
	uint32_t count, const Entity* second, const TemplateHead* other, const Type* const* other_pattern,
	uint32_t other_count)
{
	// Whether the first partial specialization is at least as specialized
	// as the second (14.5.5.2): a function of their declarations, computed
	// once per ordered pair that selections compare.
	const EntityPair key = EntityPair{ first, second };
	if (const bool* known = partial_orderings_.find(key))
		return *known;
	const bool ordered = arguments_at_least_as_specialized(head, pattern, count, other, other_pattern, other_count);
	if (partial_orderings_.find(key) == nullptr)
		partial_orderings_.insert(key, ordered);
	return ordered;
}

bool Analyzer::arguments_at_least_as_specialized(const TemplateHead* head, const Type* const* pattern, uint32_t count,
	const TemplateHead* other, const Type* const* other_pattern, uint32_t other_count)
{
	// 14.5.5.2: the first is at least as specialized as the second when the
	// second's arguments deduce from the first's with the first's
	// parameters unique types (14.8.2.4).
	TypeList unique;
	for (uint32_t i = 0; i < head->count; ++i)
	{
		// A parameter pack's unique argument pack is an expansion, which
		// only a pack expansion of the other list deduces from (14.8.2.5p9).
		unique.push_back(synthesized_argument(head->parameters[i], i));
		if (head->parameters[i]->pack)
		{
			const Type* expansion = types_.expansion(unique[i]);
			unique[i] = types_.pack(&expansion, 1);
		}
	}
	const Substitution substitution = substitution_of(head, unique.data());
	if (count != other_count)
		return false;
	TypeList deduced(other->count, nullptr);
	const uint32_t depth = other->count != 0 ? other->parameters[0]->depth : 0;
	for (uint32_t i = 0; i < count; ++i)
	{
		if (!deduce_type(other_pattern[i], substitute(pattern[i], substitution)->canonical, depth, deduced, false))
			return false;
	}
	return true;
}

bool Analyzer::explicit_variable_specialization(VariableEntity* variable, bool has_initializer, uint32_t location)
{
	// 14.7.3p13, p16: a static data member of a class template
	// specialization, defined here (with an initializer; declared
	// otherwise), not instantiated from its template's definition.
	explicit_specialization_ = false;
	VariableTemplateFacts* facts = variable->templ;
	if (facts != nullptr && facts->state != FactState::NotStarted)
		error(location, "an explicit specialization after the member's instantiation");
	if (facts != nullptr)
		facts->pattern = nullptr;
	return has_initializer;
}

} // namespace sema
