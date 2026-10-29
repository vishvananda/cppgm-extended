#pragma once

#include <cstddef>
#include <cstdint>

#include "sema/semantic_graph.h"
#include "syntax/ast.h"

// Records the analyzer passes between its declaration, type and body
// handlers (see sema/analyzer.h).
namespace sema {

// What a decl-specifier-seq said.
struct Specifiers
{
	// The type with its cv-qualifiers, the `auto` placeholder, or null when
	// no type specifier was written (constructors and similar).
	const Type* type;
	uint8_t cv;
	bool auto_type;
	bool is_typedef;
	bool is_friend;
	bool is_extern;
	bool is_static;
	bool is_constexpr;
	bool is_constinit;
	bool is_inline;
	bool is_virtual;
	bool is_thread_local;
	bool is_mutable;
	bool is_explicit;
	// A class-specifier or an enum-specifier with a body.
	bool defines;
	// Another decl-specifier precedes the class or enum specifier.
	bool leading_specifiers;
	// An elaborated-type-specifier or opaque-enum-declaration.
	bool elaborated;
	// The GNU alignment of the typedef-name naming the type (0: none; see
	// Analyzer::alias_alignments_).
	uint32_t alias_alignment;
	uint32_t location;
	// explicit(constant-expression) (C++20's, which the course's grammar
	// accepts): its condition.
	const syntax::Expr* explicit_condition;
	// The class or enumeration a class-specifier, enum-specifier or
	// elaborated-type-specifier of the sequence named, and the line its
	// declaration printed (an unnamed class's waits for a declarator).
	Entity* declared;
	Binding* declared_binding;
	const syntax::DeclSpecifier* type_specifier;
	// NameLookup::dependent_uses before the sequence was analyzed: a
	// block-scope declaration whose analysis changed it declares a
	// dependent binding (see Binding::dependent).
	uint64_t uses;
	// Only present when the sequence contains GNU mode attributes.
	syntax::Slice<syntax::DeclSpecifier> mode_specifiers;
};

// A declarator's type and declarator-id.
struct DeclaratorInfo
{
	const Type* type;
	// Null for an abstract declarator.
	const syntax::Name* name;
	// The scope a qualified declarator-id's nested-name-specifier names.
	Scope* qualifier;
	// Where names after the declarator-id are looked up (3.4.3p3): the
	// declaration's scope, or the qualifier (bridged to the declaration's
	// template parameters).
	Scope* after;
	// When the declarator declares a function: the parameter clause of that
	// function (not of a function type nested in its return type), and the
	// suffix holding it with its qualifiers and exception specification.
	const syntax::ParameterClause* parameters;
	const syntax::DeclaratorSuffix* function_suffix;
	// The suffix ending the whole declarator when it holds virt-specifiers
	// (9.2: a member-declarator's virt-specifier-seq follows its
	// declarator, the function's own suffix or, for `void (*f())() final`,
	// its return type's).
	const syntax::DeclaratorSuffix* virt_suffix;
	// When the declarator declares an array: its bound as written (null
	// for an omitted one).
	const syntax::Expr* bound;
	uint32_t location;
	// A GNU asm label's symbol name (null for none).
	const char* asm_label;
};

// The name a declaration declares: the lookup key and, for constructors,
// operators, conversion functions and destructors, the printed label.
struct DeclaredName
{
	const Identifier* key;
	const char* label;
	syntax::NameComponentKind kind;
};

// What a nested-name-specifier names: a namespace, class or scoped
// enumeration scope, or an unscoped enumeration (no scope of its own); or,
// in a template, a type that depends on a template parameter (whose
// members are known only in instantiations, 14.6.2).
struct NamePrefix
{
	Scope* scope;
	Entity* entity;
	const Type* dependent;
};

// A member function body, waiting for its outermost class to be complete
// (a complete-class context, 9.2p2).
// A member function's body, or (without a body) the default arguments of
// one of its declarations, waiting for the outermost class (9.2p2).
struct DeferredBody
{
	FunctionEntity* function;
	const Type* type;
	const syntax::ParameterClause* parameters;
	const syntax::FunctionBody* body;
	// Where the function scope is printed, and where its lookup continues.
	Scope* print;
	Scope* parent;
	// Semantics mode: a non-static data member's brace-or-equal-initializer
	// (with a null function), typed in its complete class.
	VariableEntity* member;
	// A constexpr static data member whose in-class initializer converts to
	// a base of a class being defined (with a null function): its constant
	// evaluated once the class is complete.
	VariableEntity* constant;
};

// How constructors are considered for a class object's initialization:
// direct-initialization (13.3.1.3), copy-initialization by the converting
// constructors (13.3.1.3), from an expression of another type (13.3.1.4:
// the argument by standard conversions only, 13.3.3.1p4), and copy-list-
// initialization (13.3.1.7).
enum class ConstructionKind : uint8_t
{
	Direct,
	Copy,
	Converting,
	CopyList
};

// A trailing return type's prototype scope and the clause it belongs to.
struct PendingPrototype
{
	const syntax::ParameterClause* clause;
	Scope* scope;
};

// An empty class subobject of the class being laid out, or a class
// subobject whose empty subobjects a layout search visits.
struct EmptySubobject
{
	const ClassEntity* type;
	uint64_t offset;
	// A complete object (a member's), whose virtual bases the walk visits.
	bool complete;
};

// What a search of a component's empty class subobjects does with them.
enum class EmptyWalk : uint8_t
{
	// Finds one of the type and offset of one placed before.
	Conflict,
	// Records them all (an empty base's or empty member's).
	Record,
	// Records those a later component can meet (a non-empty base's or a
	// member's): below Analyzer::empty_record_limit_, where an empty base
	// or empty potentially-overlapping member is tried first, or at or
	// past Analyzer::empty_record_tail_, the data size its placement
	// leaves (later components start there).
	RecordReachable
};

// A class waiting in Analyzer::lay_out for the classes its layout reads:
// the next base and member to look at.
struct LayoutFrame
{
	ClassEntity* entity;
	const BaseClass* base;
	const DataMember* member;
};

// A switch statement being analyzed (semantics mode): its promoted
// condition type, whether it has a default label, and where its case values
// start on the analysis's stack of them.
struct SwitchContext
{
	const Type* type;
	bool has_default;
	size_t first_value;
	// Automatic variables of the switch body in scope at this point whose
	// declarations a jump to a later label would bypass (6.7p3), and the
	// try blocks and handlers being analyzed where it began (15p2).
	uint32_t initialized;
	uint32_t handled;
};

// An expression operand waiting in the name walk's work list.
struct PendingExpr
{
	const syntax::Expr* expr;
	// An unevaluated operand (5p8): names are checked, nothing is odr-used.
	bool unevaluated;
};

} // namespace sema
