#include "sema/analyzer.h"

#include <algorithm>
#include <cstring>
#include <string>

#include "lex/identifier_table.h"
#include "support/stack_budget.h"

namespace sema {

namespace {

bool IsNamespaceBinding(const Binding* binding)
{
	return binding != nullptr &&
		(binding->kind == BindingKind::Namespace || binding->kind == BindingKind::NamespaceAlias);
}

// A non-type template parameter's name: its declarator-id's.
const Identifier* TemplateParameterName(const syntax::TemplateParameter& parameter)
{
	for (const syntax::Declarator* declarator = parameter.declarator; declarator != nullptr;
		declarator = declarator->nested)
	{
		if (declarator->name != nullptr && !declarator->name->qualified() &&
			declarator->name->last().kind == syntax::NameComponentKind::Identifier)
			return declarator->name->last().identifier;
	}
	return nullptr;
}

} // namespace

SemanticGraph::SemanticGraph()
	: types(arena), names(arena), global(nullptr), root_demands(nullptr), last_root_demand(nullptr), local_types(0),
	  scopes(0), bindings(0), entities(0), constant_evaluations(0), constant_steps(0), expressions(0), conversions(0),
	  resolutions(0), candidates(0), specializations(0), demands(0), override_steps(0), virtual_slots(0),
	  instantiated_classes(0), instantiated_functions(0), instantiated_arguments(0), instantiated_variables(0),
	  value_substitutions(0), partial_candidates(0), substitution_failures(0), base_steps(0), nontrivial_destructors(0)
{}

Analyzer::Analyzer(SemanticGraph& graph, IdentifierTable& identifiers, AnalysisMode mode)
	: graph_(graph), mode_(mode), types_(graph.types), names_(graph.names), identifiers_(identifiers),
	  arena_(graph.arena), class_depth_(0), template_depth_(0), anonymous_enums_(0), suppress_implicit_(false),
	  current_access_(TokenKind::KW_PUBLIC), member_scope_(nullptr), expression_scope_(nullptr), naming_class_(nullptr),
	  last_prefix_(NamePrefix{ nullptr, nullptr }), declaration_class_(nullptr), redeclared_(false),
	  lone_class_attributes_(nullptr), access_deferral_(nullptr), base_object_(nullptr), c_language_(false),
	  linkage_extern_(false), linkage_specified_(false), stack_base_(0), stack_budget_(stack_budget_bytes()),
	  empty_count_(0), max_empty_offset_(0), empty_record_limit_(0), empty_record_tail_(0), layout_budget_(0),
	  conflict_root_(nullptr), conflict_hint_(EmptySubobject{ nullptr, 0, false }), conversions_(graph.types),
	  body_(nullptr), demanding_(nullptr), demand_epoch_(0), root_epoch_(0), current_epoch_(0), unevaluated_(0),
	  potentially_throwing_(0), candidate_epoch_(0), association_epoch_(0), empty_list_epoch_(0), conversion_count_(0)
{
	constructor_key_ = special_key("<constructor>");
	destructor_key_ = special_key("~");
	conversion_key_ = special_key("operator <conversion>");
	nullptr_t_ = special_key("nullptr_t");
	func_ = special_key("__func__");
	function_name_ = special_key("__FUNCTION__");
	pretty_function_ = special_key("__PRETTY_FUNCTION__");
	complex_keys_[0] = special_key("_Complex");
	complex_keys_[1] = special_key("__complex__");
	complex_keys_[2] = special_key("__complex");
	int128_key_ = special_key("__int128");
	assign_key_ = special_key("operator=");
	new_array_key_ = special_key("operatornew[]");
	delete_array_key_ = special_key("operatordelete[]");
	this_key_ = special_key("this");
	decltype_key_ = special_key("decltype(...)");
	value_key_ = special_key("value(...)");
	final_key_ = special_key("final");
	override_key_ = special_key("override");
	std_key_ = special_key("std");
	initializer_list_key_ = special_key("initializer_list");
	bad_alloc_key_ = special_key("bad_alloc");
	type_pack_element_key_ = special_key("__type_pack_element");
	TemplateParameterEntity* placeholder = new_entity<TemplateParameterEntity>(
		EntityKind::TemplateParameter, special_key("auto"), nullptr, nullptr);
	auto_placeholder_ = types_.named(TypeKind::TemplateParameter, placeholder);
	placeholder->type = auto_placeholder_;
	pending_prototype_ = PendingPrototype{ nullptr, nullptr };
	completer_.analyzer = this;
	conversions_.set_completer(&completer_);
}

void Analyzer::analyze(const syntax::TranslationUnit& unit)
{
	char marker;
	stack_base_ = reinterpret_cast<uintptr_t>(&marker);
	NamespaceEntity* global = new_entity<NamespaceEntity>(EntityKind::Namespace, nullptr, nullptr, nullptr);
	Scope* scope = new_scope(ScopeKind::Namespace, nullptr, nullptr, global, "<global>");
	global->scope = scope;
	graph_.global = scope;
	// The course grammar's fundamental type name `nullptr_t` (std::nullptr_t
	// in a hosted library) is predeclared, without a line of its own.
	Entity* predeclared = new_entity<Entity>(
		EntityKind::Typedef, nullptr_t_, scope, types_.fundamental(FundamentalType::NullptrT));
	Binding* binding = new_binding(BindingKind::TypeAlias, nullptr_t_, predeclared, predeclared->type);
	binding->hidden = true;
	declare_in(scope, binding);
	// GCC's 128-bit integer types by their predeclared names: `__int128`
	// (a keyword there, a type name here) and the typedef names __int128_t
	// and __uint128_t; its extended floating types' keywords alike, and
	// Clang's __bf16.
	const struct
	{
		const char* spelling;
		FundamentalType type;
	} wide[] = { { "__int128", FundamentalType::Int128 }, { "__int128_t", FundamentalType::Int128 },
		{ "__uint128_t", FundamentalType::UnsignedInt128 }, { "_Float16", FundamentalType::Float16 },
		{ "_Float32", FundamentalType::Float32 }, { "_Float64", FundamentalType::Float64 },
		{ "_Float128", FundamentalType::Float128 }, { "_Float32x", FundamentalType::Float32x },
		{ "_Float64x", FundamentalType::Float64x }, { "__float128", FundamentalType::GnuFloat128 },
		{ "__bf16", FundamentalType::BFloat16 } };
	for (const auto& name : wide)
	{
		const Identifier* identifier = special_key(name.spelling);
		Entity* wide_type = new_entity<Entity>(EntityKind::Typedef, identifier, scope, types_.fundamental(name.type));
		Binding* wide_binding = new_binding(BindingKind::TypeAlias, identifier, wide_type, wide_type->type);
		wide_binding->hidden = true;
		declare_in(scope, wide_binding);
	}
	// The builtin functions (`__builtin_abort` ...) are declared when first
	// named (analyzer_builtins.cpp).
	const Type* void_pointer = types_.pointer(types_.fundamental(FundamentalType::Void));
	// 3.7.4p2: void* operator new(std::size_t), void* operator
	// new[](std::size_t), void operator delete(void*) noexcept and void
	// operator delete[](void*) noexcept.
	const Type* size_type = types_.fundamental(FundamentalType::UnsignedLongInt);
	const Type* allocate_type = types_.function(void_pointer, &size_type, 1, false, 0, RefQualifier::None);
	const Type* free_type = types_.function(
		types_.fundamental(FundamentalType::Void), &void_pointer, 1, false, 0, RefQualifier::None);
	predeclare_allocation(TokenKind::KW_NEW, false, allocate_type, AllocationRole::New, scope);
	predeclare_allocation(TokenKind::KW_NEW, true, allocate_type, AllocationRole::NewArray, scope);
	predeclare_allocation(TokenKind::KW_DELETE, false, free_type, AllocationRole::Delete, scope);
	predeclare_allocation(TokenKind::KW_DELETE, true, free_type, AllocationRole::DeleteArray, scope);
	root_epoch_ = ++demand_epoch_;
	current_epoch_ = root_epoch_;
	declarations(unit.declarations, scope);
	// The rules decided once every class of the unit is complete (PA6's
	// types too: an unused member's noexcept-specification is valid).
	if (!semantic())
		check_pending_exceptions();
	if (semantic())
	{
		check_defaulted_constexprs();
		check_pending_exceptions();
		// 14.7.2p5: an explicit instantiation definition's function has its
		// template's (or its class template's member's) definition in the
		// unit, before or after it (a deleted one has its definition).
		for (const NamedInstantiation& named : named_instantiations_)
		{
			const FunctionEntity* function = named.function;
			const bool deleted = function->deleted || (function->primary != nullptr && function->primary->deleted) ||
				(PatternOf(function) != nullptr && PatternOf(function)->deleted);
			if (!deleted && !InstantiationSuppressed(function) && !instantiate_definition(function))
				error(named.location,
					"an explicit instantiation definition of a function whose definition is not available");
		}
		// 14.6.4.1p8: the end of the unit is a point of instantiation of
		// each odr-used specialization, whose definition is required
		// (14.7.1p3) whether or not the unit emits it; instantiating one
		// may demand more.
		for (size_t i = 0; i < demanded_definitions_.size(); ++i)
		{
			// An implicitly declared or first-declaration-defaulted member
			// is implicitly defined (12.8p13), as the lowering synthesizes
			// it: no template definition to instantiate.
			const FunctionEntity* function = demanded_definitions_[i];
			if (!function->deleted && !function->implicit && !function->defaulted && !InstantiationSuppressed(function))
				instantiate_definition(function);
		}
	}
}

void Analyzer::predeclare_allocation(TokenKind op, bool array, const Type* type, AllocationRole role, Scope* scope)
{
	const std::string spelling = std::string("operator") + TokenKindSpelling(op) + (array ? "[]" : "");
	const Identifier* name = special_key(spelling.c_str());
	FunctionEntity* function = new_entity<FunctionEntity>(EntityKind::Function, name, scope, type);
	const std::string label = std::string("operator ") + TokenKindSpelling(op) + (array ? "[]" : "");
	function->label = copy_label(label.data(), label.size());
	function->signature = signature_of(type);
	function->is_operator = true;
	function->operator_token = op;
	function->operator_array = array;
	function->nothrow = op == TokenKind::KW_DELETE;
	function->exception_spec = true;
	function->allocation = role;
	Binding* binding = new_binding(BindingKind::Function, name, function, type);
	binding->hidden = true;
	declare_in(scope, binding);
}

// ------------------------------------------------------ graph construction

Scope* Analyzer::new_scope(ScopeKind kind, Scope* parent, Scope* print_parent, Entity* entity, const char* label)
{
	Scope* scope = static_cast<Scope*>(arena_.allocate(sizeof(Scope), alignof(Scope)));
	std::memset(static_cast<void*>(scope), 0, sizeof(Scope));
	scope->kind = kind;
	scope->dumped = true;
	scope->lists = kind == ScopeKind::TemplateParameters ? 1 : 0;
	scope->id = graph_.scopes++;
	scope->parent = parent;
	scope->entity = entity;
	scope->label = label;
	if (kind == ScopeKind::Block && parent != nullptr)
		scope->initialized_at_entry = parent->initialized;
	if (print_parent != nullptr)
		attach_scope(scope, print_parent);
	return scope;
}

void Analyzer::attach_scope(Scope* scope, Scope* print_parent)
{
	if (print_parent->last_child == nullptr)
		print_parent->children = scope;
	else
		print_parent->last_child->next_sibling = scope;
	print_parent->last_child = scope;
}

Binding* Analyzer::new_binding(BindingKind kind, const Identifier* name, Entity* entity, const Type* type)
{
	Binding* binding = static_cast<Binding*>(arena_.allocate(sizeof(Binding), alignof(Binding)));
	std::memset(static_cast<void*>(binding), 0, sizeof(Binding));
	binding->kind = kind;
	binding->name = name;
	binding->entity = entity;
	binding->type = type;
	binding->ordinal = ++graph_.bindings;
	return binding;
}

template<typename T>
T* Analyzer::new_entity(EntityKind kind, const Identifier* name, Scope* owner, const Type* type)
{
	T* entity = static_cast<T*>(arena_.allocate(sizeof(T), alignof(T)));
	std::memset(static_cast<void*>(entity), 0, sizeof(T));
	entity->kind = kind;
	entity->name = name;
	entity->owner = owner;
	entity->type = type;
	++graph_.entities;
	return entity;
}

template NamespaceEntity* Analyzer::new_entity<NamespaceEntity>(EntityKind, const Identifier*, Scope*, const Type*);
template NamespaceAliasEntity* Analyzer::new_entity<NamespaceAliasEntity>(
	EntityKind, const Identifier*, Scope*, const Type*);
template ClassEntity* Analyzer::new_entity<ClassEntity>(EntityKind, const Identifier*, Scope*, const Type*);
template EnumEntity* Analyzer::new_entity<EnumEntity>(EntityKind, const Identifier*, Scope*, const Type*);
template VariableEntity* Analyzer::new_entity<VariableEntity>(EntityKind, const Identifier*, Scope*, const Type*);
template PredefinedNameEntity* Analyzer::new_entity<PredefinedNameEntity>(
	EntityKind, const Identifier*, Scope*, const Type*);
template BindingEntity* Analyzer::new_entity<BindingEntity>(EntityKind, const Identifier*, Scope*, const Type*);
template FunctionEntity* Analyzer::new_entity<FunctionEntity>(EntityKind, const Identifier*, Scope*, const Type*);
template EnumeratorEntity* Analyzer::new_entity<EnumeratorEntity>(EntityKind, const Identifier*, Scope*, const Type*);
template TemplateParameterEntity* Analyzer::new_entity<TemplateParameterEntity>(
	EntityKind, const Identifier*, Scope*, const Type*);
template Entity* Analyzer::new_entity<Entity>(EntityKind, const Identifier*, Scope*, const Type*);
template AliasTemplateEntity* Analyzer::new_entity<AliasTemplateEntity>(
	EntityKind, const Identifier*, Scope*, const Type*);

const char* Analyzer::copy_label(const char* text, size_t length)
{
	return arena_.copy_string(text, length);
}

const Identifier* Analyzer::special_key(const char* spelling)
{
	return identifiers_.intern(spelling, static_cast<uint32_t>(std::strlen(spelling)));
}

void Analyzer::error(uint32_t location, const char* message) const
{
	// One found in types (a substitution's) has no location of its own.
	throw SemanticError(location != 0 ? location : substitution_location_, message);
}

const char Analyzer::kDeferredCall[] = "a deferred call of a reserved builtin";

void Analyzer::defer_reserved_call(uint32_t location)
{
	failure_ = SubstitutionFailure{ kDeferredCall, location, false };
}

bool Analyzer::substitution_error(uint32_t location, const char* message, bool transient)
{
	if (soft_substitution_ == 0)
		error(location, message);
	if (failure_.message == nullptr)
		failure_ = SubstitutionFailure{ message, location, transient };
	failure_.transient = failure_.transient || transient;
	return false;
}

bool Analyzer::stack_half_used() const
{
	char marker;
	return stack_bytes_used(stack_base_, &marker) > stack_budget_ / 2;
}

Scope* Analyzer::declaration_scope(Scope* scope)
{
	while (scope->kind == ScopeKind::TemplateParameters)
		scope = scope->parent;
	return scope;
}

Scope* Analyzer::enclosing_namespace(Scope* scope)
{
	while (scope->kind != ScopeKind::Namespace)
		scope = scope->parent;
	return scope;
}

bool Analyzer::hidden_below_namespace(const Scope* block, const Identifier* name) const
{
	// 3.5p6 asks for a visible declaration: one of the scopes between the
	// block and its innermost enclosing namespace (enclosing blocks,
	// parameters, a local class's members) declaring the name hides the
	// namespace's.
	for (const Scope* scope = block->parent; scope != nullptr && scope->kind != ScopeKind::Namespace;
		scope = scope->parent)
	{
		if (names_.own(scope, name, LookupFilter::Ordinary).found())
			return true;
	}
	return false;
}

bool Analyzer::encloses(const Scope* outer, const Scope* inner)
{
	for (; inner != nullptr; inner = inner->parent)
	{
		if (inner == outer)
			return true;
	}
	return false;
}

// ------------------------------------------------------------ declarations

void Analyzer::declarations(const syntax::Slice<syntax::Decl*>& list, Scope* scope)
{
	for (const syntax::Decl* decl : list)
		declaration(*decl, scope);
}

void Analyzer::declaration(const syntax::Decl& decl, Scope* scope)
{
	check_stack(decl.location);
	switch (decl.kind)
	{
	case syntax::DeclKind::Empty:
		return;
	case syntax::DeclKind::Asm:
		return asm_declaration(static_cast<const syntax::AsmDecl&>(decl), scope);
	case syntax::DeclKind::Access:
		current_access_ = static_cast<const syntax::AccessDecl&>(decl).access;
		return;
	case syntax::DeclKind::Namespace:
		return namespace_definition(static_cast<const syntax::NamespaceDecl&>(decl), scope);
	case syntax::DeclKind::NamespaceAlias:
		return namespace_alias(static_cast<const syntax::NamespaceAliasDecl&>(decl), scope);
	case syntax::DeclKind::UsingDirective:
		return using_directive(static_cast<const syntax::UsingDecl&>(decl), scope);
	case syntax::DeclKind::UsingDeclaration:
		return using_declaration(static_cast<const syntax::UsingDecl&>(decl), scope);
	case syntax::DeclKind::Alias:
		return alias_declaration(static_cast<const syntax::AliasDecl&>(decl), scope);
	case syntax::DeclKind::ExplicitInstantiation:
		return explicit_instantiation(static_cast<const syntax::ExplicitInstantiationDecl&>(decl), scope);
	case syntax::DeclKind::Template:
		return template_declaration(static_cast<const syntax::TemplateDecl&>(decl), scope);
	case syntax::DeclKind::Class:
	case syntax::DeclKind::Enum:
	{
		// A lone class or enum specifier is a simple-declaration without
		// declarators.
		syntax::DeclSpecifier specifier = syntax::DeclSpecifier();
		if (decl.kind == syntax::DeclKind::Class)
			specifier = static_cast<const syntax::ClassDecl&>(decl).specifier;
		else
		{
			specifier.kind = syntax::DeclSpecKind::Enum;
			specifier.location = decl.location;
			specifier.enum_specifier = static_cast<const syntax::EnumDecl&>(decl).enum_specifier;
		}
		syntax::SimpleDecl simple = syntax::SimpleDecl();
		simple.kind = syntax::DeclKind::Simple;
		simple.location = decl.location;
		simple.specifiers = syntax::Slice<syntax::DeclSpecifier>{ &specifier, 1 };
		simple.declarators = syntax::Slice<syntax::InitDeclarator>{ nullptr, 0 };
		return simple_declaration(simple, scope);
	}
	case syntax::DeclKind::StaticAssert:
		return static_assertion(static_cast<const syntax::StaticAssertDecl&>(decl), scope);
	case syntax::DeclKind::SpecialMember:
		return special_member(static_cast<const syntax::SpecialMemberDecl&>(decl), scope);
	case syntax::DeclKind::Function:
		return function_definition(static_cast<const syntax::FunctionDecl&>(decl), scope);
	case syntax::DeclKind::Simple:
		return simple_declaration(static_cast<const syntax::SimpleDecl&>(decl), scope);
	case syntax::DeclKind::Linkage:
	{
		// 7.5: the declarations get the named language linkage; a single
		// declaration is also treated as extern (p7).
		const syntax::LinkageDecl& linkage = static_cast<const syntax::LinkageDecl&>(decl);
		const bool saved_c = c_language_;
		const bool saved_extern = linkage_extern_;
		const bool saved_specified = linkage_specified_;
		c_language_ = linkage.language.length == 1 && linkage.language.text[0] == 'C';
		linkage_extern_ = !linkage.braced;
		linkage_specified_ = true;
		declarations(linkage.declarations, scope);
		c_language_ = saved_c;
		linkage_extern_ = saved_extern;
		linkage_specified_ = saved_specified;
		return;
	}
	case syntax::DeclKind::BitField:
		return bit_field_declaration(static_cast<const syntax::BitFieldDecl&>(decl), scope);
	}
}

void Analyzer::namespace_definition(const syntax::NamespaceDecl& decl, Scope* scope)
{
	if (scope->kind != ScopeKind::Namespace)
		error(decl.location, "a namespace can only be defined in a namespace");
	NamespaceEntity* parent = static_cast<NamespaceEntity*>(scope->entity);
	const Identifier* key = decl.name != nullptr ? decl.name : special_key("<unnamed namespace>");
	Scope* target = nullptr;
	if (const NameEntry* entry = names_.find(scope, key))
	{
		// An extension of an original namespace-definition of this
		// declarative region (7.3.1p2); an alias names a namespace but is not
		// one (7.3.2), and no other entity may share the name.
		if (entry->type != nullptr && entry->type->kind == BindingKind::Namespace)
		{
			NamespaceEntity* existing = static_cast<NamespaceEntity*>(entry->type->entity);
			if (decl.is_inline && !existing->is_inline)
				error(decl.location, "an extension cannot make a namespace inline");
			target = existing->scope;
		}
		else if (entry->type != nullptr || entry->value != nullptr)
			error(decl.location,
				entry->type != nullptr && entry->type->kind == BindingKind::NamespaceAlias
					? "a namespace alias cannot be extended"
					: "namespace name conflicts with an earlier declaration");
	}
	if (target == nullptr)
	{
		NamespaceEntity* entity = new_entity<NamespaceEntity>(EntityKind::Namespace, decl.name, scope, nullptr);
		entity->depth = parent->depth + 1;
		entity->is_inline = decl.is_inline;
		entity->internal = decl.name == nullptr || parent->internal;
		target = new_scope(
			ScopeKind::Namespace, scope, scope, entity, decl.name != nullptr ? decl.name->spelling : "<unnamed>");
		entity->scope = target;
		Binding* binding = new_binding(BindingKind::Namespace, key, entity, nullptr);
		binding->hidden = true;
		declare_in(scope, binding);
		// Unnamed and inline namespaces are nominated by the enclosing one
		// (7.3.1.1p1, 7.3.1p8); inline ones also join its inline set.
		if (decl.name == nullptr || decl.is_inline)
		{
			NamespaceList* directive = static_cast<NamespaceList*>(
				arena_.allocate(sizeof(NamespaceList), alignof(NamespaceList)));
			*directive = NamespaceList{ target, scope->directives, ++graph_.bindings };
			scope->directives = directive;
		}
		if (decl.is_inline)
		{
			NamespaceList* member = static_cast<NamespaceList*>(
				arena_.allocate(sizeof(NamespaceList), alignof(NamespaceList)));
			*member = NamespaceList{ target, parent->inline_members, graph_.bindings };
			parent->inline_members = member;
		}
	}
	// GNU abi_tag on an inline namespace: not mangled, but its members'
	// names carry the tags for implicit tagging (GCC's documentation);
	// ignored on another namespace (GCC and Clang warn).
	if (static_cast<NamespaceEntity*>(target->entity)->is_inline && decl.attributes != nullptr)
	{
		std::vector<std::string> tags;
		collect_abi_tags(decl.attributes, false, tags, decl.name);
		if (!tags.empty())
			declare_abi_tags(target, tags, false, decl.location);
	}
	declarations(decl.declarations, target);
}

NamespaceEntity* Analyzer::namespace_target(const syntax::Name& name, Scope* scope)
{
	const syntax::NameComponent& last = name.last();
	if (last.kind != syntax::NameComponentKind::Identifier || last.has_arguments)
		error(name.location, "expected a namespace name");
	LookupResult result = lookup_name(name, scope, LookupFilter::Namespace);
	if (result.binding->kind == BindingKind::NamespaceAlias)
		return static_cast<NamespaceAliasEntity*>(result.binding->entity)->target;
	return static_cast<NamespaceEntity*>(result.binding->entity);
}

void Analyzer::namespace_alias(const syntax::NamespaceAliasDecl& decl, Scope* scope)
{
	NamespaceEntity* target = namespace_target(*decl.target, scope);
	if (const NameEntry* entry = names_.find(scope, decl.name))
	{
		// Redefining an alias to denote the same namespace is allowed.
		if (entry->type != nullptr && entry->type->kind == BindingKind::NamespaceAlias &&
			static_cast<NamespaceAliasEntity*>(entry->type->entity)->target == target)
			return;
		if (entry->type != nullptr || entry->value != nullptr)
			error(decl.location, "namespace alias conflicts with an earlier declaration");
	}
	NamespaceAliasEntity* alias = new_entity<NamespaceAliasEntity>(
		EntityKind::NamespaceAlias, decl.name, scope, nullptr);
	alias->target = target;
	Binding* binding = new_binding(BindingKind::NamespaceAlias, decl.name, alias, nullptr);
	binding->hidden = true;
	declare_in(scope, binding);
}

void Analyzer::using_directive(const syntax::UsingDecl& decl, Scope* scope)
{
	if (scope->kind == ScopeKind::Class)
		error(decl.location, "a using-directive cannot appear in a class");
	NamespaceEntity* target = namespace_target(*decl.target, scope);
	NamespaceList* directive = static_cast<NamespaceList*>(
		arena_.allocate(sizeof(NamespaceList), alignof(NamespaceList)));
	*directive = NamespaceList{ target->scope, scope->directives, ++graph_.bindings };
	scope->directives = directive;
}

void Analyzer::using_declaration(const syntax::UsingDecl& decl, Scope* scope)
{
	const syntax::Name& target = *decl.target;
	const syntax::NameComponent& last = target.last();
	if (!target.qualified())
		error(decl.location, "a using-declaration needs a nested-name-specifier");
	if (last.has_arguments)
		error(decl.location, "a using-declaration cannot name a template-id");
	if (last.kind == syntax::NameComponentKind::Destructor || last.kind == syntax::NameComponentKind::Decltype)
		error(decl.location, "a using-declaration cannot name a destructor");
	const NamePrefix prefix = name_prefix(target, scope);
	// 3.4.3.1p2: in a member using-declaration, a name equal to the last
	// nested-name-specifier component's (`using Alias::Alias;`, `using
	// B<T>::B;`) names the constructors (12.9).
	const syntax::NameComponent* owner = target.components.size >= 2 ? &target.components[target.components.size - 2]
																	 : nullptr;
	const bool names_owner = owner != nullptr && owner->kind == syntax::NameComponentKind::Identifier &&
		last.kind == syntax::NameComponentKind::Identifier && owner->identifier == last.identifier &&
		declaration_scope(scope)->kind == ScopeKind::Class;
	// A member of a dependent base (14.6.2): a placeholder until the
	// instantiations, which declare what it names (unless the scope
	// already declares the name); the constructors of one are inherited by
	// the instantiations, the class declaring no member of their name (a
	// later `B<T>` still names the class template).
	if (prefix.dependent != nullptr)
	{
		if (last.kind != syntax::NameComponentKind::Identifier || names_owner)
			return;
		Scope* where = declaration_scope(scope);
		if (names_.find(where, last.identifier) == nullptr)
			declare_in(where, dependent_member_binding(prefix.dependent, last.identifier));
		return;
	}
	// `using B::B;` names B's constructors, which it inherits (12.9): no
	// name is introduced.
	if (prefix.entity != nullptr && prefix.entity->kind == EntityKind::Class &&
		last.kind == syntax::NameComponentKind::Identifier && (last.identifier == prefix.entity->name || names_owner))
	{
		Scope* where = declaration_scope(scope);
		if (where->kind == ScopeKind::Class && where->entity != nullptr)
			static_cast<ClassEntity*>(where->entity)->inherits_constructors = static_cast<ClassEntity*>(prefix.entity);
		return;
	}
	LookupResult result = LookupResult();
	if (last.kind == syntax::NameComponentKind::Identifier)
		result = lookup_in(prefix, last.identifier, LookupFilter::Ordinary);
	// A builtin is a global-namespace name, declared when first named:
	// `using ::__builtin_x;` too.
	if (!result.found() && target.global && target.components.size == 1 &&
		last.kind == syntax::NameComponentKind::Identifier)
		result = builtin_lookup(last.identifier, result);
	else if (prefix.scope != nullptr)
	{
		const Identifier* key = declared_name(target, prefix.scope).key;
		implicit_members_for(prefix.scope, key);
		result = names_.qualified(prefix.scope, key, LookupFilter::Ordinary);
	}
	if (!result.found() && decl.if_exists)
		return;
	if (!result.found())
		error(decl.location, "undeclared name in a using-declaration");
	if (result.ambiguous)
		error(decl.location, "ambiguous name in a using-declaration");
	if (IsNamespaceBinding(result.binding))
		error(decl.location, "a using-declaration cannot name a namespace");
	// 7.3.3p17: each declaration it introduces is accessible where it
	// appears, named in the class its nested-name-specifier names (11.2p5).
	// Within a templated class of a dependent base the classes the point's
	// class derives from are unknown (14.6.2p3): the instantiation, which
	// declares the using-declaration again, checks it.
	const ClassEntity* naming = prefix.scope != nullptr && prefix.scope->kind == ScopeKind::Class
		? static_cast<const ClassEntity*>(prefix.scope->entity)
		: nullptr;
	for (const Scope* s = scope; s != nullptr && naming != nullptr; s = s->parent)
	{
		if (has_dependent_bases(s))
			naming = nullptr;
	}
	if (result.binding->kind != BindingKind::Function)
	{
		check_access(result.binding, naming, nullptr, scope, decl.location);
		introduce(result.binding, scope, decl.location);
		return;
	}
	// Every function of the set, in declaration order; of the conversion
	// functions, those of the named conversion type (12.3.2p1), which is
	// looked up where the using-declaration appears.
	const Type* conversion = nullptr;
	if (last.kind == syntax::NameComponentKind::Conversion)
		conversion = type_id(*last.conversion, scope)->canonical;
	std::vector<Binding*> functions;
	for (FunctionList* function = result.functions; function != nullptr; function = function->next)
	{
		if (!function->binding->invisible &&
			(conversion == nullptr || function->binding->entity->type->canonical->element == conversion))
			functions.push_back(function->binding);
	}
	if (functions.empty())
		error(decl.location, "undeclared name in a using-declaration");
	for (const Binding* function : functions)
		check_access(function, naming, nullptr, scope, decl.location);
	for (size_t i = functions.size(); i-- > 0;)
		introduce(functions[i], scope, decl.location);
}

void Analyzer::check_template_parameter_name(Scope* scope, const Identifier* name, uint32_t location)
{
	// 14.6.1p6: a template parameter is not redeclared within its scope,
	// nested scopes included, and a template is not named as one of its
	// parameters (checked where the template is defined). The declaration
	// at `scope` is enclosed by the parameter scopes on its lookup chain up
	// to a synthesized one: a qualified declarator's bridge into its class
	// holds the enclosing template declarations' parameters, and the class
	// template's own beyond it do not enclose an out-of-class member
	// definition (14.6.1p7).
	if (template_depth_ == 0 || name == nullptr)
		return;
	for (const Scope* current = scope; current != nullptr && current->kind != ScopeKind::Namespace;
		current = current->parent)
	{
		if (current->kind != ScopeKind::TemplateParameters)
			continue;
		const NameEntry* entry = names_.find(current, name);
		if (entry != nullptr && entry->type != nullptr && entry->type->entity != nullptr &&
			entry->type->entity->kind == EntityKind::TemplateParameter &&
			static_cast<const TemplateParameterEntity*>(entry->type->entity)->qualifier == nullptr)
			error(location, "a template parameter redeclared within its scope");
		if (!current->dumped)
			return;
	}
}

void Analyzer::introduce(Binding* target, Scope* scope, uint32_t location)
{
	// The declaration a using-declaration introduces has the same entity and
	// prints as the declaration it names (7.3.3p1).
	Scope* where = declaration_scope(scope);
	check_template_parameter_name(scope, target->name, location);
	if (const NameEntry* entry = names_.find(where, target->name))
	{
		const Binding* value = entry->value;
		const bool same_value = value != nullptr && value->entity == target->entity;
		const bool same_type = entry->type != nullptr && entry->type->entity == target->entity;
		// 14p5: a class template's name is unique in its scope, also among
		// the declarations using-declarations introduce.
		const bool type_target = target->kind == BindingKind::Type || target->kind == BindingKind::TypeAlias;
		if (type_target ? IsClassTemplate(target) && value != nullptr : IsClassTemplate(entry->type))
			error(location, "using-declaration conflicts with a class template of the same name");
		if (target->kind == BindingKind::Function)
		{
			if (value != nullptr && value->kind != BindingKind::Function)
				error(location, "using-declaration conflicts with an earlier declaration");
			// A member of the class hides a base member of the same signature
			// (7.3.3p15); elsewhere a different function of that signature
			// conflicts (7.3.3p14).
			// A member template of the class hides a base member template of
			// the same function type and an equivalent head likewise.
			const FunctionEntity* introduced = static_cast<const FunctionEntity*>(target->entity);
			const FunctionList* same = introduced->templated
				? (HeadOf(introduced) != nullptr
						  ? names_.overload(where, target->name, introduced->signature, HeadOf(introduced)->shape)
						  : nullptr)
				: names_.overload(where, target->name, introduced->signature);
			if (same != nullptr && same->binding->entity != introduced)
			{
				if (where->kind == ScopeKind::Class && same->binding->entity->owner == where)
					return;
				error(location, "using-declaration conflicts with a function of the same signature");
			}
		}
		else if (target->kind == BindingKind::Type || target->kind == BindingKind::TypeAlias)
		{
			if (!same_type && entry->type != nullptr)
				error(location, "using-declaration conflicts with an earlier declaration");
		}
		else if (!same_value && value != nullptr)
			error(location, "using-declaration conflicts with an earlier declaration");
	}
	// A class a base's member template is introduced into: its own member
	// templates' redeclarations search its overload sets for such (see
	// declare_function).
	if (where->kind == ScopeKind::Class && target->kind == BindingKind::Function &&
		static_cast<const FunctionEntity*>(target->entity)->templated)
		introduced_templates_.set(where, 1);
	// A member a lookup may find through declarations of different access
	// (see accessible_declarations).
	if (where->kind == ScopeKind::Class && target->name != nullptr)
	{
		where->using_members = true;
		if (target->name->id >= using_names_.size())
			using_names_.resize(std::max<size_t>(target->name->id + 1, using_names_.size() * 2));
		using_names_[target->name->id] = true;
	}
	Binding* binding = new_binding(target->kind, target->name, target->entity, target->type);
	binding->key = target->key;
	binding->scoped = target->scoped;
	binding->label = target->label;
	binding->declaration = target->declaration;
	print_in(scope, binding);
	declare_in(where, binding);
}

void Analyzer::alias_declaration(const syntax::AliasDecl& decl, Scope* scope)
{
	// An unnamed class or enumeration its type-id defines keeps its
	// synthesized name: the course dump names classes only from typedef
	// declarators (7.1.3p9) and objects, not from alias-declarations.
	reject_noreturn(decl.attributes, "noreturn attribute on an alias-declaration");
	if (template_scope_ != nullptr && scope == template_scope_)
		return declare_alias_template(decl, scope);
	Specifiers specifiers;
	this->specifiers(decl.type->specifiers, scope, specifiers, false);
	// A deferred reserved builtin call (see deferrable_statement).
	if (deferred_reserved_call())
		return;
	if (specifiers.type == nullptr || specifiers.type == auto_placeholder_)
		error(decl.location, "alias-declaration without a type");
	const DeclaratorInfo info = declarator(decl.type->declarator, specifiers.type, scope, decl.location);
	const Type* type = info.type;
	// 9.2p9, 7.1.2p1: no virt-specifier or function-specifier here.
	if (info.virt_suffix != nullptr || specifiers.is_virtual)
		error(decl.location, "a virt-specifier or virtual in an alias-declaration");
	// 15.4p2: nor an exception-specification.
	if (DeclaratorSpecifiesExceptions(decl.type->declarator))
		error(decl.location, "an exception-specification in an alias-declaration");
	if (specifiers.declared != nullptr && specifiers.defines && specifiers.declared->name == nullptr)
		name_unnamed_type(specifiers, nullptr, scope);
	DeclaredName name = DeclaredName{ decl.name, nullptr, syntax::NameComponentKind::Identifier };
	decl.declared = declare_typedef(name, type, scope, decl.location);
	bool packed = false, no_inline = false;
	const uint64_t aligned = decl.attributes != nullptr && template_depth_ == 0
		? attribute_alignment(decl.attributes, scope, packed, no_inline)
		: 0;
	if (aligned != 0 || (specifiers.alias_alignment != 0 && type == specifiers.type))
		record_alias_alignment(specifiers, decl.declared, aligned);
}

void Analyzer::template_declaration(const syntax::TemplateDecl& decl, Scope* scope)
{
	// 14p3: a template-declaration or explicit specialization has at most
	// one declarator.
	if (decl.declaration->kind == syntax::DeclKind::Simple &&
		static_cast<const syntax::SimpleDecl*>(decl.declaration)->declarators.size > 1)
		error(decl.location, "a template declaration must declare a single entity");
	if (decl.parameters->parameters.empty())
		return explicit_specialization(decl, scope);
	// 14p2: a template-declaration is a namespace-scope or class-scope
	// declaration; 14p4: a template does not have C language linkage (a
	// class member's is ignored, 7.5p4).
	const ScopeKind home = declaration_scope(scope)->kind;
	if (home != ScopeKind::Namespace && home != ScopeKind::Class)
		error(decl.location, "a template declared in a block scope");
	// 14.5.2p2: a local class has no member templates (14.5.4p7: nor friend
	// templates); a templated lambda's closure has its call operator.
	const ClosureFacts* closure = home == ScopeKind::Class
		? graph_.closure_facts(static_cast<const ClassEntity*>(declaration_scope(scope)->entity))
		: nullptr;
	for (const Scope* s = declaration_scope(scope); home == ScopeKind::Class && s != nullptr &&
		s->kind != ScopeKind::Namespace && (closure == nullptr || closure->lambda == nullptr);
		s = s->parent)
	{
		if (s->kind == ScopeKind::Block || s->kind == ScopeKind::Function)
			error(decl.location, "a member template of a local class");
	}
	if (c_language_ && home == ScopeKind::Namespace)
		error(decl.location, "a template with C language linkage");
	// The parameters' scope holds the templated declaration's line and
	// scope; the declared name itself belongs to the enclosing scope. A
	// friend template of a class that is not templated (an instance's
	// included) is a member of its namespace (14.5.4): its parameters are
	// positioned as that namespace's templates' are.
	Scope* parameters = new_scope(ScopeKind::TemplateParameters, scope, scope, nullptr, nullptr);
	std::vector<TemplateParameterEntity*> entities;
	const bool namespace_friend = home == ScopeKind::Class && FriendDeclaration(*decl.declaration) &&
		!templated_class(declaration_scope(scope));
	Scope* saved_home = placeholder_home_;
	const Scope* saved_friend = namespace_friend_class_;
	if (namespace_friend)
		namespace_friend_class_ = declaration_scope(scope);
	placeholder_home_ = member_template_home(decl, scope);
	template_parameters(*decl.parameters, parameters, true, &entities, namespace_friend ? 0 : -1);
	placeholder_home_ = saved_home;
	const syntax::TemplateParameterList* saved_list = template_list_;
	Scope* saved_scope = template_scope_;
	ClassEntity* saved_owner = template_owner_;
	template_entities_.swap(entities);
	template_list_ = decl.parameters;
	template_scope_ = parameters;
	template_owner_ = qualifying_template(*decl.declaration, parameters);
	// 14.1p9: the template-parameter-list of a class template member's
	// definition outside its class gives no default template argument.
	for (uint32_t i = 0; template_owner_ != nullptr && i < decl.parameters->parameters.size; ++i)
	{
		const syntax::TemplateParameter* parameter = decl.parameters->parameters[i];
		if (parameter->default_type != nullptr || parameter->default_value != nullptr)
			error(parameter->location, "a default template argument on a member defined outside its class template");
	}
	++template_depth_;
	declaration(*decl.declaration, parameters);
	--template_depth_;
	namespace_friend_class_ = saved_friend;
	template_list_ = saved_list;
	template_scope_ = saved_scope;
	template_owner_ = saved_owner;
	template_entities_.swap(entities);
}

bool Analyzer::FriendDeclaration(const syntax::Decl& decl)
{
	// A declaration whose decl-specifiers include `friend`.
	const syntax::Slice<syntax::DeclSpecifier>* specifiers = nullptr;
	if (decl.kind == syntax::DeclKind::Simple)
		specifiers = &static_cast<const syntax::SimpleDecl&>(decl).specifiers;
	else if (decl.kind == syntax::DeclKind::Function)
		specifiers = &static_cast<const syntax::FunctionDecl&>(decl).specifiers;
	else if (decl.kind == syntax::DeclKind::SpecialMember)
		specifiers = &static_cast<const syntax::SpecialMemberDecl&>(decl).specifiers;
	for (uint32_t i = 0; specifiers != nullptr && i < specifiers->size; ++i)
	{
		if ((*specifiers)[i].kind == syntax::DeclSpecKind::Keyword && (*specifiers)[i].keyword == TokenKind::KW_FRIEND)
			return true;
	}
	return false;
}

void Analyzer::template_parameters(const syntax::TemplateParameterList& list, Scope* scope, bool declare,
	std::vector<TemplateParameterEntity*>* out, int32_t base_depth)
{
	// Each parameter has its position (14.5.6.1): the number of enclosing
	// template parameter lists (unless the caller gives it) and its index,
	// unnamed ones included.
	uint32_t depth = 0;
	for (const Scope* enclosing = scope->parent; base_depth < 0 && enclosing != nullptr; enclosing = enclosing->parent)
		depth += enclosing->lists;
	if (base_depth >= 0)
		depth = static_cast<uint32_t>(base_depth);
	uint32_t index = 0;
	for (const syntax::TemplateParameter* parameter : list.parameters)
	{
		const bool template_template = parameter->kind == syntax::TemplateParameterKind::Template;
		const bool non_type = parameter->kind == syntax::TemplateParameterKind::NonType;
		// C++11 14.1p1: a template template-parameter is introduced by class.
		if (template_template && parameter->key == TokenKind::KW_TYPENAME)
			error(parameter->location, "a template template-parameter introduced by typename");
		const TemplateHead* own_head = nullptr;
		if (template_template)
		{
			// Its own parameters are visible only in its parameter list.
			Scope* own = new_scope(ScopeKind::TemplateParameters, scope, nullptr, nullptr, nullptr);
			own->dumped = false;
			std::vector<TemplateParameterEntity*> inner;
			template_parameters(*parameter->parameters, own, false, &inner, base_depth >= 0 ? base_depth + 1 : -1);
			own_head = template_head(*parameter->parameters, own, inner);
		}
		const Identifier* name = parameter->name;
		const Type* value_type = nullptr;
		if (non_type)
		{
			// Its type may depend on earlier parameters (a template
			// definition's names, 14.6).
			name = TemplateParameterName(*parameter);
			++template_depth_;
			value_type = non_type_parameter_type(*parameter, scope);
			--template_depth_;
		}
		if (name != nullptr && names_.find(scope, name) != nullptr)
			error(parameter->location, "template parameter redeclared");
		// 14.6.1p6: nor one of an enclosing template's (a member template's
		// parameter named as its class template's).
		if (declare && name != nullptr && scope->parent != nullptr)
			check_template_parameter_name(scope->parent, name, parameter->location);
		if (declare && !template_template && parameter->default_type != nullptr)
			default_template_argument_names(*parameter->default_type, scope);
		// A template template-parameter's default names a template (14.1p9).
		if (declare && template_template && parameter->default_type != nullptr)
		{
			const syntax::TemplateArgument argument = syntax::TemplateArgument{ parameter->default_type, nullptr,
				false };
			const syntax::Name* name = PlainTemplateName(argument);
			if (name == nullptr || template_name_argument(*name, scope, true) == nullptr)
				error(parameter->location, "a template template-parameter's default names no template");
		}
		if (declare && parameter->default_value != nullptr)
		{
			++template_depth_;
			expression_walk(parameter->default_value, scope);
			--template_depth_;
		}
		// 14.1p11: a parameter pack has no default argument.
		const bool pack = TemplateParameterPack(*parameter);
		if (pack && (parameter->default_type != nullptr || parameter->default_value != nullptr))
			error(parameter->location, "a template parameter pack with a default argument");
		TemplateParameterEntity* entity = new_entity<TemplateParameterEntity>(
			EntityKind::TemplateParameter, name, scope, nullptr);
		entity->template_template = template_template;
		entity->non_type = non_type;
		entity->pack = pack;
		entity->value_type = value_type;
		entity->depth = static_cast<uint16_t>(depth);
		entity->index = static_cast<uint16_t>(index++);
		entity->template_head = own_head;
		entity->canonical = template_template ? positional_template_parameter(depth, entity->index, pack, own_head)
											  : positional_parameter(depth, entity->index, non_type, pack);
		entity->type = types_.named(
			template_template ? TypeKind::TemplateTemplateParameter : TypeKind::TemplateParameter, entity);
		if (out != nullptr)
			out->push_back(entity);
		if (name == nullptr)
			continue;
		Binding* binding = non_type ? new_binding(BindingKind::TemplateValue, name, entity, value_type)
									: new_binding(BindingKind::Type, name, entity, entity->type);
		binding->hidden = !declare || non_type;
		// Listed in a template template-parameter's own scope too (not
		// dumped): a default there names the earlier ones, bound by a frame
		// (see Analyzer::template_default).
		print_in(scope, binding);
		declare_in(scope, binding);
	}
}

bool Analyzer::TemplateParameterPack(const syntax::TemplateParameter& parameter)
{
	// A template parameter pack: `...` after the decl-specifiers, or (a
	// non-type parameter's `T&... r`) in its declarator.
	return parameter.pack ||
		(parameter.kind == syntax::TemplateParameterKind::NonType && parameter.declarator != nullptr &&
			parameter.declarator->pack != syntax::PackPosition::None);
}

const Type* Analyzer::non_type_parameter_type(const syntax::TemplateParameter& parameter, Scope* scope)
{
	// 14.1p4-p5: an integral or enumeration type, a pointer to object or
	// function, an lvalue reference to object or function, a pointer to
	// member or std::nullptr_t, or one that depends on an
	// earlier parameter; top-level cv-qualifiers are ignored, and an array
	// or function type is adjusted to a pointer (14.1p8).
	Specifiers specifiers;
	this->specifiers(parameter.specifiers, scope, specifiers, false);
	if (specifiers.type == nullptr || specifiers.type == auto_placeholder_)
		error(parameter.location, "non-type template parameter without a type");
	// A pack's `...` in the declarator declares the pack, not an expansion.
	syntax::Declarator bare = parameter.declarator != nullptr ? *parameter.declarator : syntax::Declarator();
	bare.pack = syntax::PackPosition::None;
	const DeclaratorInfo info = declarator(
		parameter.declarator != nullptr ? &bare : nullptr, specifiers.type, scope, parameter.location);
	const Type* type = info.type->unqualified;
	if (type->kind == TypeKind::Array)
		type = types_.pointer(type->element);
	else if (type->kind == TypeKind::Function)
		type = types_.pointer(type);
	if (dependent_type(type))
		return type;
	if (!IsIntegralType(type) && type->kind != TypeKind::Enum && !AddressParameterType(type))
		error(parameter.location, "a non-type template parameter of a type 14.1p4 does not allow");
	return type;
}

void Analyzer::static_assertion(const syntax::StaticAssertDecl& decl, Scope* scope)
{
	if (semantic() && decl.independent_condition && decl.condition_checked)
		return;
	// 7p1: the message is a string-literal, which a user-defined string
	// literal (2.14.8, its token's kind) is not.
	if (decl.has_message && decl.user_defined_message)
		error(decl.location, "a static_assert message that is not a string literal");
	// A dependent condition waits for instantiation, which is outside PA6;
	// its non-dependent names are bound where they appear (14.6p10).
	if (template_depth_ != 0)
	{
		expression_walk(decl.condition, scope);
		if (semantic() && !failed())
			decl.independent_condition = !instantiation_dependent(*decl.condition, scope, true) &&
				fixed_expression(*decl.condition, scope);
		return;
	}
	// Semantics mode types the condition first (overloaded calls in
	// unevaluated operands, 5.3.3), contextually converted to bool (7p4):
	// the conversion is evaluated with it.
	Constant value;
	if (semantic())
	{
		BodyContext* saved = body_;
		BodyContext context = BodyContext();
		if (body_ == nullptr)
			body_ = &context;
		typed_value(*decl.condition, scope);
		// A deferred reserved builtin call (see deferrable_statement).
		if (deferred_reserved_call())
		{
			body_ = saved;
			return;
		}
		if (IsScopedEnumeration(decl.condition->facts->type))
			error(decl.condition->location, "a scoped enumeration value does not convert to bool");
		convert_to_bool(*decl.condition);
		body_ = saved;
		converted_constant(*decl.condition, value, true);
	}
	else
		constant_value(*decl.condition, scope, value, true);
	if (value.type->kind == TypeKind::Enum && static_cast<const EnumEntity*>(value.type->entity)->scoped)
		error(decl.condition->location, "a scoped enumeration value does not convert to bool");
	if (value.bits == 0)
		error(decl.location, "static assertion failed");
	if (semantic() && decl.independent_condition)
		decl.condition_checked = true;
}

} // namespace sema
