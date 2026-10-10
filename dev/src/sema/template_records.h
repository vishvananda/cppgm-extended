#pragma once

#include <cstdint>

#include "sema/semantic_graph.h"
#include "support/small_list.h"

namespace syntax {
struct Expr;
struct ClassSpecifier;
struct EnumSpecifier;
struct Initializer;
struct TypeId;
}

// Template records of the semantic graph: a template's parameter
// list, and the facts that connect class templates, their specializations
// and the member classes of both (sema/class_templates.cpp builds them).
namespace sema {

// A template's parameter list (14.1) as its declarations accumulate it:
// the parameters of its first declaration in order, and each position's
// default argument (14.1p10: defaults merge across declarations), a type-id
// or a non-type parameter's expression, analyzed in the parameter scope of
// the declaration that gave it.
// A function template's head also records the parameter scope of the
// declaration that gave its function parameters' default arguments, whose
// names they use (instantiated when a call uses them, 14.7.1p12).
struct TemplateHead
{
	uint32_t count;
	// The declaration ordinal of the first declaration: a function
	// template's default arguments' definition context (only that
	// declaration gives them, 8.3.6p4), and that of the default template
	// arguments it gives.
	uint32_t ordinal;
	TemplateParameterEntity** parameters;
	const syntax::TypeId** defaults;
	const syntax::Expr** default_values;
	// Each default's definition context (14.6.4.2): `ordinal`, or for a
	// redeclaration's, the declaration ordinal of the redeclaration that
	// gave it, kept aside (see Analyzer::default_ordinal).
	Scope** default_scopes;
	Scope* argument_defaults;
	// The position of its parameter pack, if one is last (a class
	// template's pack is, 14.1p11), else `count`.
	uint32_t trailing_pack;
	// A function or alias template's: its parameters' kinds, positions and
	// non-type parameters' types, as an interned Pack (equivalent heads,
	// 14.5.6.1p6, have one shape); null for other templates.
	const Type* shape;
};

inline bool HasDefault(const TemplateHead* head, uint32_t index)
{
	return head->defaults[index] != nullptr || head->default_values[index] != nullptr;
}

// A template argument list under construction (a template-id's, a
// deduction's): inline for the usual few arguments.
typedef SmallList<const Type*, 4> TypeList;

// The parameter packs a type names (by index at one depth, see
// Analyzer::unexpanded_packs).
typedef SmallList<uint32_t, 4> PackList;

// Template arguments substituted for the parameters of one depth (by
// position): a specialization's, or those deduced so far (null entries not
// yet known). A parameter pack's argument is an argument pack (a Pack
// type); while one element of a pack expansion is substituted, `elements`
// gives each pack the expansion expands the element's index (-1 for the
// others, which a nested expansion expands whole; see substitute_list),
// and `bound_element` that index for the bound packs (TypeKind::BoundPack)
// it expands (null outside such an element).
struct Substitution
{
	uint32_t depth;
	const Type* const* arguments;
	uint32_t count;
	const int32_t* elements;
	const uint32_t* bound_element;
};

enum class TemplateRole : uint8_t
{
	// A class template: its ClassEntity is the pattern (the current
	// instantiation inside its definition).
	Template,
	// A member class of a templated class, defined with it or out of class.
	Pattern,
	// X<arguments> of a class template.
	Specialization,
	// A member class of a specialization (or of such a member class).
	MemberInstance,
	// A class template partial specialization (14.5.5): a pattern with its
	// own template head, whose argument list (in terms of its parameters)
	// the specializations it matches deduce.
	Partial
};

// A class's template facts. A template's or pattern's definition (null
// until seen) and the template parameter scope whose names it uses (the
// template's own, an out-of-class member class definition's; null for a
// member class defined in its templated class); a partial
// specialization's too, with its template and argument pattern; a
// specialization's template and canonical arguments (`dependent` when some argument
// depends on a template parameter: never instantiated; `primary_arguments`
// when they are its template's own parameters in order, which name the
// template itself only where 14.6.2.1p1 makes it the current
// instantiation) and the hidden binding template-id lookups return; a
// member instance's pattern and enclosing instance. Instances are defined
// when first required to be complete.
struct ClassTemplateFacts
{
	TemplateRole role;
	bool dependent;
	bool primary_arguments;
	// A specialization declared by `template<>` (14.7.3): its definition,
	// if any, is its own, never instantiated.
	bool explicit_specialization;
	// A specialization named by an explicit instantiation definition, and
	// by an explicit instantiation declaration, which suppresses its
	// non-inline members' implicit instantiation (14.7.2p10).
	bool explicit_instantiation;
	bool extern_instantiation;
	// A template standing for an alias template (CWG 1558): an alias
	// template-id whose aliased type does not name all of its dependent
	// arguments is a dependent specialization of it (canonical arguments),
	// which substitution forms again, so a dropped argument's invalid
	// substitution is the substitution's failure; the alias template, and
	// each specialization's aliased type (what deduction and names see of
	// it), are rare facts kept aside (see Analyzer::stand_in_alias and
	// Analyzer::aliased_of).
	bool alias_stand_in;
	// A template's partial specializations that no argument indexes (the
	// first, each linking the next), and the argument positions (below 64)
	// that index the others: a partial specialization whose argument at
	// such a position depends on no parameter is found by that argument,
	// one whose argument there is a class template's specialization by
	// that template (see Analyzer::partial_index_key); a specialization's
	// matched partial specialization, or the member template of the class
	// template whose definition outside it an instance's member template's
	// specialization instantiated (null for the primary's own definition).
	ClassEntity* partials;
	ClassEntity* next_partial;
	uint64_t partial_positions;
	ClassEntity* partial;
	FactState definition_state;
	TemplateHead* head;
	const syntax::ClassSpecifier* definition;
	Scope* definition_parameters;
	ClassEntity* pattern;
	ClassEntity* enclosing;
	const Type* const* arguments;
	uint32_t argument_count;
	// A templated class's definition: the declaration ordinal where it is
	// (its definition context, 14.6.4.2).
	uint32_t definition_ordinal;
	Binding* binding;
	// The depth of the instantiation that made the entity (its own
	// instantiation runs one deeper; Annex B bounds the nesting).
	uint32_t depth;
	// A specialization: the argument-dependent lookup that last walked its
	// template arguments (3.4.2p2; see Analyzer::associated).
	uint32_t argument_mark;
	// A template standing for a dependent template-name: the
	// template template-parameter or dependent member template-name its
	// specializations (`TT<A...>`, dependent, arguments as written) apply.
	const TemplateParameterEntity* template_name;
};

// A static data member of a templated class: a pattern member's
// out-of-class definition (its initializer, the template parameter scope
// its names use and the scope its initializer's names were checked in,
// whose bridge holds the parameter names no member hides, where it is);
// an instance member's pattern, and the
// definition instantiated from it on demand (14.7.1p1: the copied
// initializer and the scope it was analyzed in, which the lowering reads).
struct VariableTemplateFacts
{
	const VariableEntity* pattern;
	FactState state;
	bool demanded;
	// An explicit instantiation definition names it (14.7.2p1, p8): its
	// unit defines it, used or not.
	bool explicit_definition;
	// A variable template declared with auto whose initializer is
	// dependent: the decl-specifiers' cv and (`auto_type`) the declared
	// type holding auto, from which each specialization deduces its type
	// when its initializer is instantiated (see variable_specialization).
	uint8_t auto_cv;
	// A pattern member defined out of its class: the declaration ordinal
	// where (its definition context, 14.6.4.2; 0 for an in-class one, whose
	// class's is its).
	uint32_t definition_ordinal;
	const syntax::Initializer* initializer;
	// A pattern member defined out of its class as an array: the bound its
	// definition writes (null for none), which completes an instance's
	// member declared with an omitted one (3.9p6, 8.3.4p3).
	const syntax::Expr* bound;
	Scope* parameters;
	Scope* scope;
	uint32_t location;
	// Variable templates (PA15, sema/variable_templates.cpp): a template's
	// (or a partial specialization's) head and its partial
	// specializations, unindexed and indexed as a class template's (see
	// ClassTemplateFacts::partials); a partial specialization's argument
	// pattern, or a specialization's canonical arguments (`pattern` its
	// template), and the binding its template-ids name.
	uint32_t argument_count;
	TemplateHead* head;
	const Type* const* arguments;
	VariableEntity* partials;
	VariableEntity* next_partial;
	uint64_t partial_positions;
	Binding* binding;
	const Type* auto_type;
};

// A member enumeration declared (opaquely) in a templated class and
// defined out of it (14.5.1.4): the pattern's definition and the template
// parameter scope whose names it uses, and the instances' enumerations
// declared before it was seen (linked through `next`), which it defines
// then; an instance's pattern and enclosing instance class. An instance
// declared after it is defined with its class (14.7.1p1).
struct EnumTemplateFacts
{
	const syntax::EnumSpecifier* definition;
	Scope* parameters;
	EnumEntity* waiting;
	EnumEntity* pattern;
	ClassEntity* enclosing;
	EnumEntity* next;
	// The pattern's definition context (14.6.4.2): the declaration
	// ordinal where it is defined.
	uint32_t definition_ordinal;
};

// A binding of a class template's name, which 14p5 makes unique in its
// scope: no variable, function or enumerator shares it (as one may share a
// class name, 3.3.1p4).
inline bool IsClassTemplate(const Binding* binding)
{
	return binding != nullptr && binding->entity != nullptr && binding->entity->kind == EntityKind::Class &&
		static_cast<const ClassEntity*>(binding->entity)->templ != nullptr &&
		static_cast<const ClassEntity*>(binding->entity)->templ->role == TemplateRole::Template;
}

// A binding of a dependent member name's placeholder (`Q::name` for a
// dependent Q, see TemplateParameterEntity): what it names, a type or a
// value, is known only in instantiations.
inline bool IsDependentMember(const Binding* binding)
{
	return binding->entity != nullptr && binding->entity->kind == EntityKind::TemplateParameter &&
		static_cast<const TemplateParameterEntity*>(binding->entity)->qualifier != nullptr;
}

// A non-type template parameter's type that takes a pointer or reference
// argument (14.1p4: a pointer to object or function, an lvalue
// reference to object or function, std::nullptr_t), and such an argument
// (see sema/template_addresses.cpp).
bool AddressParameterType(const Type* type);
bool AddressArgument(const Type* argument);

// A value placeholder's substitution waiting for non-dependent
// arguments (see TemplateParameterEntity::pending).
struct PendingSubstitution
{
	const TemplateParameterEntity* placeholder;
	// The canonical arguments, as a Pack.
	const Type* arguments;
	uint32_t depth;
	// The substitution of another depth applied before this one (its
	// expression names parameters of both), or null.
	const PendingSubstitution* earlier;
};

// An alias template (14.5.7): its template head, and as its type the
// type-id it aliases, in terms of its (positional) parameters; a
// template-id naming it is that type substituted (p2).
// `discarded`: the parameter positions (below 64) its aliased type does not
// name, whose dependent arguments an alias template-id keeps in its
// stand-in's specialization (see ClassTemplateFacts::alias).
struct AliasTemplateEntity : Entity
{
	TemplateHead* head;
	uint64_t discarded;
	ClassEntity* stand_in;
};

} // namespace sema
