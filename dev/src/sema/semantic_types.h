#pragma once

#include <cstdint>

#include "types/fundamental_type.h"

// The type model of the semantic graph: interned types and integral
// constants. sema/type_table.h interns them; entities, scopes and bindings
// are in sema/semantic_graph.h.
namespace sema {

struct Entity;

enum class TypeKind : uint8_t
{
	Fundamental,
	Class,
	Enum,
	// A template type parameter (`typename T`).
	TemplateParameter,
	// A template template-parameter.
	TemplateTemplateParameter,
	Pointer,
	LValueReference,
	RValueReference,
	MemberPointer,
	Array,
	Function,
	// Template arguments, interned like types so argument lists
	// compare by pointer: an integral non-type argument's value (`element`
	// its unqualified integral or enumeration type, `bound` its bits as a
	// Constant keeps them), an argument pack (14.5.3: `params` its
	// elements), and a pack expansion (14.5.3p4: `element` its pattern,
	// which names parameter packs), dependent until substituted. A pointer
	// or reference argument (14.3.2p1) is a Value too: `element` its
	// pointer, std::nullptr_t or lvalue reference type, `entity` the
	// variable or function it designates (null for a null pointer); before
	// its parameter's type is known, an object or function named alone is
	// an lvalue reference to its type (see Analyzer::address_argument).
	Value,
	Pack,
	Expansion,
	// A template-name as a template template-argument (14.3.3): a
	// class template or an alias template (`entity`).
	Template,
	// A parameter pack an instantiation binds (14.5.3p5), named in
	// a pack expansion's pattern that an instance's copy keeps because
	// the pattern also names packs of a template declared in it (a member
	// template's, `pair<T, U>...`): `element` the argument pack, `entity`
	// the pack's positional parameter, `bound` its depth counted from the
	// outermost template (instances counting as the templates they
	// instantiate). Substitution and deduction take element i of it for
	// element i of that expansion (see Analyzer::substitute_list).
	BoundPack,
	// A GNU vector (vector_size, Clang's ext_vector_type; hosted
	// compilations, PA29): `bound` lanes of the scalar `element`. Its size
	// and alignment are the lanes' total rounded up to a power of two (a
	// bool vector's lanes are bits, Clang's); it is list-initialized lane by
	// lane, copied whole, compared lane by lane and converted, reduced and
	// bit-cast by the builtins. In a template a dependent lane type or size
	// keeps the size's expression as `owner` (a byte size if `variadic`),
	// as a dependent array bound does.
	Vector,
	// GNU's complex types (`_Complex T`, `__complex__ T`; hosted
	// compilations): a pair of `element` (real, imaginary), of twice its
	// size and its alignment, whose parts __real__ and __imag__ designate;
	// arithmetic types for the usual arithmetic conversions (C99 6.3.1.8).
	Complex,
	// Clang's block pointer `R (^)(P...)` (hosted compilations' Apple
	// headers): `element` the block's function type; a pointer to the
	// block literal, whose invoke function (at offset 16, taking the
	// literal first) a call runs.
	BlockPointer
};

// A class member's access (clause 11), as a declaration gives it.
enum class Access : uint8_t
{
	Public,
	Protected,
	Private
};

// cv-qualifier bits, and C11's _Atomic (Clang's C++ extension, PA29): an
// atomic type is its type so qualified, of its own size and alignment, read
// and written atomically.
enum : uint8_t
{
	kConst = 1,
	kVolatile = 2,
	kAtomic = 4
};

enum class RefQualifier : uint8_t
{
	None,
	LValue,
	RValue
};

// An interned type: structurally equal types share one record, so type
// identity is pointer identity. `canonical` is the same type with every
// function parameter adjusted (8.3.5p5: arrays and functions to pointers,
// top-level cv-qualifiers removed); declarations keep the source form,
// which the dump prints, while matching and signatures compare canonical
// types.
struct Type
{
	TypeKind kind;
	// Top-level cv-qualifiers; never on arrays (qualifying an array
	// qualifies its elements, 8.3.4p1), references or functions.
	uint8_t cv;
	// Function: a member function's cv-qualifier-seq.
	uint8_t function_cv;
	RefQualifier ref;
	// Function: the parameter list ends with `...`.
	bool variadic;
	FundamentalType fundamental;
	// Plain char keeps its own identity when -funsigned-char selects its
	// unsigned representation. Fixed when the type table is constructed.
	bool plain_char_unsigned;
	// A compound type's dependence on a template parameter, once computed
	// (0 unknown, 1 no, 2 yes; see Analyzer::dependent_type): a property of
	// its components, fixed once they exist.
	mutable uint8_t dependence;
	uint32_t param_count;
	uint32_t hash;
	// Array: the bound, 0 when unknown (or dependent, see `owner`).
	uint64_t bound;
	const Type* canonical;
	// The type without its top-level cv-qualifiers.
	const Type* unqualified;
	// Pointee, referenced type, element type, return type, or the member
	// type of a pointer to member.
	const Type* element;
	// MemberPointer: the class. Array: a bound that depends on template
	// parameters, as the placeholder of its expression (see
	// Analyzer::apply_suffix; null for a known or omitted bound). Vector:
	// likewise its size (see Analyzer::vector_type).
	const Type* owner;
	// Class, Enum, TemplateParameter, TemplateTemplateParameter.
	Entity* entity;
	const Type* const* params;

	bool is(TypeKind k) const { return kind == k; }
	bool is_reference() const { return kind == TypeKind::LValueReference || kind == TypeKind::RValueReference; }
};

// An integral constant (5.19): its type (an integral or enumeration type,
// unqualified) and value, kept sign-extended to 64 bits for signed types
// and zero-extended for unsigned ones.
struct Constant
{
	uint64_t bits;
	const Type* type;
};

} // namespace sema
