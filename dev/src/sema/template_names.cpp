#include "sema/template_packs.h"

#include "support/small_list.h"

// Template names: template template-parameters and their arguments
// (14.3.3), dependent template-ids, and alias templates (14.5.7). A
// template-name as a template argument is a Template type (a class or
// alias template) or a template template-parameter's type, canonically its
// position. A template-id whose template is a template template-parameter
// or a dependent member template-name (`Q::template name`) is a dependent
// specialization of a stand-in class template for that name, its
// arguments as written; substitution replaces the name by the template it
// stands for and forms that template's specialization, and deduction
// deduces the name from a specialization's template. An alias template's
// template-id is its aliased type substituted, once per canonical
// argument list.
namespace sema {

namespace {

uint64_t PointerWord(const void* pointer)
{
	return static_cast<uint64_t>(reinterpret_cast<uintptr_t>(pointer));
}

// A template parameter's kind in a template template-parameter's shape
// (see Analyzer::positional_template_parameter).
enum : uint64_t
{
	kShapeType = 0,
	kShapeValue = 1,
	kShapeTemplate = 2,
	kShapePack = 4
};

} // namespace

TemplateParameterEntity* Analyzer::positional_template_parameter(
	uint32_t depth, uint32_t index, bool pack, const TemplateHead* head)
{
	// One per (depth, index, pack, shape): template template-parameters at
	// a position are equivalent (14.5.6.1p6) when their own parameter
	// lists are (14.1: parameter kinds, packs, non-type parameters' types
	// and nested template template-parameters alike). The first
	// declaration's list stands for the shape.
	SmallList<uint64_t, 16> words;
	words.push_back(static_cast<uint64_t>(depth) << 32 | index);
	words.push_back(pack ? 1 : 0);
	for (uint32_t i = 0; i < head->count; ++i)
	{
		const TemplateParameterEntity* parameter = head->parameters[i];
		const uint64_t kind = parameter->template_template ? kShapeTemplate
			: parameter->non_type                          ? kShapeValue
														   : kShapeType;
		words.push_back(kind | (parameter->pack ? kShapePack : 0));
		if (parameter->template_template)
			words.push_back(PointerWord(parameter->canonical));
		else if (parameter->non_type)
			words.push_back(PointerWord(parameter->value_type->canonical));
	}
	DecltypeKey key = DecltypeKey{ words.data(), static_cast<uint32_t>(words.size()) };
	if (TemplateParameterEntity** found = template_template_parameters_.find(key))
		return *found;
	TemplateParameterEntity* entity = new_entity<TemplateParameterEntity>(
		EntityKind::TemplateParameter, nullptr, nullptr, nullptr);
	entity->template_template = true;
	entity->pack = pack;
	entity->depth = static_cast<uint16_t>(depth);
	entity->index = static_cast<uint16_t>(index);
	entity->canonical = entity;
	entity->template_head = head;
	entity->type = types_.named(TypeKind::TemplateTemplateParameter, entity);
	key.words = arena_.copy_array(key.words, key.count);
	template_template_parameters_.insert(key, entity);
	return entity;
}

const Type* Analyzer::template_shape(TemplateParameterEntity* const* parameters, uint32_t count)
{
	// A template parameter list's identity for redeclaration (14.5.6.1p6):
	// each parameter's positional type (its kind, pack and position; a
	// template template-parameter's own list), with a non-type parameter's
	// canonical type.
	TypeList elements;
	for (uint32_t i = 0; i < count; ++i)
	{
		const TemplateParameterEntity* parameter = parameters[i];
		const Type* positional = parameter->canonical->type;
		if (parameter->non_type && parameter->value_type != nullptr)
		{
			const Type* pair[2] = { positional, parameter->value_type->canonical };
			positional = types_.pack(pair, 2);
		}
		elements.push_back(positional);
	}
	return types_.pack(elements.data(), static_cast<uint32_t>(elements.size()));
}

const Type* Analyzer::template_name_type(const Binding* binding, bool injected)
{
	// The template a name denotes where a template-name may stand (a
	// template-id's name, a template template-argument, 14.3.3p1): a class
	// template's or alias template's name, a template template-parameter,
	// or the template an instantiation binds one to. An injected-class-name
	// names its class template where `injected` allows (14.6.1p1). Null for
	// any other name.
	if (binding == nullptr)
		return nullptr;
	const Entity* entity = binding->entity;
	if (binding->kind == BindingKind::TypeAlias)
	{
		if (entity != nullptr && entity->kind == EntityKind::AliasTemplate)
			return types_.named(TypeKind::Template, const_cast<Entity*>(entity));
		const Type* type = binding->type;
		if (type != nullptr && (type->kind == TypeKind::Template || type->kind == TypeKind::TemplateTemplateParameter))
			return type;
		return nullptr;
	}
	if (binding->kind != BindingKind::Type || entity == nullptr)
		return nullptr;
	if (entity->kind == EntityKind::TemplateParameter)
	{
		const TemplateParameterEntity* parameter = static_cast<const TemplateParameterEntity*>(entity);
		return parameter->template_template ? parameter->type : nullptr;
	}
	if (entity->kind != EntityKind::Class)
		return nullptr;
	const ClassEntity* cls = static_cast<const ClassEntity*>(entity);
	if (cls->templ == nullptr || IsStandIn(cls))
		return nullptr;
	const bool own_name = cls->templ->role == TemplateRole::Template && binding->declared_in != cls->scope;
	if (!own_name && !injected)
		return nullptr;
	ClassEntity* templ = class_template_of(cls);
	return templ != nullptr ? types_.named(TypeKind::Template, templ) : nullptr;
}

const syntax::Name* Analyzer::PlainTemplateName(const syntax::TemplateArgument& argument, bool* expansion)
{
	// A template argument that is only a name without template arguments
	// (as a type-id or an id-expression): what a template template-argument
	// is (14.3.3p1); with `expansion`, one followed by `...` too (a
	// template template-parameter pack's expansion, 14.5.3p4).
	const syntax::Name* name = nullptr;
	if (expansion != nullptr)
		*expansion = argument.pack;
	if (argument.type != nullptr)
	{
		const syntax::TypeId& type = *argument.type;
		const syntax::Declarator* declarator = type.declarator;
		if (type.specifiers.size != 1 || type.specifiers[0].kind != syntax::DeclSpecKind::Name)
			return nullptr;
		if (declarator != nullptr &&
			(declarator->pointers.size != 0 || declarator->name != nullptr || declarator->nested != nullptr ||
				declarator->suffixes.size != 0))
			return nullptr;
		if (declarator != nullptr && declarator->pack != syntax::PackPosition::None)
		{
			if (expansion == nullptr)
				return nullptr;
			*expansion = true;
		}
		name = type.specifiers[0].name;
		if (name->typename_keyword)
			return nullptr;
	}
	else if (argument.expression != nullptr && argument.expression->kind == syntax::ExprKind::Id)
		name = static_cast<const syntax::IdExpr*>(argument.expression)->name;
	if (name == nullptr || name->last().kind != syntax::NameComponentKind::Identifier || name->last().has_arguments)
		return nullptr;
	return name;
}

const Type* Analyzer::template_name_argument(const syntax::Name& name, Scope* scope, bool injected)
{
	// A template template-argument's id-expression (14.3.3p1): the template
	// it names, or null when it names none. A member of a dependent
	// qualifier names a member template only after `template` (14.2p4).
	// Access is checked as for any name (11p4).
	const syntax::NameComponent& last = name.last();
	LookupResult result = LookupResult();
	NamePrefix prefix = NamePrefix{ nullptr, nullptr, nullptr };
	if (!name.qualified())
		result = names_.unqualified(scope, last.identifier, LookupFilter::Ordinary);
	else
	{
		prefix = name_prefix(name, scope);
		if (prefix.dependent != nullptr)
			return last.template_keyword ? dependent_template_name(prefix.dependent, last.identifier) : nullptr;
		result = lookup_in(prefix, last.identifier, LookupFilter::Ordinary);
	}
	if (!result.found() || (result.ambiguous && !result.injected_template))
		return nullptr;
	const Type* templ = template_name_type(result.binding, injected || result.injected_template);
	if (templ == nullptr || !semantic() || template_depth_ != 0)
		return templ;
	if (prefix.scope != nullptr && prefix.scope->kind == ScopeKind::Class)
		check_access(
			result.binding, static_cast<const ClassEntity*>(prefix.scope->entity), nullptr, scope, name.location);
	else if (!name.qualified())
		check_unqualified_access(result.binding, scope, name.location);
	return templ;
}

const TemplateHead* Analyzer::TemplateHeadOf(const Type* templ)
{
	// The template parameter list of a template-name's template (null when
	// unknown: a dependent member template-name, a unique template of
	// partial ordering).
	if (templ->kind == TypeKind::Template)
	{
		if (templ->entity->kind == EntityKind::AliasTemplate)
			return static_cast<const AliasTemplateEntity*>(templ->entity)->head;
		return static_cast<const ClassEntity*>(templ->entity)->templ->head;
	}
	return static_cast<const TemplateParameterEntity*>(templ->entity)->template_head;
}

bool Analyzer::template_parameters_match(const TemplateParameterEntity* p, const TemplateParameterEntity* a)
{
	// 14.3.3p3: template parameters of one kind; non-type parameters of
	// equivalent types (one naming a parameter of its own list names the
	// parameter at that position of the other's, 14.5.6.1p6); template
	// template-parameters with matching lists.
	if (p->template_template != a->template_template || p->non_type != a->non_type)
		return false;
	if (p->template_template)
		return p->template_head == nullptr || a->template_head == nullptr ||
			template_heads_match(p->template_head, a->template_head);
	if (!p->non_type)
		return true;
	const Type* first = p->value_type->canonical;
	const Type* second = a->value_type->canonical;
	if (first == second)
		return true;
	// A parameter of a type its own list's type parameter names
	// (`template<class T, T...> class`) against one of a fixed type
	// (`template<class, int...> class`), either way: GCC accepts the pair in
	// C++11 mode (Clang the template template-parameter's direction), as
	// C++17's P0522 matching does; deducing from a specialization still
	// checks the values' types (see deduce_stand_in).
	const auto own_type_parameter = [](const Type* type, uint32_t depth) {
		const TemplateParameterEntity* entity = type->kind == TypeKind::TemplateParameter && type->cv == 0
			? static_cast<const TemplateParameterEntity*>(type->entity)
			: nullptr;
		return entity != nullptr && !IsPlaceholder(entity) && entity->depth == depth && !entity->non_type;
	};
	if (own_type_parameter(first, p->depth) != own_type_parameter(second, a->depth))
		return true;
	if (first->kind != TypeKind::TemplateParameter || second->kind != TypeKind::TemplateParameter ||
		first->cv != second->cv)
		return false;
	const TemplateParameterEntity* x = static_cast<const TemplateParameterEntity*>(first->entity);
	const TemplateParameterEntity* y = static_cast<const TemplateParameterEntity*>(second->entity);
	return !IsPlaceholder(x) && !IsPlaceholder(y) && x->depth == p->depth && y->depth == a->depth &&
		x->index == y->index && x->non_type == y->non_type && x->pack == y->pack;
}

bool Analyzer::template_heads_match(const TemplateHead* parameter, const TemplateHead* argument)
{
	// 14.3.3p3: each parameter of the argument template's list matches the
	// template template-parameter's at its position; a parameter pack of the
	// template template-parameter's matches the argument's remaining
	// parameters and packs of its kind. An argument's pack matches only such
	// a pack (p2's example: `X<C>` is ill-formed, a template parameter pack
	// does not match a template parameter).
	uint32_t next = 0;
	for (uint32_t i = 0; i < argument->count; ++i)
	{
		const TemplateParameterEntity* a = argument->parameters[i];
		if (next == parameter->count)
			return false;
		const TemplateParameterEntity* p = parameter->parameters[next];
		if (!template_parameters_match(p, a) || (a->pack && !p->pack))
			return false;
		if (!p->pack)
			++next;
	}
	return next == parameter->count || (next + 1 == parameter->count && parameter->parameters[next]->pack);
}

const Type* Analyzer::dependent_template_name(const Type* qualifier, const Identifier* name)
{
	// A member template-name of a dependent qualifier (`Q::template name`,
	// 14.2p4): one per (qualifier, name), canonically that of the canonical
	// qualifier, resolved when an instantiation substitutes the qualifier.
	const PlaceholderKey key = PlaceholderKey{ qualifier, name };
	if (TemplateParameterEntity** found = template_name_placeholders_.find(key))
		return (*found)->type;
	TemplateParameterEntity* canonical = nullptr;
	if (qualifier->canonical != qualifier)
		canonical = static_cast<TemplateParameterEntity*>(dependent_template_name(qualifier->canonical, name)->entity);
	TemplateParameterEntity* entity = new_entity<TemplateParameterEntity>(
		EntityKind::TemplateParameter, name, nullptr, nullptr);
	entity->template_template = true;
	entity->qualifier = qualifier;
	entity->canonical = canonical != nullptr ? canonical : entity;
	entity->type = types_.named(TypeKind::TemplateTemplateParameter, entity);
	template_name_placeholders_.insert(key, entity);
	return entity->type;
}

ClassEntity* Analyzer::stand_in_template(const TemplateParameterEntity* name)
{
	// The class template whose specializations are a dependent
	// template-name's template-ids: one per canonical name, never defined
	// and not declared in any scope.
	TemplateParameterEntity* canonical = name->canonical;
	if (canonical->stand_in != nullptr)
		return canonical->stand_in;
	ClassEntity* entity = new_class(TokenKind::KW_STRUCT, canonical->name, graph_.global);
	ClassTemplateFacts* facts = class_template_facts(entity, TemplateRole::Template);
	facts->template_name = canonical;
	canonical->stand_in = entity;
	return entity;
}

ClassEntity* Analyzer::alias_stand_in(const AliasTemplateEntity* alias)
{
	// The class template whose dependent specializations are an alias
	// template's template-ids that drop dependent arguments (CWG 1558):
	// one per alias template, never defined nor declared in any scope.
	AliasTemplateEntity* owner = const_cast<AliasTemplateEntity*>(alias);
	if (owner->stand_in != nullptr)
		return owner->stand_in;
	ClassEntity* entity = new_class(TokenKind::KW_STRUCT, alias->name, graph_.global);
	class_template_facts(entity, TemplateRole::Template)->alias_stand_in = true;
	alias_index_.set(entity, static_cast<uint32_t>(alias_facts_.size()));
	alias_facts_.push_back(alias);
	owner->stand_in = entity;
	return entity;
}

const AliasTemplateEntity* Analyzer::stand_in_alias(const ClassEntity* templ) const
{
	// The alias template a stand-in template stands for (null for another
	// class).
	if (templ->templ == nullptr || !templ->templ->alias_stand_in)
		return nullptr;
	return static_cast<const AliasTemplateEntity*>(alias_facts_[alias_index_.find(templ)]);
}

const Type* Analyzer::aliased_of(const Type* type) const
{
	// A class type that is an alias stand-in's specialization: the type it
	// aliases (null for another type).
	if (type->kind != TypeKind::Class)
		return nullptr;
	const ClassTemplateFacts* facts = static_cast<const ClassEntity*>(type->entity)->templ;
	if (facts == nullptr || facts->role != TemplateRole::Specialization || facts->pattern == nullptr ||
		!facts->pattern->templ->alias_stand_in)
		return nullptr;
	const uint32_t index = alias_index_.find(type->entity);
	return index != PointerMap::kMissing ? static_cast<const Type*>(alias_facts_[index]) : nullptr;
}

const Type* Analyzer::aliased_class_type(const Type* type)
{
	// An alias stand-in's specialization as the type it aliases (with its
	// cv-qualifiers); any other type itself (aliased_type has returned a
	// type that is no class).
	const Type* aliased = aliased_of(type->unqualified);
	if (aliased == nullptr)
		return type;
	return types_.qualified(aliased_type(aliased), type->cv);
}

const Type* Analyzer::declared_type(const Type* type)
{
	// The canonical type as declarations compare it: an alias template-id
	// is the type it aliases (14.5.7p2), also one whose stand-in keeps the
	// dependent arguments it drops (CWG 1558), so `void_t<typename T::a>
	// g(T)` and `void g(T)` declare one template (14.5.6.1p4-p6). Only a
	// dependent type can name a stand-in.
	// A placeholder with an earlier equivalent is that one (see
	// equivalent_representative).
	const Type* canonical = type->canonical;
	if ((alias_facts_.empty() && representatives_.empty()) || !dependent_type(canonical))
		return canonical;
	return without_stand_ins(canonical)->canonical;
}

const Type* Analyzer::without_stand_ins(const Type* type)
{
	// `type` with every alias stand-in's specialization in it replaced by
	// the type it aliases (itself when it names none); a dependent class
	// template specialization with one among its arguments formed again.
	if (!dependent_type(type))
		return type;
	check_stack(0);
	switch (type->kind)
	{
	case TypeKind::Class:
	{
		if (const Type* aliased = aliased_of(type->unqualified))
			return types_.qualified(without_stand_ins(aliased), type->cv);
		const ClassTemplateFacts* facts = static_cast<const ClassEntity*>(type->entity)->templ;
		if (facts == nullptr || facts->role != TemplateRole::Specialization || !facts->dependent ||
			IsStandIn(facts->pattern))
			return type;
		TypeList given;
		TypeList arguments;
		flattened_arguments(facts->arguments, facts->argument_count, given);
		bool changed = false;
		for (const Type* argument : given)
		{
			arguments.push_back(without_stand_ins(argument));
			changed = changed || arguments.back() != argument;
		}
		const ClassEntity* specialization = changed ? class_specialization(facts->pattern, arguments, 0, false)
													: nullptr;
		return specialization != nullptr ? types_.qualified(specialization->type, type->cv) : type;
	}
	case TypeKind::Mode:
		return types_.qualified(
			apply_mode(without_stand_ins(type->element), type->fundamental, 0, type->bound != 0), type->cv);
	case TypeKind::Pointer:
	case TypeKind::LValueReference:
	case TypeKind::RValueReference:
	case TypeKind::Expansion:
	case TypeKind::Array:
	case TypeKind::Vector:
	{
		const Type* element = without_stand_ins(type->element);
		if (element == type->element)
			return type;
		if (type->kind == TypeKind::Pointer)
			return types_.qualified(types_.pointer(element), type->cv);
		if (type->kind == TypeKind::Expansion)
			return types_.expansion(element);
		if (type->kind == TypeKind::Array)
			return types_.array(element, type->bound, type->owner);
		if (type->kind == TypeKind::Vector)
			return types_.qualified(types_.vector(element, type->bound, type->owner, type->variadic), type->cv);
		return types_.reference(element, type->kind == TypeKind::RValueReference);
	}
	case TypeKind::TemplateParameter:
	{
		const uint32_t index = placeholder_representatives_.find(type->unqualified->entity);
		return index == PointerMap::kMissing ? type : types_.qualified(representatives_[index]->type, type->cv);
	}
	case TypeKind::MemberPointer:
	{
		const Type* owner = without_stand_ins(type->owner);
		const Type* element = without_stand_ins(type->element);
		if (owner == type->owner && element == type->element)
			return type;
		return types_.qualified(types_.member_pointer(owner->unqualified, element), type->cv);
	}
	case TypeKind::Function:
	case TypeKind::Pack:
	{
		TypeList parameters;
		bool changed = false;
		for (uint32_t i = 0; i < type->param_count; ++i)
		{
			parameters.push_back(without_stand_ins(type->params[i]));
			changed = changed || parameters.back() != type->params[i];
		}
		if (type->kind == TypeKind::Pack)
			return changed ? types_.pack(parameters.data(), static_cast<uint32_t>(parameters.size())) : type;
		const Type* result = without_stand_ins(type->element);
		if (!changed && result == type->element)
			return type;
		return types_.function(result, parameters.data(), static_cast<uint32_t>(parameters.size()), type->variadic,
			type->function_cv, type->ref);
	}
	default:
		return type;
	}
}

void Analyzer::named_parameters(const Type* type, uint32_t depth, uint64_t& named, bool& opaque, uint32_t level)
{
	// The template parameters of `depth` a type names (bits by index); an
	// expression placeholder (a decltype-specifier, a value-dependent
	// argument or array bound) makes the answer unknown (`opaque`).
	if (opaque || level > 256)
	{
		opaque = true;
		return;
	}
	switch (type->kind)
	{
	case TypeKind::TemplateParameter:
	case TypeKind::TemplateTemplateParameter:
	{
		const TemplateParameterEntity* parameter = static_cast<const TemplateParameterEntity*>(type->entity);
		if (parameter->expression != nullptr || parameter->pending != nullptr)
			opaque = true;
		else if (parameter->qualifier != nullptr)
			named_parameters(parameter->qualifier, depth, named, opaque, level + 1);
		else if (parameter->depth == depth && parameter->index < 64)
			named |= uint64_t(1) << parameter->index;
		return;
	}
	case TypeKind::Class:
	{
		const ClassTemplateFacts* facts = static_cast<const ClassEntity*>(type->entity)->templ;
		if (facts == nullptr || facts->role != TemplateRole::Specialization || !facts->dependent)
		{
			// A member of a templated class (or a dependent nested class).
			if (dependent_type(type))
				opaque = true;
			return;
		}
		if (facts->pattern->templ->template_name != nullptr)
			named_parameters(facts->pattern->templ->template_name->type, depth, named, opaque, level + 1);
		for (uint32_t i = 0; i < facts->argument_count; ++i)
			named_parameters(facts->arguments[i], depth, named, opaque, level + 1);
		return;
	}
	case TypeKind::Enum:
		if (dependent_type(type))
			opaque = true;
		return;
	case TypeKind::Array:
	case TypeKind::Vector:
		if (type->owner != nullptr)
			opaque = true;
		named_parameters(type->element, depth, named, opaque, level + 1);
		return;
	case TypeKind::MemberPointer:
		named_parameters(type->owner, depth, named, opaque, level + 1);
		named_parameters(type->element, depth, named, opaque, level + 1);
		return;
	case TypeKind::Pointer:
	case TypeKind::LValueReference:
	case TypeKind::RValueReference:
	case TypeKind::Expansion:
	case TypeKind::Mode:
		named_parameters(type->element, depth, named, opaque, level + 1);
		return;
	case TypeKind::Function:
	case TypeKind::Pack:
		for (uint32_t i = 0; i < type->param_count; ++i)
			named_parameters(type->params[i], depth, named, opaque, level + 1);
		if (type->kind == TypeKind::Function)
			named_parameters(type->element, depth, named, opaque, level + 1);
		return;
	default:
		return;
	}
}

const Type* Analyzer::specialize_template(const Type* templ, TypeList& arguments, uint32_t location)
{
	// The template-id of a template-name and arguments: a class template's
	// specialization (defaults supplied, 14.1p9), an alias template's
	// substituted type (14.5.7p2), or a dependent template-name's stand-in
	// specialization.
	if (templ->kind == TypeKind::Template)
	{
		if (templ->entity->kind == EntityKind::AliasTemplate)
			return alias_specialization(static_cast<const AliasTemplateEntity*>(templ->entity), arguments, location);
		ClassEntity* cls = static_cast<ClassEntity*>(templ->entity);
		default_template_arguments(cls->templ->head, arguments, location);
		if (failed())
			return nullptr;
		ClassEntity* specialization = class_specialization(cls, arguments, location, soft_substitution_ == 0);
		if (specialization == nullptr)
			substitution_error(location, "template arguments that do not match their template's parameters");
		return specialization != nullptr ? specialization->type : nullptr;
	}
	ClassEntity* stand_in = stand_in_template(static_cast<const TemplateParameterEntity*>(templ->entity));
	return class_specialization(stand_in, arguments, location)->type;
}

Binding* Analyzer::specialization_binding(const Type* type, const Entity* templ)
{
	// The binding a template-id's lookup returns for its specialization's
	// type: a class specialization's own; for an alias template's, the
	// hidden alias its specializations share per type.
	const Type* bare = type->unqualified;
	if (type->cv == 0 && bare->kind == TypeKind::Class)
	{
		ClassTemplateFacts* facts = static_cast<const ClassEntity*>(bare->entity)->templ;
		if (facts != nullptr && facts->role == TemplateRole::Specialization && facts->binding != nullptr)
			return facts->binding;
	}
	const Type* key_arguments[1] = { type };
	const SpecializationKey probe = SpecializationKey{ nullptr, key_arguments, 1 };
	if (Binding** found = alias_specializations_.find(probe))
		return *found;
	Entity* entity = new_entity<Entity>(EntityKind::Typedef, templ->name, templ->owner, type);
	Binding* binding = new_binding(BindingKind::TypeAlias, templ->name, entity, type);
	binding->hidden = true;
	binding->declared_in = templ->owner;
	SpecializationKey key = probe;
	key.arguments = arena_.copy_array(key_arguments, 1);
	alias_specializations_.insert(key, binding);
	return binding;
}

const Type* Analyzer::substitute_template_name(const Type* type, const Substitution& substitution)
{
	// A template template-parameter's type is its argument; a dependent
	// member template-name, the member template of its name in the
	// substituted qualifier once that is no longer dependent.
	const TemplateParameterEntity* parameter = static_cast<const TemplateParameterEntity*>(type->entity);
	if (parameter->qualifier != nullptr)
	{
		const Type* qualifier = substitute(parameter->qualifier, substitution);
		if (qualifier == parameter->qualifier)
			return type;
		if (qualifier == nullptr)
			return nullptr;
		if (dependent_type(qualifier))
			return dependent_template_name(qualifier, parameter->name);
		// A substitution failure where the caller allows one (see
		// substitute_member).
		const ClassEntity* cls = ClassOf(qualifier);
		const LookupResult found = cls != nullptr && require_complete(cls)
			? names_.qualified(cls->scope, parameter->name, LookupFilter::Ordinary)
			: LookupResult();
		const Type* templ = found.found() ? template_name_type(found.binding, true) : nullptr;
		if (templ == nullptr && soft_substitution_ == 0)
			error(0, "a dependent template-name names no template in its instantiated qualifier");
		return templ;
	}
	if (parameter->depth != substitution.depth || parameter->index >= substitution.count ||
		substitution.arguments[parameter->index] == nullptr)
		return type;
	const Type* argument = substitution.arguments[parameter->index];
	if (parameter->pack && argument->kind == TypeKind::Pack && substitution.elements != nullptr &&
		substitution.elements[parameter->index] >= 0)
		argument = argument->params[substitution.elements[parameter->index]];
	return argument;
}

const Type* Analyzer::substitute_stand_in(const Type* type, const Substitution& substitution)
{
	// TT<A...>: the template its name stands for applied to the substituted
	// arguments (an expansion among them becoming its elements).
	const ClassTemplateFacts* facts = static_cast<const ClassEntity*>(type->unqualified->entity)->templ;
	const Type* templ = substitute_template_name(facts->pattern->templ->template_name->type, substitution);
	TypeList arguments;
	substitute_list(facts->arguments, facts->argument_count, substitution, arguments);
	if (templ == nullptr || AnyNull(arguments))
		return nullptr;
	const Type* result = specialize_template(templ, arguments, 0);
	return result != nullptr ? types_.qualified(result, type->cv) : nullptr;
}

bool Analyzer::deduce_stand_in(const ClassTemplateFacts* pattern, const Type* a, uint32_t depth, TypeList& arguments)
{
	// 14.8.2.5p8-p9: TT<T...> against a class template specialization: the
	// template-name deduces from the specialization's template, and the
	// arguments as written against its arguments as a template-id would
	// give them (a pack expansion ending them deduces from the rest).
	const ClassTemplateFacts* actual = static_cast<const ClassEntity*>(a->entity)->templ;
	if (actual == nullptr || actual->role != TemplateRole::Specialization)
		return false;
	const Type* templ = IsStandIn(actual->pattern) ? actual->pattern->templ->template_name->type
												   : types_.named(TypeKind::Template, actual->pattern);
	if (!deduce_type(pattern->pattern->templ->template_name->type, templ, depth, arguments, false))
		return false;
	TypeList given;
	flattened_arguments(actual->arguments, actual->argument_count, given);
	// 14.8.2.5p17 as GCC applies it: an integral value the specialization
	// holds for a non-type parameter of the template template-parameter's
	// own list is of that parameter's type (a list that matched only as
	// P0522 matches it, see template_parameters_match, does not deduce).
	const TemplateHead* own = pattern->pattern->templ->template_name->template_head;
	for (uint32_t i = 0; own != nullptr && i < own->count && i < given.size() && !own->parameters[i]->pack; ++i)
	{
		const TemplateParameterEntity* parameter = own->parameters[i];
		if (parameter->non_type && given[i]->kind == TypeKind::Value && !AddressArgument(given[i]) &&
			!dependent_type(parameter->value_type) &&
			given[i]->element->canonical != parameter->value_type->canonical->unqualified)
			return false;
	}
	const Type* p = types_.pack(pattern->arguments, pattern->argument_count);
	return deduce_pack(p, types_.pack(given.data(), static_cast<uint32_t>(given.size())), depth, arguments);
}

const Type* Analyzer::synthesized_argument(const TemplateParameterEntity* parameter, uint32_t index)
{
	// Partial ordering's unique argument for a template parameter of its
	// kind (14.5.5.2, 14.8.2.4p3): a type, a value or a template.
	if (parameter->template_template)
		return synthesized_template(index);
	if (!parameter->non_type)
		return synthesized_type(index);
	while (synthesized_values_.size() <= index)
	{
		TemplateParameterEntity* entity = new_entity<TemplateParameterEntity>(
			EntityKind::TemplateParameter, nullptr, nullptr, nullptr);
		entity->non_type = true;
		entity->depth = 0x7fff;
		entity->index = static_cast<uint16_t>(synthesized_values_.size());
		entity->canonical = entity;
		entity->type = types_.named(TypeKind::TemplateParameter, entity);
		synthesized_values_.push_back(entity->type);
	}
	return synthesized_values_[index];
}

const Type* Analyzer::synthesized_template(uint32_t index)
{
	// A unique template standing for a template template-parameter in
	// partial ordering (14.5.5.2, 14.8.2.4p3).
	while (synthesized_templates_.size() <= index)
	{
		TemplateParameterEntity* entity = new_entity<TemplateParameterEntity>(
			EntityKind::TemplateParameter, nullptr, nullptr, nullptr);
		entity->template_template = true;
		entity->depth = 0x7fff;
		entity->index = static_cast<uint16_t>(synthesized_templates_.size());
		entity->canonical = entity;
		entity->type = types_.named(TypeKind::TemplateTemplateParameter, entity);
		synthesized_templates_.push_back(entity->type);
	}
	return synthesized_templates_[index];
}

// ------------------------------------- member templates defined out of class

uint32_t Analyzer::EnclosingTemplateLists(const Scope* qualifier)
{
	// The template parameter lists a definition of a member of `qualifier`
	// (a class, or a template parameter scope bridging to one) outside its
	// class gives for its enclosing classes (14.5.2p1, 14.5.5.3p1): one
	// per class template and partial specialization among them.
	uint32_t lists = 0;
	for (const Scope* current = qualifier;
		current != nullptr && (current->kind == ScopeKind::Class || current->kind == ScopeKind::TemplateParameters);
		current = current->parent)
	{
		const ClassEntity* cls = current->kind == ScopeKind::Class ? static_cast<const ClassEntity*>(current->entity)
																   : nullptr;
		if (cls != nullptr && cls->templ != nullptr &&
			(cls->templ->role == TemplateRole::Template || cls->templ->role == TemplateRole::Partial))
			++lists;
	}
	return lists;
}

bool Analyzer::MemberTemplateLists(const Scope* qualifier, const Scope* scope)
{
	// Whether a declaration in `scope` of a member of `qualifier` has more
	// template parameter lists than its enclosing class templates: it
	// declares a member template (14.5.2p1).
	uint32_t lists = 0;
	for (const Scope* current = scope; current != nullptr && current->kind == ScopeKind::TemplateParameters;
		current = current->parent)
		++lists;
	return lists > EnclosingTemplateLists(qualifier);
}

Scope* Analyzer::member_template_frame(const FunctionEntity* function, const FunctionEntity* definition)
{
	// A specialization of an instance's member template whose definition is
	// its pattern's, outside the class template (14.5.2p1).
	ClassEntity* cls = static_cast<ClassEntity*>(function->owner->entity);
	return outside_frames(definition->template_parameters, ArgumentsOf(function), HeadOf(definition)->count, cls,
		cls->scope, definition->scope->parent);
}

Scope* Analyzer::outside_frames(
	Scope* own, const Type* const* arguments, uint32_t count, const ClassEntity* cls, Scope* scope, const Scope* bridge)
{
	// The frames of a definition outside its class templates (their
	// template parameter lists, one per enclosing class template or partial
	// specialization, then a member template's own, 14.5.2p1, 14.5.1p3) for
	// a member of instance class `cls`: each enclosing list bound to the
	// arguments of the instance's enclosing specialization it stands for
	// (the innermost list to the innermost specialization), a member
	// template's own list to `arguments` (when given); stacked over
	// `scope`, each binding only the names the definition's bridge kept (a
	// member hides a class template's parameter, 14.6.1p7).
	SmallList<Scope*, 4> headers;
	for (Scope* header = arguments != nullptr ? own->parent : own;
		header != nullptr && header->kind == ScopeKind::TemplateParameters; header = header->parent)
		headers.push_back(header);
	EnclosingSpecializations enclosing;
	enclosing_specializations(cls, enclosing);
	Scope* frame = scope;
	const size_t lists = headers.size() < enclosing.size() ? headers.size() : enclosing.size();
	for (size_t k = lists; k-- > 0;)
	{
		uint32_t bound = 0;
		const Type* const* given = enclosing_arguments(enclosing[k], bound);
		frame = instantiation_frame(headers[k], given, bound, frame, bridge);
		frame->bridge = true;
	}
	if (arguments != nullptr)
	{
		frame = instantiation_frame(own, arguments, count, frame, bridge);
		frame->bridge = true;
	}
	return frame;
}

const Type* const* Analyzer::enclosing_arguments(const ClassTemplateFacts* facts, uint32_t& count) const
{
	// The arguments an enclosing specialization's template parameter list
	// stands for: a partial specialization's list (a member's definition
	// outside it repeats it, 14.5.5.3p1), the arguments deduced when the
	// specialization matched it (14.5.5.1p2); the template's own, the
	// specialization's arguments.
	if (facts->partial != nullptr && facts->partial->templ->role == TemplateRole::Partial)
	{
		const uint32_t found = partial_deduced_index_.find(facts);
		if (found != PointerMap::kMissing)
		{
			count = facts->partial->templ->head->count;
			return partial_deduced_[found];
		}
	}
	count = facts->argument_count;
	return facts->arguments;
}

void Analyzer::enclosing_specializations(const ClassEntity* cls, EnclosingSpecializations& out)
{
	// The class template specializations an instance class is (or is a
	// member class of), innermost first, through member classes.
	for (const ClassEntity* c = cls; c != nullptr && c->templ != nullptr;)
	{
		const ClassTemplateFacts* facts = c->templ;
		if (facts->role == TemplateRole::Specialization)
		{
			out.push_back(facts);
			const Scope* owner = facts->pattern->owner;
			c = owner != nullptr && owner->kind == ScopeKind::Class ? static_cast<const ClassEntity*>(owner->entity)
																	: nullptr;
		}
		else if (facts->role == TemplateRole::MemberInstance)
			c = facts->enclosing;
		else
			break;
	}
}

ClassEntity* Analyzer::pattern_member_template(const ClassEntity* templ)
{
	// For an instance's member class template, which its copy of the class
	// template's member-specification declared: the class template's
	// member template of its name (14.5.2p1, 14.5.5.3p2), unless the
	// instance explicitly specialized it (14.7.3p16).
	const Scope* owner = templ->owner;
	if (owner == nullptr || !instance_class(owner) ||
		(templ->templ != nullptr && templ->templ->explicit_specialization))
		return nullptr;
	const ClassTemplateFacts* facts = static_cast<const ClassEntity*>(owner->entity)->templ;
	const ClassEntity* pattern_class = facts->partial != nullptr ? facts->partial : facts->pattern;
	if (pattern_class == nullptr || pattern_class->scope == nullptr)
		return nullptr;
	const LookupResult found = names_.own(pattern_class->scope, templ->name, LookupFilter::Elaborated);
	ClassEntity* pattern = found.found() && found.binding->kind == BindingKind::Type
		? class_template_of(found.binding->entity)
		: nullptr;
	return pattern != nullptr && pattern != templ && pattern->templ->role == TemplateRole::Template ? pattern : nullptr;
}

ClassEntity* Analyzer::outside_member_template(const ClassEntity* templ)
{
	// That member template when it is defined outside the class template:
	// its specializations instantiate that definition. A member template
	// of an instance's member template stands for its enclosing copy's
	// member template, which stands for the class template's member
	// template's (original_member_template), the one defined outside
	// (`template<class A> template<class B> template<class C> struct
	// X<A>::Y<B>::Z { ... }`).
	const ClassEntity* pattern = original_member_template(templ);
	for (uint32_t level = 0; pattern != nullptr && pattern->templ->definition == nullptr && level < 64; ++level)
	{
		const ClassEntity* outer = original_member_template(pattern);
		pattern = outer != pattern ? outer : nullptr;
	}
	return pattern != nullptr && pattern->templ->definition != nullptr && pattern->scope != nullptr
		? const_cast<ClassEntity*>(pattern)
		: nullptr;
}

void Analyzer::enclosing_substitution(
	const Type* const* pattern, uint32_t count, const ClassEntity* instance, TypeList& out)
{
	// A class template's member template's partial specialization's
	// argument list with the enclosing class templates' parameters replaced
	// by an instance's arguments (the outermost at depth 0).
	EnclosingSpecializations enclosing;
	enclosing_specializations(instance, enclosing);
	out.clear();
	for (uint32_t i = 0; i < count; ++i)
		out.push_back(pattern[i]);
	for (size_t level = 0; level < enclosing.size(); ++level)
	{
		uint32_t bound = 0;
		const Type* const* given = enclosing_arguments(enclosing[enclosing.size() - 1 - level], bound);
		const Substitution substitution = Substitution{ static_cast<uint32_t>(level), given, bound, nullptr };
		for (const Type*& argument : out)
			argument = substitute(argument, substitution);
	}
}

// ---------------------------------------------------- explicit(condition)

bool Analyzer::explicit_value(const syntax::Expr& condition, Scope* scope, bool converted)
{
	// explicit(constant-expression) (C++20's, accepted by the course's
	// grammar): a contextually converted constant expression of type bool;
	// a noexcept-specification's constant expression contextually converted
	// to bool (15.4p1).
	Constant value;
	if (semantic())
	{
		BodyContext* saved = body_;
		BodyContext context = BodyContext();
		if (body_ == nullptr)
			body_ = &context;
		const ExprFacts* facts = typed_value(condition, scope);
		// `converted` (an explicit-specifier's, C++20 [dcl.fct.spec]p3 and
		// [expr.const]p10 as 5.19p3 has it): the conversion to bool takes an
		// integral or unscoped enumeration value bool represents (no
		// narrowing, 8.5.4p7), or a class object's conversion function's;
		// no floating, pointer or member pointer value converts.
		const Type* source = converted && facts != nullptr && facts->type != nullptr ? facts->type->unqualified
																					 : nullptr;
		IntegralKind kind;
		if (source != nullptr && ClassOf(source) == nullptr && !failed())
		{
			if (!IntegralKindOf(source, kind))
				error(condition.location,
					"an explicit-specifier's condition of a type no converted constant expression "
					"of type bool converts (5.19p3)");
			Constant before;
			IntegralKindOf(types_.fundamental(FundamentalType::Bool), kind);
			if (constant_value(condition, scope, before, true) && !Representable(before, kind))
				error(condition.location, "an explicit-specifier's condition narrowed by conversion to bool (8.5.4p7)");
		}
		convert_to_bool(condition);
		body_ = saved;
		if (failed())
			return false;
		converted_constant(condition, value, true);
	}
	else
		constant_value(condition, scope, value, true);
	return value.bits != 0;
}

bool Analyzer::specialized_explicit(const PendingExplicit& pending, const TypeList& arguments)
{
	// A function template's explicit(condition) for a specialization: the
	// condition copied into the frame of its template parameters.
	Scope* frame = instantiation_frame(
		pending.scope, arguments.data(), static_cast<uint32_t>(arguments.size()), pending.scope->parent);
	FrameExpander expander(*this, frame);
	syntax::SyntaxCloner cloner(arena_, true, expander.get());
	// Part of the specialization's substitution: an invalid condition (an
	// inaccessible or missing conversion) is its failure.
	const syntax::Expr* copy = cloner.expr(pending.condition);
	InstantiationContext context(*this, enter_instantiation(instantiation_depth_ + 1, copy->location), true);
	return explicit_value(*copy, frame, true);
}

// ------------------------------------------------------ alias templates

void Analyzer::declare_alias_template(const syntax::AliasDecl& decl, Scope* scope)
{
	// 14.5.7p1: the templated alias-declaration's type-id, analyzed in the
	// template's parameter scope, is the alias template's type; its name is
	// declared in the enclosing scope, where no other declaration may have
	// it (14p5 for templates, 3.3.1p4).
	Specifiers specifiers;
	this->specifiers(decl.type->specifiers, scope, specifiers, false);
	if (specifiers.type == nullptr || specifiers.type == auto_placeholder_)
		error(decl.location, "alias-declaration without a type");
	if (specifiers.declared != nullptr && specifiers.defines)
		error(decl.location, "a class or enumeration defined in an alias template");
	if (!specifiers.mode_specifiers.empty())
		filter_mode_alias(specifiers);
	const DeclaratorInfo info = declarator(decl.type->declarator, specifiers.type, scope, decl.location,
		specifiers.mode_specifiers.empty() ? nullptr : &specifiers);
	if (info.virt_suffix != nullptr || specifiers.is_virtual)
		error(decl.location, "a virt-specifier or virtual in an alias-declaration");
	if (DeclaratorSpecifiesExceptions(decl.type->declarator))
		error(decl.location, "an exception-specification in an alias-declaration");
	Scope* where = declaration_scope(scope);
	check_template_parameter_name(scope, decl.name, decl.location);
	TemplateHead* head = template_head(*template_list_, template_scope_, template_entities_);
	head->shape = template_shape(head->parameters, head->count);
	const NameEntry* entry = names_.find(where, decl.name);
	// An equivalent redeclaration (in a namespace, 7.1.3p3's rule for
	// typedef-names: the same parameters and aliased type) names the same
	// alias template.
	const Binding* earlier = entry != nullptr ? entry->type : nullptr;
	if (earlier != nullptr && earlier->entity != nullptr && earlier->entity->kind == EntityKind::AliasTemplate &&
		where->kind != ScopeKind::Class && (entry->value == nullptr || entry->value->invisible))
	{
		const AliasTemplateEntity* alias = static_cast<const AliasTemplateEntity*>(earlier->entity);
		if (alias->head->shape != head->shape || alias->type->canonical != info.type->canonical)
			error(decl.location, "an alias template redeclared with a different template or type");
		decl.declared = earlier;
		return;
	}
	if (entry != nullptr && (entry->type != nullptr || (entry->value != nullptr && !entry->value->invisible)))
		error(decl.location, "an alias template redeclares a name");
	AliasTemplateEntity* entity = new_entity<AliasTemplateEntity>(
		EntityKind::AliasTemplate, decl.name, where, info.type);
	entity->head = head;
	// The parameters its type does not name (none known when it holds an
	// expression, whose names the walk does not see).
	uint64_t named = 0;
	bool opaque = false;
	if (head->count != 0)
		named_parameters(info.type, head->parameters[0]->depth, named, opaque, 0);
	for (uint32_t i = 0; i < head->count && i < 64 && !opaque; ++i)
		entity->discarded |= (named >> i & 1) == 0 ? uint64_t(1) << i : 0;
	check_template_defaults(entity->head, decl.location);
	Binding* binding = new_binding(BindingKind::TypeAlias, decl.name, entity, info.type);
	binding->hidden = true;
	declare_in(where, binding);
	decl.declared = binding;
	if (hosted_ && where == graph_.global && decl.name == type_pack_element_key_ && head->count == 2 &&
		head->parameters[0]->non_type && head->parameters[1]->pack && type_pack_element_ == nullptr)
		type_pack_element_ = entity;
}

const Type* Analyzer::alias_specialization(const AliasTemplateEntity* alias, TypeList& arguments, uint32_t location)
{
	// 14.5.7p2: the aliased type with the template arguments substituted for
	// the alias template's parameters, defaults supplied (14.1p9); once per
	// (alias template, canonical arguments).
	const TemplateHead* head = alias->head;
	default_template_arguments(head, arguments, location);
	if (failed())
		return nullptr;
	TypeList canonical;
	if (!match_template_arguments(head, arguments.data(), static_cast<uint32_t>(arguments.size()), canonical,
			soft_substitution_ == 0, location))
	{
		substitution_error(location, "template arguments that do not match the alias template's parameters");
		return nullptr;
	}
	if (canonical.size() != head->count)
	{
		substitution_error(location, "a pack expansion for an alias template's parameter that is no pack");
		return nullptr;
	}
	const SpecializationKey probe = SpecializationKey{ alias, canonical.data(),
		static_cast<uint32_t>(canonical.size()) };
	if (Binding** found = alias_specializations_.find(probe))
		return (*found)->type;
	// Clang's and GCC's builtin __type_pack_element<I, T...> (the hosted
	// builtin declarations' alias template): the I-th type of the pack,
	// chosen directly once I and the pack are known (its declared recursive
	// definition, one instantiation per element, is never used); a
	// dependent template-id is the alias's stand-in specialization, formed
	// again when substitution knows them.
	const bool pack_element = alias == type_pack_element_;
	if (pack_element && !dependent_type(canonical[0]) && !dependent_type(canonical[1]))
	{
		const Type* pack = canonical[1];
		if (canonical[0]->kind != TypeKind::Value || pack->kind != TypeKind::Pack ||
			canonical[0]->bound >= pack->param_count)
		{
			substitution_error(location, "a __type_pack_element index outside its pack");
			return nullptr;
		}
		const Type* element = pack->params[canonical[0]->bound];
		SpecializationKey key = probe;
		key.arguments = arena_.copy_array(canonical.data(), canonical.size());
		alias_specializations_.insert(key, specialization_binding(element, alias));
		return element;
	}
	// Its own parameters as the arguments (another template's template-id
	// at their positions, as in a chain of alias templates each naming the
	// previous) substitute nothing: the aliased type as it is, without
	// walking it.
	const uint32_t enclosing_location = substitution_location_;
	if (location != 0)
		substitution_location_ = location;
	const Type* type = own_arguments(head, canonical.data(), static_cast<uint32_t>(canonical.size()))
		? alias->type
		: substitute(alias->type, substitution_of(head, canonical.data()));
	substitution_location_ = enclosing_location;
	if (type == nullptr)
		return nullptr;
	// CWG 1558: a dependent argument the aliased type drops is still
	// substituted where the template-id is: the stand-in's specialization
	// keeps it (the aliased type deduced and named in its place).
	bool dropped = pack_element;
	for (uint32_t i = 0; i < canonical.size() && i < 64 && !dropped; ++i)
		dropped = (alias->discarded >> i & 1) != 0 && dependent_type(canonical[i]);
	if (dropped)
	{
		ClassEntity* specialization = class_specialization(alias_stand_in(alias), canonical, location);
		if (alias_index_.find(specialization) == PointerMap::kMissing)
		{
			alias_index_.set(specialization, static_cast<uint32_t>(alias_facts_.size()));
			alias_facts_.push_back(type);
		}
		type = specialization->type;
	}
	Binding* binding = specialization_binding(type, alias);
	SpecializationKey key = probe;
	key.arguments = arena_.copy_array(canonical.data(), canonical.size());
	alias_specializations_.insert(key, binding);
	return type;
}

} // namespace sema
