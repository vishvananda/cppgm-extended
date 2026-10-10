#include "sema/analyzer.h"

#include <cstdio>
#include <cstring>
#include <string>

#include "lex/identifier_table.h"
#include "sema/aggregates.h"
#include "sema/integral_arithmetic.h"
#include "syntax/ast_render.h"

namespace sema {

namespace {

bool IsUnion(TokenKind key)
{
	return key == TokenKind::KW_UNION;
}

} // namespace

ClassEntity* Analyzer::new_class(TokenKind key, const Identifier* name, Scope* owner)
{
	ClassEntity* entity = new_entity<ClassEntity>(EntityKind::Class, name, owner, nullptr);
	entity->key = key;
	entity->label = name != nullptr ? name->spelling : nullptr;
	entity->type = types_.named(TypeKind::Class, entity);
	if (body_ != nullptr && (owner->kind == ScopeKind::Block || owner->kind == ScopeKind::Function))
	{
		// A local class numbered among its function's classes of its name;
		// one outside a template specialization's body listed (whose names
		// Clang numbers as mangled, see ProgramLowering::name_local_classes).
		if (name != nullptr)
			entity->local_discriminator = local_number(name);
		if (body_->function == nullptr || !body_->function->instantiated)
			graph_.local_classes.push_back(entity);
	}
	return entity;
}

size_t LocalNumberHash::operator()(const LocalNumberKey& key) const
{
	return (reinterpret_cast<uintptr_t>(key.function) * 0x9E3779B97F4A7C15ull) ^ reinterpret_cast<uintptr_t>(key.key);
}

size_t ClosureContextHash::operator()(const ClosureContextKey& key) const
{
	return (reinterpret_cast<uintptr_t>(key.context) * 0x9E3779B97F4A7C15ull) ^
		(reinterpret_cast<uintptr_t>(key.signature) + key.default_argument);
}

uint16_t Analyzer::local_number(const void* key)
{
	// The count of the analyzed body's earlier local entities of `key`
	// (saturating), counting this one.
	const LocalNumberKey probe = LocalNumberKey{ body_->function, key };
	uint32_t* count = local_numbers_.find(probe);
	if (count == nullptr)
	{
		local_numbers_.insert(probe, 1);
		return 0;
	}
	const uint32_t earlier = (*count)++;
	return static_cast<uint16_t>(earlier < 0xffff ? earlier : 0xffff);
}

void Analyzer::number_unnamed_local_type(const Entity* entity)
{
	// An unnamed type defined in a function body: its number among the
	// function's (see SemanticGraph::unnamed_local_ordinal).
	static const char unnamed_key = 0;
	if (body_ != nullptr && (entity->owner->kind == ScopeKind::Block || entity->owner->kind == ScopeKind::Function))
		graph_.unnamed_local_ordinals.set(entity, local_number(&unnamed_key));
}

const char* Analyzer::anonymous_class_label(const syntax::ClassSpecifier& specifier, bool after_semicolon)
{
	// The course dump names an unnamed class by token ordinals: its
	// class-key's and the one after its closing brace, or after the `;`
	// that follows for a declarator-less declaration starting with the
	// class-key (where the course names it once the `;` is read).
	char buffer[80];
	const uint64_t end = specifier.end_token + (after_semicolon ? 1 : 0);
	const int length = std::snprintf(buffer, sizeof buffer, "__anonymous_union_type__%llu_%llu",
		static_cast<unsigned long long>(specifier.first_token), static_cast<unsigned long long>(end));
	return copy_label(buffer, static_cast<size_t>(length));
}

Entity* Analyzer::class_specifier(const syntax::ClassSpecifier& specifier, Scope* scope, Specifiers& out)
{
	const syntax::Name* name = specifier.name;
	Scope* where = declaration_scope(scope);
	Scope* print = scope;
	Scope* parent = scope;
	ClassEntity* entity = nullptr;
	if (name == nullptr)
	{
		// Named by the declaration's first declarator or typedef-name, or
		// an anonymous union (see name_unnamed_type, anonymous_union); the
		// line waits hidden in its place.
		entity = new_class(specifier.key, nullptr, where);
		entity->anonymous = true;
		number_unnamed_local_type(entity);
		Binding* binding = new_binding(BindingKind::Type, nullptr, entity, entity->type);
		binding->hidden = true;
		binding->key = specifier.key;
		print_in(print, binding);
		out.declared_binding = binding;
		class_abi_tags(entity, specifier.attributes, false, false, specifier.location);
		class_body(entity, specifier, parent, print);
		return entity;
	}
	const syntax::NameComponent& last = name->last();
	const bool under_head = template_scope_ != nullptr && scope == template_scope_;
	if (last.has_arguments && explicit_specialization_ && !under_head)
		return class_explicit_specialization(specifier, scope, out);
	if (last.has_arguments && under_head)
		return class_partial_specialization(specifier, scope, out);
	if (last.has_arguments)
		error(name->location, "a class template-id named outside a class template specialization's declaration");
	if (last.kind != syntax::NameComponentKind::Identifier)
		error(name->location, "invalid class name");
	// A named member class of an instance is defined when required.
	if (!name->qualified() && instance_class(where) &&
		declare_member_instance(&specifier, *name, specifier.key, where, out))
		return out.declared;
	// The template parameter scope a templated definition's names use.
	Scope* parameters = nullptr;
	bool existed = false;
	if (name->qualified())
	{
		// 9p11: a qualified class-head defines a class declared earlier in
		// the named class or namespace, from a scope enclosing it. An
		// explicit specialization's head (a member class of a class template
		// specialization, 14.7.3p1) is not access-checked (P0692's
		// [temp.spec]p6, GCC and Clang in C++11 mode; see specialized_class).
		Scope* saved_deferral = access_deferral_;
		const size_t pending = pending_access_.size();
		if (explicit_specialization_)
			access_deferral_ = semantic() ? scope : nullptr;
		const NamePrefix prefix = name_prefix(*name, scope, true);
		if (explicit_specialization_)
			pending_access_.resize(pending);
		access_deferral_ = saved_deferral;
		if (prefix.scope == nullptr || prefix.scope->kind == ScopeKind::Enum)
			error(name->location, "qualified class name must name a class or namespace member");
		if (!encloses(where, prefix.scope))
			error(name->location, "class definition in a scope that does not enclose it");
		const LookupResult previous = names_.qualified(prefix.scope, last.identifier, LookupFilter::Elaborated);
		if (!previous.found() || previous.binding->entity == nullptr ||
			previous.binding->entity->kind != EntityKind::Class)
			error(name->location, "qualified class definition does not match a declaration");
		entity = static_cast<ClassEntity*>(previous.binding->entity);
		where = print = parent = prefix.scope;
		if (explicit_specialization_)
			explicit_member_class(entity, prefix.scope, scope, name->location);
		// A member class of a class template defined out of it (14.5.1.2):
		// its template parameters stay visible in it.
		if (scope->kind == ScopeKind::TemplateParameters && entity->templ != nullptr &&
			entity->templ->role == TemplateRole::Pattern)
		{
			parent = qualified_lookup_scope(prefix.scope, scope);
			parameters = scope;
		}
		// Without a template-head (an explicit specialization's member
		// class): no template parameter of the enclosing definitions is in
		// scope (14.7.3p5).
		else if (scope->kind != ScopeKind::TemplateParameters)
			parent = qualified_lookup_scope(prefix.scope, scope);
	}
	else
	{
		// 14.6.1p6: a class (a class template's own name included) does not
		// redeclare a template parameter.
		check_template_parameter_name(scope, last.identifier, name->location);
		const NameEntry* entry = names_.find(where, last.identifier);
		if (entry != nullptr && entry->tag != nullptr)
		{
			if (entry->tag->entity->kind != EntityKind::Class)
				error(name->location, "class name redeclared as a different kind of type");
			entity = static_cast<ClassEntity*>(entry->tag->entity);
			existed = true;
		}
		else
		{
			check_type_name_conflict(where, last.identifier, nullptr, name->location);
			entity = new_class(specifier.key, last.identifier, where);
		}
	}
	if (entity->scope != nullptr)
		error(name->location, "class redefinition");
	if (IsUnion(specifier.key) != IsUnion(entity->key))
		error(name->location, "class-key does not agree with an earlier declaration");
	const bool pattern_member = entity->templ != nullptr && entity->templ->role == TemplateRole::Pattern;
	if (template_scope_ != nullptr && scope == template_scope_ && !pattern_member)
	{
		// The templated entity of the template declaration: a class
		// template (defined through a qualified name, it keeps its
		// parameters visible as a member class does), whose name no
		// variable, function or enumerator of its scope shares (14p5).
		const NameEntry* entry = names_.find(where, last.identifier);
		if (entry != nullptr && entry->value != nullptr)
			error(name->location, "a class template shares its name with a declaration in its scope");
		declare_class_template(entity, existed || name->qualified(), name->location);
		parameters = scope;
		if (name->qualified())
			parent = qualified_lookup_scope(where, scope);
	}
	else if (entity->templ != nullptr && entity->templ->role == TemplateRole::Template)
		error(name->location, "a class template redeclared as a class that is not a template");
	else if (!name->qualified() && template_depth_ != 0 && templated_class(where))
		class_template_facts(entity, TemplateRole::Pattern);
	if (entity->templ != nullptr &&
		(entity->templ->role == TemplateRole::Template || entity->templ->role == TemplateRole::Pattern))
		define_pattern(entity, specifier, parameters);
	Binding* binding = new_binding(BindingKind::Type, last.identifier, entity, entity->type);
	binding->key = specifier.key;
	print_in(print, binding);
	declare_in(where, binding);
	out.declared_binding = binding;
	class_abi_tags(entity, specifier.attributes, false, existed || name->qualified(), name->location);
	class_body(entity, specifier, parent, print);
	// A templated class's definition context is where it ends: its own name
	// and what its member-specification declared in enclosing namespaces
	// are in it (14.6.4.2).
	if (entity->templ != nullptr && entity->templ->definition == &specifier)
		entity->templ->definition_ordinal = definition_point();
	return entity;
}

bool Analyzer::alias_template_id(const syntax::Name& name, Scope* scope)
{
	// Whether a template-id's template-name is an alias template's.
	if (!name.last().has_arguments)
		return false;
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
	const NamePrefix saved = last_prefix_;
	const LookupResult found = name.qualified()
		? lookup_in(name_prefix(bare, scope), last.identifier, LookupFilter::Ordinary)
		: names_.unqualified(scope, last.identifier, LookupFilter::Ordinary);
	last_prefix_ = saved;
	return found.found() && found.binding->kind == BindingKind::TypeAlias && found.binding->entity != nullptr &&
		found.binding->entity->kind == EntityKind::AliasTemplate;
}

void Analyzer::explicit_member_class(ClassEntity* entity, const Scope* owner, const Scope* scope, uint32_t location)
{
	// 14.7.3p16-p17: `template<>` before the qualified class-head of a
	// member class of a class template specialization explicitly
	// specializes it for that specialization: its members are its own
	// (p5), never instantiated from the class template's. Under a further
	// template-head, a member class template of the specialization is
	// explicitly specialized as a class template: this definition, not the
	// class template's member template's (nor that one's partial
	// specializations, 14.5.5.3p2), is what its specializations
	// instantiate. Either precedes the uses that would instantiate it (p6).
	explicit_specialization_ = false;
	ClassTemplateFacts* facts = entity->templ;
	if (!instance_class(owner) || facts == nullptr)
		error(location, "an explicit specialization names no member class of a class template specialization");
	const bool member_template = template_scope_ != nullptr && scope == template_scope_;
	if (!member_template && facts->role == TemplateRole::MemberInstance)
	{
		if (facts->definition_state != FactState::NotStarted || entity->scope != nullptr)
			error(location, "an explicit specialization after the member class's instantiation");
		facts->explicit_specialization = true;
		facts->definition_state = FactState::Done;
		return;
	}
	if (!member_template || facts->role != TemplateRole::Template)
		error(location, "an explicit specialization names no member class of a class template specialization");
	facts->explicit_specialization = true;
	facts->partials = nullptr;
	facts->partial_positions = 0;
	entity->scope = nullptr;
}

uint64_t Analyzer::attribute_alignment(
	const syntax::Attribute* list, Scope* scope, bool& packed, bool& no_inline, bool on_class)
{
	// 7.6.2p2-p4: alignas(constant-expression) requests that alignment (0
	// requests none), alignas(type-id) the type's; the strictest one wins.
	// One after a decl-specifier or an array declarator's `]` appertains
	// to the type (7.1p1, 8.3.4p1), which it may not (7.6.2p1, 7.6.1p5).
	// Those after an elaborated-type-specifier's class-key are the class's
	// (`on_class`), not the declaration's.
	uint64_t alignment = 0;
	for (const syntax::Attribute* attribute = list; attribute != nullptr; attribute = attribute->next)
	{
		if (attribute->on_class != on_class)
			continue;
		switch (attribute->kind)
		{
		case syntax::AttributeKind::VectorSize:
		case syntax::AttributeKind::ExtVectorType:
		case syntax::AttributeKind::ExcludeFromExplicitInstantiation:
		case syntax::AttributeKind::AlwaysInline:
		case syntax::AttributeKind::GnuInline:
		case syntax::AttributeKind::Used:
		case syntax::AttributeKind::NoReturn:
			break;
		case syntax::AttributeKind::Alignas:
		{
			if (attribute->on_type && !attribute->gnu)
				error(attribute->location, "an alignment-specifier applied to a type");
			uint64_t value = 0;
			if (attribute->type == nullptr && attribute->expression == nullptr)
			{
				// GNU aligned without a value: the target's biggest
				// alignment (GCC's __BIGGEST_ALIGNMENT__, x86-64's 16).
				value = 16;
			}
			else if (attribute->type != nullptr)
			{
				uint64_t size = 0;
				if (!type_size(type_id(*attribute->type, scope), size, value, attribute->location))
					error(attribute->location, "alignas of an incomplete type");
			}
			else
			{
				Constant constant;
				if (semantic())
					integral_constant_operand(*attribute->expression, scope, nullptr);
				// A deferred reserved builtin call (see deferrable_statement).
				if (deferred_reserved_call())
					return alignment;
				constant_value(*attribute->expression, scope, constant, true);
				value = constant.bits;
				if ((value & (value - 1)) != 0)
					error(attribute->location, "an alignment that is not a power of two");
			}
			if (value > alignment)
				alignment = value;
			break;
		}
		case syntax::AttributeKind::NoInline:
			no_inline = true;
			break;
		case syntax::AttributeKind::Packed:
			packed = true;
			break;
		case syntax::AttributeKind::AsmLabel:
		case syntax::AttributeKind::Weak:
		case syntax::AttributeKind::Section:
		case syntax::AttributeKind::AbiTag:
		case syntax::AttributeKind::Pure:
		case syntax::AttributeKind::Const:
		case syntax::AttributeKind::UsingIfExists:
		case syntax::AttributeKind::NoUniqueAddress:
			break;
		}
	}
	return alignment;
}

void Analyzer::class_body(ClassEntity* entity, const syntax::ClassSpecifier& specifier, Scope* parent, Scope* print)
{
	Scope* scope = new_scope(ScopeKind::Class, parent, print, entity, entity->label);
	entity->scope = scope;
	entity->is_final = specifier.final;
	// 9p1: a class-head's attributes appertain to the class (7.6.3p1).
	reject_noreturn(specifier.attributes, "noreturn attribute on a class");
	bool packed = false, no_inline = false;
	// (A GNU alignment an earlier declaration of the class gave it stays,
	// the strictest winning: GCC's type attribute.)
	if (template_depth_ == 0)
	{
		entity->alignment_request = std::max(entity->alignment_request,
			static_cast<uint32_t>(attribute_alignment(specifier.attributes, parent, packed, no_inline)));
		entity->alignment_request_gnu = true;
		for (const syntax::Attribute* attribute = specifier.attributes; attribute != nullptr;
			attribute = attribute->next)
			if (attribute->kind == syntax::AttributeKind::Alignas && !attribute->gnu && !attribute->on_class)
				entity->alignment_request_gnu = false;
	}
	entity->pack = specifier.pack;
	entity->packed = packed;
	if (entity->name != nullptr)
	{
		// The injected-class-name (9p2).
		Binding* injected = new_binding(BindingKind::Type, entity->name, entity, entity->type);
		injected->hidden = true;
		injected->key = entity->key;
		declare_in(scope, injected);
	}
	base_classes(entity, specifier, parent);
	const size_t first_deferred = deferred_.size();
	const size_t first_throw = pending_throws_.size(), first_override = exception_overrides_.size();
	const TokenKind saved_access = current_access_;
	Scope* saved_member_scope = member_scope_;
	// 11p2: the class-key of the definition (not of an earlier declaration,
	// `friend class X;` before `struct X {...}`) gives the default access.
	current_access_ = specifier.key == TokenKind::KW_CLASS ? TokenKind::KW_PRIVATE : TokenKind::KW_PUBLIC;
	member_scope_ = scope;
	++class_depth_;
	const bool unnamed = entity->name == nullptr;
	unnamed_class_depth_ += unnamed ? 1 : 0;
	if (!class_instantiations_.empty() && class_instantiations_.back().copy == &specifier)
	{
		// An instance's members, the number analyzed before each kept for
		// constexpr_definition's links.
		for (uint32_t i = 0; i < specifier.members.size; ++i)
		{
			class_instantiations_.back().declared = i;
			declaration(*specifier.members[i], scope);
		}
	}
	else
		declarations(specifier.members, scope);
	--class_depth_;
	unnamed_class_depth_ -= unnamed ? 1 : 0;
	current_access_ = saved_access;
	member_scope_ = saved_member_scope;
	entity->complete = true;
	for (const DataMember* member = entity->anonymous ? entity->members : nullptr;
		member != nullptr && !entity->members_initialized; member = member->next)
		entity->members_initialized = MemberInitialized(member);
	if (template_depth_ == 0)
		virtual_base_list(entity);
	entity->dynamic = entity->polymorphic || VirtualBaseCount(entity) != 0;
	entity->empty = !entity->dynamic;
	for (const DataMember* member = entity->members; member != nullptr && entity->empty; member = member->next)
		entity->empty = (member->bit_field && member->width == 0 && member->variable->name == nullptr) ||
			(member->overlapping && ClassOf(member->variable->type) != nullptr &&
				ClassOf(member->variable->type)->empty);
	for (const BaseClass* base = entity->bases; base != nullptr && entity->empty; base = base->next)
		entity->empty = !base->is_virtual && static_cast<const ClassEntity*>(base->base)->empty;
	if (semantic() && template_depth_ == 0)
	{
		declare_destructor(entity, specifier.location);
		if (entity->inherits_constructors != nullptr)
			declare_inherited_constructors(entity);
		// The implicit copy and move members wait for a demand, but
		// implicit assignment operators that may override a base's virtual
		// operator= (10.3p2, 12.8p18, p20) are declared before the class's
		// virtual facts.
		classify_copy_members(entity);
		if (entity->polymorphic && virtual_names_.find(assign_key_) != PointerMap::kMissing)
		{
			implicit_copy_members(entity);
			FunctionEntity* const assignments[] = { entity->copy_assignment, entity->move_assignment };
			for (FunctionEntity* assignment : assignments)
			{
				if (assignment != nullptr && assignment->implicit)
					override_facts(assignment, entity, specifier.location);
			}
		}
		virtual_facts(entity, specifier.location);
		// 9.5p8: at most one variant member of a union has a
		// brace-or-equal-initializer (an anonymous struct's member's is its
		// member object's).
		if (entity->key == TokenKind::KW_UNION)
		{
			uint32_t initialized = 0;
			for (const DataMember* member = entity->members; member != nullptr; member = member->next)
				initialized += MemberInitialized(member) ? 1 : 0;
			if (initialized > 1)
				error(specifier.location, "a union has several default member initializers");
		}
	}
	else
		entity->copy_members = FactState::Done;
	// 7.6.2p5: a requested alignment weaker than the class's natural one is
	// ill-formed (the layout checks it).
	if (entity->alignment_request != 0 && template_depth_ == 0)
	{
		uint64_t size = 0, alignment = 0;
		type_size(entity->type, size, alignment, specifier.location);
	}
	if (class_depth_ == 0)
	{
		finish_class_exceptions(first_throw, first_override);
		finish_class_nesting(first_deferred);
	}
}

void Analyzer::virtual_base_list(ClassEntity* entity)
{
	// Itanium 2.4's inheritance graph order: each direct base, if virtual,
	// then its own virtual bases (already in that order), each class once
	// (listed: its position). A direct virtual base an earlier base has
	// too keeps that position and is direct (its base-specifier).
	std::vector<VirtualBase>& list = virtual_base_scratch_;
	PointerMap& listed = virtual_base_marks_;
	list.clear();
	listed.clear();
	for (BaseClass* base = entity->bases; base != nullptr; base = base->next)
	{
		const ClassEntity* cls = static_cast<const ClassEntity*>(base->base);
		if (base->is_virtual)
		{
			const uint32_t position = listed.find(cls);
			if (position == PointerMap::kMissing)
			{
				listed.set(cls, static_cast<uint32_t>(list.size()));
				list.push_back(VirtualBase{ base, true });
			}
			else
				list[position] = VirtualBase{ base, true };
		}
		for (uint32_t i = 0; i < VirtualBaseCount(cls); ++i)
		{
			const BaseClass* inherited = VirtualBases(cls)[i].base;
			if (listed.find(inherited->base) != PointerMap::kMissing)
				continue;
			listed.set(inherited->base, static_cast<uint32_t>(list.size()));
			BaseClass* made = static_cast<BaseClass*>(arena_.allocate(sizeof(BaseClass), alignof(BaseClass)));
			*made = BaseClass{ inherited->base, true, Access::Public, false, 0, nullptr };
			list.push_back(VirtualBase{ made, false });
		}
	}
	if (list.empty())
		return;
	// Recorded in the class's virtual facts (its completion fills in the
	// rest, see virtual_facts).
	VirtualBase* stored = static_cast<VirtualBase*>(
		arena_.allocate(sizeof(VirtualBase) * list.size(), alignof(VirtualBase)));
	std::copy(list.begin(), list.end(), stored);
	VirtualFacts* facts = static_cast<VirtualFacts*>(arena_.allocate(sizeof(VirtualFacts), alignof(VirtualFacts)));
	*facts = VirtualFacts{ nullptr, 0, 0, nullptr, nullptr, nullptr, false, nullptr, stored,
		static_cast<uint32_t>(list.size()), 0 };
	entity->virtuals = facts;
}

void Analyzer::base_classes(ClassEntity* entity, const syntax::ClassSpecifier& specifier, Scope* scope)
{
	// 10p2: base names are looked up ignoring non-type names and must
	// denote complete classes. The base-clause precedes the class's member
	// scope: its names are looked up where the class is defined (for a
	// qualified class-head, in the scope it names), not through earlier
	// bases. 11p6-p7: their access is checked as in the class, once its
	// whole base-specifier-list is known (11.3p2: a befriended class may
	// name the befriending class's private members there): the checks
	// failing where the names are looked up wait.
	Scope* saved_deferral = access_deferral_;
	const size_t pending = pending_access_.size();
	access_deferral_ = semantic() ? scope : nullptr;
	for (const syntax::BaseSpecifier& base : specifier.bases)
	{
		// A base-specifier pack expansion (14.5.3p4) of a template's pattern
		// is a dependent base (its instantiations' copies expand it).
		if (base.pack && template_depth_ == 0)
			error(base.location, "a pack expansion outside a template");
		// 10p1: a class-or-decltype, a decltype-specifier's type.
		const bool decltype_base = base.name->components.size == 1 && !base.name->global &&
			base.name->last().kind == syntax::NameComponentKind::Decltype;
		const LookupResult result = decltype_base ? LookupResult()
												  : lookup_name(*base.name, scope, LookupFilter::Elaborated);
		if (base.pack)
		{
			base.dependent = true;
			continue;
		}
		const Type* type = decltype_base ? decltype_base_type(*base.name->last().decltype_operand, scope)
										 : binding_type(result.binding, base.location);
		if (type == nullptr)
			error(base.location, "an invalid decltype-specifier as a base");
		type = type->unqualified;
		// A dependent base waits for the instantiations (README: resolved
		// and validated then); the pattern records that it was one.
		if (template_depth_ != 0 && dependent_type(type))
		{
			base.dependent = true;
			continue;
		}
		if (type->kind != TypeKind::Class)
			error(base.location, "base is not a class");
		ClassEntity* base_class = static_cast<ClassEntity*>(type->entity);
		if (!require_complete(base_class))
			error(base.location, "base class is incomplete");
		// 9p3: a final class is no base. A nondependent base of a template
		// is validated where the template is defined, its access included
		// (the README; 11p4 names it from the class-head).
		if (base_class->is_final)
			error(base.location, "a final class used as a base class");
		if (semantic() && template_depth_ != 0 && !decltype_base && last_prefix_.scope != nullptr &&
			last_prefix_.scope->kind == ScopeKind::Class)
			check_access(result.binding, static_cast<const ClassEntity*>(last_prefix_.scope->entity), nullptr, scope,
				base.location);
		if (IsUnion(base_class->key) || IsUnion(entity->key))
			error(base.location, "unions have no base classes");
		BaseClass* node = static_cast<BaseClass*>(arena_.allocate(sizeof(BaseClass), alignof(BaseClass)));
		// 11.2p2: without an access-specifier a class's base is private, a
		// struct's public (by the defining class-key).
		const TokenKind access = base.has_access   ? base.access
			: specifier.key == TokenKind::KW_CLASS ? TokenKind::KW_PRIVATE
												   : TokenKind::KW_PUBLIC;
		*node = BaseClass{ base_class, base.is_virtual,
			access == TokenKind::KW_PRIVATE         ? Access::Private
				: access == TokenKind::KW_PROTECTED ? Access::Protected
													: Access::Public,
			base.dependent, 0, nullptr };
		if (entity->last_base == nullptr)
			entity->bases = node;
		else
			entity->last_base->next = node;
		entity->last_base = node;
		++entity->base_count;
		entity->polymorphic = entity->polymorphic || base_class->polymorphic;
		entity->has_conversion_functions = entity->has_conversion_functions || base_class->has_conversion_functions;
	}
	distinct_direct_bases(entity, specifier.location);
	access_deferral_ = saved_deferral;
	for (size_t i = pending; i < pending_access_.size(); ++i)
	{
		const PendingAccess check = pending_access_[i];
		check_access(check.binding, check.naming, check.object, entity->scope, check.location);
	}
	pending_access_.resize(pending);
}

void Analyzer::distinct_direct_bases(const ClassEntity* entity, uint32_t location)
{
	// 10.1p3: a class is not a direct base class more than once (however
	// named: a typedef, an instantiation's substituted parameter or pack
	// element). A short list is compared pairwise, a long one through a set.
	if (entity->base_count < 2)
		return;
	if (entity->base_count <= 8)
	{
		for (const BaseClass* a = entity->bases; a != nullptr; a = a->next)
		{
			for (const BaseClass* b = a->next; b != nullptr; b = b->next)
			{
				if (a->base == b->base)
					error(location, "a class named twice as a direct base class");
			}
		}
		return;
	}
	PointerMap seen;
	for (const BaseClass* node = entity->bases; node != nullptr; node = node->next)
	{
		if (seen.find(node->base) != PointerMap::kMissing)
			error(location, "a class named twice as a direct base class");
		seen.set(node->base, 1);
	}
}

const Type* Analyzer::decltype_base_type(const syntax::Expr& operand, Scope* scope)
{
	// A template definition's decltype-specifier base whose operand is not
	// type-dependent is the same type in every instantiation (14.6.2.2p1):
	// typed where it appears, so that it is validated there (14.6p10).
	if (template_depth_ == 0 || type_dependent(&operand, scope))
		return decltype_type(operand, scope);
	const uint32_t depth = template_depth_;
	template_depth_ = 0;
	const Type* type = decltype_type(operand, scope);
	template_depth_ = depth;
	return type;
}

void Analyzer::anonymous_union(ClassEntity* entity, const Specifiers& specifiers, Scope* scope, uint32_t location)
{
	// 9.5p5: an anonymous union's members are injected into the enclosing
	// scope; in a named namespace or the global namespace it must be
	// static (not in an unnamed one, whose members are already unit-local,
	// 3.5p4). An unnamed struct without declarators declares nothing,
	// except as a (GNU) member.
	Scope* where = declaration_scope(scope);
	if (!IsUnion(entity->key) && where->kind != ScopeKind::Class)
		error(location, "declaration does not declare anything");
	const bool unnamed_namespace = where->kind == ScopeKind::Namespace && where->entity != nullptr &&
		where->entity->name == nullptr && static_cast<const NamespaceEntity*>(where->entity)->depth != 0;
	if (where->kind == ScopeKind::Namespace && !specifiers.is_static && !unnamed_namespace)
		error(location, "anonymous union in a named or the global namespace must be static");
	const char* label = anonymous_class_label(
		*specifiers.type_specifier->class_specifier, !specifiers.leading_specifiers);
	entity->label = label;
	entity->scope->label = label;
	for (const Binding* member = entity->scope->bindings; member != nullptr; member = member->next)
	{
		// A nested unnamed class's placeholder line is not a member.
		if (member->hidden)
			continue;
		if (member->kind != BindingKind::Variable)
			error(location, "an anonymous union can only have non-static data members");
		const NameEntry* entry = names_.find(where, member->name);
		if (entry != nullptr && (entry->value != nullptr || entry->type != nullptr))
			error(location, "anonymous union member conflicts with a declaration in the enclosing scope");
		Binding* injected = new_binding(BindingKind::Variable, member->name, member->entity, member->type);
		print_in(scope, injected);
		declare_in(where, injected);
	}
	if (where->kind == ScopeKind::Class)
	{
		// The unnamed member object, which the class's storage names.
		ClassEntity* owner = static_cast<ClassEntity*>(where->entity);
		Entity* storage = new_entity<VariableEntity>(EntityKind::Variable, nullptr, where, entity->type);
		add_data_member(owner, storage, 0, false);
		Binding* binding = new_binding(BindingKind::Variable, nullptr, storage, entity->type);
		binding->hidden = true;
		entity->storage = binding;
		owner->non_pod = owner->non_pod || current_access_ != TokenKind::KW_PUBLIC;
		return;
	}
	if (!semantic() || template_depth_ != 0)
		return;
	// Semantics mode: the unnamed object of a namespace- or block-scope
	// anonymous union, a typed identity outside ordinary lookup, which the
	// injected member names access.
	const char* key = "__anonymous_union_type__";
	std::string storage_label = std::string("__anonymous_union_storage__") + (label + std::strlen(key));
	VariableEntity* storage = new_entity<VariableEntity>(EntityKind::Variable, nullptr, where, entity->type);
	storage->defined = true;
	storage->is_static = specifiers.is_static;
	// At namespace scope it is static: internal linkage (3.5p3).
	storage->internal_linkage = where->kind == ScopeKind::Namespace;
	Binding* binding = new_binding(BindingKind::Variable, nullptr, storage, entity->type);
	binding->label = copy_label(storage_label.data(), storage_label.size());
	binding->hidden = true;
	entity->storage = binding;
	specifiers.type_specifier->class_specifier->storage = binding;
	BodyContext* saved = body_;
	BodyContext context = BodyContext();
	if (body_ == nullptr)
		body_ = &context;
	default_initialize(storage, entity->type, location);
	body_ = saved;
}

void Analyzer::name_unnamed_type(Specifiers& specifiers, const Identifier* name, Scope* scope)
{
	// An unnamed class or enumeration takes the name of the declaration's
	// first declarator (for a typedef or alias, 7.1.3p9; the course dump
	// extends this to objects at namespace and block scope). Class members
	// do not name their class's unnamed classes except through a typedef.
	Scope* where = declaration_scope(scope);
	if (where->kind == ScopeKind::Class && !specifiers.is_typedef)
		name = nullptr;
	// Semantics mode names an unnamed class or enumeration only by a
	// typedef (7.1.3p9); objects leave it to its stable identity (an
	// unnamed local one is then Ut<n>_ in a host name, as in GCC and
	// Clang, not the object's name; one at namespace scope has no linkage).
	if (semantic() && !specifiers.is_typedef)
		name = nullptr;
	Binding* binding = specifiers.declared_binding;
	if (binding == nullptr)
		return;
	if (specifiers.declared->kind == EntityKind::Class)
	{
		ClassEntity* entity = static_cast<ClassEntity*>(specifiers.declared);
		const char* label = name != nullptr ? name->spelling
											: anonymous_class_label(*specifiers.type_specifier->class_specifier, false);
		entity->label = label;
		entity->scope->label = label;
		if (name != nullptr)
		{
			entity->name = name;
			binding->name = name;
			binding->hidden = false;
		}
		return;
	}
	EnumEntity* entity = static_cast<EnumEntity*>(specifiers.declared);
	if (name != nullptr)
	{
		entity->name = name;
		entity->label = name->spelling;
		binding->name = name;
		binding->hidden = false;
		return;
	}
	char buffer[40];
	const int length = std::snprintf(buffer, sizeof buffer, "__anonymous_enum%u", ++anonymous_enums_);
	entity->label = copy_label(buffer, static_cast<size_t>(length));
}

LookupResult Analyzer::earlier_class(Scope* scope, const Identifier* name)
{
	// A namespace's or block's class of the name, also one only friend
	// declarations declared, which no lookup finds yet (7.3.1.2p3, 11.3p11):
	// a later declaration there redeclares that class (and makes it
	// visible, see NameLookup::declare).
	LookupResult result = names_.declared(scope, name, LookupFilter::Elaborated);
	const NameEntry* entry = result.found() ? nullptr : names_.find(scope, name);
	if (entry != nullptr && entry->tag != nullptr && entry->tag->invisible)
		result.binding = entry->tag;
	return result;
}

Entity* Analyzer::elaborated_class(
	const syntax::DeclSpecifier& specifier, Scope* scope, bool lone_declaration, bool is_friend, Binding*& printed)
{
	const syntax::Name& name = *specifier.name;
	const syntax::NameComponent& last = name.last();
	const TokenKind key = specifier.keyword;
	if (last.kind != syntax::NameComponentKind::Identifier)
		error(name.location, "invalid class name");
	Scope* where = declaration_scope(scope);
	if (last.has_arguments)
	{
		// A class template specialization named by an elaborated-type-
		// specifier (friend declarations included); declaring one alone
		// after `template<>` declares its explicit specialization (14.7.3).
		if (lone_declaration && !is_friend && explicit_specialization_)
		{
			explicit_specialization_ = false;
			return specialized_class(name, key, scope);
		}
		if (lone_declaration && !is_friend && template_scope_ != nullptr && scope == template_scope_)
			return partial_specialization(name, key, scope);
		if (lone_declaration && !is_friend)
			error(name.location, "a class template-id declared outside a class template specialization's declaration");
		// 14.5.4p8: a friend declaration does not declare a partial
		// specialization.
		if (is_friend && template_scope_ != nullptr && scope == template_scope_)
			error(name.location, "a friend declaration of a partial specialization");
		// 7.1.6.3p2: nor names an alias template's specialization.
		if (alias_template_id(name, scope))
			error(name.location, "an elaborated-type-specifier names an alias template specialization");
		const LookupResult result = lookup_name(name, scope, LookupFilter::Elaborated);
		// A substitution's failure (14.8.2p8): no entity.
		if (failed())
			return nullptr;
		if (result.binding->kind != BindingKind::Type || result.binding->entity->kind != EntityKind::Class)
			error(name.location, "elaborated type specifier names a different kind of type");
		ClassEntity* entity = static_cast<ClassEntity*>(result.binding->entity);
		if (IsUnion(key) != IsUnion(entity->key))
			error(name.location, "class-key does not agree with an earlier declaration");
		if (is_friend)
			befriend_class(entity, scope);
		return entity;
	}
	// A named member class of an instance is defined when required.
	if (lone_declaration && !is_friend && !name.qualified() && instance_class(where))
	{
		Specifiers declared = Specifiers();
		if (declare_member_instance(nullptr, name, key, where, declared))
		{
			printed = declared.declared_binding;
			return declared.declared;
		}
	}
	ClassEntity* entity = nullptr;
	// 14.1p9: a friend class template declaration has no default template
	// argument.
	if (is_friend && template_scope_ != nullptr && scope == template_scope_)
	{
		for (const syntax::TemplateParameter* parameter : template_list_->parameters)
		{
			if (parameter->default_type != nullptr || parameter->default_value != nullptr)
				error(parameter->location, "a default template argument in a friend class template declaration");
		}
	}
	if (name.qualified() || !lone_declaration || is_friend)
	{
		// 3.4.4p2: an elaborated name is looked up ignoring non-type names
		// (for a friend, only in the innermost enclosing namespace).
		LookupResult result = LookupResult();
		if (name.qualified())
		{
			result = lookup_name(name, scope, LookupFilter::Elaborated);
			if (failed())
				return nullptr;
			// A member of a dependent qualifier (14.6.2.1) names a class its
			// instantiations find (7.1.6.3p2 checked there).
			if (result.found() && IsDependentMember(result.binding))
				return result.binding->entity;
		}
		else if (is_friend)
		{
			// The name's lookup meets the template parameters first, which
			// an elaborated-type-specifier cannot name (7.1.6.3p2: `friend
			// class T;` is ill-formed; 14.6.1p6).
			// The lookup (3.4.1, ignoring non-type names, 3.4.4p2) considers
			// no scope outside the innermost enclosing non-class scope: the
			// namespace (7.3.1.2p3), or a local class's block (11.3p11); the
			// enclosing classes' members are found (`friend class X;` in a
			// nested class names its enclosing class's X), not their
			// injected-class-names (a friend template declaration in a
			// class template or its partial specialization names the
			// template, 14.6.1p1; another the class, as found outside).
			check_template_parameter_name(scope, last.identifier, name.location);
			for (Scope* s = scope; s != nullptr && !result.found(); s = s->parent)
			{
				if (s->kind == ScopeKind::Class)
				{
					result = names_.declared(s, last.identifier, LookupFilter::Elaborated);
					if (result.found() && result.binding->entity == s->entity)
						result = LookupResult();
				}
				else if (s->kind == ScopeKind::Namespace || s->kind == ScopeKind::Block)
				{
					result = earlier_class(s, last.identifier);
					break;
				}
			}
		}
		else
		{
			result = names_.unqualified(scope, last.identifier, LookupFilter::Elaborated);
			// 3.3.2p7: declared in the nearest enclosing namespace or block
			// scope, as an earlier friend declaration may have declared it.
			Scope* target = where;
			while (!result.found() && target->kind != ScopeKind::Namespace && target->kind != ScopeKind::Block)
				target = target->parent;
			if (!result.found())
				result = earlier_class(target, last.identifier);
		}
		if (result.found())
		{
			// 7.1.6.3p2-p3, a substitution's failure (14.8.2p8).
			const char* failure = result.binding->kind == BindingKind::TypeAlias
				? "elaborated type specifier names a typedef"
				: result.binding->entity->kind != EntityKind::Class
				? "elaborated type specifier names a different kind of type"
				: IsUnion(key) != IsUnion(static_cast<ClassEntity*>(result.binding->entity)->key)
				? "class-key does not agree with an earlier declaration"
				: nullptr;
			if (failure != nullptr)
			{
				substitution_error(name.location, failure);
				return nullptr;
			}
			entity = static_cast<ClassEntity*>(result.binding->entity);
			// A friend template declaration redeclares the class template
			// (every specialization is befriended, 14.5.4p1).
			if (is_friend && template_scope_ != nullptr && scope == template_scope_)
				declare_class_template(entity, true, name.location);
			// An elaborated-type-specifier redeclaring a class only friend
			// declarations declared makes it visible (3.3.2p7).
			if (!is_friend && result.binding->invisible)
			{
				Binding* binding = new_binding(BindingKind::Type, last.identifier, entity, entity->type);
				binding->key = key;
				declare_in(entity->owner, binding);
			}
			if (is_friend)
				befriend_class(entity, scope);
			return entity;
		}
		// Not found: the class is declared in the nearest enclosing
		// namespace or block scope (3.3.2p7), for a friend the innermost
		// enclosing non-class scope (7.3.1.2p3, 11.3p11); a friend template
		// declaration declares a class template there (14.5.4p1).
		Scope* target = where;
		while (target->kind != ScopeKind::Namespace && target->kind != ScopeKind::Block)
			target = target->parent;
		check_type_name_conflict(target, last.identifier, nullptr, name.location);
		entity = new_class(key, last.identifier, target);
		if (is_friend && template_scope_ != nullptr && scope == template_scope_)
			declare_class_template(entity, false, name.location);
		Binding* binding = new_binding(BindingKind::Type, last.identifier, entity, entity->type);
		binding->key = key;
		binding->invisible = is_friend;
		print_in(target == where ? scope : target, binding);
		declare_in(target, binding);
		printed = binding;
		if (is_friend)
			befriend_class(entity, scope);
		return entity;
	}
	// `class-key identifier ;` declares the name in this scope (3.3.2p6),
	// or redeclares the class already declared here (not a template
	// parameter, 14.6.1p6).
	check_template_parameter_name(scope, last.identifier, name.location);
	const NameEntry* entry = names_.find(where, last.identifier);
	bool existed = false;
	if (entry != nullptr && entry->tag != nullptr)
	{
		if (entry->tag->entity->kind != EntityKind::Class)
			error(name.location, "class name redeclared as a different kind of type");
		entity = static_cast<ClassEntity*>(entry->tag->entity);
		existed = true;
		if (IsUnion(key) != IsUnion(entity->key))
			error(name.location, "class-key does not agree with an earlier declaration");
	}
	else
	{
		check_type_name_conflict(where, last.identifier, nullptr, name.location);
		entity = new_class(key, last.identifier, where);
	}
	if (template_scope_ != nullptr && scope == template_scope_)
	{
		// 14p5, as for a class template's definition.
		if (entry != nullptr && entry->value != nullptr)
			error(name.location, "a class template shares its name with a declaration in its scope");
		declare_class_template(entity, existed, name.location);
	}
	else if (entity->templ != nullptr && entity->templ->role == TemplateRole::Template)
		error(name.location, "a class template redeclared as a class that is not a template");
	else if (template_depth_ != 0 && templated_class(where))
		class_template_facts(entity, TemplateRole::Pattern);
	Binding* binding = new_binding(BindingKind::Type, last.identifier, entity, entity->type);
	binding->key = key;
	print_in(scope, binding);
	declare_in(where, binding);
	printed = binding;
	if (lone_class_attributes_ != nullptr && !is_friend)
	{
		reject_noreturn(lone_class_attributes_, "noreturn attribute on a class");
		class_abi_tags(entity, lone_class_attributes_, true, existed, name.location);
		// GNU aligned after the class-key requests the class's alignment
		// before its definition (GCC's type attribute; ignored once it is
		// defined, as GCC warns).
		bool packed = false, no_inline = false;
		if (entity->scope == nullptr && template_depth_ == 0)
			entity->alignment_request = std::max(entity->alignment_request,
				static_cast<uint32_t>(attribute_alignment(lone_class_attributes_, scope, packed, no_inline, true)));
	}
	return entity;
}

// ------------------------------------------------------------ enumerations

EnumEntity* Analyzer::redeclared_enum(const syntax::EnumSpecifier& specifier, Scope* target, const Type* underlying)
{
	// 7.2p3: a redeclaration agrees on being scoped and on the underlying
	// type; an unscoped enumeration without a fixed type cannot be
	// redeclared.
	const NameEntry* entry = names_.find(target, specifier.name->last().identifier);
	if (entry == nullptr || entry->tag == nullptr)
		return nullptr;
	if (entry->tag->entity->kind != EntityKind::Enum)
		error(specifier.location, "enumeration name redeclared as a different kind of type");
	EnumEntity* entity = static_cast<EnumEntity*>(entry->tag->entity);
	const Type* expected = underlying;
	if (expected == nullptr && specifier.scoped)
		expected = types_.fundamental(FundamentalType::Int);
	if (entity->scoped != specifier.scoped || !entity->fixed || expected == nullptr ||
		entity->underlying->canonical != expected->canonical)
		error(specifier.location, "enumeration redeclared differently");
	return entity;
}

Entity* Analyzer::enum_specifier(
	const syntax::EnumSpecifier& specifier, Scope* scope, bool lone_declaration, Binding*& printed)
{
	// 7.2p1: an enum-head's attributes appertain to the enumeration
	// (7.6.3p1).
	reject_noreturn(specifier.attributes, "noreturn attribute on an enumeration");
	const Type* underlying = nullptr;
	if (specifier.underlying != nullptr)
	{
		// 7.2p2: an integral type; one that depends on a template parameter
		// is checked in the instantiations.
		underlying = type_id(*specifier.underlying, scope)->unqualified;
		if (!IsIntegralType(underlying) && !(template_depth_ != 0 && dependent_type(underlying)))
			error(specifier.location, "enumeration underlying type is not integral");
	}
	Scope* where = declaration_scope(scope);
	const syntax::Name* name = specifier.name;
	if (name != nullptr && name->last().has_arguments)
		error(name->location, "template-ids are not supported here");
	// 14.6.1p6: neither an enumeration defined here nor its enumerators
	// redeclare a template parameter.
	if (template_depth_ != 0 && name != nullptr && !name->qualified() && (specifier.has_body || lone_declaration))
		check_template_parameter_name(scope, name->last().identifier, name->location);
	for (uint32_t i = 0; template_depth_ != 0 && i < specifier.enumerators.size; ++i)
		check_template_parameter_name(scope, specifier.enumerators[i].name, specifier.enumerators[i].location);
	if (name != nullptr && !specifier.has_body && !lone_declaration)
	{
		// An elaborated enum specifier names an existing enumeration
		// (7.1.6.3p2) and is written without `class`/`struct`.
		if (specifier.scoped || underlying != nullptr)
			error(specifier.location, "invalid elaborated enumeration specifier");
		const LookupResult result = lookup_name(*name, scope, LookupFilter::Elaborated);
		if (failed())
			return nullptr;
		if (result.binding->kind != BindingKind::Type || result.binding->entity->kind != EntityKind::Enum)
			error(name->location, "elaborated enum specifier does not name an enumeration");
		return result.binding->entity;
	}
	if (name == nullptr)
	{
		if (!specifier.has_body || specifier.scoped)
			error(specifier.location, "an enumeration without a name needs a body and cannot be scoped");
		EnumEntity* entity = new_entity<EnumEntity>(EntityKind::Enum, nullptr, where, nullptr);
		entity->type = types_.named(TypeKind::Enum, entity);
		number_unnamed_local_type(entity);
		entity->fixed = underlying != nullptr;
		entity->underlying = underlying;
		class_abi_tags(entity, specifier.attributes, false, false, specifier.location);
		Binding* binding = new_binding(BindingKind::Type, nullptr, entity, entity->type);
		binding->hidden = true;
		binding->key = TokenKind::KW_ENUM;
		print_in(scope, binding);
		printed = binding;
		enumerators(entity, specifier, binding, scope, where);
		return entity;
	}
	if (name->qualified())
		return qualified_enum_definition(specifier, underlying, scope, printed);
	const Identifier* identifier = name->last().identifier;
	if (EnumEntity* entity = redeclared_enum(specifier, where, underlying))
	{
		class_abi_tags(entity, specifier.attributes, false, true, specifier.location);
		// The course dump keeps the first declaration's line for an opaque
		// redeclaration or the definition that completes it.
		if (specifier.has_body)
		{
			if (entity->defined)
				error(specifier.location, "enumeration redefinition");
			const NameEntry* entry = names_.find(where, identifier);
			enumerators(entity, specifier, entry->tag, entity->scoped ? entity->scope : scope,
				entity->scoped ? entity->scope : where);
		}
		return entity;
	}
	check_type_name_conflict(where, identifier, nullptr, name->location);
	if (!specifier.has_body && !specifier.scoped && underlying == nullptr)
		error(specifier.location, "an opaque unscoped enumeration needs an underlying type");
	EnumEntity* entity = new_entity<EnumEntity>(EntityKind::Enum, identifier, where, nullptr);
	entity->type = types_.named(TypeKind::Enum, entity);
	entity->scoped = specifier.scoped;
	entity->fixed = specifier.scoped || underlying != nullptr;
	entity->underlying = underlying != nullptr ? underlying
		: specifier.scoped                     ? types_.fundamental(FundamentalType::Int)
											   : nullptr;
	entity->label = identifier->spelling;
	// GNU abi_tag on an enumeration: GCC mangles it as a class's (Clang
	// rejects it).
	class_abi_tags(entity, specifier.attributes, false, false, specifier.location);
	Binding* binding = new_binding(BindingKind::Type, identifier, entity, entity->type);
	binding->key = TokenKind::KW_ENUM;
	binding->scoped = specifier.scoped;
	print_in(scope, binding);
	declare_in(where, binding);
	printed = binding;
	// A scoped enumeration's scope exists (and is printed) from its first
	// declaration.
	if (specifier.scoped)
		entity->scope = new_scope(ScopeKind::Enum, scope, scope, entity, entity->label);
	if (specifier.has_body)
		enumerators(entity, specifier, binding, specifier.scoped ? entity->scope : scope,
			specifier.scoped ? entity->scope : where);
	// An instance's member enumeration declared opaquely: its pattern's may
	// be defined out of class (14.5.1.4).
	else if (instance_class(where))
		declare_member_enum_instance(entity, static_cast<ClassEntity*>(where->entity));
	return entity;
}

Entity* Analyzer::qualified_enum_definition(
	const syntax::EnumSpecifier& specifier, const Type* underlying, Scope* scope, Binding*& printed)
{
	// 7.2p4: a member enumeration declared (opaquely) in a class or
	// namespace may be defined outside it, in an enclosing scope. The
	// enumerators of an unscoped one are members of the enumeration's scope
	// (7.2p10) and print there, spelled by its first declaration. The course
	// dump prints a scoped definition where it appears, under its qualified
	// name, with its enumerators in a scope of that name; lookup through the
	// enumeration still finds them.
	const syntax::Name& name = *specifier.name;
	const NamePrefix prefix = name_prefix(name, scope, true);
	if (prefix.scope == nullptr || prefix.scope->kind == ScopeKind::Enum)
		error(name.location, "qualified enumeration name must name a class or namespace member");
	Scope* where = declaration_scope(scope);
	if (!encloses(where, prefix.scope))
		error(name.location, "enumeration definition in a scope that does not enclose it");
	EnumEntity* entity = redeclared_enum(specifier, prefix.scope, underlying);
	if (entity == nullptr)
		error(name.location, "qualified enumeration does not match a declaration");
	class_abi_tags(entity, specifier.attributes, false, true, specifier.location);
	if (!specifier.has_body)
		return entity;
	if (entity->defined)
		error(specifier.location, "enumeration redefinition");
	// A member enumeration of a templated class defined out of it
	// (14.5.1.4): the instances' enumerations instantiate this definition,
	// those declared before it once it is analyzed.
	EnumEntity* waiting = nullptr;
	// Its values' names include the definition's template parameters.
	Scope* values = nullptr;
	if (template_depth_ != 0 && templated_class(prefix.scope))
	{
		if (scope->kind == ScopeKind::TemplateParameters)
			values = qualified_lookup_scope(entity->scoped ? entity->scope : prefix.scope, scope);
		EnumTemplateFacts* facts = enum_template_facts(entity);
		facts->definition = &specifier;
		facts->parameters = scope->kind == ScopeKind::TemplateParameters ? scope : nullptr;
		facts->definition_ordinal = definition_point();
		waiting = facts->waiting;
		facts->waiting = nullptr;
	}
	if (!entity->scoped)
	{
		const Binding* first = names_.find(prefix.scope, name.last().identifier)->tag;
		enumerators(entity, specifier, first, prefix.scope, prefix.scope, values);
	}
	else
	{
		const std::string written = syntax::RenderName(name);
		const char* label = copy_label(written.data(), written.size());
		Binding* binding = new_binding(BindingKind::Type, name.last().identifier, entity, entity->type);
		binding->label = label;
		binding->key = TokenKind::KW_ENUM;
		binding->scoped = true;
		print_in(scope, binding);
		printed = binding;
		Scope* print = new_scope(ScopeKind::Enum, entity->scope, scope, entity, label);
		enumerators(entity, specifier, binding, print, entity->scope, values);
	}
	for (EnumEntity* instance = waiting; instance != nullptr; instance = instance->templ->next)
		instantiate_member_enum(instance);
	return entity;
}

void Analyzer::enumerators(EnumEntity* entity, const syntax::EnumSpecifier& specifier, const Binding* declaration,
	Scope* print, Scope* lookup, Scope* values)
{
	// 7.2p5: with a fixed underlying type each value converts to it without
	// narrowing; otherwise each enumerator has the type of its value, an
	// omitted value is the previous one plus one (in a larger type if
	// needed), and the underlying type is chosen from the range (7.2p6).
	// The values are analyzed in `values` (an instantiated definition's
	// frame over `lookup`), by default where the enumerators are declared.
	if (values == nullptr)
		values = lookup;
	entity->defined = true;
	const size_t base = enumerator_scratch_.size();
	Constant previous = Constant{ 0, nullptr };
	IntegralKind fixed_kind = IntegralKind{ 32, true };
	// A fixed underlying type that depends on a template parameter leaves
	// the values to the instantiations: the pattern's count in int.
	const bool dependent = entity->fixed && !IntegralKindOf(entity->underlying, fixed_kind);
	const bool fixed = entity->fixed && !dependent;
	for (const syntax::Enumerator& enumerator : specifier.enumerators)
	{
		// A template's enumerator values wait for its instantiations; their
		// non-dependent names are bound where they appear (14.6p10).
		if (enumerator.value != nullptr && template_depth_ != 0)
			expression_walk(enumerator.value, values);
		Constant value = Constant{ 0, fixed ? entity->underlying : types_.fundamental(FundamentalType::Int) };
		if (enumerator.value != nullptr && template_depth_ == 0)
		{
			// Semantics mode types the value first (5.19: its operands'
			// conversions and calls decide it; sizeof's operand is typed; a
			// class operand converts to an integral type, 5.19p6, or to a
			// fixed underlying type, 7.2p5).
			if (semantic())
				integral_constant_operand(*enumerator.value, values, fixed ? entity->underlying : nullptr);
			// A deferred reserved builtin call (see deferrable_statement).
			if (deferred_reserved_call())
				return;
			constant_value(*enumerator.value, values, value, true);
			if (IsScopedEnumeration(value.type))
				error(
					enumerator.value->location, "a scoped enumeration value does not convert implicitly to an integer");
			if (entity->fixed)
			{
				if (!Representable(value, fixed_kind))
					error(enumerator.value->location, "enumerator value is not representable in the underlying type");
				else if (!HeldAsWideConstant(value, fixed_kind))
					error(
						enumerator.value->location, "a 128-bit enumerator value beyond the 64-bit constants evaluated");
				value = Constant{ ConvertIntegral(value.bits, fixed_kind), entity->underlying };
			}
			else
				value.type = PromotedType(types_, value.type);
		}
		else if (previous.type != nullptr && dependent)
			value = Constant{ previous.bits + 1, previous.type };
		else if (previous.type != nullptr)
			value = next_enumerator_value(entity, previous, fixed_kind, enumerator.location);
		previous = value;
		const NameEntry* entry = names_.find(lookup, enumerator.name);
		if (entry != nullptr &&
			(entry->value != nullptr ||
				(entry->type != nullptr && (entry->type->kind != BindingKind::Type || IsClassTemplate(entry->type)))))
			error(enumerator.location, "enumerator conflicts with an earlier declaration");
		EnumeratorEntity* constant = new_entity<EnumeratorEntity>(
			EntityKind::Enumerator, enumerator.name, lookup, entity->type);
		constant->enumeration = entity;
		constant->value = value;
		Binding* binding = new_binding(BindingKind::Enumerator, enumerator.name, constant, entity->type);
		binding->declaration = declaration;
		print_in(print, binding);
		declare_in(lookup, binding);
		enumerator_scratch_.push_back(constant);
	}
	if (!entity->fixed)
		entity->underlying = enumeration_underlying(base);
	// After the closing brace each enumerator has the enumeration's type.
	IntegralKind kind = fixed_kind;
	if (!dependent)
		IntegralKindOf(entity->underlying, kind);
	for (size_t i = base; i < enumerator_scratch_.size(); ++i)
	{
		EnumeratorEntity* constant = enumerator_scratch_[i];
		IntegralKind value_kind;
		if (IntegralKindOf(constant->value.type, value_kind) && value_kind.is_signed &&
			static_cast<int64_t>(constant->value.bits) < 0)
			entity->negative = true;
		constant->value = Constant{ ConvertIntegral(constant->value.bits, kind), entity->type };
	}
	enumerator_scratch_.resize(base);
}

Constant Analyzer::next_enumerator_value(
	const EnumEntity* entity, const Constant& previous, IntegralKind fixed_kind, uint32_t location)
{
	// The previous value plus one, as a mathematical value: in the fixed
	// underlying type, or in the previous type if it fits there and
	// otherwise in the smallest standard type that holds it (7.2p5).
	const bool is_signed = IsSignedConstant(previous);
	const int64_t signed_previous = static_cast<int64_t>(previous.bits);
	if ((is_signed && signed_previous == INT64_MAX) || (!is_signed && previous.bits == UINT64_MAX))
	{
		if (entity->fixed || !is_signed)
			error(location, "enumerator value overflows");
		return Constant{ uint64_t(INT64_MAX) + 1, types_.fundamental(FundamentalType::UnsignedLongInt) };
	}
	const uint64_t next = previous.bits + 1;
	const bool negative = is_signed && static_cast<int64_t>(next) < 0;
	const Constant wide = Constant{ next,
		types_.fundamental(is_signed ? FundamentalType::LongLongInt : FundamentalType::UnsignedLongLongInt) };
	if (entity->fixed)
	{
		if (!Representable(wide, fixed_kind))
			error(location, "enumerator value is not representable in the underlying type");
		return Constant{ ConvertIntegral(next, fixed_kind), entity->underlying };
	}
	IntegralKind previous_kind;
	IntegralKindOf(previous.type, previous_kind);
	if (Representable(wide, previous_kind))
		return Constant{ next, previous.type };
	return Constant{ next, SmallestTypeFor(types_, next, negative) };
}

const Type* Analyzer::enumeration_underlying(size_t base)
{
	// The first of int, unsigned int, long, unsigned long, long long and
	// unsigned long long that represents every value (GCC's choice).
	static const FundamentalType kCandidates[] = { FundamentalType::Int, FundamentalType::UnsignedInt,
		FundamentalType::LongInt, FundamentalType::UnsignedLongInt, FundamentalType::LongLongInt,
		FundamentalType::UnsignedLongLongInt };
	for (FundamentalType candidate : kCandidates)
	{
		const Type* type = types_.fundamental(candidate);
		IntegralKind kind;
		IntegralKindOf(type, kind);
		bool fits = true;
		for (size_t i = base; fits && i < enumerator_scratch_.size(); ++i)
			fits = Representable(enumerator_scratch_[i]->value, kind);
		if (fits)
			return type;
	}
	return types_.fundamental(FundamentalType::UnsignedLongLongInt);
}

// ------------------------------------------------------ deferred and implicit

void Analyzer::finish_class_nesting(size_t first_deferred)
{
	// The outermost class is complete: the constants of static data members
	// whose initializers needed its layout, its members' brace-or-equal-
	// initializers, then its member functions' default arguments and
	// bodies, are analyzed in declaration order (a local class in one of
	// them completes and drains its own first). The initializers come first
	// so an implicit constructor a body odr-uses knows them. A local class
	// in a hosted wrapper's body (see define_function) is none of its
	// statements: only its own wrapper members defer.
	const uint32_t saved_reserved = deferring_reserved_;
	deferring_reserved_ = 0;
	for (size_t i = first_deferred; i < deferred_.size(); ++i)
	{
		VariableEntity* variable = deferred_[i].constant;
		if (variable == nullptr)
			continue;
		variable->constant_deferred = false;
		record_constant_initializer(variable, variable->member_initializer, deferred_[i].parent);
	}
	for (size_t i = first_deferred; i < deferred_.size(); ++i)
	{
		VariableEntity* member = deferred_[i].member;
		if (member == nullptr)
			continue;
		// A class template instance's waits for a constructor that uses it
		// (unless one planned while the class was completed already has).
		const ClassTemplateFacts* facts = static_cast<const ClassEntity*>(member->owner->entity)->templ;
		if (template_depth_ == 0 && !member->initializer_used && facts != nullptr &&
			(facts->role == TemplateRole::Specialization || facts->role == TemplateRole::MemberInstance) &&
			!facts->explicit_specialization)
		{
			member->initializer_pending = true;
			pending_member_initializers_.set(member, static_cast<uint32_t>(pending_member_scopes_.size()));
			pending_member_scopes_.push_back(PendingMemberInitializer{ deferred_[i].parent, names_.limit() });
			continue;
		}
		member_default_initializer(member, deferred_[i].parent);
	}
	for (size_t i = first_deferred; i < deferred_.size(); ++i)
	{
		const DeferredBody body = deferred_[i];
		// A body a constant evaluation analyzed ahead is cleared.
		if (body.member != nullptr || body.function == nullptr)
			continue;
		// A member template's body, and in an instance a body of a member
		// (or a friend defined in the class, whose scope `print` is) of its
		// member class template, is a template definition wherever its class
		// is (its specializations instantiate it). A templated class's own
		// members are analyzed in its definition (its default arguments'
		// names bound there, 14.6p10).
		const bool member_template = body.function->templated ||
			(template_depth_ == 0 && (templated_class(body.function->owner) || templated_class(body.print)));
		template_depth_ += member_template ? 1 : 0;
		if (body.body == nullptr)
		{
			if (!member_template)
				type_default_arguments(body.function, body.parameters, body.parent);
		}
		else
		{
			// A body calling a reserved builtin this compiler lacks (see
			// define_function) is deferred with its emission: an inline
			// member's then fails, as does instantiating a template's.
			const bool reserved = reserved_wrapper(*body.body, body.parent);
			if (reserved && !member_template)
				body.function->unanalyzed_body = true;
			wrapper_body(body.function, body.type, body.parameters, *body.body, body.print, body.parent, reserved);
		}
		template_depth_ -= member_template ? 1 : 0;
	}
	deferred_.resize(first_deferred);
	deferring_reserved_ = saved_reserved;
}

void Analyzer::define_implicit_default_constructor(ClassEntity* entity)
{
	// 12.1p5, 12.1p6: a class without a user-declared constructor has an
	// implicitly declared default constructor, defined when it is odr-used;
	// the course dump shows the definition as an empty function scope.
	if (entity->user_declared_constructor || entity->implicit_default_constructor != nullptr || !entity->complete ||
		entity->anonymous)
		return;
	entity->implicit_default_constructor = new_scope(
		ScopeKind::Function, entity->scope, entity->scope, entity, entity->label);
}

} // namespace sema
