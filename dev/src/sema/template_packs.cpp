#include "sema/template_packs.h"

#include <algorithm>
#include <string>

#include "lex/identifier_table.h"

// Parameter packs (14.5.3). A pack's argument is an argument pack, an
// interned Pack type; a pack expansion in a template definition's types is
// an Expansion type, which substitution expands element by element in the
// lists it may appear in (function parameters, template arguments, argument
// packs), and which deduction matches against the remaining arguments. An
// instantiation's copies of syntax expand their pack expansions as they are
// made (syntax::PackExpander, bound here by FrameExpander), so analysis and
// lowering never meet an unexpanded pack. A copy keeps an expansion whose
// pattern also names a pack of a template declared in the copy (a member
// template's `pair<T, U>...`): the frame's pack is named there as a
// BoundPack, which expands in lockstep with the expansion's other packs.
namespace sema {

namespace {

void AddPack(PackList& out, uint32_t index)
{
	for (uint32_t known : out)
	{
		if (known == index)
			return;
	}
	out.push_back(index);
}

// Finds the template parameter packs a template definition's expression
// names, by ordinary lookup where it appears (see expression_packs).
class DefinitionPacks : public syntax::PackExpander
{
public:
	DefinitionPacks(NameLookup& names, Scope* scope) : names_(names), scope_(scope) {}

	int32_t pack_length(const Identifier* name) override
	{
		const LookupResult found = names_.unqualified(scope_, name, LookupFilter::Ordinary);
		if (!found.found() || found.ambiguous || found.binding->entity == nullptr ||
			found.binding->entity->kind != EntityKind::TemplateParameter)
			return -1;
		return static_cast<const TemplateParameterEntity*>(found.binding->entity)->pack ? 0 : -1;
	}
	const Identifier* element_name(const Identifier* name, uint32_t) override { return name; }
	void mismatch(uint32_t) override {}

private:
	NameLookup& names_;
	Scope* scope_;
};

// The function parameter packs a template definition's expression names
// (each expands its type's template parameter packs, 14.5.3p5).
class ParameterPacks : public syntax::PackExpander
{
public:
	ParameterPacks(NameLookup& names, Scope* scope) : names_(names), scope_(scope) {}

	int32_t pack_length(const Identifier* name) override
	{
		const LookupResult found = names_.unqualified(scope_, name, LookupFilter::Ordinary);
		return found.found() && !found.ambiguous && found.binding->kind == BindingKind::Parameter &&
				found.binding->type != nullptr && found.binding->type->kind == TypeKind::Expansion
			? 0
			: -1;
	}
	const Identifier* element_name(const Identifier* name, uint32_t) override { return name; }
	void mismatch(uint32_t) override {}

private:
	NameLookup& names_;
	Scope* scope_;
};

// The declarations a template definition's expression names (each name's
// first identifier, looked up where the expression appears; see
// instantiation_dependent), null for a name found ambiguous.
class NamedDeclarations : public syntax::PackExpander
{
public:
	NamedDeclarations(NameLookup& names, Scope* scope, std::vector<const Binding*>& out)
		: names_(names), scope_(scope), out_(out)
	{}

