#include "sema/analyzer.h"
#include "sema/template_packs.h"

#include <algorithm>
#include <cstring>
#include <string>

#include "lex/identifier_table.h"
#include "lex/text_literal.h"
#include "preprocess/preprocessor.h"
#include "support/internal_error.h"
#include "syntax/ast_render.h"

namespace sema {

const char* LiteralOperatorSuffix(const FunctionEntity* function)
{
	return function->name->spelling + sizeof kLiteralOperatorPrefix - 1;
}

namespace {

// Declarations of one variable agree on its type, except that an array's
// major bound may be omitted in some (3.5p10, 3.9p3).
bool CompatibleVariableTypes(const Type* a, const Type* b)
{
	if (a->canonical == b->canonical)
		return true;
	return a->kind == TypeKind::Array && b->kind == TypeKind::Array && a->element->canonical == b->element->canonical &&
		(a->bound == 0 || b->bound == 0);
}

const syntax::Name* DeclaratorId(const syntax::Declarator* declarator)
{
	for (; declarator != nullptr; declarator = declarator->nested)
	{
		if (declarator->name != nullptr)
			return declarator->name;
	}
	return nullptr;
}

// The function declarator whose type is the declaration's: the one
// Analyzer::declarator applies last (each level's ptr-operators, then its
// suffixes from the last), or null when that is no function suffix.
const syntax::DeclaratorSuffix* OwnFunctionSuffix(const syntax::Declarator* declarator)
{
	const syntax::DeclaratorSuffix* own = nullptr;
	for (; declarator != nullptr; declarator = declarator->nested)
	{
		if (declarator->suffixes.size != 0)
			own = &declarator->suffixes[0];
		else if (!declarator->pointers.empty())
			own = nullptr;
	}
	return own != nullptr && own->kind == syntax::DeclaratorSuffixKind::Function ? own : nullptr;
}

// The first alignment-specifier (7.6.2) of an attribute list, GNU aligned
// apart.
const syntax::Attribute* AlignmentSpecifier(const syntax::Attribute* list)
{
	for (; list != nullptr; list = list->next)
	{
		if (list->kind == syntax::AttributeKind::Alignas && !list->gnu)
			return list;
	}
	return nullptr;
}

// Whether an attribute list, or a declarator's (its parts'), holds the
// standard `[[noreturn]]` (GNU noreturn apart).
bool StandardNoReturn(const syntax::Attribute* list)
{
	for (; list != nullptr; list = list->next)
	{
		if (list->kind == syntax::AttributeKind::NoReturn && list->standard)
			return true;
	}
	return false;
}

bool StandardNoReturn(const syntax::Declarator* declarator)
{
	for (; declarator != nullptr; declarator = declarator->nested)
	{
		if (StandardNoReturn(declarator->attributes))
			return true;
	}
	return false;
}

bool HasDefaultArgument(const syntax::ParameterClause* clause)
{
	for (uint32_t i = 0; clause != nullptr && i < clause->parameters.size; ++i)
	{
		if (clause->parameters[i]->default_argument != nullptr)
			return true;
	}
	return false;
}

bool IsClassObjectType(const Type* type)
{
	if (type->is_reference())
		type = type->element;
	while (type->kind == TypeKind::Array)
		type = type->element;
	return type->unqualified->kind == TypeKind::Class;
}

// The number of code units of a string literal's value with its null.
uint64_t StringLiteralLength(const TokenSpelling& spelling)
{
	uint64_t total = 1;
	const char* text = spelling.text;
	uint32_t length = spelling.length;
	// Concatenated pieces are separated by single spaces.
	uint32_t begin = 0;
	while (begin < length)
	{
		const uint32_t end = TextLiteralPieceEnd(text, length, begin);
		TextLiteralParts parts;
		if (!SplitTextLiteral(text + begin, end - begin, parts))
			return 0;
		if (parts.raw)
			total += parts.body_end - parts.body_begin;
		else
		{
			uint32_t position = parts.body_begin;
			while (position < parts.body_end)
			{
				LiteralCharacter character;
				if (!DecodeLiteralCharacter(text + begin, parts.body_end, position, character))
					return 0;
				++total;
			}
		}
		begin = end + 1;
	}
	return total;
}

} // namespace

// ------------------------------------------------------------------ names

DeclaredName Analyzer::declared_name(const syntax::Name& name, Scope* target)
{
	const syntax::NameComponent& last = name.last();
	DeclaredName result = DeclaredName{ nullptr, nullptr, last.kind };
	if (last.has_arguments)
		error(name.location, "template-ids are not supported here");
	switch (last.kind)
	{
	case syntax::NameComponentKind::Identifier:
		result.key = last.identifier;
		// A constructor is declared with its class's name (12.1p1).
		if (target->kind == ScopeKind::Class && target->entity->name == last.identifier)
		{
			result.key = constructor_key_;
			result.label = last.identifier->spelling;
		}
		return result;
	case syntax::NameComponentKind::Decltype:
		error(name.location, "a decltype-specifier does not name a declaration");
	default:
		break;
	}
	syntax::Name unqualified = syntax::Name();
	unqualified.components = syntax::Slice<syntax::NameComponent>{ const_cast<syntax::NameComponent*>(&last), 1 };
	unqualified.location = last.location;
	std::string label = syntax::RenderName(unqualified);
	// The course dump spells a conversion function's name as its tokens
	// run together (`operatorconstBase&`).
	if (last.kind == syntax::NameComponentKind::Conversion)
		label.erase(std::remove(label.begin(), label.end(), ' '), label.end());
	result.label = copy_label(label.data(), label.size());
	switch (last.kind)
	{
	case syntax::NameComponentKind::Destructor:
		result.key = destructor_key_;
		break;
	case syntax::NameComponentKind::Conversion:
		result.key = conversion_key_;
		break;
	case syntax::NameComponentKind::Operator:
	{
		// Keyed by the operator itself, not by how it was spelled
		// (`operator and` is `operator&&`).
		std::string key = std::string("operator") + TokenKindSpelling(last.op) + (last.array_form ? "[]" : "");
		result.key = special_key(key.c_str());
		break;
	}
	default:
		// A literal operator: `operator""` and its ud-suffix identifier.
		result.key = special_key((std::string(kLiteralOperatorPrefix) + last.identifier->spelling).c_str());
		break;
	}
	return result;
}

Scope* Analyzer::declarator_target(const syntax::Name& name, Scope* qualifier, Scope* scope, uint32_t location)
{
	(void)name;
	// A qualified declarator-id declares a member of the named class or
	// namespace, from a namespace scope enclosing it (7.3.1.2p2, 9.3p2).
	Scope* where = declaration_scope(scope);
	if (where->kind != ScopeKind::Namespace)
		error(location, "qualified name in a declaration outside namespace scope");
	if (!encloses(where, qualifier))
		error(location, "qualified definition in a scope that does not enclose the member");
	return qualifier;
}

void Analyzer::check_parameter_redeclaration(const Identifier* name, Scope* scope, uint32_t location)
{
	// A name declared in the outermost block of a function body, a handler
	// or a controlled statement cannot redeclare a parameter, exception
	// declaration or condition variable of the enclosing construct (3.3.3p2,
	// 3.3.3p4), nor one a selection statement's init-statement declares,
	// there or in its condition (C++17 [stmt.if]p3, [basic.scope.block]p3:
	// the condition's block is guarded then, its parent the init-
	// statement's).
	if (scope->kind != ScopeKind::Block || !scope->guarded)
		return;
	const NameEntry* entry = names_.find(scope->parent, name);
	const Scope* construct = scope->parent;
	if ((entry == nullptr || entry->value == nullptr) && construct->kind == ScopeKind::Block && construct->guarded &&
		construct->parent->selection_init)
		entry = names_.find(construct->parent, name);
	if (entry != nullptr && entry->value != nullptr)
		error(location, "redeclaration of a name of the enclosing construct");
}

void Analyzer::check_type_name_conflict(Scope* target, const Identifier* name, const Entity* entity, uint32_t location)
{
	// A class or enumeration name may share its scope with one variable,
	// function or enumerator (which hides it, 3.3.10p2) but not with a
	// typedef-name for another type, a namespace or a template parameter.
	const NameEntry* entry = names_.find(target, name);
	if (entry == nullptr || entry->type == nullptr || entry->type->invisible)
		return;
	// A new class or enumeration (`entity` null) meets no typedef-name for
	// itself (3.3.1p4, 7.1.3p6: `typedef int S; struct S;` is ill-formed).
	const Binding* type = entry->type;
	if (type->entity == entity)
		return;
	if (type->kind == BindingKind::TypeAlias && entity != nullptr && type->type->unqualified == entity->type)
		return;
	error(location, "type name conflicts with an earlier declaration");
}

// ------------------------------------------------------ simple declarations

void Analyzer::simple_declaration(const syntax::SimpleDecl& decl, Scope* scope)
{
	Specifiers specifiers;
	const bool lone = decl.declarators.empty();
	Scope* saved_class = declaration_class_;
	Scope* saved_deferral = access_deferral_;
	if (!lone && scope->kind == ScopeKind::Namespace)
		declaration_class_ = member_declaration_class(decl.declarators[0].declarator, scope);
	const size_t pending = lone ? pending_access_.size() : begin_declaration_access(scope);
	const bool saved_owner_specifiers = owner_specifiers_;
	Scope* saved_home = placeholder_home_;
	owner_specifiers_ = template_owner_ != nullptr && scope == template_scope_;
	placeholder_home_ = nullptr;
	if (!lone && scope == template_scope_)
		qualifying_template(decl, scope, &placeholder_home_);
	const syntax::Attribute* saved_lone = lone_class_attributes_;
	lone_class_attributes_ = lone ? decl.attributes : nullptr;
	this->specifiers(decl.specifiers, scope, specifiers, lone);
	lone_class_attributes_ = saved_lone;
	owner_specifiers_ = saved_owner_specifiers;
	placeholder_home_ = saved_home;
	declaration_class_ = saved_class;
	// A deferred reserved builtin call in a decltype-specifier (see
	// deferrable_statement).
	if (deferred_reserved_call())
	{
		access_deferral_ = saved_deferral;
		return;
	}
	const size_t shared = pending_access_.size();
	if (lone && specifiers.declared == nullptr && specifiers.is_friend && specifiers.type != nullptr)
	{
		// 11.3p3: `friend simple-type-specifier ;` and `friend
		// typename-specifier ;` befriend the class type they designate and
		// are ignored for any other type. One that depends on a template
		// parameter waits for the instantiations, which analyze it again.
		if (declaration_scope(scope)->kind != ScopeKind::Class)
			error(decl.location, "friend declaration outside a class");
		if (specifiers.type == auto_placeholder_)
			error(decl.location, "auto in a friend declaration");
		const Type* type = specifiers.type->unqualified;
		if (type->kind == TypeKind::Class && (template_depth_ == 0 || !dependent_type(type)))
			befriend_class(static_cast<ClassEntity*>(type->entity), scope);
		return;
	}
	if (lone)
	{
		// No declarator: the declaration must declare a class or
		// enumeration, or be an anonymous union (7p3, 9.5p5), without a
		// function-specifier (7.1.2p1).
		if (specifiers.declared == nullptr)
			error(decl.location, "declaration does not declare anything");
		if (specifiers.is_virtual)
			error(decl.location, "virtual in a declaration that declares no function");
		// 7.1.5p1: constexpr only on a variable's definition or a function's
		// or static data member's declaration (`constexpr struct S {...};`
		// declares a type).
		if (specifiers.is_constexpr)
			error(decl.location, "constexpr in a declaration that declares no variable or function");
		if (specifiers.declared->kind == EntityKind::Class && specifiers.defines &&
			static_cast<ClassEntity*>(specifiers.declared)->anonymous)
			anonymous_union(static_cast<ClassEntity*>(specifiers.declared), specifiers, scope, decl.location);
		else if (specifiers.declared->kind == EntityKind::Enum && specifiers.defines &&
			static_cast<EnumEntity*>(specifiers.declared)->name == nullptr)
			name_unnamed_type(specifiers, nullptr, scope);
		return;
	}
	if (specifiers.type == nullptr)
		error(decl.location, "declaration without a type");
	if (decl.decomposition)
	{
		// C++17's structured binding declaration (analyzer_bindings.cpp).
		const Type* saved_auto = auto_deduced_;
		auto_deduced_ = nullptr;
		structured_binding_declaration(decl, specifiers, scope);
		auto_deduced_ = saved_auto;
		settle_declaration_access(pending, pending, nullptr);
		access_deferral_ = saved_deferral;
		return;
	}
	if (specifiers.declared != nullptr && specifiers.defines && specifiers.declared->name == nullptr)
	{
		const syntax::Name* id = DeclaratorId(decl.declarators[0].declarator);
		const bool simple = id != nullptr && !id->qualified() &&
			id->last().kind == syntax::NameComponentKind::Identifier;
		name_unnamed_type(specifiers, simple ? id->last().identifier : nullptr, scope);
		// Semantics mode: a class a simple-declaration's declarators name
		// gets its stable identity (an alias-declaration's keeps its label).
		if (specifiers.declared->kind == EntityKind::Class)
			name_local_type(static_cast<ClassEntity*>(specifiers.declared));
	}
	bool packed = false, no_inline = false;
	const uint64_t alignment = decl.attributes != nullptr && template_depth_ == 0
		? attribute_alignment(decl.attributes, scope, packed, no_inline)
		: 0;
	// The declaration's own declarators deduce one auto (7.1.6.4p7); an
	// initializer's nested declarations (a lambda's body) their own.
	const Type* saved_auto = auto_deduced_;
	auto_deduced_ = nullptr;
	for (const syntax::InitDeclarator& init : decl.declarators)
	{
		// 7p1, 8.3p1: the attributes before the declaration and those after
		// its declarator-id appertain to the declared entity (as GNU ones
		// after an array suffix do for GCC).
		uint64_t entity_alignment = alignment;
		bool entity_no_inline = no_inline;
		bool entity_packed = packed;
		const syntax::Attribute* specifier = AlignmentSpecifier(decl.attributes);
		for (const syntax::Declarator* part = init.declarator; part != nullptr; part = part->nested)
		{
			if (part->attributes != nullptr && template_depth_ == 0)
				entity_alignment = std::max(
					entity_alignment, attribute_alignment(part->attributes, scope, entity_packed, entity_no_inline));
			if (specifier == nullptr)
				specifier = AlignmentSpecifier(part->attributes);
		}
		declare_declarator(specifiers, init, scope, decl.location);
		// A deferred reserved builtin call (see deferrable_statement): the
		// declaration is analyzed again, names only.
		if (deferred_reserved_call())
			break;
		settle_declaration_access(
			pending, &init == &decl.declarators[decl.declarators.size - 1] ? pending : shared, init.declared);
		// 7.6.2p1: an alignment-specifier applies to a variable or data
		// member, not a typedef or function.
		Entity* entity = init.declared != nullptr ? init.declared->entity : nullptr;
		const bool variable = !specifiers.is_typedef && (entity == nullptr || entity->kind == EntityKind::Variable);
		if (specifier != nullptr && !variable)
			error(specifier->location, "an alignment-specifier on a typedef or function declaration");
		if ((specifiers.alias_alignment != 0 || (specifiers.is_typedef && entity_alignment != 0)) &&
			init.declared != nullptr && template_depth_ == 0)
			record_alias_alignment(specifiers, init.declared, specifiers.is_typedef ? entity_alignment : 0);
		if (entity != nullptr && entity->kind == EntityKind::Variable)
		{
			request_alignment(static_cast<VariableEntity*>(entity), entity_alignment, specifier);
			overlapping_attribute(static_cast<VariableEntity*>(entity), decl.attributes, init.declarator);
			// GNU packed on a non-static data member: the member alone is
			// packed (alignment 1 unless it requests one), as a packed
			// class's members are.
			if (entity_packed && static_cast<VariableEntity*>(entity)->data_member != nullptr)
				const_cast<DataMember*>(static_cast<VariableEntity*>(entity)->data_member)->packed = true;
		}
		// GNU noinline depends on nothing a template parameter supplies: a
		// member template keeps it for its specializations.
		if (template_depth_ != 0)
		{
			for (const syntax::Attribute* a = decl.attributes; a != nullptr; a = a->next)
				entity_no_inline = entity_no_inline || a->kind == syntax::AttributeKind::NoInline;
			for (const syntax::Declarator* part = init.declarator; part != nullptr; part = part->nested)
				for (const syntax::Attribute* a = part->attributes; a != nullptr; a = a->next)
					entity_no_inline = entity_no_inline || a->kind == syntax::AttributeKind::NoInline;
		}
		if (entity != nullptr && entity->kind == EntityKind::Function && entity_no_inline)
			static_cast<FunctionEntity*>(entity)->no_inline = true;
		// 7.6.3p1, 7.6.1p5: the standard noreturn appertains to a
		// function's declarator-id only (GNU's is ignored elsewhere).
		if ((specifiers.is_typedef || (entity != nullptr && entity->kind == EntityKind::Variable)) &&
			(StandardNoReturn(decl.attributes) || StandardNoReturn(init.declarator)))
			error(decl.location, "noreturn attribute requires a function");
		if (entity != nullptr && entity->kind == EntityKind::Function)
			effect_attributes(static_cast<FunctionEntity*>(entity), decl.attributes, init.declarator);
		if (entity != nullptr)
			symbol_attributes(entity, decl.attributes, init.declarator);
	}
	auto_deduced_ = saved_auto;
	access_deferral_ = saved_deferral;
}

namespace {

// A section name of the token-safe Linux subset (pa27 README 9, pa8's
// section metadata): one or more plain "..." pieces, together nonempty,
// of ASCII letters, digits, `_` and `.` (no prefix, escape or other
// character). Returns false otherwise.
bool TokenSafeSection(const TokenSpelling& spelling, std::string& name)
{
	bool inside = false;
	for (uint32_t i = 0; i < spelling.length; ++i)
	{
		const char c = spelling.text[i];
		if (c == '"')
			inside = !inside;
		else if (inside &&
			((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_' || c == '.'))
			name.push_back(c);
		else if (inside || (c != ' ' && c != '\t' && c != '\n'))
			return false;
	}
	return !name.empty();
}

} // namespace

void Analyzer::symbol_attributes(Entity* entity, const syntax::Attribute* leading, const syntax::Declarator* declarator)
{
	// GNU weak and section (or their [[gnu::...]] forms), before the
	// declaration, after its declarator-id or (GNU ones) after a function
	// declarator's parameter clause. weak: a function or variable with
	// external linkage (GCC: a weak declaration must be public). section:
	// an object of static storage duration, one token-safe name across its
	// declarations (pa27 README 9); a typedef's is an error (GCC, Clang).
	// Function sections are outside PA27 and ignored (the reference, like
	// the stage base, accepts them), as GCC ignores a typedef's weak and an
	// attribute-specifier appertaining to a type (7.1p1, 8.3.5p1).
	const auto apply = [&](const syntax::Attribute* list) {
		for (const syntax::Attribute* attribute = list; attribute != nullptr; attribute = attribute->next)
		{
			if ((attribute->kind != syntax::AttributeKind::Weak && attribute->kind != syntax::AttributeKind::Section) ||
				(attribute->standard && attribute->on_type))
				continue;
			const bool variable = entity->kind == EntityKind::Variable;
			if (!variable && entity->kind != EntityKind::Function)
			{
				if (attribute->kind == syntax::AttributeKind::Section)
					error(attribute->location, "a section attribute on a declaration of no object");
				continue;
			}
			if (!variable && attribute->kind == syntax::AttributeKind::Section)
				continue;
			const VariableEntity* object = variable ? static_cast<const VariableEntity*>(entity) : nullptr;
			const bool internal = variable ? object->internal_linkage
										   : static_cast<const FunctionEntity*>(entity)->internal_linkage;
			const ScopeKind home = entity->owner->kind;
			const bool static_storage = !variable || home == ScopeKind::Namespace || object->is_static ||
				object->is_extern || object->is_thread_local;
			SemanticGraph::SymbolAttributes& own = graph_.own_symbol_attributes(entity);
			if (attribute->kind == syntax::AttributeKind::Weak)
			{
				if (internal || !static_storage ||
					(variable && home != ScopeKind::Namespace && home != ScopeKind::Class))
					error(attribute->location, "a weak declaration of an entity without external linkage");
				own.weak = true;
				continue;
			}
			if (!static_storage)
				error(attribute->location, "a section attribute on a variable without static storage duration");
			if (object->is_thread_local)
				error(attribute->location, "a section attribute on a thread-local variable is not supported");
			std::string name;
			if (!TokenSafeSection(static_cast<const syntax::LiteralExpr*>(attribute->expression)->spelling, name))
				error(attribute->location, "a section name outside ASCII letters, digits, `_` and `.`");
			if (own.section != nullptr && name != own.section)
				error(attribute->location, "a section attribute conflicting with an earlier declaration's");
			if (own.section == nullptr)
				own.section = copy_label(name.data(), name.size());
		}
	};
	apply(leading);
	for (const syntax::Declarator* part = declarator; part != nullptr; part = part->nested)
		apply(part->attributes);
	// GNU abi_tag: on a function or a variable (not a data member or
	// typedef, as GCC and Clang reject; a block-scope object's are moot).
	std::vector<std::string> tags;
	collect_abi_tags(leading, false, tags);
	for (const syntax::Declarator* part = declarator; part != nullptr; part = part->nested)
		collect_abi_tags(part->attributes, false, tags);
	if (tags.empty())
		return;
	const bool data_member = entity->kind == EntityKind::Variable && static_cast<const VariableEntity*>(entity)->member;
	if ((entity->kind != EntityKind::Function && entity->kind != EntityKind::Variable) || data_member)
		error(declarator != nullptr ? declarator->location : 0,
			"an ABI tag on a declaration of no function, variable or class");
	declare_abi_tags(entity, tags, redeclared_, declarator != nullptr ? declarator->location : 0);
}

namespace {

// An abi_tag argument (pa28): plain or u8 "..." pieces, together an
// identifier (GCC's rule: a tag is mangled as a <source-name>). Returns
// false otherwise.
bool AbiTagName(const TokenSpelling& spelling, std::string& name)
{
	bool inside = false;
	for (uint32_t i = 0; i < spelling.length; ++i)
	{
		const char c = spelling.text[i];
		if (c == '"')
			inside = !inside;
		else if (inside &&
			((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_' || (c >= '0' && c <= '9' && !name.empty())))
			name.push_back(c);
		else if (!inside && c == 'u' && i + 2 < spelling.length && spelling.text[i + 1] == '8' &&
			spelling.text[i + 2] == '"')
			++i;
		else if (inside || (c != ' ' && c != '\t' && c != '\n'))
			return false;
	}
	return !name.empty();
}

} // namespace

void Analyzer::collect_abi_tags(
	const syntax::Attribute* list, bool on_class, std::vector<std::string>& tags, const Identifier* implied)
{
	// The abi_tag arguments among `list` (those after an elaborated
	// type's class-key when `on_class`); abi_tag without arguments names
	// `implied` (an inline namespace's name, as GCC and Clang take it) and
	// is an error elsewhere. Of several abi_tag attributes on a
	// declaration the first one's are its tags (Clang's rule, `tags`
	// holding an earlier list's; GCC keeps the last attribute's).
	bool skipped = false;
	for (const syntax::Attribute* attribute = list; attribute != nullptr; attribute = attribute->next)
	{
		if (attribute->kind != syntax::AttributeKind::AbiTag || attribute->on_class != on_class)
			continue;
		if (!attribute->continued)
			skipped = !tags.empty();
		if (skipped)
			continue;
		if (attribute->expression == nullptr)
		{
			if (implied == nullptr)
				error(attribute->location, "an ABI tag attribute without arguments outside an inline namespace");
			tags.push_back(implied->spelling);
			continue;
		}
		std::string name;
		if (!AbiTagName(static_cast<const syntax::LiteralExpr*>(attribute->expression)->spelling, name))
			error(attribute->location, "an ABI tag that is no identifier");
		tags.push_back(name);
	}
}

void Analyzer::declare_abi_tags(
	const void* owner, std::vector<std::string>& tags, bool redeclaration, uint32_t location)
{
	// The first declaration gives an entity its ABI tags; a later one may
	// repeat some (GCC and Clang reject one adding a tag). The table keeps
	// them sorted and unique, the order of their <abi-tag>s.
	std::sort(tags.begin(), tags.end());
	tags.erase(std::unique(tags.begin(), tags.end()), tags.end());
	const SemanticGraph::AbiTags* known = graph_.abi_tags(owner);
	if (redeclaration)
	{
		for (const std::string& tag : tags)
		{
			bool found = false;
			for (uint32_t i = 0; known != nullptr && i < known->count && !found; ++i)
				found = tag == known->tags[i];
			if (!found)
				error(location, "a redeclaration adding an ABI tag");
		}
		return;
	}
	if (known != nullptr)
	{
		for (uint32_t i = 0; i < known->count; ++i)
			tags.push_back(known->tags[i]);
		std::sort(tags.begin(), tags.end());
		tags.erase(std::unique(tags.begin(), tags.end()), tags.end());
	}
	const char** copy = arena_.allocate_array<const char*>(tags.size());
	for (size_t i = 0; i < tags.size(); ++i)
		copy[i] = copy_label(tags[i].data(), tags[i].size());
	graph_.set_abi_tags(owner, SemanticGraph::AbiTags{ copy, static_cast<uint32_t>(tags.size()) });
}

void Analyzer::class_abi_tags(
	const Entity* entity, const syntax::Attribute* list, bool on_class, bool redeclaration, uint32_t location)
{
	// A class's (or enumeration's) ABI tags, after the class-key of its
	// definition or of a declaration declaring it alone; ignored once it
	// is defined (GCC and Clang warn).
	if (list == nullptr)
		return;
	std::vector<std::string> tags;
	collect_abi_tags(list, on_class, tags);
	const bool defined = entity->kind == EntityKind::Class && static_cast<const ClassEntity*>(entity)->scope != nullptr;
	if (!tags.empty() && !(on_class && defined))
		declare_abi_tags(entity, tags, redeclaration, location);
}

void Analyzer::note_block_linkage(const Entity* entity, Scope* block, bool c_linkage)
{
	// 3.5p6: a block-scope declaration with linkage that denotes no
	// earlier declaration declares a member of the innermost enclosing
	// namespace without declaring its name there (see BlockLinkage).
	const uint32_t newest = block_linkage_index_.find(entity->name);
	block_linkages_.push_back(
		BlockLinkage{ enclosing_namespace(block), entity, c_linkage, newest == PointerMap::kMissing ? 0 : newest });
	block_linkage_index_.set(entity->name, static_cast<uint32_t>(block_linkages_.size()));
}

void Analyzer::agree_block_linkage(const Entity* entity, bool c_linkage, bool internal, uint32_t location)
{
	// The first namespace-scope declaration of a name a block-scope
	// declaration there declared redeclares that entity (3.5p6; a function
	// of the same parameter-type-list): of the same kind, with the same
	// type (3.5p10), linkage (7.1.1p7: `void g() { extern int y; } static
	// int y;` is ill-formed) and language linkage (7.5p5).
	if (block_linkages_.empty())
		return;
	const uint32_t newest = block_linkage_index_.find(entity->name);
	for (uint32_t i = newest == PointerMap::kMissing ? 0 : newest; i != 0; i = block_linkages_[i - 1].next)
	{
		const BlockLinkage& record = block_linkages_[i - 1];
		if (record.space != entity->owner)
			continue;
		const Entity* earlier = record.entity;
		if (earlier->kind != entity->kind)
			error(location, "a declaration of another kind of entity than a block-scope declaration of its name");
		if (entity->kind == EntityKind::Function)
		{
			const FunctionEntity* function = static_cast<const FunctionEntity*>(entity);
			const FunctionEntity* block = static_cast<const FunctionEntity*>(earlier);
			if (signature_of(function->type) != signature_of(block->type))
				continue;
			if (declared_type(function->type)->element != declared_type(block->type)->element)
				error(location, "a declaration of a function with another type than a block-scope declaration's");
		}
		else if (!CompatibleVariableTypes(earlier->type, entity->type))
			error(location, "a declaration of a variable with another type than a block-scope declaration's");
		if (internal)
			error(location, "an internal declaration of an entity a block-scope declaration gave external linkage");
		if (c_linkage != record.c_linkage)
			agree_language_linkage(record.c_linkage, location);
	}
}

void Analyzer::agree_language_linkage(bool c_linkage, uint32_t location)
{
	// 7.5p5: two declarations of an entity giving it different language
	// linkages make the program ill-formed; one without a linkage
	// specification after the first keeps the first's.
	if (c_linkage != c_language_ && (c_language_ || linkage_specified_))
		error(location, "declarations of an entity with different language linkages");
}

void Analyzer::reject_noreturn(const syntax::Attribute* list, const char* message) const
{
	// 7.6.3p1: the standard noreturn appertains only to the declarator-id
	// of a function declaration (GNU's is ignored elsewhere).
	for (; list != nullptr; list = list->next)
	{
		if (list->kind == syntax::AttributeKind::NoReturn && list->standard)
			error(list->location, message);
	}
}

void Analyzer::effect_attributes(
	FunctionEntity* function, const syntax::Attribute* leading, const syntax::Declarator* declarator)
{
	// GNU pure and const (before the declaration or after its
	// declarator-id): the function's call effects, the strongest any of its
	// declarations states (const over pure, as GCC merges them); Clang's
	// exclude_from_explicit_instantiation, which any declaration gives;
	// [[noreturn]] and GNU noreturn.
	const auto apply = [this, function](const syntax::Attribute* list) {
		for (const syntax::Attribute* attribute = list; attribute != nullptr; attribute = attribute->next)
		{
			// 7.6.3p1: the standard noreturn appertains to the declarator-id
			// only, not to a type it follows (7.1p1, 8.3.5p1, 8.3.4p1) nor
			// to an elaborated-type-specifier's class (7.1.6.3p1).
			if (attribute->kind == syntax::AttributeKind::NoReturn && attribute->standard &&
				(attribute->on_type || attribute->on_class))
				error(attribute->location, "noreturn attribute applied to a type");
			const FunctionEffect effect = attribute->kind == syntax::AttributeKind::Const ? FunctionEffect::ReadNone
				: attribute->kind == syntax::AttributeKind::Pure                          ? FunctionEffect::ReadOnly
																						  : FunctionEffect::Default;
			if (effect > function->effects)
				function->effects = effect;
			if (attribute->kind == syntax::AttributeKind::ExcludeFromExplicitInstantiation)
				function->excluded_instantiation = true;
			if (attribute->kind == syntax::AttributeKind::AlwaysInline)
				function->always_inline = true;
			if (attribute->kind == syntax::AttributeKind::GnuInline)
				function->gnu_inline = true;
			if (attribute->kind == syntax::AttributeKind::Used)
				function->used = true;
			if (attribute->kind == syntax::AttributeKind::NoReturn)
				function->noreturn = true;
		}
	};
	apply(leading);
	for (const syntax::Declarator* part = declarator; part != nullptr; part = part->nested)
		apply(part->attributes);
}

void Analyzer::overlapping_attribute(
	VariableEntity* variable, const syntax::Attribute* leading, const syntax::Declarator* declarator)
{
	// [[no_unique_address]] before the declaration or after its
	// declarator-id makes a non-static data member of class type
	// potentially overlapping (C++20's 10.3p2; ignored on others, as GCC
	// and Clang do).
	DataMember* member = const_cast<DataMember*>(variable->data_member);
	if (member == nullptr || member->bit_field || ClassOf(variable->type) == nullptr ||
		variable->type->kind == TypeKind::Array)
		return;
	const auto marks = [](const syntax::Attribute* list) {
		for (const syntax::Attribute* attribute = list; attribute != nullptr; attribute = attribute->next)
		{
			if (attribute->kind == syntax::AttributeKind::NoUniqueAddress)
				return true;
		}
		return false;
	};
	bool overlapping = marks(leading);
	for (const syntax::Declarator* part = declarator; part != nullptr && !overlapping; part = part->nested)
		overlapping = marks(part->attributes);
	member->overlapping = overlapping;
}

void Analyzer::request_alignment(VariableEntity* variable, uint64_t alignment, const syntax::Attribute* specifier)
{
	// The strictest requested alignment. 7.6.2p5: with an
	// alignment-specifier, the combined request (GNU aligned counted, as
	// Clang reads it) may not be weaker than the type's alignment; GNU
	// aligned alone only strengthens it.
	if (specifier != nullptr && alignment != 0 && template_depth_ == 0 && !variable->type->is_reference())
	{
		uint64_t size = 0, natural = 0;
		if (object_layout(variable->type, size, natural) && alignment < natural)
			error(specifier->location, "an alignment-specifier weaker than the alignment of the entity's type");
	}
	if (alignment > variable->alignment_request)
		variable->alignment_request = static_cast<uint32_t>(alignment);
}

void Analyzer::declare_declarator(
	const Specifiers& specifiers, const syntax::InitDeclarator& init, Scope* scope, uint32_t location)
{
	// A function declared with a placeholder return type (C++14's
	// 7.1.6.4p2: its own function declarator; a definition later deduces
	// it, see function_definition).
	auto_declarator_ = specifiers.type == auto_placeholder_ && !specifiers.is_typedef;
	placeholder_suffix_ = auto_declarator_ ? OwnFunctionSuffix(init.declarator) : nullptr;
	DeclaratorInfo info = declarator(init.declarator, specifiers.type, scope, location, &specifiers);
	auto_declarator_ = false;
	placeholder_suffix_ = nullptr;
	// A deferred reserved builtin call in an array bound (see
	// deferrable_statement).
	if (deferred_reserved_call())
		return;
	if (info.name == nullptr)
		error(location, "declarator without a name");
	// A template-id declarator-id of a variable declares a partial
	// specialization of a variable template under a template-head, and an
	// explicit one under `template<>`.
	if (info.name->last().has_arguments && !specifiers.is_typedef && info.type->kind != TypeKind::Function)
	{
		// The specialization is emitted where odr-used, not here.
		if (explicit_specialization_)
			variable_explicit_specialization(specifiers, info, init.initializer, scope, info.location);
		else if (template_depth_ != 0 && template_scope_ == scope)
			variable_partial_specialization(specifiers, info, init.initializer, scope, info.location);
		else if (template_depth_ == 0)
			error(info.location, "a template-id declarator outside a specialization's declaration");
		return;
	}
	// 7.1.2p1: function-specifiers only in function declarations; 7.1.2p3:
	// inline not on a block-scope function declaration (nor a block-scope
	// variable's, which the course's inline variables exclude too).
	const bool function = !specifiers.is_typedef && info.type->kind == TypeKind::Function;
	if (specifiers.is_virtual && !function)
		error(info.location, "virtual in a declaration that declares no function");
	if (specifiers.is_explicit && !function)
		error(info.location, "explicit in a declaration that declares no function");
	if (specifiers.is_inline && (specifiers.is_typedef || declaration_scope(scope)->kind == ScopeKind::Block))
		error(info.location, "inline in a typedef or block-scope declaration");
	// 7.1.1p10: mutable only on a data member (see declare_variable).
	if (specifiers.is_mutable && (specifiers.is_typedef || info.type->kind == TypeKind::Function))
		error(info.location, "mutable in a typedef or function declaration");
	if (specifiers.is_typedef)
	{
		if (info.name->qualified() || init.initializer != nullptr || info.virt_suffix != nullptr)
			error(info.location, "invalid typedef declarator");
		// 7.1.5p1: a typedef declaration declares no variable or function.
		if (specifiers.is_constexpr)
			error(info.location, "constexpr in a typedef declaration");
		// 15.4p2: nor has it an exception-specification.
		if (DeclaratorSpecifiesExceptions(init.declarator))
			error(info.location, "an exception-specification in a typedef declaration");
		if (info.type == auto_placeholder_)
			error(info.location, "typedef of auto");
		Binding* binding = declare_typedef(
			declared_name(*info.name, declaration_scope(scope)), info.type, scope, info.location);
		if (declaration_scope(scope)->kind == ScopeKind::Block)
			binding->dependent = names_.dependent_uses() != specifiers.uses;
		init.declared = binding;
		return;
	}
	const Type* declared_auto = holds_auto(info.type) ? info.type : nullptr;
	if (declared_auto != nullptr)
	{
		// 7.1.6.4p4-p5: a data member declared auto is a static one.
		if (declaration_scope(scope)->kind == ScopeKind::Class && !specifiers.is_static)
			error(info.location, "auto in a non-static data member's declaration");
		info.type = deduced_type(specifiers, info.type, init.initializer, scope, info.location);
		// A deferred reserved builtin call in the initializer (see
		// deferrable_statement) leaves the variable undeclared.
		if (deferred_reserved_call())
			return;
	}
	if (info.type->kind == TypeKind::Function)
	{
		const bool deleted = init.initializer != nullptr && init.initializer->kind == syntax::InitializerKind::Delete;
		const bool defaulted = init.initializer != nullptr &&
			init.initializer->kind == syntax::InitializerKind::Default;
		Binding* binding = declare_function(specifiers, info, scope, defaulted, info.location, deleted);
		FunctionEntity* function = static_cast<FunctionEntity*>(binding->entity);
		if (semantic() && template_depth_ == 0)
			pure_specifier(function, init.initializer,
				declaration_scope(scope)->kind == ScopeKind::Class && !specifiers.is_friend &&
					info.qualifier == nullptr,
				info.location);
		note_user_provided(function, init.initializer);
		if (defaulted)
			default_definition(function, info.qualifier == nullptr, info.location);
		init.declared = binding;
		return;
	}
	init.declared = declare_variable(specifiers, info, init.initializer, scope, info.location);
	// An auto variable template whose initializer is dependent: its
	// specializations deduce their types (see variable_specialization).
	if (declared_auto != nullptr && template_depth_ != 0 && dependent_type(info.type) &&
		init.declared->entity != nullptr && init.declared->entity->kind == EntityKind::Variable)
	{
		VariableTemplateFacts* facts = static_cast<VariableEntity*>(init.declared->entity)->templ;
		if (facts != nullptr && facts->head != nullptr)
		{
			facts->auto_type = declared_auto;
			facts->auto_cv = specifiers.cv;
		}
	}
	if (hosted_ && info.type->kind == TypeKind::Array && info.type->bound == 0 && init.declared->entity != nullptr &&
		init.declared->entity->kind == EntityKind::Variable &&
		static_cast<VariableEntity*>(init.declared->entity)->member)
	{
		// The member's own array is the suffix nearest its declarator-id:
		// `T m[0]` is GNU's zero-length array, `T m[]` a flexible array
		// member.
		const syntax::DeclaratorSuffix* own = nullptr;
		for (const syntax::Declarator* declarator = init.declarator; declarator != nullptr;
			declarator = declarator->nested)
		{
			if (declarator->suffixes.size != 0)
				own = &declarator->suffixes[0];
		}
		static_cast<VariableEntity*>(init.declared->entity)->zero_length = own != nullptr && own->bound != nullptr;
	}
	if (!semantic())
		return;
	note_nothrow_target(init.declared->entity, info.type, init.declarator, scope);
	if (init.initializer != nullptr && template_depth_ == 0 &&
		nothrow_targets_.find(init.declared->entity) != PointerMap::kMissing)
		check_nothrow_initializer(*init.initializer);
}

bool Analyzer::holds_auto(const Type* type) const
{
	// The type of a declarator applying operators to auto (auto_declarator_;
	// a function's placeholder return type keeps its decl-specifiers' cv).
	while (type->is_reference() || type->unqualified->kind == TypeKind::Pointer ||
		type->unqualified->kind == TypeKind::MemberPointer)
		type = type->unqualified->element;
	return type->unqualified == auto_placeholder_;
}

const Type* Analyzer::deduced_type(const Specifiers& specifiers, const Type* declared,
	const syntax::Initializer* initializer, Scope* scope, uint32_t location)
{
	// 7.1.6.4p6: the variable's declared type with auto replaced by an
	// invented type template parameter U (auto itself keeps no
	// cv-qualifiers: the decl-specifiers' qualify U) is a function
	// template's parameter type P, the initializer (`= e`, or `( e )`, p3)
	// its call's argument: U is deduced as 14.8.2.1 deduces it (an array
	// or function decayed and top-level cv dropped unless P is a
	// reference, an lvalue given to `auto&&` deduced as `A&`, p3; an
	// overload set by trial against each function, p6), and the variable
	// has the declared type with U in auto's place. A deduction that
	// fails makes the declaration ill-formed, as does a U other than an
	// earlier declarator's of the declaration (p7). The initializer is no
	// unevaluated operand (a lambda-expression may be it, 5.1.2p2):
	// semantics mode types it as its initialization will (again);
	// otherwise, and in a template definition, its decltype type (an
	// lvalue's an lvalue reference, a named variable an lvalue), whose
	// instantiations deduce when it is dependent.
	const syntax::Expr* expression = initializer == nullptr ? nullptr
		: initializer->kind == syntax::InitializerKind::Assign || initializer->kind == syntax::InitializerKind::Braced
		? initializer->expression
		: initializer->kind == syntax::InitializerKind::Parenthesized && initializer->arguments.size == 1
		? initializer->arguments[0]
		: nullptr;
	if (expression == nullptr)
		error(location, "auto without an initializer of one expression (7.1.6.4p3)");
	if (expression->kind == syntax::ExprKind::BracedInit)
		return list_deduced_type(
			specifiers, declared, static_cast<const syntax::BracedInitList&>(*expression), scope, location);
	const Type* p = replaced_auto(declared, auto_placeholder_, specifiers.cv);
	TypeList deduced;
	deduced.push_back(nullptr);
	const Type* type = nullptr;
	ValueCategory category = ValueCategory::PRValue;
	if (semantic() && template_depth_ == 0)
	{
		BodyContext* saved = body_;
		const syntax::Initializer* saved_initializing = initializing_;
		BodyContext context = BodyContext();
		if (body_ == nullptr)
		{
			body_ = &context;
			initializing_ = initializer;
		}
		ExprFacts* facts = declared == auto_placeholder_ ? typed_value(*expression, scope) : typed(*expression, scope);
		body_ = saved;
		initializing_ = saved_initializing;
		if (facts == nullptr)
			return declared;
		// A block-scope initializer is typed once: its initialization reads
		// these facts as they are (kImplicit), recording its conversion; one
		// typed outside a body (in a scratch context) and an overload set
		// (which the initialization resolves) are typed again.
		if (saved != nullptr && (facts->flags & kOverloadSet) == 0)
			facts->flags |= kImplicit;
		// An overload set has no type: the initialization selects the
		// function the trial matched (13.4).
		if ((facts->flags & kOverloadSet) != 0)
			deduce_overloaded(p, facts->functions, 0, deduced);
		else
		{
			check_resolved(*expression);
			type = facts->type;
			category = facts->category;
		}
	}
	else
	{
		type = decltype_type(*expression, scope);
		if (type->kind == TypeKind::LValueReference || expression->kind == syntax::ExprKind::Id)
			category = ValueCategory::LValue;
		if (type->is_reference())
			type = type->element;
	}
	// The auto placeholder is the template parameter of depth and index 0.
	if (type != nullptr && !deduce_argument(p, type, category, 0, deduced))
		deduced[0] = nullptr;
	const Type* replacing = deduced[0];
	if (replacing == nullptr)
	{
		// A template definition's dependent initializer: its instantiations
		// deduce.
		if (type != nullptr && template_depth_ != 0 && dependent_type(type))
			return type;
		error(location, "auto deduction fails for the initializer (7.1.6.4p6)");
	}
	if (auto_deduced_ != nullptr && replacing->canonical != auto_deduced_->canonical &&
		(template_depth_ == 0 || (!dependent_type(replacing) && !dependent_type(auto_deduced_))))
		error(location, "auto deduced as different types by one declaration's declarators (7.1.6.4p7)");
	auto_deduced_ = replacing;
	return replaced_auto(declared, replacing, specifiers.cv);
}

const Type* Analyzer::replaced_auto(const Type* declared, const Type* type, uint8_t cv)
{
	// `declared` (see holds_auto) with auto replaced by `type` qualified by
	// the decl-specifiers' `cv`; a reference to a reference U collapses
	// (8.3.2p6).
	if (declared->unqualified == auto_placeholder_)
		return types_.qualified(type, static_cast<uint8_t>(cv | declared->cv));
	const Type* element = replaced_auto(declared->unqualified->element, type, cv);
	if (declared->unqualified->kind == TypeKind::MemberPointer)
		return types_.qualified(types_.member_pointer(declared->unqualified->owner, element), declared->cv);
	if (!declared->is_reference())
		return types_.qualified(types_.pointer(element), declared->cv);
	if (element->is_reference())
		return types_.reference(element->element,
			declared->kind == TypeKind::RValueReference && element->kind == TypeKind::RValueReference);
	return types_.reference(element, declared->kind == TypeKind::RValueReference);
}

Binding* Analyzer::declare_typedef(const DeclaredName& name, const Type* type, Scope* scope, uint32_t location)
{
	if (name.label != nullptr)
		error(location, "invalid typedef name");
	Scope* where = declaration_scope(scope);
	check_template_parameter_name(scope, name.key, location);
	Entity* entity = nullptr;
	if (const NameEntry* entry = names_.find(where, name.key))
	{
		if (entry->value != nullptr && !entry->value->invisible)
			error(location, "typedef-name conflicts with an earlier declaration");
		if (const Binding* existing = entry->type)
		{
			// A typedef may redefine a name in a non-class scope to the type it
			// already denotes (7.1.3p3), or a class name in a class (7.1.3p4).
			if (existing->kind == BindingKind::Namespace || existing->kind == BindingKind::NamespaceAlias ||
				(existing->entity != nullptr && existing->entity->kind == EntityKind::TemplateParameter))
				error(location, "typedef-name conflicts with an earlier declaration");
			bool replaced = false;
			if (binding_type(existing, location) != type)
			{
				replaced = predeclared_extended_float(existing, where);
				if (!replaced)
					error(location, "typedef redefinition with a different type");
			}
			if (where->kind == ScopeKind::Class && existing->kind == BindingKind::TypeAlias)
				error(location, "member typedef redeclared");
			if (existing->kind == BindingKind::TypeAlias && !replaced)
				entity = existing->entity;
		}
	}
	if (entity == nullptr)
		entity = new_entity<Entity>(EntityKind::Typedef, name.key, where, type);
	Binding* binding = new_binding(BindingKind::TypeAlias, name.key, entity, type);
	print_in(scope, binding);
	declare_in(where, binding);
	return binding;
}

bool Analyzer::predeclared_extended_float(const Binding* existing, const Scope* where) const
{
	if (where != graph_.global || !existing->hidden || existing->kind != BindingKind::TypeAlias ||
		existing->entity == nullptr || existing->entity->kind != EntityKind::Typedef)
		return false;
	const Type* type = existing->entity->type;
	if (type->kind != TypeKind::Fundamental)
		return false;
	return type->fundamental >= FundamentalType::Float16 && type->fundamental <= FundamentalType::Float64x;
}

const Type* Analyzer::complete_array_from_initializer(
	const Type* type, const syntax::Initializer* initializer, bool typed)
{
	// 8.3.4p3, 8.5.1p4, 8.5.2p1: an array of unknown bound takes its bound
	// from a braced initializer list or a string literal. A list's
	// elements are counted with brace elision (8.5.1p11) by its typed
	// initialization (see braced_list); untyped, one per element.
	if (initializer == nullptr || type->kind != TypeKind::Array || type->bound != 0 || type->owner != nullptr)
		return type;
	const syntax::Expr* expr = initializer->expression;
	if (initializer->kind != syntax::InitializerKind::Assign && initializer->kind != syntax::InitializerKind::Braced)
		return type;
	uint64_t bound = 0;
	if (expr->kind == syntax::ExprKind::BracedInit)
	{
		const syntax::BracedInitList* list = static_cast<const syntax::BracedInitList*>(expr);
		bound = typed ? 0 : list->elements.size;
		if (list->elements.size == 1 && list->elements[0]->kind == syntax::ExprKind::Literal &&
			IsIntegralType(type->element) && type->element->unqualified->fundamental != FundamentalType::Bool)
		{
			const syntax::LiteralExpr* literal = static_cast<const syntax::LiteralExpr*>(list->elements[0]);
			TextLiteralParts parts;
			if (SplitTextLiteral(literal->spelling.text, literal->spelling.length, parts) && parts.quote == '"')
				bound = StringLiteralLength(literal->spelling);
		}
	}
	else if (expr->kind == syntax::ExprKind::Literal)
		bound = StringLiteralLength(static_cast<const syntax::LiteralExpr*>(expr)->spelling);
	if (bound == 0)
		return type;
	return types_.array(type->element, bound);
}

Binding* Analyzer::declare_variable(const Specifiers& specifiers, const DeclaratorInfo& info,
	const syntax::Initializer* initializer, Scope* scope, uint32_t location, bool structured_binding)
{
	const Type* type = info.type;
	// A constexpr object is const (7.1.5p9).
	if (specifiers.is_constexpr)
		type = types_.qualified(type, kConst);
	Scope* where = declaration_scope(scope);
	Scope* print = scope;
	const bool qualified = info.qualifier != nullptr;
	if (qualified)
		print = where = declarator_target(*info.name, info.qualifier, scope, location);
	// Semantics mode types the initialization of objects defined outside
	// classes and a static data member's in-class initializer (not a
	// complete-class context, 9.2p2); a non-static member's initializer
	// is typed in the complete class.
	const bool typed_initialization = semantic() && template_depth_ == 0 &&
		(where->kind != ScopeKind::Class || qualified || (specifiers.is_static && initializer != nullptr));
	type = complete_array_from_initializer(type, initializer, typed_initialization);
	DeclaredName name = declared_name(*info.name, where);
	// 9.2p13: a non-static data member may have its class's name unless the
	// class has a user-declared constructor (it hides the injected-class-
	// name, 3.3.10p2).
	if (name.label != nullptr && name.kind == syntax::NameComponentKind::Identifier && !qualified &&
		where->kind == ScopeKind::Class && !specifiers.is_static &&
		!static_cast<const ClassEntity*>(where->entity)->user_declared_constructor)
		name = DeclaredName{ info.name->last().identifier, nullptr, name.kind };
	if (name.label != nullptr)
		error(location, "invalid variable name");
	if (!qualified)
		check_template_parameter_name(scope, name.key, location);
	if (type->unqualified == types_.fundamental(FundamentalType::Void))
		error(location, "variable of type void");
	const bool member = where->kind == ScopeKind::Class && !specifiers.is_static;
	// A declaration directly in a linkage-specification is extern (7.5p7).
	const bool is_extern = specifiers.is_extern || (linkage_extern_ && where->kind == ScopeKind::Namespace);
	if (specifiers.is_constinit &&
		(member ||
			(where->kind == ScopeKind::Block && !specifiers.is_static && !specifiers.is_thread_local && !is_extern)))
		error(location, "__constinit requires a variable with static or thread storage duration");
	// A static data member's declaration in its class is not a definition;
	// its qualified namespace-scope one is (9.4.2p2). One declared inline in
	// its class is (C++17's 9.4.2p3, accepted in hosted compilations as an
	// inline variable is: libc++'s integral_constant::value).
	const bool inline_member = hosted_ && specifiers.is_inline && specifiers.is_static &&
		where->kind == ScopeKind::Class && !qualified;
	bool definition = ((where->kind != ScopeKind::Class || qualified) && (!is_extern || initializer != nullptr)) ||
		inline_member;
	check_variable_specifiers(specifiers, type, where, qualified, member, definition, initializer != nullptr, location);
	check_parameter_redeclaration(name.key, where, location);
	VariableEntity* variable = earlier_variable(
		specifiers, name, where, qualified, initializer != nullptr, definition, location);
	const bool redeclaration = variable != nullptr;
	// 9.4.2p3: a static data member initialized in its class (const
	// integral, or constexpr) is still defined at namespace scope when
	// odr-used, by a definition without an initializer; the object takes
	// the in-class value (recorded for integral constants).
	const bool initialized_in_class = qualified && where->kind == ScopeKind::Class && variable->is_static &&
		(variable->constant == ConstantState::Value || variable->is_constexpr);
	if (initialized_in_class && initializer != nullptr)
		error(location, "the definition of a static data member initialized in its class repeats the initializer");
	if (variable != nullptr)
	{
		if (!CompatibleVariableTypes(variable->type, type))
			error(location, "conflicting declaration of a variable");
		// 7.1.1p7-p8 (`extern int d; static int d;`).
		if (specifiers.is_static && where->kind == ScopeKind::Namespace && !variable->internal_linkage)
			error(location, "a static declaration of a variable declared with external linkage");
		if (where->kind == ScopeKind::Namespace && !qualified && variable->c_linkage != c_language_)
			agree_language_linkage(variable->c_linkage, location);
		if (definition && variable->defined)
			error(location, "redefinition of a variable");
		if (variable->type->kind == TypeKind::Array && variable->type->bound == 0)
			variable->type = type;
		// 8.3.4p3: an omitted bound is the bound an earlier declaration
		// of the entity specified.
		else if (type->kind == TypeKind::Array && type->bound == 0 && variable->type->kind == TypeKind::Array)
			type = variable->type;
		if (info.asm_label != nullptr && graph_.asm_label(variable) == nullptr)
			graph_.set_asm_label(variable, info.asm_label);
	}
	else
		variable = new_variable(specifiers, info.asm_label, name, where, type, is_extern, member,
			initializer != nullptr, location, structured_binding);
	// A braced list's initialization completes an omitted bound (8.5.1p4).
	const bool bound_from_list = typed_initialization && type->kind == TypeKind::Array && type->bound == 0 &&
		initializer != nullptr &&
		(initializer->kind == syntax::InitializerKind::Assign ||
			initializer->kind == syntax::InitializerKind::Braced) &&
		initializer->expression->kind == syntax::ExprKind::BracedInit;
	if (member || definition)
	{
		// An array's bound may depend on template parameters (known in the
		// instantiations).
		const Type* object = bound_from_list ? type->element : type;
		while (object->kind == TypeKind::Array && (object->bound != 0 || object->owner != nullptr || member))
			object = object->element;
		if ((object->kind == TypeKind::Array && !specifiers.is_static) ||
			(object->unqualified->kind == TypeKind::Class && template_depth_ == 0 &&
				!require_complete(static_cast<ClassEntity*>(object->unqualified->entity))))
			error(location, "variable of incomplete type");
		if (semantic() && template_depth_ == 0)
			check_abstract_object(object, location);
	}
	// A static data member of a templated class defined out of it: its
	// instances instantiate this definition when needed (14.7.1p1); one
	// defined inline in it, a definition whose initializer is each
	// instance's in-class one.
	if ((qualified || (inline_member && (template_scope_ == nullptr || scope != template_scope_))) && definition &&
		template_depth_ != 0 && templated_class(where))
	{
		VariableTemplateFacts* facts = variable_template_facts(variable);
		facts->initializer = qualified ? initializer : nullptr;
		facts->bound = info.bound;
		facts->parameters = scope->kind == ScopeKind::TemplateParameters ? scope : nullptr;
		facts->scope = info.after;
		facts->location = location;
		facts->definition_ordinal = definition_point();
	}
	variable->defined = variable->defined || definition;
	variable->is_constexpr = variable->is_constexpr || specifiers.is_constexpr;
	variable->is_constinit = variable->is_constinit || specifiers.is_constinit;
	variable->is_mutable = variable->is_mutable || specifiers.is_mutable;
	// A namespace-scope variable or static data member under a
	// template-head is a variable template; a static data member template's
	// definition outside its class gives its initializer.
	if (template_scope_ != nullptr && scope == template_scope_ && !qualified &&
		(where->kind == ScopeKind::Namespace || (where->kind == ScopeKind::Class && specifiers.is_static)))
		declare_variable_template(variable, initializer, scope, scope, location);
	else if (template_scope_ != nullptr && scope == template_scope_ && qualified &&
		variable_template_of(names_.qualified(where, name.key, LookupFilter::Ordinary).binding) != nullptr)
		declare_variable_template(variable, initializer, scope, info.after, location);
	Binding* binding = new_binding(BindingKind::Variable, name.key, variable, type);
	print_in(print, binding);
	declare_in(where, binding);
	// A block-scope variable's type named something dependent (for auto,
	// its initializer did; see Binding::dependent).
	if (where->kind == ScopeKind::Block)
		binding->dependent = names_.dependent_uses() != specifiers.uses;
	// A qualified definition's initializer uses the names after its
	// declarator-id (3.4.3p3), with the template parameters its bridge kept.
	Scope* lookup = qualified ? info.after : scope;
	if (specifiers.is_constexpr && semantic() && template_depth_ == 0 && !member)
		check_constexpr_variable(variable, location);
	// 9.4.2p3: a static data member initialized in its class is const of
	// integral or enumeration type, or constexpr.
	if (initializer != nullptr && where->kind == ScopeKind::Class && !member && !qualified &&
		!specifiers.is_constexpr && !inline_member && template_depth_ == 0 &&
		((type->cv & kConst) == 0 || (!IsIntegralType(type) && type->unqualified->kind != TypeKind::Enum)))
		error(location, "a static data member initialized in its class must be const integral or constexpr");
	// 6.7p3: a switch label or goto may not bypass an automatic variable's
	// initializer, or its default constructor or destructor unless trivial
	// (12.1p5, 12.4p5: implicit or defaulted, with trivial subobjects).
	// A template's declaration counts when it has an initializer (its
	// constructors and destructor wait for its instantiations). One with
	// an initializer counts from its point of declaration (3.3.2p1): a
	// goto out of a statement expression in the initializer bypasses it
	// not.
	const bool pattern_body = body_ != nullptr && body_->pattern;
	const bool counted = (typed_initialization || pattern_body) && body_ != nullptr &&
		where->kind == ScopeKind::Block && !specifiers.is_static && !specifiers.is_extern &&
		!specifiers.is_thread_local;
	if (counted && initializer != nullptr)
	{
		++where->initialized;
		if (body_->switch_statement != nullptr)
			++body_->switch_statement->initialized;
	}
	if (initializer != nullptr)
	{
		const uint64_t values = names_.value_parameter_uses();
		bool deferred = false;
		if (typed_initialization)
		{
			if (where->kind == ScopeKind::Class && !qualified)
				variable->member_initializer = initializer;
			deferred = !typed_initializer(variable, type, initializer, lookup, location);
		}
		if (deferred)
		{
			// A hosted wrapper's initializer calling a reserved builtin this
			// compiler lacks: its names only, as a template's (see
			// define_function).
			body_->pattern = true;
			initializer_names(initializer, lookup, IsClassObjectType(type));
			body_->pattern = false;
		}
		else if (typed_initialization)
		{
			if (bound_from_list)
			{
				type = initializer->expression->facts->type;
				if (type->bound == 0)
					error(location, "variable of incomplete type");
				variable->type = type;
				binding->type = type;
			}
		}
		else if (member && semantic() && !qualified)
		{
			// 9.2p2: a member's brace-or-equal-initializer is typed in the
			// complete class (see finish_class_nesting); a template's is
			// name-checked there.
			variable->member_initializer = initializer;
			deferred_.push_back(DeferredBody{ nullptr, type, nullptr, nullptr, lookup, lookup, variable });
		}
		else
			initializer_names(initializer, lookup, IsClassObjectType(type));
		const ExprFacts* initial = initializer->expression != nullptr ? initializer->expression->facts : nullptr;
		if (variable->is_constexpr ||
			((type->cv & kConst) != 0 && (IsIntegralType(type) || type->kind == TypeKind::Enum)))
			binding->value_dependent = names_.value_parameter_uses() != values ||
				(initial != nullptr && (initial->flags & kValueDependentUse) != 0);
		if (template_depth_ == 0 && !deferred)
			record_constant_initializer(variable, initializer, lookup);
		// 9.4.2p3: a const static data member's in-class initializer is a
		// constant expression (a constexpr one's is diagnosed with its
		// evaluation, 7.1.5p9).
		if (semantic() && template_depth_ == 0 && where->kind == ScopeKind::Class && !member && !qualified &&
			!specifiers.is_constexpr && variable->constant == ConstantState::None)
			error(location, "a static data member's in-class initializer is not a constant expression");
	}
	else if (typed_initialization && definition && !initialized_in_class)
	{
		// A static data member's definition is in its class (9.4.2p2, 11p6).
		expression_scope_ = lookup;
		default_initialize(variable, type, location);
		// 7.1.5p9: a constexpr object default-initialized by a constexpr
		// constructor.
		if (variable->is_constexpr || variable->is_constinit)
			record_constant_initializer(variable, nullptr, lookup);
	}
	// 12.4p11: the object's destruction where its storage duration ends.
	if (typed_initialization && definition && !member)
		check_destruction(type, scope, location);
	// One without, by its default constructor or destructor.
	if (counted && initializer == nullptr && !pattern_body &&
		((variable->constructor != nullptr && !variable->constructor->trivial) || nontrivial_destruction(type)))
	{
		++where->initialized;
		if (body_->switch_statement != nullptr)
			++body_->switch_statement->initialized;
	}
	redeclared_ = redeclaration;
	return binding;
}

bool Analyzer::typed_initializer(
	VariableEntity* variable, const Type* type, const syntax::Initializer* initializer, Scope* scope, uint32_t location)
{
	// False when a hosted wrapper's reserved builtin call stopped the typing
	// (see define_function).
	if (deferring_reserved_ == 0)
	{
		initialize_variable(variable, type, initializer, scope, location);
		return true;
	}
	if (deferred_reserved_call())
		return false;
	const uint32_t unevaluated = unevaluated_;
	initialize_variable(variable, type, initializer, scope, location);
	if (!deferred_reserved_call())
		return true;
	consume_deferred_call();
	unevaluated_ = unevaluated;
	if (body_ != nullptr && body_->function != nullptr)
		body_->function->unanalyzed_body = true;
	return false;
}

void Analyzer::check_variable_specifiers(const Specifiers& specifiers, const Type* type, const Scope* where,
	bool qualified, bool member, bool definition, bool initialized, uint32_t location)
{
	// 7.1.1p10: mutable applies to a non-static data member that is
	// neither declared const nor a reference.
	if (specifiers.is_mutable)
	{
		const Type* object = type;
		while (object->kind == TypeKind::Array)
			object = object->element;
		if (!member || (object->cv & kConst) != 0 || type->is_reference())
			error(location, "mutable on a variable that is no non-const, non-reference data member");
	}
	// 8.5p5: a block-scope declaration of a variable with linkage (extern)
	// has no initializer.
	if (specifiers.is_extern && initialized && where->kind == ScopeKind::Block)
		error(location, "an initializer on a block-scope extern declaration");
	// 8.3.2p5: a reference definition outside a class needs an initializer.
	// A single linkage declaration is extern as well (7.5p7), already
	// accounted for by the computed definition state.
	if (type->is_reference() && !initialized && definition && where->kind != ScopeKind::Class)
		error(location, "reference declared without an initializer");
	// 7.1.5p1: constexpr applies to a variable's definition (`extern
	// constexpr int n;` is none) or a static data member's declaration,
	// not to a non-static data member; 9.4.2p3: a static data member
	// declared constexpr in its class has a brace-or-equal-initializer.
	if (specifiers.is_constexpr)
	{
		const bool in_class = where->kind == ScopeKind::Class && !qualified;
		if (in_class && member)
			error(location, "a constexpr non-static data member");
		else if (in_class && !initialized)
			error(location, "a constexpr static data member declared without an initializer");
		else if (!in_class && !definition)
			error(location, "a constexpr variable declaration that is not a definition");
	}
}

VariableEntity* Analyzer::earlier_variable(const Specifiers& specifiers, const DeclaredName& name, Scope* where,
	bool qualified, bool initialized, bool& definition, uint32_t location)
{
	// The variable an earlier declaration of the name declared: the member
	// a qualified definition names (an explicit specialization's own, which
	// may not be a definition), the scope's variable, or, for a block-scope
	// extern declaration, the enclosing namespace's (3.5p6).
	VariableEntity* variable = nullptr;
	if (qualified)
	{
		const LookupResult previous = names_.qualified(where, name.key, LookupFilter::Ordinary);
		if (!previous.found() || previous.binding->kind != BindingKind::Variable)
			error(location, "qualified definition does not match a declaration");
		// 7.1.1p1, 9.4.2p2: `static` only in the member's declaration in
		// its class, not on its definition (GCC, Clang).
		if (specifiers.is_static && where->kind == ScopeKind::Class)
			error(location, "static on a static data member's definition outside its class");
		variable = static_cast<VariableEntity*>(previous.binding->entity);
		// 9.2p1, 9.4.2p2: only a static data member is defined outside its
		// class (checked at the declaration, used or not).
		if (variable->member)
			error(location, "an out-of-class definition names a non-static data member");
		if (explicit_specialization_ && instance_class(where))
			definition = explicit_variable_specialization(variable, initialized, location);
	}
	else if (explicit_specialization_)
		error(location, "an explicit specialization of a variable is not supported");
	else if (const NameEntry* entry = names_.find(where, name.key))
	{
		if (entry->value != nullptr)
		{
			if (entry->value->kind != BindingKind::Variable || where->kind == ScopeKind::Class ||
				(where->kind != ScopeKind::Namespace &&
					!(specifiers.is_extern && static_cast<VariableEntity*>(entry->value->entity)->is_extern)))
				error(location, "redeclaration of a name in the same scope");
			variable = static_cast<VariableEntity*>(entry->value->entity);
		}
		if (entry->type != nullptr && (entry->type->kind != BindingKind::Type || IsClassTemplate(entry->type)))
			error(location, "variable name conflicts with an earlier declaration");
		if (entry->type != nullptr && entry->type->entity->kind == EntityKind::TemplateParameter)
			error(location, "variable name conflicts with a template parameter");
	}
	if (variable == nullptr && where->kind == ScopeKind::Block && specifiers.is_extern)
	{
		// 3.5p6: a block-scope extern declaration declares the variable of
		// the innermost enclosing namespace that a visible earlier
		// declaration there declared (a hidden one with external linkage is
		// the same entity, 3.5p9); otherwise a variable with external
		// linkage, distinct from a hidden internal one.
		if (const NameEntry* outer = names_.find(enclosing_namespace(where), name.key))
		{
			if (outer->value != nullptr && outer->value->kind == BindingKind::Variable)
			{
				VariableEntity* previous = static_cast<VariableEntity*>(outer->value->entity);
				if (!previous->internal_linkage || !hidden_below_namespace(where, name.key))
					variable = previous;
			}
		}
	}
	return variable;
}

VariableEntity* Analyzer::new_variable(const Specifiers& specifiers, const char* asm_label, const DeclaredName& name,
	Scope* where, const Type* type, bool is_extern, bool member, bool initialized, uint32_t location,
	bool structured_binding)
{
	// A variable's first declaration: its storage, linkage (3.5p3-p4) and
	// language linkage (7.5p1), a data member's place in its class; a
	// tuple-like structured binding's reference variable (11.5p3) records
	// its binding facts too.
	VariableEntity* variable = structured_binding
		? new_entity<BindingEntity>(EntityKind::Variable, name.key, where, type)
		: new_entity<VariableEntity>(EntityKind::Variable, name.key, where, type);
	variable->structured_binding = structured_binding;
	if (asm_label != nullptr)
		graph_.set_asm_label(variable, asm_label);
	variable->is_extern = is_extern;
	variable->is_static = specifiers.is_static;
	variable->member = member;
	variable->is_thread_local = specifiers.is_thread_local;
	variable->is_inline = hosted_ && specifiers.is_inline &&
		(where->kind == ScopeKind::Namespace || (where->kind == ScopeKind::Class && specifiers.is_static));
	// A block-scope one of static or thread storage duration numbered
	// among its function's of its name (apart from its classes, as Clang
	// numbers them; Itanium 5.1.7).
	if (where->kind == ScopeKind::Block && body_ != nullptr && (specifiers.is_static || specifiers.is_thread_local) &&
		name.key != nullptr)
	{
		if (const uint16_t earlier = local_number(&name.key->length))
			graph_.local_discriminators.set(variable, earlier);
	}
	if (where->kind == ScopeKind::Namespace)
	{
		// 3.5p3-p4: static, or const and not extern, or a member of an
		// unnamed namespace; 7.5p1: a namespace-scope name's language
		// linkage.
		const Type* object = type;
		while (object->kind == TypeKind::Array)
			object = object->element;
		// A C-linkage one of an unnamed namespace is external (7.5p6, as
		// for functions; Clang's reading).
		variable->internal_linkage = specifiers.is_static ||
			(static_cast<const NamespaceEntity*>(where->entity)->internal && !c_language_) ||
			((object->cv & kConst) != 0 && !is_extern && !type->is_reference() && !variable->is_inline);
		variable->c_linkage = c_language_;
		agree_block_linkage(variable, variable->c_linkage, variable->internal_linkage, location);
	}
	else if (where->kind == ScopeKind::Block && is_extern)
		note_block_linkage(variable, where, c_language_);
	if (member && where->entity != nullptr && where->entity->kind == EntityKind::Class)
	{
		ClassEntity* owner = static_cast<ClassEntity*>(where->entity);
		add_data_member(owner, variable, 0, false);
		owner->non_pod = owner->non_pod || current_access_ != TokenKind::KW_PUBLIC || initialized ||
			type->is_reference();
		owner->not_aggregate = owner->not_aggregate || current_access_ != TokenKind::KW_PUBLIC || initialized;
	}
	return variable;
}

void Analyzer::add_data_member(ClassEntity* entity, Entity* variable, uint64_t width, bool bit_field)
{
	DataMember* member = static_cast<DataMember*>(arena_.allocate(sizeof(DataMember), alignof(DataMember)));
	const Access access = current_access_ == TokenKind::KW_PRIVATE ? Access::Private
		: current_access_ == TokenKind::KW_PROTECTED               ? Access::Protected
																   : Access::Public;
	*member = DataMember{ variable, width, bit_field, access, false, false, 0, entity->member_count++, nullptr };
	if (variable->kind == EntityKind::Variable)
		static_cast<VariableEntity*>(variable)->data_member = member;
	if (entity->last_member == nullptr)
		entity->members = member;
	else
		entity->last_member->next = member;
	entity->last_member = member;
}

void Analyzer::note_user_provided(const FunctionEntity* function, const syntax::Initializer* initializer)
{
	// A constructor, destructor or copy assignment operator declared in its
	// class without `= default` or `= delete` is user-provided (8.4.2p4),
	// which makes the class no POD for the purpose of layout. A copy
	// assignment operator is a non-template operator= of one parameter of
	// type X, X&, const X&, volatile X& or const volatile X& (12.8p17).
	const Scope* owner = function->owner;
	if (owner->kind != ScopeKind::Class ||
		(initializer != nullptr &&
			(initializer->kind == syntax::InitializerKind::Default ||
				initializer->kind == syntax::InitializerKind::Delete)))
		return;
	ClassEntity* entity = static_cast<ClassEntity*>(owner->entity);
	bool special = function->constructor || function->name == destructor_key_;
	if (function->name == assign_key_ && !function->templated)
	{
		const Type* type = function->type;
		if (type->param_count == 1 && !type->variadic)
		{
			const Type* parameter = type->params[0];
			if (parameter->is_reference())
				parameter = parameter->element;
			special = parameter->unqualified == entity->type;
		}
	}
	entity->non_pod = entity->non_pod || special;
	entity->not_aggregate = entity->not_aggregate || function->constructor;
}

Binding* Analyzer::declare_function(const Specifiers& specifiers, const DeclaratorInfo& info, Scope* scope,
	bool definition, uint32_t location, bool deleted)
{
	if (specifiers.is_constinit)
		error(location, "__constinit on a function declaration");
	// 7.1.5p8 (C++11): a constexpr non-static member function other than a
	// constructor is const (not under -std=c++14, see set_cxx14).
	if (specifiers.is_constexpr && !cxx14_ && !specifiers.is_static && !specifiers.is_friend &&
		info.type->kind == TypeKind::Function && (info.type->function_cv & kConst) == 0)
	{
		Scope* owner = info.qualifier != nullptr ? info.qualifier : declaration_scope(scope);
		const syntax::NameComponentKind kind = info.name->last().kind;
		const Identifier* key = owner->kind == ScopeKind::Class ? declared_name(*info.name, owner).key : nullptr;
		if (key != nullptr && kind != syntax::NameComponentKind::Destructor && key != constructor_key_)
		{
			DeclaratorInfo adjusted = info;
			adjusted.type = types_.function(info.type->element, info.type->params, info.type->param_count,
				info.type->variadic, static_cast<uint8_t>(info.type->function_cv | kConst), info.type->ref);
			// An out-of-class definition redeclares the member its class
			// declared: a static one keeps its type.
			const FunctionList* declared = info.qualifier != nullptr
				? names_.overload(info.qualifier, key, signature_of(adjusted.type))
				: nullptr;
			if (info.qualifier == nullptr ||
				(declared != nullptr && !static_cast<const FunctionEntity*>(declared->binding->entity)->is_static))
				return declare_function(specifiers, adjusted, scope, definition, location, deleted);
		}
	}
	// 14.7.3p5: `template<>` before the template-head of a member of an
	// explicitly specialized member class template (a templated class of
	// the specialization) introduces an ordinary member definition of it.
	if (explicit_specialization_ && info.qualifier != nullptr && templated_class(info.qualifier) &&
		scope->kind == ScopeKind::TemplateParameters && scope == template_scope_)
		explicit_specialization_ = false;
	if (explicit_specialization_ && !specifiers.is_friend && scope->kind == ScopeKind::TemplateParameters &&
		scope == template_scope_ && !template_entities_.empty())
		return member_template_specialization(info, scope, definition, location);
	if (explicit_specialization_ && !specifiers.is_friend)
		return explicit_function_specialization(specifiers, info, scope, definition, location, deleted);
	// 14.5.4p9: a friend declaration naming a function template's
	// specialization has no default arguments and is not inline.
	if (specifiers.is_friend && info.name->last().has_arguments &&
		(specifiers.is_inline || HasDefaultArgument(info.parameters)))
		error(location, "a default argument or inline in a friend declaration of a specialization");
	Scope* where = declaration_scope(scope);
	Scope* print = scope;
	const bool qualified = info.qualifier != nullptr;
	bool invisible = false;
	bool pattern_friend = false;
	ClassEntity* befriending = nullptr;
	if (qualified && specifiers.is_friend)
		return qualified_friend(specifiers, info, scope, location);
	if (qualified)
		print = where = declarator_target(*info.name, info.qualifier, scope, location);
	else if (specifiers.is_friend)
	{
		// A friend first declared in a class is a member of the innermost
		// enclosing namespace, invisible to ordinary lookup there (7.3.1.2p3).
		if (where->kind != ScopeKind::Class)
			error(location, "friend declaration outside a class");
		if (info.name->last().has_arguments)
			return friend_specialization(info, scope, location);
		befriending = static_cast<ClassEntity*>(where->entity);
		// 14.6.5p1, 14.5.4p4: a templated class's friend is declared (and
		// one defined there, defined) by each instantiation of the class;
		// the template's own declaration is only their pattern, which
		// neither redeclares nor is found as the namespace's function.
		pattern_friend = templated_class(where);
		print = where = enclosing_namespace(where);
		invisible = true;
	}
	// 7.1.1p1, 9.4: `static` only in a member's declaration in its class,
	// not on its definition outside it (GCC, Clang).
	if (qualified && specifiers.is_static && where->kind == ScopeKind::Class)
		error(location, "static on a member function's declaration outside its class");
	const DeclaredName name = declared_name(*info.name, where);
	// 14.6.1p6 (a friend's name belongs to its namespace, outside the
	// template parameters' scope).
	if (!qualified && !specifiers.is_friend)
		check_template_parameter_name(scope, name.key, location);
	// 12.3.2p1: a conversion function has no return type.
	if (name.kind == syntax::NameComponentKind::Conversion && specifiers.type != nullptr)
		error(location, "a conversion function with a return type");
	if (qualified)
		implicit_members_for(where, name.key);
	// A member of a templated class defined out of it uses its class's
	// template parameters (14.5.1.1); another declaration under a template
	// declaration's parameter list declares a function template.
	// A member template defined out of its class template has a template
	// parameter list beyond its enclosing class templates' (14.5.2p1): a
	// function template of the templated class.
	const bool pattern_member = qualified && templated_class(info.qualifier) &&
		!MemberTemplateLists(info.qualifier, scope);
	const bool templated = scope->kind == ScopeKind::TemplateParameters && scope == template_scope_ && !pattern_member;
	// 14.5.3p5: a parameter pack is named only in the pattern of a pack
	// expansion; a variadic function template's function parameter that is
	// no function parameter pack (and its return type) expands none, so
	// neither names one outside the expansions in it.
	if (templated && info.type->kind == TypeKind::Function &&
		std::any_of(template_entities_.begin(), template_entities_.end(),
			[](const TemplateParameterEntity* parameter) { return parameter->pack; }))
	{
		PackList packs;
		for (uint32_t i = 0; i < info.type->param_count && packs.empty(); ++i)
		{
			const Type* parameter = info.type->params[i];
			if (parameter->kind != TypeKind::Expansion && dependent_type(parameter))
				unexpanded_packs(parameter, kAnyDepth, packs, nullptr, false);
		}
		if (packs.empty() && info.type->element != nullptr && dependent_type(info.type->element))
			unexpanded_packs(info.type->element, kAnyDepth, packs, nullptr, false);
		if (!packs.empty())
			error(location, "a function declaration names a parameter pack it does not expand");
	}
	// 14.5.2p3: a member function template is not virtual; 14.5.2p2: a
	// destructor is not a member template.
	if (templated && where->kind == ScopeKind::Class && specifiers.is_virtual)
		error(location, "a virtual member function template");
	if (templated && name.key == destructor_key_)
		error(location, "a destructor declared as a template");
	// Conversion functions share one lookup key, but the conversion type is
	// part of each one's name (12.3.2p1: `operator int` and `operator long`
	// are different functions), so it stays in the signature, as a function
	// template's return type does in its (1.3.18).
	const Type* signature = templated || name.kind == syntax::NameComponentKind::Conversion ? declared_type(info.type)
																							: signature_of(info.type);
	FunctionEntity* function = pattern_friend
		? nullptr
		: earlier_function(name, where, signature, templated, invisible, location);
	bool first = false;
	// 8.3.5p6, 12.1p4, 12.4p2: a ref-qualifier only on a non-static member
	// function other than a constructor or destructor.
	if (info.type->canonical->ref != RefQualifier::None &&
		(where->kind != ScopeKind::Class || specifiers.is_static || name.key == constructor_key_ ||
			name.key == destructor_key_))
		error(location, "a ref-qualifier on a function that is not a non-static member function");
	// 8.3.5p6, 9.4.1p2, 12.1p4, 12.4p2: likewise a cv-qualifier-seq.
	if (info.type->canonical->function_cv != 0 &&
		(where->kind != ScopeKind::Class || specifiers.is_static || name.key == constructor_key_ ||
			name.key == destructor_key_))
		error(location, "a cv-qualifier on a function that is not a non-static member function");
	if (function == nullptr && where->kind == ScopeKind::Class && !templated)
		check_member_overload(where, name.key, info.type, specifiers.is_static, location);
	// 3.6.1p3: a program that declares main inline, static or constexpr is
	// ill-formed (any declaration of the global namespace's main).
	if ((specifiers.is_inline || specifiers.is_static || specifiers.is_constexpr) &&
		where->kind == ScopeKind::Namespace && where->parent == nullptr && !templated && name.key != nullptr &&
		std::strcmp(name.key->spelling, "main") == 0)
		error(location, "main declared inline, static or constexpr");
	if (function != nullptr)
		redeclare_function(function, specifiers, info, scope, where, definition, deleted,
			template_depth_ == 0 || pattern_member || templated, location);
	else
	{
		function = new_function(specifiers, info, name, scope, where, signature, templated, location);
		first = true;
	}
	// 8.3.6p6: a class template member's default arguments are given on
	// its declaration in the class; 8.3.6p4: only a non-template function's
	// later declarations may add some.
	if ((pattern_member || (templated && !first)) && HasDefaultArgument(info.parameters))
		error(location,
			pattern_member ? "a default argument added outside the class template"
						   : "a default argument added by a later declaration of a function template");
	if (templated && first)
	{
		// A function template's head's shape keys its redeclarations.
		TemplateHead* head = template_head(*template_list_, template_scope_, template_entities_);
		head->shape = template_shape(head->parameters, head->count);
		function_template_facts(function)->head = head;
		for (const syntax::FunctionQualifier& qualifier : info.function_suffix != nullptr
				? info.function_suffix->qualifiers
				: syntax::Slice<syntax::FunctionQualifier>())
			function->templ->trailing_result = function->templ->trailing_result ||
				qualifier.kind == syntax::FunctionQualifierKind::TrailingReturn;
	}
	else if (templated)
		merge_template_head(HeadOf(function), *template_list_, template_scope_, location);
	const bool was_defined = function->defined;
	function->is_inline = function->is_inline || specifiers.is_inline || specifiers.is_constexpr;
	function->defined = function->defined || definition || deleted;
	function->deleted = function->deleted || deleted;
	if (semantic() && template_depth_ == 0 && !templated)
	{
		if (!first && definition)
			virtual_definition(function, location);
		virtual_declaration(function, specifiers, info, first, location);
		// C++14's 7.1.6.4p14: a function with a placeholder return type is
		// not virtual (nor overrides a virtual one).
		if (function->is_virtual && function->placeholder_result)
			error(location, "a virtual function with a placeholder return type");
		const Type* canonical = info.type->canonical;
		check_abstract_object(canonical->element, location);
		for (uint32_t i = 0; i < canonical->param_count; ++i)
			check_abstract_object(canonical->params[i], location);
	}
	if (befriending != nullptr)
		befriend_function(function, befriending);
	if (semantic() && (template_depth_ == 0 || templated))
		check_operator_declaration(function, location);
	Binding* binding = new_binding(BindingKind::Function, name.key, function, info.type);
	binding->label = name.label;
	binding->invisible = invisible;
	print_in(print, binding);
	if (pattern_friend)
		binding->declared_in = where;
	else
		declare_in(where, binding);
	// A friend no visible declaration declared is found through its
	// class only (3.4.2p4).
	if (invisible && befriending != nullptr && !pattern_friend)
		names_.befriend(befriending->scope, binding);
	// A template's definition keeps its own parameter names and template
	// parameter scope (its instantiations use them).
	const bool keep_definition = was_defined && !definition && (templated || pattern_member);
	if (info.parameters != nullptr && !keep_definition)
		function->parameters = info.parameters;
	if ((templated && (first || definition)) || (pattern_member && definition))
		function->template_parameters = scope;
	if (semantic() && template_depth_ == 0 && info.parameters != nullptr)
	{
		// Typed now unless the declaration is a member's in its class (see
		// record_default_arguments): they name no local variable (8.3.6p7).
		const Scope* saved_defaults = default_argument_scope_;
		default_argument_scope_ = info.after;
		record_default_arguments(function, info.parameters, info.after);
		default_argument_scope_ = saved_defaults;
		if (!first)
			note_added_default_arguments(function, *info.parameters);
	}
	else if (semantic() && templated && info.parameters != nullptr)
		record_template_default_arguments(function, *info.parameters, scope);
	if (semantic() && template_depth_ != 0 && info.parameters != nullptr)
		default_argument_names(function, info.parameters, info.after);
	redeclared_ = !first;
	return binding;
}

Binding* Analyzer::qualified_friend(
	const Specifiers& specifiers, const DeclaratorInfo& info, Scope* scope, uint32_t location)
{
	Scope* where = declaration_scope(scope);
	// 11.3p? (7.3.1.2p3): a qualified friend declaration names a function
	// its namespace or class declared before; it befriends the function
	// and declares no name (lookup, ADL included, finds it only as its
	// own declarations make it visible).
	if (where->kind != ScopeKind::Class || where->entity == nullptr)
		error(location, "friend declaration outside a class");
	// A templated class's is its instances' to resolve (14.6.5p1); a
	// friend template names a function template of its qualifier
	// (14.5.4p1), matched by template equivalence (14.5.6.1).
	if (templated_class(where) || info.name->last().has_arguments)
		return friend_specialization(info, scope, location);
	const DeclaredName name = declared_name(*info.name, info.qualifier);
	implicit_members_for(info.qualifier, name.key);
	// 14.5.4p5: under a class template's own template-head, a member of
	// the class template (not a member template), befriended for every
	// specialization.
	const bool friend_template = scope->kind == ScopeKind::TemplateParameters && scope == template_scope_ &&
		!(templated_class(info.qualifier) && !MemberTemplateLists(info.qualifier, scope));
	const FunctionList* same = friend_template
		? names_.overload(info.qualifier, name.key, declared_type(info.type),
			  template_shape(template_entities_.data(), static_cast<uint32_t>(template_entities_.size())))
		: names_.overload(info.qualifier, name.key, signature_of(info.type));
	if (same == nullptr || same->binding->entity->owner != info.qualifier)
		error(location, "a qualified friend declaration names no declared function");
	FunctionEntity* function = static_cast<FunctionEntity*>(same->binding->entity);
	befriend_function(function, static_cast<ClassEntity*>(where->entity));
	// 7.1.2p5, 9.2p9: no `virtual` or virt-specifier here.
	if (semantic() && template_depth_ == 0)
		virtual_declaration(function, specifiers, info, false, location);
	return same->binding;
}

FunctionEntity* Analyzer::earlier_function(
	const DeclaredName& name, Scope* where, const Type* signature, bool templated, bool& invisible, uint32_t location)
{
	// The function (or function template) an earlier declaration in the
	// scope declared that this one redeclares, else null; a function a
	// using-declaration introduced is hidden or conflicts.
	FunctionEntity* function = nullptr;
	// A declaration of a builtin function's name in the global namespace
	// redeclares the builtin (GCC's), declared first.
	if (where == graph_.global && !templated)
		builtin_function(name.key);
	if (const NameEntry* entry = names_.find(where, name.key))
	{
		// A using-declaration naming a dependent base's member declared a
		// placeholder whatever it names (functions, which this one joins or
		// hides, in the instantiations).
		const bool dependent_member = entry->type != nullptr && IsDependentMember(entry->type);
		if (entry->value != nullptr && entry->value->kind != BindingKind::Function)
			error(location, "function conflicts with an earlier declaration");
		if (entry->type != nullptr && (entry->type->kind != BindingKind::Type || IsClassTemplate(entry->type)) &&
			!dependent_member)
			error(location, "function conflicts with an earlier declaration");
		if (entry->type != nullptr && entry->type->entity->kind == EntityKind::TemplateParameter && !dependent_member)
			error(location, "function conflicts with a template parameter");
		// 14.5.6.1p4: an equivalent function template declaration (the same
		// template parameter count, and the same function type up to
		// parameter names) redeclares the template; the overload index keys
		// templates by both and holds only the scope's own.
		const FunctionList* same = templated
			? names_.overload(where, name.key, signature,
				  template_shape(template_entities_.data(), static_cast<uint32_t>(template_entities_.size())))
			: names_.overload(where, name.key, signature);
		// The index holds a scope's own templates only: a base's member
		// template a using-declaration introduced, of the same
		// parameter-type-list, qualifiers and an equivalent template-head
		// (searched in a class some using-declaration introduced one into).
		if (same == nullptr && templated && where->kind == ScopeKind::Class &&
			introduced_templates_.find(where) != PointerMap::kMissing)
		{
			const Type* shape = template_shape(
				template_entities_.data(), static_cast<uint32_t>(template_entities_.size()));
			for (const FunctionList* node = entry->functions; node != nullptr && same == nullptr; node = node->next)
			{
				const FunctionEntity* other = static_cast<const FunctionEntity*>(node->binding->entity);
				if (other->owner != where && other->templated &&
					signature_of(other->signature) == signature_of(signature) && HeadOf(other) != nullptr &&
					HeadOf(other)->shape == shape)
					same = node;
			}
		}
		if (same != nullptr && same->binding->entity->owner != where)
		{
			// Introduced by a using-declaration: a member hides a base member
			// (or member template) of the same signature (7.3.3p15);
			// elsewhere the two declarations conflict (7.3.3p14).
			if (where->kind != ScopeKind::Class)
				error(location, "function conflicts with a using-declaration");
			names_.remove_function(where, name.key, same->binding->entity);
		}
		else if (same != nullptr)
		{
			function = static_cast<FunctionEntity*>(same->binding->entity);
			invisible = invisible && same->binding->invisible;
		}
	}
	return function;
}

void Analyzer::redeclare_function(FunctionEntity* function, const Specifiers& specifiers, const DeclaratorInfo& info,
	Scope* scope, const Scope* where, bool definition, bool deleted, bool exceptions, uint32_t location)
{
	// A later declaration of a function agrees with the earlier ones
	// (`exceptions`: its exception-specification is checked).
	// 13.1p2: functions differing only in return type cannot overload;
	// C++14's 7.1.6.4p13: a placeholder return type is redeclared as
	// the placeholder (not the type its definition deduced).
	if (declared_type(function->type)->element != declared_type(info.type)->element ||
		function->placeholder_result != holds_auto(info.type->element))
		error(location, "functions that differ only in their return type");
	if (where->kind == ScopeKind::Class && info.qualifier == nullptr && !specifiers.is_friend)
		error(location, "member function redeclared in its class");
	// 8.4.3p4: a deleted definition is the first declaration.
	if (deleted)
		error(location, "deleted definition of a function declared before");
	if (definition && function->defined)
		error(location, "redefinition of a function");
	// 7.1.1p7-p8: successive declarations agree on linkage; a static
	// one cannot follow a declaration with external linkage.
	if (specifiers.is_static && where->kind == ScopeKind::Namespace && !function->internal_linkage)
		error(location, "a static declaration of a function declared with external linkage");
	// A builtin's declarations need not repeat its linkage or exception
	// facts (GCC's are implicit).
	const bool builtin = function->builtin != BuiltinFunction::None;
	if (where->kind == ScopeKind::Namespace && info.qualifier == nullptr && !function->implicit && !builtin &&
		function->c_linkage != c_language_)
		agree_language_linkage(function->c_linkage, location);
	// 7.1.5p1: if any declaration of a function or function template is
	// constexpr, all of them are.
	if (function->is_constexpr != specifiers.is_constexpr && !function->implicit)
		error(location, "declarations of a function that disagree on constexpr");
	// 15.4p3: once a declaration does not allow all exceptions, every
	// declaration has a compatible exception-specification (a
	// destructor's omitted one is its implicit one, 12.4p3, 15.4p14,
	// known where its class is not a template). A replaceable allocation
	// function declared only implicitly (3.7.4p2, no explicit declaration
	// yet: none has given its parameter clause) is not compared with that
	// declaration, as GCC and the reference do; one <new> declared is.
	const bool implicit_allocation = function->allocation != AllocationRole::None && function->parameters == nullptr;
	if (exceptions && !function->implicit && !builtin && !implicit_allocation)
		check_redeclared_exceptions(function, info, info.after != nullptr ? info.after : scope, location);
	else if (implicit_allocation && info.function_suffix != nullptr && template_depth_ == 0)
	{
		// Its own types are still complete ones (15.4p2).
		for (const syntax::FunctionQualifier& qualifier : info.function_suffix->qualifiers)
		{
			for (uint32_t i = 0;
				qualifier.kind == syntax::FunctionQualifierKind::Throw && i < qualifier.throw_types.size; ++i)
			{
				if (const Type* type = type_id(*qualifier.throw_types[i], info.after != nullptr ? info.after : scope))
					check_complete_throw_type(AdjustedThrowType(types_, type), qualifier.throw_types[i]->location);
			}
		}
	}
	if (info.asm_label != nullptr && graph_.asm_label(function) == nullptr)
		graph_.set_asm_label(function, info.asm_label);
}

FunctionEntity* Analyzer::new_function(const Specifiers& specifiers, const DeclaratorInfo& info,
	const DeclaredName& name, Scope* scope, Scope* where, const Type* signature, bool templated, uint32_t location)
{
	// A function's first declaration (of a function template, `templated`):
	// its kind, linkage (3.5p3-p6) and language linkage (7.5p1), operator
	// and exception facts.
	// 8.3p1: a qualified declarator-id names a member its class or
	// namespace declared before; a function template's by template
	// equivalence (14.5.6.1p4-p6, 14.5.2p1).
	if (info.qualifier != nullptr)
		error(location, "qualified definition does not match a declaration");
	FunctionEntity* function = new_entity<FunctionEntity>(EntityKind::Function, name.key, where, info.type);
	function->label = name.label != nullptr ? name.label : name.key->spelling;
	if (info.asm_label != nullptr)
		graph_.set_asm_label(function, info.asm_label);
	function->templated = templated;
	function->signature = signature;
	function->placeholder_result = holds_auto(info.type->element);
	function->member = where->kind == ScopeKind::Class;
	function->is_static = function->member && specifiers.is_static;
	function->is_constexpr = specifiers.is_constexpr;
	function->is_virtual = specifiers.is_virtual;
	function->constructor = name.key == constructor_key_;
	function->destructor = name.key == destructor_key_;
	function->conversion = name.key == conversion_key_;
	if (function->member && info.type->canonical->ref != RefQualifier::None)
		++ref_qualified_members_;
	if (function->conversion && where->kind == ScopeKind::Class)
		static_cast<ClassEntity*>(where->entity)->has_conversion_functions = true;
	if (specifiers.is_virtual && where->entity != nullptr && where->entity->kind == EntityKind::Class)
		static_cast<ClassEntity*>(where->entity)->polymorphic = true;
	if (where->kind == ScopeKind::Namespace)
	{
		// 3.5p3-p4 and 7.5p1 for the first declaration.
		// A C-linkage function of an unnamed namespace is the one function
		// of its name in every namespace (7.5p6), external as GCC and
		// Clang take it.
		function->internal_linkage = specifiers.is_static ||
			(static_cast<const NamespaceEntity*>(where->entity)->internal && !c_language_);
		function->c_linkage = c_language_;
		if (!templated)
			agree_block_linkage(function, function->c_linkage, function->internal_linkage, location);
	}
	else if (where->kind == ScopeKind::Block && !templated)
	{
		// 3.5p6: a block-scope function declaration denotes the function
		// a visible earlier declaration of the enclosing namespace
		// declared (or a hidden one with external linkage, 3.5p9), and
		// takes its linkage; it keeps its own entity, whose default
		// arguments are its own (8.3.6p4).
		const FunctionList* same = names_.overload(enclosing_namespace(where), name.key, signature);
		FunctionEntity* outer = same != nullptr ? static_cast<FunctionEntity*>(same->binding->entity) : nullptr;
		if (outer != nullptr && (!outer->internal_linkage || !hidden_below_namespace(where, name.key)))
		{
			// 3.5p10: with the function's type (13.1p2; C++14's
			// 7.1.6.4p13: a placeholder return type as the placeholder).
			if (declared_type(outer->type)->element != declared_type(info.type)->element ||
				outer->placeholder_result != function->placeholder_result)
				error(location, "a block-scope declaration of a function with another type");
			function->internal_linkage = outer->internal_linkage;
			function->c_linkage = outer->c_linkage;
			function->denotes = outer->denotes != nullptr ? outer->denotes : outer;
		}
		else
		{
			function->c_linkage = c_language_;
			note_block_linkage(function, where, c_language_);
		}
	}
	if (name.kind == syntax::NameComponentKind::LiteralOperator)
		function->operator_token = TokenKind::KW_OPERATOR;
	if (name.kind == syntax::NameComponentKind::Operator)
	{
		function->is_operator = true;
		function->operator_token = info.name->last().op;
		function->operator_array = info.name->last().array_form;
		// 12.5p1, p4: a class's allocation and deallocation functions
		// are static members.
		if (function->member &&
			(function->operator_token == TokenKind::KW_NEW || function->operator_token == TokenKind::KW_DELETE))
			function->is_static = true;
		// 13.5p6: any other operator function that is a member is a
		// non-static one; C++23's static operator() is an extension of
		// hosted compilations (its calls are a static member's, 13.3.1p4).
		else if (function->is_static && !(hosted_ && function->operator_token == TokenKind::OP_LPAREN))
			error(location, "a static member operator function");
	}
	function->nothrow = NonThrowingSpecification(info.function_suffix);
	function->exception_spec = HasExceptionSpecification(info.function_suffix);
	function->system_nothrow = function->nothrow && function->c_linkage && positions_ != nullptr &&
		positions_->in_system_header(location);
	pending_noexcept(function, info.function_suffix, info.parameters, info.after != nullptr ? info.after : scope);
	record_throw_specification(function, info.function_suffix, info.after != nullptr ? info.after : scope);
	// 15.4p15: a deallocation function (3.7.4.2) without an explicit
	// exception-specification is noexcept(true).
	if (function->is_operator && function->operator_token == TokenKind::KW_DELETE && !function->exception_spec)
		function->nothrow = true;
	function->is_explicit = specifiers.is_explicit;
	// explicit(condition): decided now, or (in a template) by each
	// specialization, the condition possibly naming its parameters.
	if (specifiers.explicit_condition != nullptr && template_depth_ == 0)
		function->is_explicit = explicit_value(*specifiers.explicit_condition, scope, true);
	else if (specifiers.explicit_condition != nullptr)
	{
		function->is_explicit = false;
		explicit_index_.set(function, static_cast<uint32_t>(explicits_.size()));
		explicits_.push_back(PendingExplicit{ specifiers.explicit_condition, scope });
	}
	return function;
}

Binding* Analyzer::friend_specialization(const DeclaratorInfo& info, Scope* scope, uint32_t location)
{
	// 14.5.4p1: a friend declaration whose declarator-id is a template-id
	// befriends that specialization of a function template declared before
	// (its arguments explicit or deduced from the declared type, 14.8.2.6).
	// A templated class's is its instances' to resolve (14.6.5p1): the
	// pattern's names a placeholder of no scope.
	Scope* where = declaration_scope(scope);
	ClassEntity* befriending = static_cast<ClassEntity*>(where->entity);
	FunctionEntity* function = nullptr;
	if (!templated_class(where))
	{
		if (!info.name->last().has_arguments)
			error(location, "a qualified friend declaration names no declared function");
		function = declared_specialization(info, scope, location);
		if (function == nullptr)
			error(location, "a friend declaration names no function template specialization");
		befriend_function(function, befriending);
	}
	else
	{
		function = new_entity<FunctionEntity>(EntityKind::Function, specialization_key(*info.name),
			info.qualifier != nullptr ? info.qualifier : enclosing_namespace(where), info.type);
		function->label = function->name->spelling;
		function->signature = signature_of(info.type);
	}
	Binding* binding = new_binding(BindingKind::Function, function->name, function, info.type);
	binding->hidden = true;
	binding->invisible = true;
	binding->declared_in = function->owner;
	return binding;
}

void Analyzer::check_member_overload(
	Scope* scope, const Identifier* key, const Type* type, bool is_static, uint32_t location)
{
	// 13.1p2: member functions of one name and parameter-type-list cannot
	// overload when one is static, or when some but not all have a
	// ref-qualifier (cv-qualifiers aside). The overload index is probed
	// with each qualification of the parameter-type-list (a conversion
	// function's signature keeps its type, which is part of its name,
	// 12.3.2p1); a signature no declaration formed is not interned.
	// A non-static function without a ref-qualifier (the common case) can
	// conflict only with a static one (its own signature, a redeclaration,
	// unless it is cv-qualified) or with a ref-qualified one (probed only
	// once the unit declared some).
	const Type* canonical = declared_type(type);
	const Type* result = key == conversion_key_ ? canonical->element : types_.fundamental(FundamentalType::Void);
	const bool plain = !is_static && canonical->ref == RefQualifier::None;
	static const RefQualifier kRefs[] = { RefQualifier::None, RefQualifier::LValue, RefQualifier::RValue };
	for (uint8_t cv = 0; cv <= (kConst | kVolatile); ++cv)
	{
		for (const RefQualifier ref : kRefs)
		{
			if (plain &&
				(ref == RefQualifier::None ? cv != 0 || canonical->function_cv == 0 : ref_qualified_members_ == 0))
				continue;
			const Type* signature = types_.find_function(
				result, canonical->params, canonical->param_count, canonical->variadic, cv, ref);
			const FunctionList* node = signature != nullptr ? names_.overload(scope, key, signature) : nullptr;
			if (node == nullptr)
				continue;
			const FunctionEntity* other = static_cast<const FunctionEntity*>(node->binding->entity);
			if (other->owner != scope || other->templated)
				continue;
			if (other->is_static != is_static)
				error(location, "a static and a non-static member function with the same parameters");
			if ((ref == RefQualifier::None) != (canonical->ref == RefQualifier::None))
				error(
					location, "member functions with the same parameters, some with a ref-qualifier and some without");
		}
	}
}

const Type* Analyzer::signature_of(const Type* function)
{
	// The canonical parameter-type-list and qualifiers, as a function type
	// returning void (13.1: the return type does not distinguish overloads),
	// as declarations compare types (see declared_type).
	const Type* canonical = declared_type(function);
	return types_.function(types_.fundamental(FundamentalType::Void), canonical->params, canonical->param_count,
		canonical->variadic, canonical->function_cv, canonical->ref);
}

void Analyzer::function_definition(const syntax::FunctionDecl& decl, Scope* scope)
{
	Specifiers specifiers;
	Scope* saved_class = declaration_class_;
	Scope* saved_deferral = access_deferral_;
	if (scope->kind == ScopeKind::Namespace)
		declaration_class_ = member_declaration_class(decl.declarator, scope);
	const size_t pending = begin_declaration_access(scope);
	const bool saved_owner_specifiers = owner_specifiers_;
	Scope* saved_home = placeholder_home_;
	owner_specifiers_ = template_owner_ != nullptr && scope == template_scope_;
	placeholder_home_ = nullptr;
	if (scope == template_scope_)
		qualifying_template(decl, scope, &placeholder_home_);
	this->specifiers(decl.specifiers, scope, specifiers, false);
	owner_specifiers_ = saved_owner_specifiers;
	placeholder_home_ = saved_home;
	declaration_class_ = saved_class;
	if (specifiers.type == nullptr)
		error(decl.location, "function definition without a return type");
	// A placeholder return type (the course's extension, see apply_suffix)
	// keeps the decl-specifiers' cv for its deduction; it is the defined
	// function's own return type only.
	const Type* declared = specifiers.type == auto_placeholder_ && specifiers.cv != 0
		? types_.qualified(auto_placeholder_, specifiers.cv)
		: specifiers.type;
	auto_declarator_ = specifiers.type == auto_placeholder_ && !specifiers.is_typedef;
	placeholder_suffix_ = auto_declarator_ ? OwnFunctionSuffix(decl.declarator) : nullptr;
	const DeclaratorInfo info = declarator(decl.declarator, declared, scope, decl.location, &specifiers);
	auto_declarator_ = false;
	placeholder_suffix_ = nullptr;
	if (info.type->kind != TypeKind::Function || info.name == nullptr)
		error(decl.location, "function definition of a non-function");
	Binding* binding = declare_function(specifiers, info, scope, true, decl.location);
	settle_declaration_access(pending, pending, binding);
	access_deferral_ = saved_deferral;
	note_user_provided(static_cast<FunctionEntity*>(binding->entity), nullptr);
	decl.declared = binding;
	bool packed = false, no_inline = false;
	attribute_alignment(decl.attributes, scope, packed, no_inline);
	static_cast<FunctionEntity*>(binding->entity)->no_inline =
		static_cast<FunctionEntity*>(binding->entity)->no_inline || no_inline;
	effect_attributes(static_cast<FunctionEntity*>(binding->entity), decl.attributes, decl.declarator);
	symbol_attributes(binding->entity, decl.attributes, decl.declarator);
	// The function scope sits where the definition does: in its class or
	// namespace for a qualified definition, and a friend defined in a class
	// stays in the class's scope for lookup (3.4.1p9).
	Scope* home = info.qualifier != nullptr ? info.qualifier : scope;
	// The declared type (a constexpr member's made const, 7.1.5p8).
	const Type* type = binding->type->kind == TypeKind::Function ? binding->type : info.type;
	static_cast<FunctionEntity*>(binding->entity)->definition_location = decl.location + 1;
	define_function(static_cast<FunctionEntity*>(binding->entity), type, info.parameters, decl.body, home, info.after);
}

void Analyzer::special_member(const syntax::SpecialMemberDecl& decl, Scope* scope)
{
	Specifiers specifiers;
	this->specifiers(decl.specifiers, scope, specifiers, false);
	if (specifiers.type != nullptr)
		error(decl.location, "constructor, destructor or conversion function with a type");
	const syntax::Name& name = *decl.name;
	Scope* owner = declaration_scope(scope);
	if (name.qualified())
		owner = name_prefix(name, scope, true).scope;
	if (owner == nullptr || owner->kind != ScopeKind::Class)
		error(decl.location, "constructor, destructor or conversion function outside its class");
	const syntax::NameComponent& last = name.last();
	const Type* result = types_.fundamental(FundamentalType::Void);
	// A conversion-type-id is looked up like the names after the
	// declarator-id (3.4.3p3, 3.4.5p7).
	Scope* after = name.qualified() ? qualified_lookup_scope(owner, scope) : scope;
	if (last.kind == syntax::NameComponentKind::Conversion)
		result = type_id(*last.conversion, after);
	else if (last.kind == syntax::NameComponentKind::Identifier && last.identifier != owner->entity->name)
		error(decl.location, "constructor name does not match its class");
	// 12.4p1: `~` followed by the class's own name.
	else if (last.kind == syntax::NameComponentKind::Destructor &&
		(last.destructor_decltype || last.identifier != owner->entity->name))
		error(decl.location, "destructor name does not match its class");
	const DeclaratorInfo info = declarator(decl.declarator, result, scope, decl.location);
	// 12.3.2p1: a conversion function has no parameters.
	if (last.kind == syntax::NameComponentKind::Conversion &&
		(info.type->canonical->param_count != 0 || info.type->canonical->variadic))
		error(decl.location, "a conversion function with parameters");
	const bool deleted = decl.initializer != nullptr && decl.initializer->kind == syntax::InitializerKind::Delete;
	Binding* binding = declare_function(specifiers, info, scope, decl.definition, decl.location, deleted);
	FunctionEntity* function = static_cast<FunctionEntity*>(binding->entity);
	if (semantic() && template_depth_ == 0)
		pure_specifier(function, decl.initializer,
			declaration_scope(scope)->kind == ScopeKind::Class && !specifiers.is_friend && !name.qualified(),
			decl.location);
	note_user_provided(function, decl.initializer);
	bool packed = false, no_inline = false;
	attribute_alignment(decl.attributes, scope, packed, no_inline);
	function->no_inline = function->no_inline || no_inline;
	effect_attributes(function, decl.attributes, decl.declarator);
	symbol_attributes(function, decl.attributes, decl.declarator);
	if (decl.initializer != nullptr && decl.initializer->kind == syntax::InitializerKind::Default)
		default_definition(function, !name.qualified(), decl.location);
	decl.declared = binding;
	if (function->constructor)
	{
		// 12.8p6: no constructor takes its own class by value (with any
		// further parameters defaulted).
		const Type* type = function->type->canonical;
		bool copies_by_value = type->param_count != 0 && type->params[0]->unqualified == owner->entity->type;
		for (uint32_t i = 1; copies_by_value && i < type->param_count; ++i)
			copies_by_value = function->default_arguments != nullptr && function->default_arguments[i] != nullptr;
		if (copies_by_value)
			error(decl.location, "a constructor taking its own class by value");
		static_cast<ClassEntity*>(owner->entity)->user_declared_constructor = true;
	}
	if (decl.definition)
	{
		function->definition_location = decl.location + 1;
		define_function(function, binding->type->kind == TypeKind::Function ? binding->type : info.type,
			info.parameters, decl.body, name.qualified() ? owner : scope, info.after);
	}
}

void Analyzer::default_definition(FunctionEntity* function, bool first, uint32_t location)
{
	// 8.4.2p1: only a special member function may be explicitly defaulted
	// (a default or copy or move constructor, a copy or move assignment
	// operator, a destructor), with the implicit declaration's type but
	// for the reference parameter's const (p1-p2) and without default
	// arguments (p1). Defaulted on its first declaration it is not
	// user-provided (p4); its definition is the implicit one either way
	// (p5).
	const ClassEntity* entity = function->member && function->owner->kind == ScopeKind::Class
		? static_cast<const ClassEntity*>(function->owner->entity)
		: nullptr;
	const Type* type = function->type->canonical;
	bool special = false;
	if (entity != nullptr && function->constructor)
	{
		special = true;
		for (uint32_t i = 0; special && i < type->param_count; ++i)
			special = function->default_arguments != nullptr && function->default_arguments[i] != nullptr;
		special = special || copy_kind(function, entity) != CopyKind::None;
	}
	else if (entity != nullptr && function->destructor)
		special = true;
	else if (entity != nullptr && function->name == assign_key_)
	{
		// X& operator=(const X&) or X& operator=(X&&): the result an
		// lvalue reference to X (8.4.2p1), the parameter a reference.
		const CopyKind kind = copy_kind(function, entity);
		special = kind != CopyKind::None && type->params[0]->is_reference() &&
			type->element->kind == TypeKind::LValueReference && type->element->element == entity->type;
	}
	if (!special)
		error(location, "only special member functions can be defaulted");
	for (uint32_t i = 0; function->default_arguments != nullptr && i < type->param_count; ++i)
	{
		if (function->default_arguments[i] != nullptr)
			error(location, "an explicitly defaulted function has a default argument");
	}
	function->defined = true;
	function->synthesized = true;
	function->defaulted = function->defaulted || first;
	// Defaulted on its first declaration it is defined in its class (`=
	// default ;` is a function-body, 8.4.1), so inline (7.1.2p3): every
	// unit odr-using it defines it (weak; a vtable's slot is a use).
	if (first)
	{
		function->is_inline = true;
		function->inline_member = true;
	}
	// 8.4.2p2 for one declared constexpr on its first declaration: decided
	// once the unit's classes are complete (see check_defaulted_constexprs;
	// a class template specialization's is merely not constexpr, 7.1.5p6).
	if (first && function->is_constexpr && entity != nullptr && semantic() && template_depth_ == 0 &&
		!instance_class(function->owner))
		defaulted_constexprs_.push_back(DefaultedConstexpr{ function, location });
	if (first || !semantic() || template_depth_ != 0)
		return;
	// p5: defaulted after its first declaration (its class complete), it
	// may not be implicitly defined as deleted.
	ClassEntity* cls = const_cast<ClassEntity*>(entity);
	if (function->copy_kind != CopyKind::None)
		defaulted_copy_facts(function, cls, function->copy_kind);
	else if (function->constructor && function->inits == nullptr)
		implicit_default_facts(cls, function);
	else if (function->destructor)
		function->deleted = function->deleted || destructor_deleted(cls);
	if (function->deleted)
		error(location, "an explicitly defaulted function is defined as deleted");
	// Not defaulted on its first declaration, it is user-provided (8.4.2p4),
	// so not trivial (12.1p5, 12.4p5, 12.8p12, p25): it is called.
	function->trivial = false;
	check_defaulted_constexpr(function, location);
	// A virtual destructor defined here looks up its deallocation function.
	virtual_definition(function, location);
}

void Analyzer::bit_field_declaration(const syntax::BitFieldDecl& decl, Scope* scope)
{
	if (scope->kind != ScopeKind::Class)
		error(decl.location, "bit-field outside a class");
	Specifiers specifiers;
	this->specifiers(decl.specifiers, scope, specifiers, false);
	if (specifiers.type == nullptr || specifiers.type == auto_placeholder_)
		error(decl.location, "bit-field without a type");
	// 7.6.2p1: an alignment-specifier may not apply to a bit-field.
	for (const syntax::Attribute* attribute = decl.attributes; attribute != nullptr; attribute = attribute->next)
	{
		if (attribute->kind == syntax::AttributeKind::Alignas)
			error(attribute->location, "an alignment-specifier on a bit-field");
	}
	ClassEntity* owner = static_cast<ClassEntity*>(scope->entity);
	for (const syntax::BitFieldDeclarator& field : decl.declarators)
	{
		const Type* type = specifiers.type;
		DeclaratorInfo info = DeclaratorInfo();
		for (const syntax::Declarator* part = field.declarator; part != nullptr; part = part->nested)
		{
			for (const syntax::Attribute* attribute = part->attributes; attribute != nullptr;
				attribute = attribute->next)
			{
				if (attribute->kind == syntax::AttributeKind::Alignas)
					error(attribute->location, "an alignment-specifier on a bit-field");
			}
		}
		if (field.declarator != nullptr)
		{
			info = declarator(field.declarator, type, scope, decl.location);
			type = info.type;
		}
		// 9.6p3: an integral or enumeration type and a non-negative width,
		// zero only when unnamed; not an atomic one (C11 6.7.2.1p5, Clang).
		if (!IsIntegralType(type) && type->unqualified->kind != TypeKind::Enum &&
			!(template_depth_ != 0 && dependent_type(type)))
			error(decl.location, "bit-field of non-integral type");
		if ((type->cv & kAtomic) != 0)
			error(decl.location, "bit-field of an atomic type");
		uint64_t width = 0;
		if (template_depth_ == 0)
		{
			Constant value;
			if (semantic())
				integral_constant_operand(*field.width, scope, nullptr);
			constant_value(*field.width, scope, value, true);
			if (IsScopedEnumeration(value.type) || (IsSignedConstant(value) && static_cast<int64_t>(value.bits) < 0))
				error(field.width->location, "invalid bit-field width");
			width = value.bits;
		}
		if (info.name == nullptr)
		{
			Entity* unnamed = new_entity<VariableEntity>(EntityKind::Variable, nullptr, scope, type);
			add_data_member(owner, unnamed, width, true);
			continue;
		}
		if (width == 0 && template_depth_ == 0)
			error(decl.location, "named bit-field of width zero");
		Binding* binding = declare_variable(specifiers, info, nullptr, scope, decl.location);
		if (owner->last_member != nullptr && owner->last_member->variable == binding->entity)
		{
			owner->last_member->width = width;
			owner->last_member->bit_field = true;
			static_cast<VariableEntity*>(binding->entity)->bit_field = true;
		}
	}
}

void Analyzer::require_definition_types(const Type* type, uint32_t location)
{
	// 8.3.5p9: a function definition's parameter and return types are not
	// incomplete classes (a member defined in its class is analyzed once
	// the class is complete, so its own class is): a class template
	// specialization among them is instantiated (14.7.1p1), whether or not
	// the body needs it.
	const Type* canonical = type->canonical;
	for (uint32_t i = 0; i <= canonical->param_count; ++i)
	{
		const ClassEntity* cls = ClassOf(i < canonical->param_count ? canonical->params[i] : canonical->element);
		if (cls != nullptr && !require_complete(cls))
			error(location, "a parameter or return type of a function definition is incomplete");
	}
}

void Analyzer::define_function(FunctionEntity* function, const Type* type, const syntax::ParameterClause* parameters,
	const syntax::FunctionBody& body, Scope* print, Scope* parent)
{
	// A body in a class member-specification waits for the outermost class
	// (9.2p2); such an inline member is emitted where odr-used.
	function->body = &body;
	function->definition_ordinal = definition_point();
	// A constexpr function is emitted where odr-used too (course view).
	if (function->is_constexpr)
		function->inline_member = true;
	// A member (or friend) defined in a class template instance's
	// member-specification: its body is its pattern's, instantiated when
	// the function is odr-used (14.7.1p2).
	if (class_depth_ != 0 && instance_class(member_scope_) && !function->templated)
	{
		function->inline_member = true;
		function->is_inline = true;
		function->instantiated = true;
		return;
	}
	if (class_depth_ != 0)
	{
		function->inline_member = true;
		function->is_inline = true;
		// A member function an unnamed class defines (the lowering names
		// such classes as Clang does, ProgramLowering::name_deferred_members).
		if (unnamed_class_depth_ != 0)
			graph_.unnamed_member_definitions = true;
		if (function->is_constexpr)
			deferred_constexpr_.set(function, static_cast<uint32_t>(deferred_.size()));
		deferred_.push_back(DeferredBody{ function, type, parameters, &body, print, parent });
		return;
	}
	// An inline function, emitted only where odr-used, that calls a reserved
	// builtin name (17.6.4.3.2p1) no registry holds and nothing declares: a
	// hosted header's wrapper of a builtin this compiler lacks, whose
	// emission then fails (README: an unimplemented reserved builtin may
	// defer its wrapper, but ordinary lookup and type errors in it are
	// still diagnosed). Its body is analyzed; only the full-expressions
	// holding such a call (a mem-initializer's: the ctor-initializer) are
	// name-checked rather than typed, and a constant evaluation reaching
	// one fails (the interpreter's untyped expressions).
	if (function->is_inline && !function->templated && template_depth_ == 0 && reserved_wrapper(body, parent))
	{
		function->unanalyzed_body = true;
		wrapper_body(function, type, parameters, body, print, parent, true);
		return;
	}
	function_body(function, type, parameters, body, print, parent);
}

bool Analyzer::reserved_wrapper(const syntax::FunctionBody& body, Scope* parent)
{
	// The body calls a reserved builtin name no registry holds (its first
	// such, FunctionBody::reserved_call) that nothing declares.
	return body.reserved_call != nullptr && semantic() &&
		!unqualified(parent, body.reserved_call, LookupFilter::Ordinary).found();
}

void Analyzer::wrapper_body(FunctionEntity* function, const Type* type, const syntax::ParameterClause* parameters,
	const syntax::FunctionBody& body, Scope* print, Scope* parent, bool reserved)
{
	// The deferral (see define_function) covers this body's own statements
	// and ctor-initializer, not another body analyzed inside it (a local
	// class's member, one a constant evaluation analyzes ahead), whose own
	// reserved calls decide, nor an instantiation (InstantiationContext).
	const uint32_t saved = deferring_reserved_;
	deferring_reserved_ = reserved ? 1 : 0;
	function_body(function, type, parameters, body, print, parent);
	deferring_reserved_ = saved;
	// Every deferred call is consumed where its full-expression began, or
	// by the body (see function_body).
	if (deferred_reserved_call())
		throw InternalError("a deferred reserved builtin call outside every full-expression", failure_.location);
}

} // namespace sema
