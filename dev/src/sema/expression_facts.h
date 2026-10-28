#pragma once

#include <cstdint>

#include "sema/semantic_graph.h"

// What semantic analysis established about expressions: each analyzed
// syntax::Expr points to one ExprFacts in the semantic arena. The records
// are typed facts for later phases and for the --emit-semantics view; the
// tree is not copied.
namespace sema {

// 3.10p1.
enum class ValueCategory : uint8_t
{
	PRValue,
	LValue,
	XValue
};

// The first step of a standard conversion sequence (13.3.3.1.1).
enum class LvalueTransform : uint8_t
{
	None,
	LvalueToRvalue,
	ArrayToPointer,
	FunctionToPointer
};

// The second step: a promotion or conversion (clause 4).
enum class ValueConversion : uint8_t
{
	None,
	// 4.5, 4.6.
	IntegralPromotion,
	FloatingPromotion,
	// 4.7, 4.8, 4.9.
	IntegralConversion,
	FloatingConversion,
	FloatingIntegral,
	// 4.10: a null pointer constant to a pointer, member pointer or
	// std::nullptr_t; an object pointer to cv void*; a pointer to a derived
	// class to a pointer to its base (also a reference binding to a base
	// subobject, 13.3.3.1.4p1).
	NullPointer,
	PointerToVoid,
	DerivedToBase,
	// 4.11p2: a pointer to member of a base class to a pointer to member
	// of a class derived from it (the null value stays null).
	MemberToDerived,
	// 4.12: from an arithmetic or unscoped enumeration type, and from a
	// pointer, pointer to member or std::nullptr_t (which ranks below
	// other conversions, 13.3.3.2p4).
	Boolean,
	PointerBoolean,
	// GNU complex types: a real value to a complex type (its real part,
	// the imaginary part zero; ComplexReal rank), or a complex value to
	// another complex type part by part (Conversion rank; Promotion when
	// the parts promote, 4.5, 4.6).
	Complex
};

// 13.3.3.1.1 Table 12; a user-defined conversion sequence ranks below a
// standard one, an ellipsis conversion below all (13.3.3.2p2). GNU: a real
// value made complex ranks below every standard conversion (GCC and Clang
// prefer `f(int)` to `f(_Complex double)` for a double argument).
enum class ConversionRank : uint8_t
{
	Exact,
	Promotion,
	Conversion,
	ComplexReal,
	UserDefined,
	Ellipsis
};

enum class ConversionForm : uint8_t
{
	Standard,
	// 8.5.3: a reference binds to the operand (directly) or to a temporary
	// initialized from it by the standard conversion recorded.
	Reference,
	// An argument matched by `...` (13.3.3.1.3).
	Ellipsis
};

// An implicit conversion sequence (13.3.3.1) from an expression to the type
// its context needs: an initialized object or reference, a parameter, an
// operand.
struct Conversion
{
	ConversionForm form;
	LvalueTransform first;
	ValueConversion second;
	// The third step, a qualification adjustment (4.4).
	bool qualification;
	ConversionRank rank;
	// The conversion initializes an object, parameter or result (8.5)
	// rather than preparing an operand of a built-in operator (5p9-p10).
	bool initialization;
	// Reference: binds an rvalue reference; binds to a temporary; the
	// operand is a function lvalue.
	bool rvalue_reference;
	bool temporary;
	bool function_lvalue;
	// The implicit object parameter of a member function declared without
	// a ref-qualifier (13.3.3.2p3: no rvalue-reference preference).
	bool implicit_object;
	// A braced-init-list converted to std::initializer_list<X> (13.3.3.1.5p2:
	// its elements' worst conversion), which 13.3.3.2p3 prefers.
	bool initializer_list;
	// The type converted to (the reference type for a binding).
	const Type* target;
	// The type the standard conversion yields (for a binding, the referenced
	// type or the temporary's).
	const Type* result;
	// A user-defined conversion (13.3.3.1.2): the converting constructor
	// that makes the class object (the temporary a binding refers to) or
	// the conversion function whose result converts (12.3.2), and the
	// conversion of the operand to its parameter (the implicit object
	// parameter's binding); a conversion function's second standard
	// conversion, from its result to the target (else null).
	const FunctionEntity* constructor;
	const Conversion* argument;
	const Conversion* after;
};

// ExprFacts::flags.
enum : uint32_t
{
	// An integer literal with value zero: a null pointer constant (4.10p1 as
	// CWG 903 resolved it).
	kNullLiteral = 1,
	// Names a set of overloaded functions, resolved by its context (13.4).
	kOverloadSet = 2,
	// Stands for its constant value (a folded builtin query, a
	// value-initialized scalar, an enumerator).
	kFolded = 4,
	// Designates a bit-field (8.5.3p5 binds no non-const reference to it).
	kBitField = 8,
	// A member function call or member access through the implicit `this`
	// (9.3.1p3).
	kImplicitThis = 16,
	// A direct call of `entity` (its callee designates the function).
	kDirectCall = 32,
	// A member of an anonymous union named through the name its enclosing
	// scope injected (9.5p5): an access to the union's unnamed object.
	kAnonymousMember = 64,
	// A subscript written index first (`1[a]`, 5.2.1p1).
	kCommuted = 128,
	// A cast to a reference type (its operand as a glvalue of the target
	// type, 5.2.9p2, 5.4) or to a pointer-to-member type (a member pointer
	// value): the view shows the operand with the cast's type.
	kReferenceCast = 256,
	// A non-static member function named through an object: only callable
	// (5.2.5p4).
	kBoundMember = 512,
	// A static data member with a constant value (9.4.2p3): read as that
	// value (`value`), which is no odr-use (3.2p2).
	kStaticConstant = 1024,
	// An operator expression that calls the operator function `entity`
	// (13.3.1.2); its operands are the call's arguments (a member's object
	// first), their conversions recorded on them.
	kOperatorCall = 2048,
	// A type-generic builtin call (sema::Intrinsic in `value`), expanded in
	// place.
	kIntrinsic = 4096,
	// A pseudo-destructor name or call (5.2.4): of a scalar object, no
	// effect but the object expression's evaluation.
	kPseudoDestructor = 8192,
	// A user-defined string literal (2.14.8p5): the call of its literal
	// operator (`entity`) with the literal and its length.
	kUserLiteral = 16384,
	// A class operand of a built-in operator (or a condition) converted by
	// the conversion function `entity` (13.3.1.2p7, 13.3.1.5): the call's
	// result, the operand's own facts in `original`.
	kUserConverted = 32768,
	// A direct call of a virtual function not named by a qualified-id: the
	// final overrider in the object's dynamic type is called (5.2.2p1,
	// 10.3p15).
	kVirtualCall = 65536,
	// A static_cast (or a C-style cast taken as one, 5.4p4) between a class
	// and a base of it: its result designates the operand's base class
	// subobject (a reference cast, 5.2.9p2, or a C-style pointer cast also
	// casting away constness; another pointer's is its recorded derived-
	// to-base conversion) or the derived object containing the operand
	// (5.2.9p2, p11).
	kBaseCast = 131072,
	kDerivedCast = 262144,
	// A sizeof or alignof whose queried type is a class template
	// specialization: a literal beside it keeps its widening (the course's
	// presentation of such arithmetic).
	kSpecializationQuery = 524288,
	// Its analysis named something whose type may depend on template
	// parameters (NameLookup::dependent_uses): in an instantiation, it may
	// be type-dependent in its template (14.6.2.2).
	kDependentUse = 1048576,
	// Value dependence retained after instantiation (14.6.2.3), distinct
	// from type dependence: an integral parameter or sizeof(T) has a fixed
	// result type, but makes an array bound's declared type dependent.
	kValueDependentUse = 2147483648u,
	// An operand of an implied construct the analysis made (6.5.4p1's
	// equivalent of a range-based for): typed when made, its facts are
	// returned as they are when it is typed again.
	kImplicit = 2097152,
	// A typeid of a glvalue of polymorphic class type (5.2.8p2): its operand
	// is evaluated and the type information of the object's dynamic type
	// designated; any other typeid designates that of `target` (p3-p5).
	kDynamicType = 4194304,
	// `typeid(a) == typeid(b)` or `!=`: the course's std::type_info
	// compares its objects' addresses (each type's object is unique).
	kTypeIdentity = 8388608,
	// A dynamic_cast checked at run time (5.2.7p8): to a class derived
	// from the operand's, or unrelated to it.
	kRuntimeCheck = 16777216,
	// A braced-init-list initializing std::initializer_list<E>: the array
	// `const E[N]` (`target`) its elements initialize, and the object
	// referring to it (8.5.4p5).
	kInitializerList = 33554432,
	// A conditional operand known never to be evaluated (its temporaries
	// made without cleanups). The analysis no longer guesses one from a
	// local's literal initializer (the course keeps every emitted arm's
	// cleanups), so none is set at present.
	kThenUnreachable = 67108864,
	kElseUnreachable = 134217728,
	// An array new- or delete-expression of class elements whose class's
	// usual operator delete[] (its member lookup's, 12.5p4-p5, whatever
	// the expression's `::` or placement arguments) takes the size: a host
	// object's array then has a cookie (Itanium 2.7).
	kSizedArrayDelete = 268435456,
	// The braced list of an object of static storage duration's own
	// initializer that gives its class's flexible array member (GNU, hosted
	// compilations) elements: the object's data hold them after its other
	// members, the object growing by their bytes (GCC's size).
	kFlexibleElements = 536870912,
	// A delete-expression whose object's class is incomplete at it
	// (5.3.5p5): no destructor runs and the global deallocation function
	// frees the storage (undefined if the complete class's destructor is
	// non-trivial or it declares one).
	kIncompleteDelete = 1073741824
};

// The call a user-defined literal makes (2.14.8; ExprFacts::value of a
// kUserLiteral expression): a string literal's (the array and its length,
// p5), a number's value converted to the literal operator's parameter
// (p3-p4: unsigned long long or long double), a literal operator
// template's specialization of the number's characters, without arguments,
// a character literal's value converted to the parameter of the literal
// operator its call selects (p6), or a raw literal operator's (p3-p4: the
// number's characters as a null-terminated string, `operator "" X("n")`).
enum : uint64_t
{
	kLiteralCallString = 0,
	kLiteralCallCooked = 1,
	kLiteralCallTemplate = 2,
	kLiteralCallCharacter = 3,
	kLiteralCallRaw = 4
};

struct ExprFacts
{
	// 5p5, 5p6: never a reference type; glvalues keep their cv-qualifiers.
	// Null while an overloaded function name is unresolved.
	const Type* type;
	ValueCategory category;
	uint32_t flags;
	// What an id-expression, member access or direct call designates or
	// selected: a variable, parameter, function, enumerator or member.
	const Entity* entity;
	// The conversion the expression's context applies to it, if any
	// (records are shared: one per distinct conversion in a unit).
	const Conversion* conversion;
	// One of, by the expression's kind:
	union
	{
		// A function name: the functions its lookup found (kOverloadSet
		// while unresolved).
		const FunctionList* functions;
		// kFolded, a literal or sizeof: the value's bits in `type` (as a
		// Constant keeps them).
		// Conditional: whether its builtin condition permits hosted reference
		// folding, recorded once when typed, independent of the selected arm.
		uint64_t value;
		// kReferenceCast: the cast's reference or member pointer type; a
		// compound assignment: the type its operation is performed in
		// (5.17p7: E1 op E2 after the usual arithmetic conversions, a
		// shift's promoted E1, or E1's pointer type).
		const Type* target;
		// kUserConverted: the converted operand's own facts.
		const ExprFacts* original;
	};

	Constant constant() const { return Constant{ value, type }; }
};

// The functions a definition (or the translation unit's namespace-scope
// code) odr-uses, in the order met: the demand edges that decide which
// implicitly defined and inline member functions a unit emits.
struct DemandList
{
	FunctionEntity* function;
	// Called, not only named (its address taken): a member declared but
	// not defined in the unit is then emitted as a declaration.
	bool call;
	DemandList* next;
};

} // namespace sema
