#include "sema/template_packs.h"

#include <cstring>

#include "lex/identifier_table.h"

// Function templates: type template parameters, explicit template
// arguments of a template-id, deduction from a call's arguments (14.8.2.1)
// or from a target type (14.8.2.2), and the substitution that makes each
// specialization's declaration once per (template, arguments). Template
// parameters are positional (see TemplateParameterEntity): substitution and
// deduction read a parameter's depth and index, never its name. The
// specialization's definition is instantiated on demand
// (Analyzer::instantiate_definition).
namespace sema {

namespace {

// Two non-type arguments of one value, the first converted to the second's
// type (an argument written for a parameter of dependent type keeps its
// own type until the parameter's is known).
bool SameValue(const Type* first, const Type* second)
{
	if (first == second)
		return true;
	if (first->kind != TypeKind::Value || second->kind != TypeKind::Value)
		return false;
	// Pointer and reference arguments: one designated entity (or
	// both null pointers).
	if (AddressArgument(first) || AddressArgument(second))
		return AddressArgument(first) && AddressArgument(second) && first->entity == second->entity;
	IntegralKind kind;
	return IntegralKindOf(second->element, kind) && Representable(Constant{ first->bound, first->element }, kind) &&
		ConvertIntegral(first->bound, kind) == second->bound;
}

bool NonstaticMember(const FunctionEntity* function)
{
	return function->member && !function->is_static;
}

// Whether a (canonical) type has an array bound that is the non-type
// template parameter (depth, index)'s name; deep types answer yes.
bool BoundNames(const Type* type, uint32_t depth, uint32_t index, uint32_t level)
{
	if (level > 16)
		return true;
	switch (type->kind)
	{
	case TypeKind::Pointer:
	case TypeKind::LValueReference:
	case TypeKind::RValueReference:
	case TypeKind::Expansion:
		return BoundNames(type->element, depth, index, level + 1);
	case TypeKind::Array:
	case TypeKind::Vector:
		if (type->owner != nullptr)
		{
			const TemplateParameterEntity* bound =
				static_cast<const TemplateParameterEntity*>(type->owner->entity)->named;
			if (bound != nullptr && bound->depth == depth && bound->index == index)
				return true;
		}
		return BoundNames(type->element, depth, index, level + 1);
	case TypeKind::MemberPointer:
		return BoundNames(type->owner, depth, index, level + 1) || BoundNames(type->element, depth, index, level + 1);
	case TypeKind::Function:
	case TypeKind::Pack:
		for (uint32_t i = 0; i < type->param_count; ++i)
		{
			if (BoundNames(type->params[i], depth, index, level + 1))
				return true;
		}
		return type->kind == TypeKind::Function && BoundNames(type->element, depth, index, level + 1);
	case TypeKind::Class:
	{
		const ClassTemplateFacts* facts = static_cast<const ClassEntity*>(type->entity)->templ;
		for (uint32_t i = 0;
			facts != nullptr && facts->role == TemplateRole::Specialization && i < facts->argument_count; ++i)
		{
			if (BoundNames(facts->arguments[i], depth, index, level + 1))
				return true;
		}
		return false;
	}
	default:
		return false;
	}
}

// Whether a (canonical) parameter type has a template parameter of
// `depth` in a deduced context (14.8.2.5p8), p5's non-deduced contexts
// passed over: a dependent name's qualifier, a decltype-specifier's or a
// template argument's expression, an array bound that is no parameter's
// name. A class template-id whose kind this does not classify, or a deep
// type, answers yes.
bool Deducible(const Type* type, uint32_t depth, uint32_t level)
{
	if (level > 16)
		return true;
	switch (type->kind)
	{
	case TypeKind::TemplateParameter:
	case TypeKind::TemplateTemplateParameter:
	{
		const TemplateParameterEntity* parameter = static_cast<const TemplateParameterEntity*>(
			type->unqualified->entity);
		return !IsPlaceholder(parameter) && parameter->depth == depth;
	}
	case TypeKind::Pointer:
	case TypeKind::LValueReference:
	case TypeKind::RValueReference:
	case TypeKind::Expansion:
		return Deducible(type->element, depth, level + 1);
	case TypeKind::Array:
	case TypeKind::Vector:
		if (type->owner != nullptr)
		{
			const TemplateParameterEntity* bound =
				static_cast<const TemplateParameterEntity*>(type->owner->entity)->named;
			if (bound != nullptr && bound->depth == depth)
				return true;
		}
		return Deducible(type->element, depth, level + 1);
	case TypeKind::MemberPointer:
		return Deducible(type->owner, depth, level + 1) || Deducible(type->element, depth, level + 1);
	case TypeKind::Function:
	case TypeKind::Pack:
		for (uint32_t i = 0; i < type->param_count; ++i)
		{
			if (Deducible(type->params[i], depth, level + 1))
				return true;
		}
		return type->kind == TypeKind::Function && Deducible(type->element, depth, level + 1);
	case TypeKind::Class:
	{
		const ClassTemplateFacts* facts = static_cast<const ClassEntity*>(type->unqualified->entity)->templ;
		if (facts == nullptr || facts->role != TemplateRole::Specialization || !facts->dependent ||
			facts->template_name != nullptr)
			return true;
		// A template template-parameter's specialization TT<...> (its
		// stand-in, see Analyzer::stand_in_template) deduces TT too.
		const ClassEntity* templ = facts->pattern;
		if (templ != nullptr && templ->templ != nullptr && templ->templ->template_name != nullptr &&
			Deducible(templ->templ->template_name->type, depth, level + 1))
			return true;
		for (uint32_t i = 0; i < facts->argument_count; ++i)
		{
			if (Deducible(facts->arguments[i], depth, level + 1))
				return true;
		}
		return false;
	}
	default:
		return false;
	}
}

// A reference type's referred cv-qualifiers, an array's its elements'
// (3.9.3p5), for partial ordering (14.8.2.4p6).
uint8_t ReferredCv(const Type* reference)
{
	const Type* type = reference->element;
	while (type->kind == TypeKind::Array)
		type = type->element;
	return type->cv & (kConst | kVolatile);
}

} // namespace

size_t SpecializationHash::operator()(const SpecializationKey& key) const
{
	size_t hash = reinterpret_cast<uintptr_t>(key.primary) * 0x9E3779B97F4A7C15ull;
	for (uint32_t i = 0; i < key.count; ++i)
		hash = (hash ^ reinterpret_cast<uintptr_t>(key.arguments[i])) * 0x100000001B3ull;
	return hash;
}

bool SpecializationKey::operator==(const SpecializationKey& other) const
{
	if (primary != other.primary || count != other.count)
		return false;
	for (uint32_t i = 0; i < count; ++i)
	{
		if (arguments[i] != other.arguments[i])
			return false;
	}
	return true;
}

Substitution Analyzer::substitution_of(const TemplateHead* head, const Type* const* arguments)
{
	return Substitution{ head->count != 0 ? head->parameters[0]->depth : 0u, arguments, head->count };
}

const Type* Analyzer::substitute_member(const Type* type, const Substitution& substitution)
{
	// A member (class or enumeration) of a templated class, or a dependent
	// name `Q::name`: the member of that name in the substituted enclosing
	// class once it is no longer dependent (a nesting the stack cannot hold
	// is an error).
	check_stack(0);
	const Type* enclosing = nullptr;
	const Identifier* name = type->entity->name;
	if (type->kind == TypeKind::TemplateParameter)
		enclosing = static_cast<const TemplateParameterEntity*>(type->entity)->qualifier;
	else
		enclosing = type->entity->owner->entity->type;
	const Type* substituted = substitute(enclosing, substitution);
	if (substituted == enclosing)
		return type;
	if (substituted == nullptr)
		return nullptr;
	if (dependent_type(substituted))
	{
		if (type->kind == TypeKind::TemplateParameter)
			return types_.qualified(dependent_member(substituted, name), type->cv);
		return type;
	}
	// A substitution failure (14.8.2p8: no such member type, a qualifier
	// that is no complete class) is a null result where the caller allows
	// one (see soft_substitution_), else an error.
	// A class being defined (its member-specification under way, 9.2p2)
	// has the members declared so far (the substitution not memoized).
	const ClassEntity* cls = substituted != nullptr ? ClassOf(substituted) : nullptr;
	const bool defining = cls != nullptr && !cls->complete && cls->scope != nullptr;
	if (defining)
		++incomplete_classes_seen_;
	if (cls == nullptr || (!defining && !require_complete(cls)))
	{
		substitution_error(0, "a dependent name's qualifier is not a complete class", cls != nullptr);
		return nullptr;
	}
	const LookupResult found = names_.qualified(cls->scope, name,
		type->kind == TypeKind::TemplateParameter ? LookupFilter::Ordinary : LookupFilter::Elaborated);
	if (!found.found() || found.ambiguous ||
		(found.binding->kind != BindingKind::Type && found.binding->kind != BindingKind::TypeAlias))
	{
		substitution_error(0,
			found.ambiguous ? "an ambiguous dependent name"
							: "a dependent name does not name a type in its instantiated qualifier");
		return nullptr;
	}
	// Access is checked as part of a deduction's substitution (14.8.2p8), in
	// the context of the template deduced (11p4: as its class's member or a
	// friend; not its caller's).
	if (soft_substitution_ != 0 && substitution_context_ != nullptr &&
		!(found.binding->access == Access::Public && found.binding->declared_in == cls->scope))
	{
		Scope context = Scope();
		context.kind = substitution_context_->kind == EntityKind::Function ? ScopeKind::Function : ScopeKind::Class;
		context.parent = substitution_context_->owner;
		context.entity = const_cast<Entity*>(substitution_context_);
		check_access(found.binding, cls, nullptr, &context, 0);
		if (failed())
			return nullptr;
	}
	return types_.qualified(binding_type(found.binding, 0), type->cv);
}

const Type* Analyzer::substitute(const Type* type, const Substitution& substitution)
{
	switch (type->kind)
	{
	case TypeKind::TemplateParameter:
	{
		const TemplateParameterEntity* parameter = static_cast<const TemplateParameterEntity*>(type->entity);
		if (parameter->qualifier != nullptr)
			return substitute_member(type, substitution);
		if (parameter->expression != nullptr)
			return parameter->non_type ? substitute_value(type, substitution) : substitute_decltype(type, substitution);
		if (parameter->depth != substitution.depth || parameter->index >= substitution.count ||
			substitution.arguments[parameter->index] == nullptr || type->unqualified == auto_placeholder_)
			return type;
		// A pack an enclosing expansion substitutes element by element
		// stands for that element.
		const Type* argument = substitution.arguments[parameter->index];
		if (parameter->pack && argument->kind == TypeKind::Pack && substitution.elements != nullptr &&
			substitution.elements[parameter->index] >= 0)
		{
			argument = argument->params[substitution.elements[parameter->index]];
			// A partially bound pack's remaining expansion supplies its
			// pattern here; substitute_list retains the enclosing expansion.
			if (argument->kind == TypeKind::Expansion)
				argument = argument->element;
		}
		return types_.qualified(argument, type->cv);
	}
	case TypeKind::TemplateTemplateParameter:
		return substitute_template_name(type, substitution);
	case TypeKind::Class:
	{
		ClassEntity* entity = static_cast<ClassEntity*>(type->entity);
		const ClassTemplateFacts* facts = entity->templ;
		if (facts == nullptr)
			return dependent_type(type) ? substitute_member(type, substitution) : type;
		if (facts->template_name != nullptr)
			return type;
		if (facts->role == TemplateRole::Specialization && facts->dependent && IsStandIn(facts->pattern))
			return substitute_stand_in(type, substitution);
		// An alias template-id that drops dependent arguments (CWG 1558):
		// formed again from the substituted arguments.
		if (facts->role == TemplateRole::Specialization && facts->pattern->templ->alias_stand_in)
		{
			TypeList given;
			TypeList arguments;
			flattened_arguments(facts->arguments, facts->argument_count, given);
			substitute_list(given.data(), static_cast<uint32_t>(given.size()), substitution, arguments);
			if (AnyNull(arguments))
				return nullptr;
			const Type* result = alias_specialization(stand_in_alias(facts->pattern), arguments, 0);
			return result != nullptr ? types_.qualified(result, type->cv) : nullptr;
		}
		// The arguments as a template-id gives them (argument packs'
		// elements in their place, pack expansions expanded), formed into a
		// specialization again.
		TypeList given;
		TypeList arguments;
		ClassEntity* templ = nullptr;
		if (facts->role == TemplateRole::Template)
		{
			// The current instantiation: the template's own parameters.
			templ = entity;
			for (uint32_t i = 0; i < facts->head->count; ++i)
			{
				const Type* parameter = facts->head->parameters[i]->type;
				given.push_back(facts->head->parameters[i]->pack ? types_.expansion(parameter) : parameter);
			}
			substitute_list(given.data(), static_cast<uint32_t>(given.size()), substitution, arguments);
		}
		else if (facts->role == TemplateRole::Specialization && facts->dependent)
		{
			templ = facts->pattern;
			flattened_arguments(facts->arguments, facts->argument_count, given);
			substitute_list(given.data(), static_cast<uint32_t>(given.size()), substitution, arguments);
		}
		else if (facts->role == TemplateRole::Pattern)
			return substitute_member(type, substitution);
		else
			return type;
		if (AnyNull(arguments))
			return nullptr;
		// Arguments of the wrong kinds (a template template-argument whose
		// parameters do not match, 14.3.3) fail a substitution whose failure
		// is a result (14.8.2p8).
		ClassEntity* specialization = class_specialization(templ, arguments, 0, soft_substitution_ == 0);
		if (specialization == nullptr)
		{
			substitution_error(0, "template arguments that do not match their template's parameters");
			return nullptr;
		}
		// The current instantiation substituted by its own parameters stays
		// itself.
		if (facts->role == TemplateRole::Template && specialization->templ->primary_arguments)
			return type;
		return types_.qualified(specialization->type, type->cv);
	}
	case TypeKind::Enum:
		return dependent_type(type) ? substitute_member(type, substitution) : type;
	// A component's substitution failure (see substitute_member) fails the
	// compound type, and so does forming an invalid type (14.8.2p8: a
	// pointer to a reference, a reference to void, an array of void,
	// references, functions or an abstract class, a pointer to member of a
	// non-class, a function returning an array or a function or with a
	// parameter of type void).
	case TypeKind::Pointer:
	{
		const Type* element = substitute(type->element, substitution);
		const Type* pointer = element != nullptr ? checked_pointer(element, 0) : nullptr;
		return pointer != nullptr ? types_.qualified(pointer, type->cv) : nullptr;
	}
	case TypeKind::LValueReference:
	case TypeKind::RValueReference:
	{
		const Type* element = substitute(type->element, substitution);
		if (element != nullptr && element->unqualified == types_.fundamental(FundamentalType::Void))
		{
			substitution_error(0, "reference to void");
			return nullptr;
		}
		return element != nullptr ? types_.reference(element, type->kind == TypeKind::RValueReference) : nullptr;
	}
	case TypeKind::MemberPointer:
	{
		const Type* owner = substitute(type->owner, substitution);
		const Type* element = substitute(type->element, substitution);
		if (owner == nullptr || element == nullptr)
			return nullptr;
		if ((owner->unqualified->kind != TypeKind::Class && !dependent_type(owner)) || element->is_reference() ||
			element->unqualified == types_.fundamental(FundamentalType::Void))
		{
			substitution_error(0, "an invalid pointer to member type");
			return nullptr;
		}
		return types_.qualified(types_.member_pointer(owner->unqualified, element), type->cv);
	}
	case TypeKind::Array:
	{
		if (type->owner != nullptr)
			return substitute_bound(type, substitution);
		const Type* element = substitute(type->element, substitution);
		return element != nullptr ? checked_array(element, type->bound, 0) : nullptr;
	}
	case TypeKind::Vector:
	{
		if (type->owner == nullptr)
			return type;
		const Type* vector = substitute_vector_size(type->unqualified, substitution);
		return vector != nullptr ? types_.qualified(vector, type->cv) : nullptr;
	}
	case TypeKind::Function:
		return substitute_function(type, substitution, true);
	case TypeKind::Pack:
	{
		TypeList elements;
		substitute_list(type->params, type->param_count, substitution, elements);
		return AnyNull(elements) ? nullptr : types_.pack(elements.data(), static_cast<uint32_t>(elements.size()));
	}
	case TypeKind::Expansion:
	{
		// Its pattern's bound packs are its own.
		Substitution whole = substitution;
		whole.bound_element = nullptr;
		const Type* pattern = substitute(type->element, whole);
		return pattern != nullptr ? types_.expansion(pattern) : nullptr;
	}
	case TypeKind::BoundPack:
	{
		// The element of the expansion being substituted, or the pattern
		// of the unbound expansion left in a partially bound pack.
		if (substitution.bound_element == nullptr || *substitution.bound_element >= type->element->param_count)
			return type;
		const Type* argument = type->element->params[*substitution.bound_element];
		if (argument->kind == TypeKind::Expansion)
			argument = argument->element;
		return types_.qualified(argument, type->cv);
	}
	default:
		return type;
	}
}

const Type* Analyzer::substitute_function(const Type* type, const Substitution& substitution, bool leading)
{
	// A function parameter pack's expansion becomes its elements. The
	// parts are substituted in lexical order, the first failure ending it
	// (14.8.2p7): a leading return type before the parameters, a trailing
	// one after them.
	const Type* result = leading ? substitute(type->element, substitution) : nullptr;
	if (leading && result == nullptr)
		return nullptr;
	TypeList parameters;
	for (uint32_t i = 0; i < type->param_count; ++i)
	{
		const size_t before = parameters.size();
		substitute_list(type->params + i, 1, substitution, parameters);
		for (size_t k = before; k < parameters.size(); ++k)
		{
			if (parameters[k] == nullptr)
				return nullptr;
		}
	}
	if (!leading)
		result = substitute(type->element, substitution);
	if (result == nullptr)
		return nullptr;
	const Type* void_type = types_.fundamental(FundamentalType::Void);
	bool invalid = result->kind == TypeKind::Function || result->kind == TypeKind::Array;
	for (const Type* parameter : parameters)
		invalid = invalid || parameter->unqualified == void_type;
	if (invalid)
	{
		substitution_error(0, "a function type returning an array or a function, or with a parameter of type void");
		return nullptr;
	}
	// A parameter or return type of an abstract class is no substitution
	// failure: it is checked at the function's definition and calls (the
	// hosts' rule in every mode, P0929R2).
	return types_.function(result, parameters.data(), static_cast<uint32_t>(parameters.size()), type->variadic,
		type->function_cv, type->ref);
}

bool Analyzer::AnyNull(const TypeList& types)
{
	for (const Type* type : types)
	{
		if (type == nullptr)
			return true;
	}
	return false;
}

const Type* Analyzer::without_cv(const Type* type, uint8_t cv)
{
	// `type` less the cv-qualifiers `cv`: an array's are its elements'
	// (3.9.3p5); a dependent bound kept.
	if (type->kind == TypeKind::Array)
		return types_.array(without_cv(type->element, cv), type->bound, type->owner);
	return types_.qualified(type->unqualified, static_cast<uint8_t>(type->cv & ~cv));
}

const Type* Analyzer::ordering_type(const Type* type)
{
	// A type less its top-level cv-qualifiers for partial ordering
	// (14.8.2.4p7): an array's are its elements' (3.9.3p5), so `const
	// T[N]` orders as T[N].
	const Type* element = type;
	while (element->kind == TypeKind::Array)
		element = element->element;
	return type->kind == TypeKind::Array ? without_cv(type, element->cv) : type->unqualified;
}

bool Analyzer::deduce_class(const Type* p, const Type* a, uint32_t depth, TypeList& arguments)
{
	// 14.8.2.5p8: TT<T...> against a specialization of the same template,
	// argument by argument.
	const ClassTemplateFacts* pattern = static_cast<const ClassEntity*>(p->entity)->templ;
	const ClassTemplateFacts* actual = static_cast<const ClassEntity*>(a->entity)->templ;
	if (pattern != nullptr && pattern->role == TemplateRole::Specialization && IsStandIn(pattern->pattern))
		return deduce_stand_in(pattern, a, depth, arguments);
	if (pattern == nullptr || actual == nullptr || actual->role != TemplateRole::Specialization)
		return false;
	const bool own = pattern->role == TemplateRole::Template;
	const ClassEntity* templ = own ? static_cast<const ClassEntity*>(p->entity) : pattern->pattern;
	if ((!own && (pattern->role != TemplateRole::Specialization || !pattern->dependent)) || actual->pattern != templ)
		return false;
	// A pattern whose arguments have a pack expansion keeps them as written,
	// their parameters unknown (see default_template_arguments): A's
	// arguments as a template-id gives them (an argument pack's elements in
	// its place, the defaults used) are compared in order, a trailing
	// expansion's pattern with each remaining one (14.8.2.5p9).
	for (uint32_t i = 0; !own && i < pattern->argument_count; ++i)
	{
		if (pattern->arguments[i]->kind != TypeKind::Expansion)
			continue;
		TypeList given;
		flattened_arguments(actual->arguments, actual->argument_count, given);
		return deduce_list(pattern->arguments, pattern->argument_count, given.data(),
			static_cast<uint32_t>(given.size()), depth, arguments);
	}
	const uint32_t count = own ? pattern->head->count : pattern->argument_count;
	if (count != actual->argument_count)
		return false;
	for (uint32_t i = 0; i < count; ++i)
	{
		const Type* expected = own ? pattern->head->parameters[i]->canonical->type : pattern->arguments[i];
		// The current instantiation's parameter pack stands for its own
		// expansion.
		if (own && pattern->head->parameters[i]->pack)
		{
			const Type* expansion = types_.expansion(expected);
			expected = types_.pack(&expansion, 1);
		}
		if (!deduce_type(expected, actual->arguments[i], depth, arguments, false))
			return false;
	}
	return true;
}

bool Analyzer::deduce_array_bound(const Type* array, uint64_t count, uint32_t depth, TypeList& arguments)
{
	// 14.8.2.5p8, p17: a dependent bound that names a non-type parameter
	// deduces from an array's size, of any integral type. Other dependent
	// bounds remain non-deduced contexts (p5).
	const TemplateParameterEntity* bound = static_cast<const TemplateParameterEntity*>(array->owner->entity)->named;
	if (bound == nullptr || bound->depth != depth || bound->index >= arguments.size() || count == 0)
		return true;
	const Type* value = types_.value(types_.fundamental(FundamentalType::UnsignedLongInt), count);
	if (arguments[bound->index] == nullptr)
		arguments[bound->index] = value;
	else if (!SameValue(arguments[bound->index], value))
		return false;
	return true;
}

bool Analyzer::deduce_type(const Type* p, const Type* a, uint32_t depth, TypeList& arguments, bool extra_cv)
{
	// 14.8.2.5: P and A match structurally, each template parameter of P
	// deducing the part of A in its place. `extra_cv` allows A fewer
	// cv-qualifiers than P at this level (14.8.2.1p3-p4). An array type has
	// its elements' cv-qualifiers (3.9.3p5): `const T` deduces T = int[3]
	// from const int[3], and `const T[3]` matches int[3] where `const T`
	// matches int. A dependent name is a non-deduced context (14.8.2.5p5).
	p = p->canonical;
	a = a->canonical;
	// A bound pack stands for its element of the expansion being matched
	// (see deduce_elements).
	if (p->unqualified->kind == TypeKind::BoundPack)
	{
		const Type* pack = p->unqualified->element;
		if (deduced_element_ < 0 || static_cast<uint32_t>(deduced_element_) >= pack->param_count)
			return p == a;
		return deduce_type(types_.qualified(pack->params[deduced_element_], p->cv), a, depth, arguments, extra_cv);
	}
	if (p->kind == TypeKind::TemplateTemplateParameter)
	{
		// A template template-parameter deduces a template-name (14.8.2.5p8:
		// TT); a dependent member template-name is a non-deduced context.
		const TemplateParameterEntity* parameter = static_cast<const TemplateParameterEntity*>(p->entity);
		if (parameter->qualifier != nullptr)
			return true;
		if (parameter->depth != depth || parameter->index >= arguments.size())
			return p == a;
		if (a->kind != TypeKind::Template && a->kind != TypeKind::TemplateTemplateParameter)
			return false;
		if (arguments[parameter->index] == nullptr)
		{
			arguments[parameter->index] = a;
			return true;
		}
		return arguments[parameter->index] == a;
	}
	if (p->unqualified->kind == TypeKind::TemplateParameter)
	{
		const TemplateParameterEntity* parameter = static_cast<const TemplateParameterEntity*>(p->unqualified->entity);
		if (IsPlaceholder(parameter))
			return true;
		if (parameter->depth != depth || parameter->index >= arguments.size())
			return p == a;
		const Type* element = a;
		while (element->kind == TypeKind::Array)
			element = element->element;
		if ((p->cv & ~element->cv) != 0 && !extra_cv)
			return false;
		const Type* deduced = without_cv(a, p->cv);
		if (arguments[parameter->index] == nullptr)
		{
			arguments[parameter->index] = deduced;
			return true;
		}
		return arguments[parameter->index] == deduced;
	}
	// An alias template-id dropping dependent arguments deduces as the type
	// it aliases (CWG 1558).
	if (p->unqualified->kind == TypeKind::Class || a->unqualified->kind == TypeKind::Class)
	{
		const Type* aliased_p = aliased_type(p);
		const Type* aliased_a = aliased_type(a);
		if (aliased_p != p || aliased_a != a)
			return deduce_type(aliased_p, aliased_a, depth, arguments, extra_cv);
	}
	if (p->kind != a->kind || (p->cv != a->cv && !(extra_cv && (a->cv & ~p->cv) == 0)))
		return false;
	switch (p->kind)
	{
	case TypeKind::Pointer:
		return deduce_type(p->element, a->element, depth, arguments, extra_cv);
	case TypeKind::LValueReference:
	case TypeKind::RValueReference:
		return deduce_type(p->element, a->element, depth, arguments, false);
	case TypeKind::MemberPointer:
		// 14.8.2.1p4: a pointer to member qualification-converts as a pointer
		// does (4.4p2), its class as written.
		return deduce_type(p->owner, a->owner, depth, arguments, false) &&
			deduce_type(p->element, a->element, depth, arguments, extra_cv);
	case TypeKind::Array:
		// 14.8.2.5p8: a bound that is a non-type parameter's name deduces
		// it from A's (of any integral type, p17); another dependent bound
		// is a non-deduced context (p5).
		if (p->owner != nullptr)
		{
			if (a->owner == nullptr && !deduce_array_bound(p, a->bound, depth, arguments))
				return false;
		}
		else if (p->bound != a->bound)
			return false;
		return deduce_type(p->element, a->element, depth, arguments, extra_cv);
	case TypeKind::Vector:
		// Like an array's (Clang's, GCC's): a size that is a non-type
		// parameter's name deduces it from A's lane count (its byte size for
		// vector_size), another dependent size is a non-deduced context; the
		// lane type deduces from A's.
		if (p->owner == nullptr || a->owner != nullptr)
			return p->unqualified == a->unqualified;
		if (const TemplateParameterEntity* size = static_cast<const TemplateParameterEntity*>(p->owner->entity)->named)
		{
			if (size->depth == depth && size->index < arguments.size())
			{
				const uint64_t count = p->variadic ? a->bound * FundamentalTypeSize(a->element->fundamental) : a->bound;
				const Type* value = types_.value(types_.fundamental(FundamentalType::UnsignedLongInt), count);
				if (arguments[size->index] == nullptr)
					arguments[size->index] = value;
				else if (!SameValue(arguments[size->index], value))
					return false;
			}
		}
		return deduce_type(p->element->unqualified, a->element->unqualified, depth, arguments, false);
	case TypeKind::Function:
	{
		// A function parameter pack ending P's parameters deduces from A's
		// remaining ones (14.8.2.5p10), as an argument pack's elements do.
		if (p->variadic != a->variadic || p->function_cv != a->function_cv || p->ref != a->ref)
			return false;
		const bool trailing = p->param_count != 0 && p->params[p->param_count - 1]->kind == TypeKind::Expansion;
		if (trailing)
		{
			const Type* pattern = types_.pack(p->params, p->param_count);
			if (!deduce_pack(pattern, types_.pack(a->params, a->param_count), depth, arguments))
				return false;
			return deduce_type(p->element, a->element, depth, arguments, false);
		}
		if (p->param_count != a->param_count)
			return false;
		for (uint32_t i = 0; i < p->param_count; ++i)
		{
			// Partial ordering: an A that is a pack expansion matches only
			// one in P (14.8.2.5p9).
			if (a->params[i]->kind == TypeKind::Expansion ||
				!deduce_type(p->params[i], a->params[i], depth, arguments, false))
				return false;
		}
		return deduce_type(p->element, a->element, depth, arguments, false);
	}
	case TypeKind::Class:
		if (p->unqualified == a->unqualified)
			return true;
		return dependent_type(p) && deduce_class(p->unqualified, a->unqualified, depth, arguments);
	case TypeKind::Pack:
		return deduce_pack(p, a, depth, arguments);
	case TypeKind::Value:
		// A value argument of a pattern whose parameter's type was still
		// dependent (unconverted) equals A's converted one of that value.
		return SameValue(p, a);
	default:
		return p->unqualified == a->unqualified;
	}
}

bool Analyzer::deduce_from_base(const Type* p, const Type* a, uint32_t depth, TypeList& arguments)
{
	// 14.8.2.1p4: P a specialization pattern and A a class derived from a
	// specialization of its template: the unique base that matches. Each
	// base class is tried once (its derivation mark; the deduction runs no
	// derivation query), however many paths lead to it.
	const ClassEntity* cls = ClassOf(a);
	if (cls == nullptr || p->unqualified->kind != TypeKind::Class || !dependent_type(p) || !require_complete(cls))
		return false;
	TypeList found;
	bool unique = true;
	const uint32_t visit = conversions_.new_visit();
	SmallList<const ClassEntity*, 8> pending;
	pending.push_back(cls);
	while (!pending.empty())
	{
		const ClassEntity* current = pending.back();
		pending.pop_back();
		for (const BaseClass* base = current->bases; base != nullptr; base = base->next)
		{
			ClassEntity* base_class = static_cast<ClassEntity*>(base->base);
			if (base_class->derivation_visit == visit)
				continue;
			base_class->derivation_visit = visit;
			TypeList trial = arguments;
			if (deduce_class(p->unqualified, base_class->type, depth, trial))
			{
				if (!found.empty() && found != trial)
					unique = false;
				found = trial;
			}
			else
				pending.push_back(base_class);
		}
	}
	if (found.empty() || !unique)
		return false;
	arguments = found;
	return true;
}

bool Analyzer::complete_deduction(FunctionEntity* primary, TypeList& arguments)
{
	// 14.8.2p5: parameters left undeduced take their default arguments
	// (14.1p9), substituted with those known before them: analyzed once per
	// (template, canonical earlier arguments), the key's count the
	// parameter's position; a parameter pack not otherwise deduced is an
	// empty pack (14.8.1p3).
	const TemplateHead* head = HeadOf(primary);
	bool arguments_dependent = false;
	uint32_t checked = 0;
	for (uint32_t i = 0; i < arguments.size(); ++i)
	{
		if (arguments[i] != nullptr)
		{
			arguments[i] = arguments[i]->canonical;
			continue;
		}
		if (head->parameters[i]->pack)
		{
			arguments[i] = types_.pack(nullptr, 0);
			continue;
		}
		if (!HasDefault(head, i))
			return false;
		const SpecializationKey probe = SpecializationKey{ primary, arguments.data(), i };
		if (const Type** found = deduced_defaults_.find(probe))
		{
			arguments[i] = *found;
			continue;
		}
		// Extend the dependence of the prefix only when a default needs it;
		// repeated defaults must not rescan all earlier arguments.
		for (; template_depth_ != 0 && checked < i && !arguments_dependent; ++checked)
			arguments_dependent = dependent_type(arguments[checked]);
		const Type* argument = template_default(head, arguments.data(), i, arguments_dependent, true);
		if (argument == nullptr || failed())
			return false;
		argument = argument->canonical;
		SpecializationKey key = probe;
		key.arguments = arena_.copy_array(arguments.data(), i);
		deduced_defaults_.insert(key, argument);
		arguments[i] = argument;
	}
	return true;
}

bool Analyzer::deduce_argument(
	const Type* p, const Type* a, ValueCategory category, uint32_t depth, TypeList& arguments)
{
	// 14.8.2.1p2-p4: an argument's type A against its parameter's type P
	// (arrays and functions decayed and top-level cv dropped unless P is a
	// reference; an lvalue passed to a `T&&` deduces an lvalue reference).
	if (p->is_reference())
	{
		const bool forwarding = p->kind == TypeKind::RValueReference && p->element->cv == 0 &&
			p->element->kind == TypeKind::TemplateParameter;
		if (forwarding && category == ValueCategory::LValue)
			a = types_.reference(a, false);
		p = p->element;
		return deduce_type(p, a, depth, arguments, true) || deduce_from_base(p, a, depth, arguments);
	}
	if (a->kind == TypeKind::Array)
		a = types_.pointer(a->element);
	else if (a->kind == TypeKind::Function)
		a = types_.pointer(a);
	else
		a = a->unqualified;
	return deduce_type(p->unqualified, a, depth, arguments, true) ||
		(p->unqualified->kind == TypeKind::Pointer && a->kind == TypeKind::Pointer &&
			deduce_from_base(p->unqualified->element, a->element, depth, arguments)) ||
		deduce_from_base(p->unqualified, a, depth, arguments);
}

void Analyzer::deduce_overloaded(const Type* p, const FunctionList* functions, uint32_t depth, TypeList& arguments)
{
	// 14.8.2.1p6, 14.8.2.5p5: an overload set argument deduces by trial
	// against each of its functions (the type a non-static member's
	// address has, a pointer to member): the one function that deduces is
	// the argument. A set containing a function template, or one where no
	// function or several deduce, leaves the parameter a non-deduced
	// context (its conversion then selects the function, 13.4).
	TypeList trial;
	TypeList found;
	const FunctionEntity* matched = nullptr;
	for (const FunctionList* node = functions; node != nullptr; node = node->next)
	{
		const FunctionEntity* function = static_cast<const FunctionEntity*>(node->binding->entity);
		if (function->templated)
			return;
		if (node->binding->invisible || function == matched)
			continue;
		const Type* a = function->type->canonical;
		if (function->member && !function->is_static)
			a = types_.member_pointer(function->owner->entity->type, a);
		trial = arguments;
		if (!deduce_argument(p, a, ValueCategory::LValue, depth, trial))
			continue;
		if (matched != nullptr)
			return;
		matched = function;
		found = trial;
	}
	if (matched != nullptr)
		arguments = found;
}

FunctionEntity* Analyzer::deduce_for_operand(FunctionEntity* primary, const ExprFacts* from)
{
	// A function template called with one operand (a converting constructor
	// template, 13.3.1.4): the operand initializes the first parameter that
	// is no pack expansion (a function parameter pack before it is a
	// non-deduced context, deduced empty: 14.8.2.5p5, 14.8.1p3), else it is
	// the one element of the trailing pack; that parameter's type against
	// the operand's (14.8.2.1), the other template arguments from their
	// defaults (14.8.2p5); null when deduction fails.
	const TemplateHead* head = HeadOf(primary);
	const Type* type = primary->type->canonical;
	if (head == nullptr || type->param_count == 0)
		return nullptr;
	const uint32_t depth = head->count != 0 ? head->parameters[0]->depth : 0;
	TypeList arguments(head->count, nullptr);
	uint32_t first = 0;
	while (first < type->param_count && type->params[first]->kind == TypeKind::Expansion)
		++first;
	if (first < type->param_count)
	{
		if (dependent_type(aliased_type(type->params[first])) &&
			!deduce_argument(type->params[first], from->type, from->category, depth, arguments))
			return nullptr;
		return deduced_specialization(primary, arguments);
	}
	// The trailing pack's pattern deduces one element of each pack it names
	// (14.8.2.1p1; a bound pack's length is the instance's: not one here).
	const Type* pattern = type->params[type->param_count - 1]->element;
	PackList packs;
	TypeList bound;
	unexpanded_packs(pattern, depth, packs, &bound);
	if (!bound.empty())
		return nullptr;
	if (!packs.empty() && !deduce_argument(pattern, from->type, from->category, depth, arguments))
		return nullptr;
	for (uint32_t index : packs)
	{
		if (arguments[index] == nullptr)
			return nullptr;
		const Type* element = arguments[index]->canonical;
		arguments[index] = types_.pack(&element, 1);
	}
	return deduced_specialization(primary, arguments);
}

FunctionEntity* Analyzer::deduce_for_call(
	FunctionEntity* primary, const syntax::Slice<syntax::Expr*>& call_arguments, const TypeList* explicit_arguments)
{
	// 14.8.2.1: each argument's type against its parameter's, where the
	// parameter's type depends on a template parameter still to deduce; a
	// function parameter pack ending the list against the arguments left.
	const TemplateHead* head = HeadOf(primary);
	if (head == nullptr)
		return nullptr;
	TypeList arguments(head->count, nullptr);
	if (explicit_arguments != nullptr)
	{
		// 14.8.1: explicit arguments of their parameters' kinds, converted.
		TypeList converted;
		if (!match_template_arguments(head, explicit_arguments->data(),
				static_cast<uint32_t>(explicit_arguments->size()), converted, false, 0, true))
			return nullptr;
		for (size_t i = 0; i < converted.size(); ++i)
			arguments[i] = converted[i];
	}
	const uint32_t depth = head->count != 0 ? head->parameters[0]->depth : 0;
	const Type* type = primary->type->canonical;
	// 14.8.1p9: a trailing function parameter pack whose packs were all
	// given explicit arguments (of one length) and that has more call
	// arguments takes those as its leading elements, whose parameters'
	// types are fixed, and deduces the rest from the arguments after them.
	TypeList prefixes(head->count, nullptr);
	uint32_t prefix = 0;
	if (explicit_arguments != nullptr && type->param_count != 0 &&
		type->params[type->param_count - 1]->kind == TypeKind::Expansion)
	{
		PackList packs;
		unexpanded_packs(type->params[type->param_count - 1]->element, depth, packs);
		int32_t length = packs.empty() ? -1 : 0;
		for (uint32_t k = 0; k < packs.size() && length >= 0; ++k)
		{
			const Type* given = arguments[packs[k]];
			if (given == nullptr || given->kind != TypeKind::Pack ||
				(k != 0 && static_cast<uint32_t>(length) != given->param_count))
				length = -1;
			else
				length = static_cast<int32_t>(given->param_count);
		}
		if (length >= 0 && call_arguments.size > type->param_count - 1 + static_cast<uint32_t>(length))
		{
			prefix = static_cast<uint32_t>(length);
			for (uint32_t index : packs)
			{
				prefixes[index] = arguments[index];
				arguments[index] = nullptr;
			}
		}
	}
	// Parameters whose types involve explicitly specified arguments only
	// (14.8.1p6) are substituted first, so their arguments convert (an
	// expansion of a pack given explicitly expands).
	TypeList parameters;
	if (explicit_arguments != nullptr)
	{
		// 14.8.2p2: an invalid type fails deduction; 14.8.2p7: substituted
		// in lexical order, a leading return type first.
		const Entity* saved = substitution_context_;
		substitution_context_ = primary;
		SoftSubstitution soft(*this);
		const Substitution substitution = Substitution{ depth, arguments.data(), head->count, nullptr };
		const bool result = primary->templ->trailing_result || substitute(type->element, substitution) != nullptr;
		if (result)
			substitute_list(type->params, type->param_count, substitution, parameters);
		substitution_context_ = saved;
		if (!result || soft.failed() || AnyNull(parameters))
			return nullptr;
	}
	else
	{
		for (uint32_t i = 0; i < type->param_count; ++i)
			parameters.push_back(type->params[i]);
	}
	const uint32_t total = static_cast<uint32_t>(parameters.size());
	const bool trailing = total != 0 && parameters[total - 1]->kind == TypeKind::Expansion;
	const uint32_t fixed = trailing ? total - 1 : total;
	const uint32_t count = call_arguments.size < fixed ? call_arguments.size : fixed;
	for (uint32_t i = 0; i < count; ++i)
	{
		// 14.8.2.1p1: a braced-init-list argument is a non-deduced context,
		// as is a function parameter pack that does not end the list
		// (14.8.2.5p5). A parameter whose template parameters are all in
		// non-deduced contexts takes no part (14.8.2.5p4 and p16's note: its
		// argument converts to the substituted type, 14.8.1p6).
		// An alias template-id that drops dependent arguments takes part as
		// the type it aliases (CWG 1558).
		const Type* p = parameters[i]->canonical;
		if (p->kind == TypeKind::Expansion || !dependent_type(aliased_type(p)) ||
			!Deducible(aliased_type(p)->canonical, depth, 0))
			continue;
		if (call_arguments[i]->kind == syntax::ExprKind::BracedInit)
		{
			if (!deduce_list_elements(
					p, static_cast<const syntax::BracedInitList&>(*call_arguments[i]), depth, arguments))
				return nullptr;
			continue;
		}
		const ExprFacts* facts = call_arguments[i]->facts;
		if ((facts->flags & kBoundMember) != 0)
			return nullptr;
		if ((facts->flags & kOverloadSet) != 0)
			deduce_overloaded(p, facts->functions, depth, arguments);
		else if (facts->type == nullptr || !deduce_argument(p, facts->type, facts->category, depth, arguments))
			return nullptr;
	}
	if (trailing && call_arguments.size > fixed + prefix &&
		!deduce_call_pack(parameters[total - 1]->element->canonical, call_arguments, fixed + prefix, depth, arguments))
		return nullptr;
	for (uint32_t index = 0; index < head->count; ++index)
	{
		// The explicit elements lead the deduced ones.
		if (prefixes[index] == nullptr)
			continue;
		TypeList elements;
		flattened_arguments(&prefixes[index], 1, elements);
		if (arguments[index] != nullptr)
			flattened_arguments(&arguments[index], 1, elements);
		arguments[index] = types_.pack(elements.data(), static_cast<uint32_t>(elements.size()));
	}
	return deduced_specialization(primary, arguments, &call_arguments);
}

FunctionEntity* Analyzer::deduce_for_target(FunctionEntity* primary, const Type* function, const Type* given)
{
	// 14.8.2.2: the template's function type against the target's, a
	// top-level T&& against an lvalue reference taken as T (14.8.2.5p10);
	// `given`, a Pack of converted explicit arguments, supplies the leading
	// parameters (14.8.1p3).
	const TemplateHead* head = HeadOf(primary);
	if (head == nullptr)
		return nullptr;
	TypeList arguments(head->count, nullptr);
	for (uint32_t i = 0; given != nullptr && i < given->param_count && i < head->count; ++i)
		arguments[i] = given->params[i];
	const uint32_t depth = head->count != 0 ? head->parameters[0]->depth : 0;
	if (!deduce_type(declaration_pattern(primary->type->canonical, function, depth), function, depth, arguments, false))
		return nullptr;
	return deduced_specialization(primary, arguments);
}

FunctionEntity* Analyzer::deduce_for_conversion(FunctionEntity* primary, const Type* required, bool named)
{
	// 14.8.2.3: a conversion function template's return type P against the
	// type A its result is required to have, P's reference dropped (p2). A
	// reference P's result binds a reference A: A is the referred type, its
	// cv-qualifiers kept (p4: `const int&` from `operator T&()` deduces
	// T = const int). A non-reference P yields a prvalue, whose required
	// type is no reference (`cv2 T2`, 13.3.1.6p1, or the temporary's type
	// under 13.3.1.5, 8.5.3p5): an array or function P decays and P's
	// top-level cv-qualifiers are ignored (p3), and so are A's (p4: `const
	// X&` from `operator T()` deduces T = X, as both hosts do). Only if that
	// fails (p6): a reference A may be more cv-qualified than the deduced A
	// (p5; A's cv-qualifiers P lacks dropped), and a pointer or pointer to
	// member A deduces with the cv-qualifiers of its levels dropped (p7; the
	// deduced A converts to A by a qualification conversion, which the
	// candidate's second standard conversion checks). The alternatives are
	// tried in that order, the first that deduces taken (both hosts: p6's
	// "more than one possible deduced A", `const int* const&` from
	// `operator T*&()`, deduces T = const int).
	//
	// A conversion-function-id names its complete type (`operator const
	// int`, `operator int&`, 14.5.2p6): P is deduced from it unadjusted.
	const TemplateHead* head = HeadOf(primary);
	if (head == nullptr)
		return nullptr;
	const Type* p = primary->type->canonical->element;
	if (named)
	{
		TypeList named_arguments(head->count, nullptr);
		return deduce_type(p, required, head->count != 0 ? head->parameters[0]->depth : 0, named_arguments, false)
			? deduced_specialization(primary, named_arguments)
			: nullptr;
	}
	const bool binds = p->is_reference() && required->is_reference();
	if (p->is_reference())
		p = p->element;
	const Type* a = required->is_reference() ? required->element : required;
	if (!binds)
	{
		p = p->kind == TypeKind::Array      ? types_.pointer(p->element)
			: p->kind == TypeKind::Function ? types_.pointer(p)
											: p->unqualified;
		a = a->unqualified;
	}
	const uint32_t depth = head->count != 0 ? head->parameters[0]->depth : 0;
	TypeList arguments(head->count, nullptr);
	bool deduced = deduce_type(p, a, depth, arguments, false);
	if (!deduced && binds && (a->cv & ~p->cv) != 0)
	{
		for (uint32_t i = 0; i < head->count; ++i)
			arguments[i] = nullptr;
		deduced = deduce_type(p, without_cv(a, static_cast<uint8_t>(a->cv & ~p->cv)), depth, arguments, false);
	}
	if (!deduced && (a->unqualified->kind == TypeKind::Pointer || a->unqualified->kind == TypeKind::MemberPointer))
	{
		for (uint32_t i = 0; i < head->count; ++i)
			arguments[i] = nullptr;
		deduced = deduce_type(p, without_level_cv(a->unqualified), depth, arguments, false);
	}
	return deduced ? deduced_specialization(primary, arguments) : nullptr;
}

const Type* Analyzer::without_level_cv(const Type* type)
{
	// A pointer or pointer to member type with the cv-qualifiers of every
	// level it points through dropped (4.4's similar types' cv-unqualified
	// form).
	type = type->unqualified;
	if (type->kind == TypeKind::Pointer)
		return types_.pointer(without_level_cv(type->element));
	if (type->kind == TypeKind::MemberPointer)
		return types_.member_pointer(type->owner, without_level_cv(type->element));
	return type;
}

bool Analyzer::fixed_call_arguments(
	const FunctionEntity* primary, const TypeList& arguments, const syntax::Slice<syntax::Expr*>& call_arguments)
{
	// CWG 1391: after deduction and defaults, an impossible conversion to
	// an originally nondependent parameter fails before signature
	// substitution. Parameters made nondependent by explicit arguments
	// still wait for overload resolution. A pack's bound length locates
	// the next fixed parameter without substituting the pack's types.
	const Type* type = primary->type->canonical;
	const TemplateHead* head = HeadOf(primary);
	const uint32_t depth = head->count != 0 ? head->parameters[0]->depth : 0;
	uint32_t argument = 0;
	for (uint32_t i = 0; i < type->param_count && argument < call_arguments.size; ++i)
	{
		const Type* parameter = type->params[i];
		if (parameter->kind == TypeKind::Expansion)
		{
			PackList packs;
			TypeList bound;
			unexpanded_packs(parameter->element, depth, packs, &bound);
			int32_t length = BoundLength(bound);
			for (uint32_t index : packs)
			{
				const Type* pack = arguments[index];
				if (pack == nullptr || pack->kind != TypeKind::Pack)
					return true;
				if (length >= 0 && static_cast<uint32_t>(length) != pack->param_count)
					return false;
				length = static_cast<int32_t>(pack->param_count);
			}
			if (length < 0)
				return true;
			argument += static_cast<uint32_t>(length);
			continue;
		}
		if (!dependent_type(parameter))
		{
			Conversion conversion = Conversion();
			if (!implicit_sequence(*call_arguments[argument], parameter, conversion))
				return false;
		}
		++argument;
	}
	return true;
}

FunctionEntity* Analyzer::deduced_specialization(
	FunctionEntity* primary, TypeList& arguments, const syntax::Slice<syntax::Expr*>* call_arguments)
{
	// 14.8.2p5-p8: the deduced arguments completed by the defaults and
	// substituted into the function type; an invalid type or expression
	// in either fails deduction.
	if (!deduced_values_typed(primary, arguments))
		return nullptr;
	const Entity* saved = substitution_context_;
	substitution_context_ = primary;
	SoftSubstitution soft(*this);
	FunctionEntity* function = nullptr;
	if (complete_deduction(primary, arguments) &&
		(call_arguments == nullptr || fixed_call_arguments(primary, arguments, *call_arguments)))
		function = specialize(primary, arguments);
	substitution_context_ = saved;
	return soft.failed() ? nullptr : function;
}

bool Analyzer::deduced_values_typed(const FunctionEntity* primary, const TypeList& arguments)
{
	// 14.8.2.5p17: a non-type argument deduced from a template-id has
	// exactly its parameter's type, cv-qualifiers aside (A<1>, an int,
	// does not deduce `short s` in f(A<s>)); one an array bound of the
	// function's parameters may deduce is of any integral type. Explicit
	// arguments are already converted.
	const TemplateHead* head = HeadOf(primary);
	for (uint32_t i = 0; i < arguments.size() && i < head->count; ++i)
	{
		const TemplateParameterEntity* parameter = head->parameters[i];
		const Type* declared = parameter->value_type;
		if (arguments[i] == nullptr || !parameter->non_type || declared == nullptr || dependent_type(declared))
			continue;
		const bool pack = arguments[i]->kind == TypeKind::Pack;
		const uint32_t count = pack ? arguments[i]->param_count : 1;
		for (uint32_t k = 0; k < count; ++k)
		{
			const Type* value = pack ? arguments[i]->params[k] : arguments[i];
			if (value->kind == TypeKind::Value && !AddressArgument(value) &&
				value->element->unqualified != declared->unqualified &&
				!BoundNames(primary->type->canonical, parameter->depth, parameter->index, 0))
				return false;
		}
	}
	return true;
}

const Type* Analyzer::synthesized_type(uint32_t index)
{
	// A unique type standing for a template parameter in partial ordering
	// (14.8.2.4p3): a parameter of no template's depth.
	while (synthesized_.size() <= index)
	{
		TemplateParameterEntity* entity = new_entity<TemplateParameterEntity>(
			EntityKind::TemplateParameter, nullptr, nullptr, nullptr);
		entity->depth = 0x7fff;
		entity->index = static_cast<uint16_t>(synthesized_.size());
		entity->canonical = entity;
		entity->type = types_.named(TypeKind::TemplateParameter, entity);
		synthesized_.push_back(entity->type);
	}
	return synthesized_[index];
}

bool Analyzer::at_least_as_specialized(
	const FunctionEntity* first, const FunctionEntity* second, uint32_t arguments, Ordering form)
{
	const TemplateHead* head = HeadOf(first);
	const TemplateHead* other = HeadOf(second);
	if (head == nullptr || other == nullptr || other->count == 0)
		return false;
	const FunctionOrderKey key{ first, second, first->type, second->type, head, other,
		soft_substitution_ != 0 ? substitution_context_ : nullptr, arguments, deduced_element_,
		static_cast<uint8_t>(form) };
	if (const bool* known = function_orderings_.find(key))
	{
		++graph_.function_order_reuses;
		return *known;
	}
	++graph_.function_order_builds;
	const uint32_t incomplete = incomplete_classes_seen_;
	const bool result = compute_function_order(first, second, arguments, form);
	// An incomplete or rejected substitution is not an ordering fact.
	if (!failed() && incomplete_classes_seen_ == incomplete && function_orderings_.find(key) == nullptr)
		function_orderings_.insert(key, result);
	return result;
}

bool Analyzer::compute_function_order(
	const FunctionEntity* first, const FunctionEntity* second, uint32_t arguments, Ordering form)
{
	// 14.8.2.4: the second template's parameters deduced from the first's
	// function parameter types (its template parameters unique types; a
	// pack's one unique type standing for all its elements), references
	// and top-level cv-qualifiers dropped (p5-p7). A call orders by the
	// parameters it has arguments for (p3), so only those are substituted
	// (a result's decltype-specifier is not analyzed with unique types); a
	// function parameter pack stands for each argument it takes. p8: a type
	// from the first's pack against one of the second's that is no pack
	// fails; the second's pack deduces its packs' elements from each type
	// it stands against. 14.5.6.2p3: of a non-static member and a function
	// that is none (an operator's candidates), the member has its object
	// parameter first.
	const TemplateHead* head = HeadOf(first);
	const TemplateHead* other = HeadOf(second);
	if (head == nullptr || other == nullptr || other->count == 0)
		return false;
	TypeList unique(head->count, nullptr);
	for (uint32_t i = 0; i < head->count; ++i)
		unique[i] = synthesized_argument(head->parameters[i], i);
	const Substitution substitution = substitution_of(head, unique.data());
	if (form == Ordering::FunctionType)
		return function_type_at_least_as_specialized(first, second, substitution);
	const bool inserting = form == Ordering::CallInserting && NonstaticMember(first) != NonstaticMember(second);
	const bool from_inserted = inserting && NonstaticMember(first);
	const bool to_inserted = inserting && NonstaticMember(second);
	const Type* from = first->type;
	const Type* to = second->type->canonical;
	const bool from_pack = from->param_count != 0 && from->params[from->param_count - 1]->kind == TypeKind::Expansion;
	const bool to_pack = to->param_count != 0 && to->params[to->param_count - 1]->kind == TypeKind::Expansion;
	const uint32_t from_fixed = from->param_count - (from_pack ? 1 : 0) + (from_inserted ? 1 : 0);
	const uint32_t to_fixed = to->param_count - (to_pack ? 1 : 0) + (to_inserted ? 1 : 0);
	const uint32_t depth = other->parameters[0]->depth;
	TypeList deduced(other->count, nullptr);
	TypeList packed;
	for (uint32_t i = 0; i < arguments; ++i)
	{
		if (i >= from_fixed && !from_pack)
			break;
		if (i >= to_fixed && !to_pack)
			break;
		const bool a_from_pack = i >= from_fixed;
		const Type* parameter = ordering_parameter(first, a_from_pack ? from_fixed : i, from_inserted);
		const Type* given = substitute(a_from_pack ? parameter->element : parameter, substitution)->canonical;
		const Type* a = given->is_reference() ? given->element : given;
		if (i >= to_fixed)
		{
			packed.push_back(ordering_type(a));
			continue;
		}
		if (a_from_pack)
			return false;
		const Type* p = ordering_parameter(second, i, to_inserted)->canonical;
		p = p->is_reference() ? p->element : p;
		if (!deduce_type(ordering_type(p), ordering_type(a), depth, deduced, false))
			return false;
	}
	if (packed.empty())
		return true;
	const Type* pattern = to->params[to->param_count - 1]->element;
	if (pattern->is_reference())
		pattern = pattern->element;
	return deduce_pack_types(
		ordering_type(pattern), packed.data(), static_cast<uint32_t>(packed.size()), depth, deduced);
}

bool Analyzer::function_type_at_least_as_specialized(
	const FunctionEntity* first, const FunctionEntity* second, const Substitution& substitution)
{
	// 14.8.2.4p3 bullet 3 (an address, 14.5.6.2p1): the second's function
	// type P against the first's with unique types A, as 14.8.2.5 matches
	// them (no reference or cv adjustment of the parameters, p5-p7 apply
	// to the whole types). p10: a trailing function parameter pack of A
	// for which P has no corresponding parameter is ignored; against one
	// P has that is no pack deduction fails. The return types are compared
	// unless either names a dependent name or expression (not analyzed
	// with unique types).
	const Type* from = first->type;
	const Type* to = second->type->canonical;
	const bool to_pack = to->param_count != 0 && to->params[to->param_count - 1]->kind == TypeKind::Expansion;
	TypeList given;
	for (uint32_t i = 0; i < from->param_count; ++i)
	{
		const Type* parameter = from->params[i];
		if (parameter->kind == TypeKind::Expansion && i + 1 == from->param_count && i >= to->param_count && !to_pack)
			break;
		const Type* substituted = substitute(
			parameter->kind == TypeKind::Expansion ? parameter->element : parameter, substitution);
		if (substituted == nullptr)
			return false;
		given.push_back(parameter->kind == TypeKind::Expansion ? types_.expansion(substituted) : substituted);
	}
	const Type* void_type = types_.fundamental(FundamentalType::Void);
	const Type* a = types_.function(
		void_type, given.data(), static_cast<uint32_t>(given.size()), from->variadic, from->function_cv, from->ref);
	const Type* p = types_.function(void_type, to->params, to->param_count, to->variadic, to->function_cv, to->ref);
	const TemplateHead* other = HeadOf(second);
	const uint32_t depth = other->parameters[0]->depth;
	TypeList deduced(other->count, nullptr);
	if (!deduce_type(p, a, depth, deduced, false))
		return false;
	if (names_placeholder(from->element, 0) || names_placeholder(to->element, 0))
		return true;
	const Type* result = substitute(from->element, substitution);
	return result != nullptr && deduce_type(to->element, result, depth, deduced, false);
}

const Type* Analyzer::ordering_parameter(const FunctionEntity* function, uint32_t index, bool inserted)
{
	// A function template's parameter `index` for partial ordering; with
	// `inserted`, a non-static member's first is its object parameter
	// (14.5.6.2p3): `cv A&`, `cv A&&` for a `&&` ref-qualifier.
	const Type* type = function->type;
	if (!inserted)
		return type->params[index];
	if (index != 0)
		return type->params[index - 1];
	return types_.reference(
		types_.qualified(function->owner->entity->type, type->function_cv), type->ref == RefQualifier::RValue);
}

bool Analyzer::result_at_least_as_specialized(const FunctionEntity* first, const FunctionEntity* second)
{
	// 14.8.2.4p3 bullet 2 (a call to a conversion function): the second
	// template's parameters deduced from the first's return type (its
	// template parameters unique types), references and top-level
	// cv-qualifiers dropped (p5, p7).
	const TemplateHead* head = HeadOf(first);
	const TemplateHead* other = HeadOf(second);
	if (head == nullptr || other == nullptr || other->count == 0)
		return false;
	TypeList unique(head->count, nullptr);
	for (uint32_t i = 0; i < head->count; ++i)
		unique[i] = synthesized_argument(head->parameters[i], i);
	const Type* a = substitute(first->type->element, substitution_of(head, unique.data()));
	if (a == nullptr)
		return false;
	a = a->canonical;
	const Type* p = second->type->canonical->element;
	TypeList deduced(other->count, nullptr);
	return deduce_type(ordering_type(p->is_reference() ? p->element : p),
		ordering_type(a->is_reference() ? a->element : a), other->parameters[0]->depth, deduced, false);
}

int Analyzer::conversion_template_order(const FunctionEntity* first, const FunctionEntity* second)
{
	// 14.5.2p8, 13.3.3p1: two conversion function templates' specializations
	// by the partial ordering of their templates (14.5.6.2) over their
	// return types; of two each at least as specialized whose return types
	// are references, an lvalue reference over an rvalue one, else the more
	// cv-qualified referred type (14.8.2.4p6, p9).
	const bool forward = result_at_least_as_specialized(first, second);
	const bool backward = result_at_least_as_specialized(second, first);
	if (forward != backward)
		return forward ? 1 : -1;
	const Type* p = first->type->canonical->element;
	const Type* q = second->type->canonical->element;
	if (!forward || !p->is_reference() || !q->is_reference())
		return 0;
	if (p->kind != q->kind)
		return p->kind == TypeKind::LValueReference ? 1 : -1;
	const uint8_t pcv = ReferredCv(p);
	const uint8_t qcv = ReferredCv(q);
	if (pcv == qcv)
		return 0;
	return (pcv & qcv) == qcv ? 1 : (pcv & qcv) == pcv ? -1 : 0;
}

int Analyzer::reference_ordering(
	const FunctionEntity* first, const FunctionEntity* second, uint32_t arguments, Ordering form)
{
	// 14.8.2.4p9-p10: where deduction succeeds both ways and both
	// parameters are references, an lvalue reference is more specialized
	// than an rvalue one, and of two of one kind the more cv-qualified
	// referenced type is; one template wins when it is more specialized
	// for some parameter and the other for none (a member's inserted
	// object parameter too, 14.5.6.2p3).
	const bool inserting = form == Ordering::CallInserting && NonstaticMember(first) != NonstaticMember(second);
	const bool a_inserted = inserting && NonstaticMember(first);
	const bool b_inserted = inserting && NonstaticMember(second);
	const uint32_t a_count = first->type->param_count + (a_inserted ? 1 : 0);
	const uint32_t b_count = second->type->param_count + (b_inserted ? 1 : 0);
	int order = 0;
	for (uint32_t i = 0; i < arguments && i < a_count && i < b_count; ++i)
	{
		const Type* p = ordering_parameter(first, i, a_inserted)->canonical;
		const Type* q = ordering_parameter(second, i, b_inserted)->canonical;
		if (!p->is_reference() || !q->is_reference())
			continue;
		int step = 0;
		if (p->kind != q->kind)
			step = p->kind == TypeKind::LValueReference ? 1 : -1;
		else
		{
			const uint8_t pcv = ReferredCv(p);
			const uint8_t qcv = ReferredCv(q);
			if (pcv != qcv && (pcv & qcv) == qcv)
				step = 1;
			else if (pcv != qcv && (pcv & qcv) == pcv)
				step = -1;
		}
		if (step != 0 && order != 0 && step != order)
			return 0;
		if (step != 0)
			order = step;
	}
	return order;
}

bool Analyzer::trailing_pack_unused(const FunctionEntity* primary, const FunctionEntity* other, uint32_t arguments)
{
	// A trailing function parameter pack the call gives no argument, for
	// which `other` has no corresponding parameter and no trailing pack
	// (CWG 1395's tie-breaker: of two templates each at least as
	// specialized, `other` is then the more specialized). A parameter with
	// a default argument corresponds (14.5.6.2p5's g(T, T = T()) and
	// g(T, U...) stay ambiguous).
	const Type* type = primary->type->canonical;
	return type->param_count != 0 && type->params[type->param_count - 1]->kind == TypeKind::Expansion &&
		arguments < type->param_count && other->type->param_count < type->param_count && !has_trailing_pack(other);
}

bool Analyzer::has_trailing_pack(const FunctionEntity* primary)
{
	const Type* type = primary->type->canonical;
	return type->param_count != 0 && type->params[type->param_count - 1]->kind == TypeKind::Expansion;
}

FunctionEntity* Analyzer::specialize(FunctionEntity* primary, const TypeList& given)
{
	// One declaration per (template, arguments), found again by that key;
	// the arguments of their parameters' kinds, values converted (14.3).
	TypeList arguments;
	if (!match_template_arguments(
			HeadOf(primary), given.data(), static_cast<uint32_t>(given.size()), arguments, false, 0))
		return nullptr;
	SpecializationKey probe = SpecializationKey{ primary, arguments.data(), static_cast<uint32_t>(arguments.size()) };
	if (Entity** found = specializations_.find(probe))
		return static_cast<FunctionEntity*>(*found);
	// A substitution needing the specialization it is forming (its result
	// type's default template argument calling it again) fails, as the
	// hosts' (GCC's) does, rather than recursing without end (14.7.1p15).
	// Reached again from a class template instantiation the substitution
	// started, it is not circular: that class is not instantiated again,
	// its members found as far as they are declared, so the specialization
	// is formed there (and is the one this substitution then finds).
	const size_t classes = class_instantiations_.size();
	for (const ActiveSpecialization& active : specializing_)
	{
		if (active.key == probe && active.classes == classes)
		{
			substitution_error(0, "a function template specialization's substitution needs itself", true);
			return nullptr;
		}
	}
	// Its function type in lexical order (14.8.2p7).
	const Substitution substitution = substitution_of(HeadOf(primary), arguments.data());
	specializing_.push_back(ActiveSpecialization{ probe, classes });
	const Type* type = primary->type->kind == TypeKind::Function
		? substitute_function(primary->type, substitution, !primary->templ->trailing_result)
		: substitute(primary->type, substitution);
	specializing_.pop_back();
	if (type == nullptr || type->kind != TypeKind::Function)
		return nullptr;
	if (Entity** found = specializations_.find(probe))
		return static_cast<FunctionEntity*>(*found);
	FunctionEntity* function = new_entity<FunctionEntity>(EntityKind::Function, primary->name, primary->owner, type);
	function->label = primary->label;
	function->member = primary->member;
	function->is_static = primary->is_static;
	function->constructor = primary->constructor;
	function->conversion = primary->conversion;
	function->is_operator = primary->is_operator;
	function->operator_token = primary->operator_token;
	function->operator_array = primary->operator_array;
	function->is_explicit = primary->is_explicit;
	// An invalid explicit(condition) fails the substitution before the
	// specialization is registered (14.8.2p8, as GCC and Clang take it).
	const uint32_t pending = explicit_index_.find(primary);
	if (pending != PointerMap::kMissing)
	{
		const bool earlier = failed();
		function->is_explicit = specialized_explicit(explicits_[pending], arguments);
		if (!earlier && failed())
			return nullptr;
	}
	function->is_inline = primary->is_inline;
	// A deleted template's specializations are deleted (8.4.3p2: using one
	// refers to the deleted function; its failure in a substitution,
	// 14.8.2p8).
	function->deleted = primary->deleted;
	function->is_constexpr = primary->is_constexpr;
	function->nothrow = primary->nothrow;
	function->exception_spec = primary->exception_spec;
	// A noexcept-specification's expression (the template's), substituted
	// on first need.
	function->noexcept_pending = primary->noexcept_pending;
	function->internal_linkage = primary->internal_linkage;
	function->no_inline = primary->no_inline;
	function->effects = primary->effects;
	// [[noreturn]] or GNU noreturn on the template (7.6.3p1).
	function->noreturn = primary->noreturn;
	function->signature = signature_of(type);
	function->parameters = primary->parameters;
	// Its own default arguments, instantiated as calls use them: the
	// template's parameters before its first function parameter pack and
	// after its last keep theirs, a pack's elements have none (8.3.6p3).
	if (primary->default_arguments != nullptr)
	{
		const Type* pattern = primary->type->canonical;
		const uint32_t count = type->param_count;
		const syntax::Expr** defaults = arena_.allocate_array<const syntax::Expr*>(count);
		std::memset(static_cast<void*>(defaults), 0, sizeof(syntax::Expr*) * count);
		uint32_t front = 0;
		for (; front < pattern->param_count && front < count && pattern->params[front]->kind != TypeKind::Expansion;
			++front)
			defaults[front] = primary->default_arguments[front];
		for (uint32_t back = 1; front < pattern->param_count && back <= pattern->param_count - front &&
			back <= count - front && pattern->params[pattern->param_count - back]->kind != TypeKind::Expansion;
			++back)
			defaults[count - back] = primary->default_arguments[pattern->param_count - back];
		function->default_arguments = defaults;
	}
	function->primary = primary;
	function->instantiated = true;
	function_template_facts(function)->arguments = arena_.copy_array(arguments.data(), arguments.size());
	SpecializationKey key = probe;
	key.arguments = ArgumentsOf(function);
	specializations_.insert(key, function);
	++graph_.specializations;
	return function;
}

void Analyzer::substitute_throw_specification(FunctionEntity* function)
{
	// A function template specialization's dynamic-exception-specification
	// (needed with its definition): the template's, substituted.
	const FunctionEntity* primary = function->primary;
	const SemanticGraph::ThrowSpecification* thrown = graph_.throw_specification(primary);
	if (thrown == nullptr || graph_.throw_specification(function) != nullptr || HeadOf(primary) == nullptr)
		return;
	// A pack expansion becomes one type per element (14.5.3p4).
	const Substitution substitution = substitution_of(HeadOf(primary), ArgumentsOf(function));
	TypeList list;
	substitute_list(thrown->types, thrown->count, substitution, list);
	const Type** types = arena_.allocate_array<const Type*>(list.size());
	for (uint32_t i = 0; i < list.size(); ++i)
	{
		const Type* type = list[i];
		if (type == nullptr || type->kind == TypeKind::Pack || type->kind == TypeKind::Expansion)
			return;
		types[i] = AdjustedThrowType(types_, type);
	}
	graph_.set_throw_specification(
		function, SemanticGraph::ThrowSpecification{ types, static_cast<uint32_t>(list.size()) });
}

const TemplateHead* Analyzer::SoleTemplateHead(const FunctionList* functions)
{
	// The template head whose parameters' types the explicit arguments of
	// the function templates among `functions` then know (a pointer
	// parameter's resolves a set of overloaded functions, 14.3.2p5): the
	// one template's, or the longest of several whose parameters agree in
	// kind and type where both have them; null for none or disagreeing ones.
	const TemplateHead* found = nullptr;
	for (const FunctionList* node = functions; node != nullptr; node = node->next)
	{
		const FunctionEntity* function = static_cast<const FunctionEntity*>(node->binding->entity);
		if (!function->templated)
			continue;
		const TemplateHead* head = HeadOf(function);
		if (head == nullptr)
			return nullptr;
		if (found == nullptr)
		{
			found = head;
			continue;
		}
		const uint32_t common = found->count < head->count ? found->count : head->count;
		for (uint32_t i = 0; i < common; ++i)
		{
			const TemplateParameterEntity* a = found->parameters[i];
			const TemplateParameterEntity* b = head->parameters[i];
			if (a->non_type != b->non_type || a->template_template != b->template_template || a->pack != b->pack ||
				(a->non_type &&
					(a->value_type == nullptr || b->value_type == nullptr ||
						a->value_type->canonical != b->value_type->canonical)))
				return nullptr;
		}
		if (head->count > found->count)
			found = head;
	}
	return found;
}

const FunctionList* Analyzer::explicit_specializations(
	const syntax::NameComponent& component, const FunctionList* functions, Scope* scope)
{
	// A template-id names the specializations of the templates found whose
	// parameters its arguments supply (14.8.1; a parameter pack takes the
	// arguments left), as an overload set; a template whose remaining
	// parameters have defaults takes them. One with a parameter left that
	// has none is the template with its explicit arguments, whose target
	// type deduces the rest (13.4p1-p2, 14.8.1p3): a hidden binding of the
	// template whose type is the Pack of its converted explicit arguments
	// (see select_overloaded).
	TypeList arguments;
	if (!template_arguments(component, scope, arguments, SoleTemplateHead(functions)))
		return nullptr;
	FunctionList* head = nullptr;
	FunctionList* tail = nullptr;
	for (const FunctionList* node = functions; node != nullptr; node = node->next)
	{
		// A substitution failure drops the template (14.8.2p2).
		FunctionEntity* primary = static_cast<FunctionEntity*>(node->binding->entity);
		if (!primary->templated || HeadOf(primary) == nullptr)
			continue;
		TypeList converted;
		if (!match_template_arguments(HeadOf(primary), arguments.data(), static_cast<uint32_t>(arguments.size()),
				converted, false, component.location, true))
			continue;
		TypeList complete(HeadOf(primary)->count, nullptr);
		for (size_t i = 0; i < converted.size(); ++i)
			complete[i] = converted[i];
		bool partial = false, packs = false;
		for (uint32_t i = 0; i < HeadOf(primary)->count && !partial; ++i)
		{
			partial = complete[i] == nullptr && !HeadOf(primary)->parameters[i]->pack &&
				!HasDefault(HeadOf(primary), i);
			packs = packs || (complete[i] == nullptr && HeadOf(primary)->parameters[i]->pack);
		}
		FunctionEntity* function = partial ? primary : deduced_specialization(primary, complete);
		if (function == nullptr)
			continue;
		// A pack the arguments leave is empty without a target (14.8.1p3)
		// but deduced from one: the specialization's binding then keeps the
		// explicit arguments too (see select_overloaded).
		Binding* binding = new_binding(BindingKind::Function, primary->name, function,
			partial || packs ? types_.pack(converted.data(), static_cast<uint32_t>(converted.size())) : function->type);
		binding->hidden = true;
		FunctionList* item = static_cast<FunctionList*>(arena_.allocate(sizeof(FunctionList), alignof(FunctionList)));
		*item = FunctionList{ binding, nullptr };
		if (tail == nullptr)
			head = item;
		else
			tail->next = item;
		tail = item;
	}
	if (head == nullptr)
		substitution_error(component.location, "no function template matches the template arguments");
	return head;
}

} // namespace sema
