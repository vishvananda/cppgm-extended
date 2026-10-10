#pragma once

#include <cstdint>
#include <stdexcept>

#include "lex/token_kind.h"
#include "sema/binding_records.h"
#include "sema/function_records.h"
#include "sema/semantic_types.h"
#include "sema/vtable_records.h"

struct Identifier;

namespace syntax {
struct Expr;
struct Name;
struct FunctionBody;
struct ParameterClause;
}

// The semantic graph of one translation unit: interned types
// (sema/semantic_types.h), entities, scopes, and the bindings that connect
// names in scopes to entities (sema/entity_records.h). Records live in the
// unit's semantic arena and are referenced by pointer; they point back into
// the syntax tree for source facts instead of copying it. sema/analyzer.h builds the graph,
// sema/lookup.h searches it and sema/semantic_dump.h prints the PA6 view
// of it.
namespace sema {

struct Entity;
struct Scope;
struct Binding;
struct ClassEntity;
struct FunctionEntity;
struct VariableEntity;
struct DemandList;

enum class BindingKind : uint8_t
{
	// Printed by the dump.
	Type,
	TypeAlias,
	Enumerator,
	Function,
	Variable,
	Parameter,
	// A non-type template parameter of a template definition: a
	// value-dependent prvalue of the parameter's type (`type`); its
	// entity is the TemplateParameterEntity. Instantiations bind the
	// parameter's name to its value, as an enumerator.
	TemplateValue,
	// Lookup only.
	Namespace,
	NamespaceAlias
};

// One declaration of a name: it binds the name in a scope to an entity and
// records what this declaration said about it.
struct Binding
{
	BindingKind kind;
	// A class member declaration's access (a using-declaration's own).
	Access access;
	// Not printed: an unnamed class's placeholder line until a declarator
	// names the class, and the lookup-only kinds.
	bool hidden : 1;
	// Not found by ordinary lookup (a friend declaration, 7.3.1.2p3); still
	// matched by redeclarations.
	bool invisible : 1;
	bool scoped : 1;
	// Declared by an instantiation with a type that may depend on its
	// template's parameters in the pattern (a parameter of a dependent
	// type, a block-scope declaration whose declaration named something
	// dependent, a local class, a non-type template parameter of a
	// dependent type bound in a frame): an expression naming it may be
	// type-dependent in its template (14.6.2.2p3; see
	// NameLookup::dependent_uses).
	bool dependent : 1;
	// A typedef with a GNU alignment (in Analyzer::alias_alignments_).
	bool aligned_alias : 1;
	// Type lines: the class-key or enum-key this declaration wrote (and
	// whether it was a scoped enumeration's).
	TokenKind key;
	// Its place in the unit's declaration order (a redeclaration of an
	// entity in its entry keeps the first visible declaration's): a
	// template definition's lookups at instantiation see the
	// namespace-scope declarations before its definition only (14.6.4.2,
	// see NameLookup::limit).
	uint32_t ordinal;
	const Identifier* name;
	// The printed name when it is not `name` (operators, conversion
	// functions, destructors, a qualified enumeration definition).
	const char* label;
	Entity* entity;
	// The type this declaration gives the entity: the source form of a
	// function declaration, a parameter's declared type.
	const Type* type;
	// Enumerator lines: the enumeration declaration whose spelling they
	// print.
	const Binding* declaration;
	// The next binding printed in the same scope.
	Binding* next;
	// The scope the binding was declared in (for a member, the class whose
	// member it makes the entity).
	const Scope* declared_in;
};

enum class ScopeKind : uint8_t
{
	Namespace,
	TemplateParameters,
	Class,
	Enum,
	Function,
	Block
};

struct Scope
{
	ScopeKind kind;
	// Printed by the dump (the parameter scope of a template
	// template-parameter is not).
	bool dumped;
	// The outermost block of a function body, handler or substatement: it
	// cannot redeclare the enclosing construct's names (3.3.3p2, 3.3.3p4).
	// A declarator's function parameter scope (Function, no entity): one
	// of the scopes a parameter-declaration-clause's later parameters'
	// types see its earlier parameters in (see Analyzer::apply_suffix), not
	// the trailing return type's (see Analyzer::parameter_word).
	bool guarded : 1;
	// A try block's or handler's block (15p2), or a statement expression's
	// (GCC): no jump enters it.
	bool handles : 1;
	// A class scope a using-declaration introduces a member into (7.3.3p1).
	bool using_members : 1;
	// The block of a selection statement's init-statement (see
	// syntax::Stmt::selection_init): its names are the condition's and the
	// substatements' outermost blocks' too (C++17 [stmt.if]p3,
	// [basic.scope.block]p3).
	bool selection_init : 1;
	// The template parameters of a definition outside its class or
	// namespace (a bridge, see Analyzer::qualified_lookup_scope, or an
	// instantiation's frames of one, see Analyzer::outside_frames): an
	// unqualified lookup that has passed them into that class searches
	// no template parameter scope beyond it (the class definitions' are
	// not in scope, 14.5.1p3, 14.6.1p7).
	bool bridge : 1;
	// A template parameter scope: the template parameter lists it stands
	// for in the depth of template parameters declared below it (1 for a
	// template-head's; none for an instantiation frame's, whose parameters
	// are bound; for the bridge of an out-of-class member definition, its
	// lists beyond one per enclosing class template, which the class's own
	// scopes already count).
	uint8_t lists;
	uint32_t id;
	// Where unqualified lookup continues: the lexical parent, or for an
	// out-of-class definition the scope of its class or namespace.
	Scope* parent;
	// The namespace, class, enumeration or function.
	Entity* entity;
	// The printed name of a namespace, class, enumeration or function scope.
	const char* label;
	Binding* bindings;
	Binding* last_binding;
	Scope* children;
	Scope* last_child;
	Scope* next_sibling;
	// Using-directives in this scope (7.3.4), with the implicit ones for
	// unnamed and inline member namespaces (7.3.1.1p1, 7.3.1p8).
	NamespaceList* directives;
	// Lookup traversal mark (cycle protection through directives).
	uint32_t visit;
	// Base-graph traversal mark of a class scope (each class once per
	// derivation test).
	uint32_t base_visit;
	// Block scopes (semantics mode): the automatic variables declared so
	// far whose initialization a jump into their scope would bypass
	// (6.7p3), and the enclosing block's count when this one began.
	uint32_t initialized;
	uint32_t initialized_at_entry;
};

// The template levels enclosing a scope that instances stand for (its
// enclosing classes instantiated from templates): a bound pack's and a
// member template's parameters' depths counted from the outermost
// template (sema/template_packs.cpp).
uint32_t InstanceLevels(const Scope* scope);

// Distinct functions of one name declared or introduced in one scope.
struct FunctionList
{
	Binding* binding;
	FunctionList* next;
};

// The declarations of one name in one scope, as lookup needs them: the
// newest type-like binding (class or enumeration name, typedef-name,
// namespace or alias, template parameter), the class or enumeration name
// itself (which a same-named typedef-name does not replace for elaborated
// lookup), and the newest other binding with the overload set of
// functions. A variable, function or enumerator hides a class or
// enumeration name of the same scope (3.3.10p2).
struct NameEntry
{
	Binding* type;
	Binding* tag;
	Binding* value;
	FunctionList* functions;
};

// A semantic error of a translation unit at a syntax-tree location.
class SemanticError : public std::runtime_error
{
public:
	SemanticError(uint32_t location, const char* message) : std::runtime_error(message), location_(location) {}

	uint32_t location() const { return location_; }

private:
	uint32_t location_;
};

} // namespace sema