	int32_t pack_length(const Identifier* name) override
	{
		const LookupResult found = names_.unqualified(scope_, name, LookupFilter::Ordinary);
		if (found.found())
			out_.push_back(found.ambiguous ? nullptr : found.binding);
		return -1;
	}
	const Identifier* element_name(const Identifier* name, uint32_t) override { return name; }
	void mismatch(uint32_t) override {}

private:
	NameLookup& names_;
	Scope* scope_;
	std::vector<const Binding*>& out_;
};

} // namespace

bool Analyzer::instantiation_dependent(const syntax::Expr& expr, Scope* scope, bool contextual_access)
{
	// Whether a template definition's expression is value-dependent
	// (14.6.2.3p1-p2, conservatively instantiation-dependent): it names a
	// template parameter, a type, variable or parameter of dependent type,
	// a local declaration whose initializer depends on a template parameter,
	// a const variable or non-static data member of a templated class (its
	// value may depend on its initializer, `this->m` on the class), a
	// member function or enumerator of one, nests a pack expansion, applies
	// sizeof..., or uses a templated class's `this`; a call of a function
	// that is no constexpr one, decltype(g()), `new int`, a member typedef
	// of int and a catch parameter of type int are not.
	std::vector<const Binding*> named;
	NamedDeclarations finder(names_, scope, named);
	std::vector<const Identifier*> names;
	syntax::SyntaxCloner cloner(arena_, true, &finder);
	cloner.collect_packs(&expr, names);
	if (cloner.nested_expansions() != 0 || cloner.pack_sizes() != 0 ||
		(cloner.this_uses() != 0 && TemplatedClassScope(scope)))
		return true;
	for (const Binding* binding : named)
	{
		if (binding == nullptr || binding->dependent || binding->value_dependent ||
			(binding->entity != nullptr && binding->entity->kind == EntityKind::TemplateParameter))
			return true;
		// Class names and members can carry access obligations whose answer
		// differs between specializations (for example friendship granted to
		// just one). A reusable assertion must not share those checks.
		if (contextual_access &&
			((binding->declared_in != nullptr && binding->declared_in->kind == ScopeKind::Class) ||
				(binding->type != nullptr && binding->type->unqualified->kind == TypeKind::Class)))
			return true;
		const bool member = binding->declared_in != nullptr && binding->declared_in->kind == ScopeKind::Class &&
			TemplatedClassScope(binding->declared_in);
		switch (binding->kind)
		{
		case BindingKind::Type:
		case BindingKind::TypeAlias:
		case BindingKind::Parameter:
			if (binding->type == nullptr || dependent_type(binding->type))
				return true;
			break;
		case BindingKind::Variable:
		{
			const VariableEntity* variable = static_cast<const VariableEntity*>(binding->entity);
			if (binding->type == nullptr || dependent_type(binding->type) ||
				(member && (!variable->is_static || (binding->type->cv & kConst) != 0)))
				return true;
			break;
		}
		case BindingKind::Namespace:
		case BindingKind::NamespaceAlias:
			break;
		default:
			if (member)
				return true;
			break;
		}
	}
	return false;
}

Analyzer::FrameExpander::FrameExpander(Analyzer& analyzer, const Scope* scope) : analyzer_(analyzer)
{
	// Frames bind a pack's name to its argument pack (see bind_pack; a
	// function parameter pack's to its types', see function_pack_scope);
	// an inner frame's pack of a name hides an outer one's.
	for (const Scope* current = scope; current != nullptr; current = current->parent)
	{
		if (current->kind != ScopeKind::TemplateParameters)
			continue;
		for (const Binding* binding = current->bindings; binding != nullptr; binding = binding->next)
		{
			const Type* type = binding->type;
			if (type != nullptr && type->kind == TypeKind::BoundPack)
				type = type->element;
			if (binding->kind == BindingKind::TypeAlias && binding->entity == nullptr && type != nullptr &&
				type->kind == TypeKind::Pack && binding->name != nullptr && pack_length(binding->name) < 0)
				add(binding->name, type->param_count);
		}
	}
}

int32_t Analyzer::FrameExpander::pack_length(const Identifier* name)
{
	for (const std::pair<const Identifier*, uint32_t>& pack : packs_)
	{
		if (pack.first == name)
			return static_cast<int32_t>(pack.second);
	}
	return -1;
}

const Identifier* Analyzer::FrameExpander::element_name(const Identifier* name, uint32_t index)
{
	return analyzer_.pack_element_name(name, index);
}

void Analyzer::FrameExpander::mismatch(uint32_t location)
{
	// 14.5.3p5: ill-formed, or a substitution's failure (14.8.2p8; the
	// copy's clients check before analyzing it).
	analyzer_.substitution_error(location, "a pack expansion of parameter packs of different lengths");
}

const Identifier* Analyzer::pack_element_name(const Identifier* name, uint32_t index)
{
	// The pack's name with `__pack` and the index: a name reserved to the
	// implementation (17.6.4.3.2p1), so no declaration of the program's
	// has it, and a valid LowIR name for a function parameter pack's
	// elements' parameters and slots.
	std::string spelling = name->spelling;
	spelling += "__pack";
	spelling += std::to_string(index);
	return identifiers_.intern(spelling.data(), static_cast<uint32_t>(spelling.size()));
}

uint32_t InstanceLevels(const Scope* scope)
{
	// Its enclosing classes instantiated from a template (not declared by
	// `template<>`).
	uint32_t levels = 0;
	for (; scope != nullptr; scope = scope->parent)
	{
		const ClassEntity* cls = scope->kind == ScopeKind::Class && scope->entity != nullptr &&
				scope->entity->kind == EntityKind::Class
			? static_cast<const ClassEntity*>(scope->entity)
			: nullptr;
		if (cls != nullptr && cls->templ != nullptr && cls->templ->role == TemplateRole::Specialization &&
			!cls->templ->explicit_specialization && !cls->templ->dependent)
			++levels;
	}
	return levels;
}

void Analyzer::bind_pack(Scope* frame, const TemplateParameterEntity* parameter, const Type* argument, int32_t element)
{
	// A parameter pack's name bound to its argument pack as a BoundPack
	// (whose elements sizeof... counts and a copy's expansions take,
	// enumerable in the frame for FrameExpander, and which an expansion
	// the copy keeps names), and each element's name to the element as a
	// parameter that is no pack is bound; for one element of an expansion
	// (`element`, see Substitution::elements), that element's name only.
	const Type* bound = types_.bound_pack(
		argument, parameter->canonical, parameter->depth + InstanceLevels(frame->parent));
	Binding* alias = new_binding(BindingKind::TypeAlias, parameter->name, nullptr, bound);
	alias->hidden = true;
	print_in(frame, alias);
	declare_in(frame, alias);
	const uint32_t first = element >= 0 ? static_cast<uint32_t>(element) : 0;
	const uint32_t end = element >= 0 ? first + 1 : argument->param_count;
	for (uint32_t i = first; i < end && i < argument->param_count; ++i)
	{
		const Identifier* name = pack_element_name(parameter->name, i);
		const Type* element = argument->params[i];
		Binding* binding = parameter->value_type != nullptr
			? value_binding(parameter, element, frame, name)
			: new_binding(BindingKind::TypeAlias, name, const_cast<TemplateParameterEntity*>(parameter), element);
		binding->hidden = true;
		declare_in(frame, binding);
	}
}

void Analyzer::unexpanded_packs(const Type* type, uint32_t depth, PackList& out, TypeList* bound, bool placeholders)
{
	// The template parameter packs of depth `depth` (any, for kAnyDepth) a
	// type names outside the pack expansions nested in it (each of which
	// expands its own, 14.5.3p5), by index; and in `bound`, if given, the
	// bound packs it names there. Without `placeholders`, the packs a
	// placeholder's expression names are left out (expression_packs also
	// gives those its type-ids' expansions expand).
	for (;;)
	{
		switch (type->kind)
		{
		case TypeKind::BoundPack:
			if (bound != nullptr && std::find(bound->begin(), bound->end(), type->unqualified) == bound->end())
				bound->push_back(type->unqualified);
			return;
		case TypeKind::TemplateParameter:
		case TypeKind::TemplateTemplateParameter:
		{
			const TemplateParameterEntity* parameter = static_cast<const TemplateParameterEntity*>(type->entity);
			if (parameter->qualifier != nullptr)
			{
				type = parameter->qualifier;
				continue;
			}
			if (parameter->expression != nullptr)
			{
				if (parameter->packs != nullptr && placeholders)
					unexpanded_packs(parameter->packs, depth, out, bound);
				return;
			}
			if (parameter->pack && (depth == kAnyDepth || parameter->depth == depth))
				AddPack(out, parameter->index);
			return;
		}
		case TypeKind::Class:
		{
			const ClassTemplateFacts* facts = static_cast<const ClassEntity*>(type->entity)->templ;
			// A dependent template-name's specialization names its name's
			// packs too (`TT<A>...` for a template template-parameter pack).
			if (facts != nullptr && facts->role == TemplateRole::Specialization && facts->dependent &&
				IsStandIn(facts->pattern))
				unexpanded_packs(facts->pattern->templ->template_name->type, depth, out, bound, placeholders);
			if (facts != nullptr && facts->role == TemplateRole::Specialization && facts->dependent)
			{
				for (uint32_t i = 0; i < facts->argument_count; ++i)
				{
					if (facts->arguments[i]->kind != TypeKind::Expansion)
						unexpanded_packs(facts->arguments[i], depth, out, bound, placeholders);
				}
			}
			return;
		}
		case TypeKind::Pointer:
		case TypeKind::LValueReference:
		case TypeKind::RValueReference:
			type = type->element;
			continue;
		case TypeKind::Array:
			if (type->owner != nullptr)
				unexpanded_packs(type->owner, depth, out, bound, placeholders);
			type = type->element;
			continue;
		case TypeKind::MemberPointer:
			unexpanded_packs(type->owner, depth, out, bound, placeholders);
			type = type->element;
			continue;
		case TypeKind::Function:
		case TypeKind::Pack:
			for (uint32_t i = 0; i < type->param_count; ++i)
			{
				if (type->params[i]->kind != TypeKind::Expansion)
					unexpanded_packs(type->params[i], depth, out, bound, placeholders);
			}
			if (type->kind == TypeKind::Pack)
				return;
			type = type->element;
			continue;
		default:
			return;
		}
	}
}

const Type* Analyzer::expression_packs(const syntax::Expr& expr, Scope* scope, const Type** nested)
{
	// The template parameter packs a template definition's expression (a
	// placeholder's) names outside the expansions nested in it: a Pack of
	// their canonical types, null for none; and (`nested`) those the
	// expansions nested in it name.
	DefinitionPacks finder(names_, scope);
	std::vector<const Identifier*> names, inner;
	syntax::SyntaxCloner cloner(arena_, true, &finder);
	cloner.collect_packs(&expr, names, nested != nullptr ? &inner : nullptr);
	const Type* packs = canonical_packs(names, scope);
	if (nested == nullptr)
		return packs;
	*nested = canonical_packs(inner, scope);
	if (cloner.nested_expansions() != 0 && packs != nullptr)
	{
		// A nested expansion of a function parameter pack expands the packs
		// its type's pattern names: the pattern joins the nested packs (see
		// element_renames, which finds those packs in it).
		ParameterPacks parameters(names_, scope);
		std::vector<const Identifier*> outer, nested_parameters;
		syntax::SyntaxCloner walker(arena_, true, &parameters);
		walker.collect_packs(&expr, outer, &nested_parameters);
		if (!nested_parameters.empty())
		{
			TypeList all;
			for (uint32_t i = 0; *nested != nullptr && i < (*nested)->param_count; ++i)
				all.push_back((*nested)->params[i]);
			for (const Identifier* name : nested_parameters)
				all.push_back(names_.unqualified(scope, name, LookupFilter::Ordinary).binding->type->element);
			*nested = types_.pack(all.data(), static_cast<uint32_t>(all.size()));
		}
	}
	return packs;
}

const Type* Analyzer::canonical_packs(const std::vector<const Identifier*>& names, Scope* scope)
{
	// The template parameter packs of these names (found by
	// DefinitionPacks) as a Pack of their canonical types, null for none.
	if (names.empty())
		return nullptr;
	TypeList packs;
	for (const Identifier* name : names)
	{
		const LookupResult found = names_.unqualified(scope, name, LookupFilter::Ordinary);
		packs.push_back(static_cast<const TemplateParameterEntity*>(found.binding->entity)->canonical->type);
	}
	return types_.pack(packs.data(), static_cast<uint32_t>(packs.size()));
}

void Analyzer::substitute_list(
	const Type* const* types, uint32_t count, const Substitution& substitution, TypeList& out)
{
	// A list whose elements are types or template arguments substituted (a
	// function's parameters, a specialization's arguments, a pack's
	// elements): a pack expansion becomes one substituted pattern per
	// element of the argument packs of the packs of the substitution's
	// depth it expands (14.5.3p6), all of one length (p7), each substituted
	// with those packs standing for that element; an expansion of packs
	// not known (or of other depths) stays one, its pattern substituted.
	// The bound packs the pattern names expand with it (14.5.3p5), of its
	// length too. Nested template-ids and dependent names recurse through
	// here: a nesting the stack cannot hold is an error (Annex B).
	check_stack(0);
	for (uint32_t i = 0; i < count; ++i)
	{
		const Type* type = types[i];
		if (type->kind != TypeKind::Expansion)
		{
			out.push_back(substitute(type, substitution));
			continue;
		}
		PackList packs;
		TypeList bound;
		unexpanded_packs(type->element, substitution.depth, packs, &bound);
		int32_t length = packs.empty() ? -1 : 0;
		bool mismatch = false;
		// A pattern naming only bound packs (a shared member template head's
		// `A<B, const Ts&...>`, Ts its instance's pack) is of their length.
		if (packs.empty() && !bound.empty())
		{
			PackList other;
			unexpanded_packs(type->element, kAnyDepth, other);
			if (other.empty())
			{
				length = BoundLength(bound);
				mismatch = length < 0;
			}
		}
		for (uint32_t k = 0; k < packs.size() && length >= 0; ++k)
		{
			const uint32_t index = packs[k];
			const Type* argument = index < substitution.count ? substitution.arguments[index] : nullptr;
			if (argument == nullptr || argument->kind != TypeKind::Pack)
				length = -1;
			else if (k != 0 && static_cast<uint32_t>(length) != argument->param_count)
				mismatch = true;
			else
				length = static_cast<int32_t>(argument->param_count);
		}
		if (length >= 0 && !bound.empty() && BoundLength(bound) != length)
			mismatch = true;
		if (mismatch)
		{
			substitution_error(0, "a pack expansion of parameter packs of different lengths");
			out.push_back(nullptr);
			return;
		}
		if (length < 0)
		{
			// The pattern's bound packs are still its own.
			Substitution whole = substitution;
			whole.bound_element = nullptr;
			const Type* pattern = substitute(type->element, whole);
			// `__integer_pack (N)...` (an expansion naming no pack, see
			// integer_pack_argument): once N is known, its values 0, 1,
			// ..., N-1 of N's type.
			if (pattern != nullptr && packs.empty() && bound.empty() && pattern->kind == TypeKind::Value &&
				pattern->entity == nullptr && !dependent_type(pattern))
			{
				if (IsSignedConstant(Constant{ pattern->bound, pattern->element }) &&
					static_cast<int64_t>(pattern->bound) < 0)
				{
					substitution_error(0, "__integer_pack of a negative count");
					out.push_back(nullptr);
					return;
				}
				for (uint64_t value = 0; value < pattern->bound; ++value)
					out.push_back(types_.value(pattern->element, value));
				continue;
			}
			out.push_back(pattern != nullptr ? types_.expansion(pattern) : nullptr);
			continue;
		}
		SmallList<int32_t, 8> elements(substitution.count, -1);
		for (uint32_t k = 0; substitution.elements != nullptr && k < substitution.count; ++k)
			elements[k] = substitution.elements[k];
		Substitution element = substitution;
		element.elements = elements.data();
		uint32_t position = 0;
		element.bound_element = bound.empty() ? nullptr : &position;
		for (int32_t j = 0; j < length; ++j)
		{
			// A pack can be only partially supplied (`Head, Tail...`). Its
			// residual expansion substitutes patterns elementwise and stays
			// around the complete result, rather than inside a nested type-id.
			bool residual = false;
			for (uint32_t index : packs)
			{
				elements[index] = j;
				residual = residual || substitution.arguments[index]->params[j]->kind == TypeKind::Expansion;
			}
			position = static_cast<uint32_t>(j);
			for (const Type* pack : bound)
				residual = residual || pack->element->params[j]->kind == TypeKind::Expansion;
			const Type* result = substitute(type->element, element);
			out.push_back(residual && result != nullptr ? types_.expansion(result) : result);
		}
	}
}

int32_t Analyzer::BoundLength(const TypeList& bound)
{
	// The common length of bound packs, -1 for packs of different lengths.
	for (const Type* pack : bound)
	{
		if (pack->element->param_count != bound[0]->element->param_count)
			return -1;
	}
	return bound.empty() ? -1 : static_cast<int32_t>(bound[0]->element->param_count);
}

const Type* const* Analyzer::effective_arguments(const Substitution& substitution, TypeList& scratch)
{
	// The arguments with each pack an enclosing expansion is substituting
	// element by element replaced by that element.
	if (substitution.elements == nullptr)
		return substitution.arguments;
	scratch.clear();
	for (uint32_t i = 0; i < substitution.count; ++i)
	{
		const Type* argument = substitution.arguments[i];
		const int32_t element = substitution.elements[i];
		scratch.push_back(element >= 0 && argument != nullptr && argument->kind == TypeKind::Pack
				? argument->params[element]
				: argument);
	}
	return scratch.data();
}

void Analyzer::flattened_arguments(const Type* const* arguments, uint32_t count, TypeList& out)
{
	// A specialization's arguments as a template-id would give them: each
	// argument pack's elements in its place.
	for (uint32_t i = 0; i < count; ++i)
	{
		if (arguments[i]->kind != TypeKind::Pack)
		{
			out.push_back(arguments[i]);
			continue;
		}
		for (uint32_t j = 0; j < arguments[i]->param_count; ++j)
			out.push_back(arguments[i]->params[j]);
	}
}

void Analyzer::expanded_argument(
	const syntax::TemplateArgument& argument, Scope* scope, uint32_t location, TypeList& out)
{
	// A template argument pack expansion (14.5.3p4): in a template
	// definition an Expansion of its pattern, which names a pack (p5); a
	// pack a frame binds, named alone, is its elements. A template
	// template-parameter pack's pattern is its name.
	if (integer_pack_argument(argument, scope, out))
		return;
	bool expansion = false;
	const syntax::Name* name = PlainTemplateName(argument, &expansion);
	const Type* type = name != nullptr ? template_name_argument(*name, scope, false) : nullptr;
	if (type == nullptr)
		type = template_argument(argument, scope, location);
	if (type->kind == TypeKind::BoundPack || type->kind == TypeKind::Pack)
	{
		flattened_arguments(type->kind == TypeKind::Pack ? &type : &type->element, 1, out);
		return;
	}
	PackList packs;
	TypeList bound;
	unexpanded_packs(type, kAnyDepth, packs, &bound);
	if (packs.empty() && bound.empty())
		error(location, "a pack expansion that names no parameter pack");
	bound_expansion(types_.expansion(type), out);
}

void Analyzer::bound_expansion(const Type* expansion, TypeList& out)
{
	// A template argument's pack expansion whose pattern names only packs
	// an instantiation binds (a member template's head in an instance,
	// which shares its class template's syntax: `L<list<E, Us...>...>`, E
	// the instance's pack) expands now, as a copy's expansion would
	// (14.5.3p6): an argument list it leaves non-dependent is the
	// instance's to check (14.7.1p1). Another stays one argument.
	PackList packs;
	TypeList bound;
	unexpanded_packs(expansion->element, kAnyDepth, packs, &bound);
	if (packs.empty() && !bound.empty() && !names_placeholder(expansion->element, 0))
		substitute_list(&expansion, 1, Substitution{ 0, nullptr, 0, nullptr, nullptr }, out);
	else
		out.push_back(expansion);
}

const Type* Analyzer::pack_expansion_type(const Type* pattern, uint32_t location)
{
	// A declarator's pack ellipsis (8.3.5p14: a function parameter pack),
	// only in a template definition, naming a pack (14.5.3p5). A pack a
	// frame binds, named alone, is its elements (an inner expansion the
	// copy kept because an outer one names packs not yet known).
	if (pattern->kind == TypeKind::BoundPack)
		return pattern->element;
	PackList packs;
	TypeList bound;
	if (template_depth_ != 0)
		unexpanded_packs(pattern, kAnyDepth, packs, &bound);
	if (packs.empty() && bound.empty())
		error(location, "a pack expansion that names no parameter pack");
	return types_.expansion(pattern);
}

template<typename Deduce>
bool Analyzer::deduce_elements(
	const Type* pattern, uint32_t count, bool counted, uint32_t depth, TypeList& arguments, Deduce deduce)
{
	// A pack expansion's pattern against each of `count` arguments
	// (14.8.2.1p1, 14.8.2.5p8): each deduces an element of each pack the
	// pattern names, and the other parameters as one argument does; the
	// packs' arguments are the packs of those elements, agreeing with any
	// deduced before. A pattern naming no pack of the template is a
	// non-deduced context. A bound pack the pattern names stands for its
	// element there, and has `count` elements where that is the
	// expansion's length (`counted`: no argument is itself an expansion).
	PackList packs;
	TypeList bound;
	unexpanded_packs(pattern, depth, packs, &bound);
	if (packs.empty())
		return true;
	if (!bound.empty() && counted && BoundLength(bound) != static_cast<int32_t>(count))
		return false;
	std::vector<TypeList> collected(packs.size());
	TypeList trial;
	const int32_t saved_element = deduced_element_;
	for (uint32_t j = 0; j < count; ++j)
	{
		trial = arguments;
		for (uint32_t index : packs)
		{
			const Type* known = arguments[index];
			trial[index] = known != nullptr && known->kind == TypeKind::Pack && j < known->param_count
				? known->params[j]
				: nullptr;
		}
		deduced_element_ = bound.empty() ? -1 : static_cast<int32_t>(j);
		const bool deduced = deduce(j, trial);
		deduced_element_ = saved_element;
		if (!deduced)
			return false;
		for (uint32_t k = 0; k < packs.size(); ++k)
		{
			if (trial[packs[k]] == nullptr)
				return false;
			collected[k].push_back(trial[packs[k]]->canonical);
		}
		for (uint32_t i = 0; i < arguments.size(); ++i)
		{
			bool pack = false;
			for (uint32_t index : packs)
				pack = pack || index == i;
			if (!pack && trial[i] != nullptr)
				arguments[i] = trial[i];
		}
	}
	for (uint32_t k = 0; k < packs.size(); ++k)
	{
		const Type* pack = types_.pack(collected[k].data(), static_cast<uint32_t>(collected[k].size()));
		if (arguments[packs[k]] != nullptr && arguments[packs[k]] != pack)
			return false;
		arguments[packs[k]] = pack;
	}
	return true;
}

bool Analyzer::deduce_pack(const Type* p, const Type* a, uint32_t depth, TypeList& arguments)
{
	return deduce_list(p->params, p->param_count, a->params, a->param_count, depth, arguments);
}

bool Analyzer::deduce_list(
	const Type* const* p, uint32_t count, const Type* const* a, uint32_t a_count, uint32_t depth, TypeList& arguments)
{
	// 14.8.2.5p9: argument lists (or packs) compared element by element; a
	// pack expansion ending P's deduces from A's remaining elements (p8,
	// p10); one elsewhere makes the whole list a non-deduced context.
	const bool trailing = count != 0 && p[count - 1]->kind == TypeKind::Expansion;
	const uint32_t fixed = trailing ? count - 1 : count;
	for (uint32_t i = 0; i < fixed; ++i)
	{
		if (p[i]->kind == TypeKind::Expansion)
			return true;
	}
	if (trailing ? a_count < fixed : a_count != count)
		return false;
	for (uint32_t i = 0; i < fixed; ++i)
	{
		// 14.8.2.5p9 (partial ordering): an A that is a pack expansion
		// matches only one in P.
		if (a[i]->kind == TypeKind::Expansion)
			return false;
		if (!deduce_type(p[i], a[i], depth, arguments, false))
			return false;
	}
	if (!trailing)
		return true;
	return deduce_pack_types(p[count - 1]->element, a + fixed, a_count - fixed, depth, arguments);
}

bool Analyzer::deduce_pack_types(
	const Type* pattern, const Type* const* types, uint32_t count, uint32_t depth, TypeList& arguments)
{
	// A pack expansion's pattern against each of `count` types (an A that
	// is itself an expansion, in partial ordering, by its pattern, whose
	// bound packs are the pattern's own).
	bool counted = true;
	for (uint32_t j = 0; j < count; ++j)
		counted = counted && types[j]->kind != TypeKind::Expansion;
	return deduce_elements(pattern, count, counted, depth, arguments, [&](uint32_t j, TypeList& trial) {
		if (types[j]->kind != TypeKind::Expansion)
			return deduce_type(pattern, types[j], depth, trial, false);
		deduced_element_ = -1;
		return deduce_type(pattern, types[j]->element, depth, trial, false);
	});
}

bool Analyzer::deduce_call_pack(const Type* pattern, const syntax::Slice<syntax::Expr*>& call_arguments, uint32_t first,
	uint32_t depth, TypeList& arguments)
{
	// 14.8.2.1p1: a function parameter pack ending the parameter list
	// against each remaining argument (a braced-init-list deduces nothing,
	// so its pack elements stay undeduced).
	return deduce_elements(
		pattern, call_arguments.size - first, true, depth, arguments, [&](uint32_t j, TypeList& trial) {
			const syntax::Expr* argument = call_arguments[first + j];
			if (argument->kind == syntax::ExprKind::BracedInit)
				return true;
			const ExprFacts* facts = argument->facts;
			if (NonstaticMemberName(facts))
				return false;
			if ((facts->flags & kOverloadSet) != 0)
			{
				deduce_overloaded(pattern, facts->functions, depth, trial);
				return true;
			}
			return facts->type != nullptr && deduce_argument(pattern, facts->type, facts->category, depth, trial);
		});
}

Scope* Analyzer::function_pack_scope(Scope* parent, const syntax::ParameterClause& clause,
	const std::pair<const Identifier*, uint32_t>* packs, size_t count, const Type* type)
{
	// Each named function parameter pack of an instantiated definition's
	// (or lambda's) clause, whose copy has one parameter per element, bound
	// to the pack of those parameters' types (sizeof... counts it), in a
	// scope between the definition's frame and its body.
	if (count == 0)
		return parent;
	Scope* scope = new_scope(ScopeKind::TemplateParameters, parent, nullptr, nullptr, nullptr);
	scope->dumped = false;
	uint32_t position = 0;
	size_t next = 0;
	for (const syntax::ParameterDeclaration* parameter : clause.parameters)
	{
		const syntax::Declarator* declarator = parameter->declarator;
		if (declarator == nullptr || declarator->pack == syntax::PackPosition::None || next >= count)
		{
			++position;
			continue;
		}
		const std::pair<const Identifier*, uint32_t> pack = packs[next++];
		if (pack.first != nullptr && position + pack.second <= type->param_count)
		{
			Binding* binding = new_binding(
				BindingKind::TypeAlias, pack.first, nullptr, types_.pack(type->params + position, pack.second));
			binding->hidden = true;
			declare_in(scope, binding);
		}
		position += pack.second;
	}
	return scope;
}

bool Analyzer::pack_size(const Identifier* pack, Scope* scope, uint64_t& count)
{
	// 5.3.3p5: sizeof... of a pack an instantiation binds to an argument
	// pack (a template parameter pack's, see bind_pack, or a function
	// parameter pack's).
	const LookupResult found = names_.unqualified(scope, pack, LookupFilter::Ordinary);
	const Type* type = found.found() && found.binding->kind == BindingKind::TypeAlias ? found.binding->type : nullptr;
	if (type != nullptr && type->kind == TypeKind::BoundPack)
		type = type->element;
	if (type == nullptr || type->kind != TypeKind::Pack)
		return false;
	count = type->param_count;
	return true;
}

} // namespace sema
