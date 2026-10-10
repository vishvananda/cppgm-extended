#pragma once

#include <cstdint>

// The words of a placeholder's structural key (PA15-PA18, see
// Analyzer::decltype_words and Analyzer::name_words): shared by the analysis,
// which keys equivalent operands (14.5.6.1p5-p6) by them, and the lowering,
// which mangles a keyed placeholder's operand from the resolutions they
// record (Itanium 5.1.6) without looking any name up again.
//
// A word's tag is its low byte, its value above it. A key is the three
// prefix words (the namespace scope, bit 0 set when the operand names a
// dependent entity or type; the class scope, bit 0 set for a value
// placeholder; a pointer or reference parameter's type, or 0), then the
// operand's words in prefix order:
// - an expression: kWordNode of its syntax::ExprKind (the operator, cast
//   keyword, count or flags above), its components after it; an integral
//   constant (kWordConstant); another literal (kWordLiteral); an absent
//   operand (kWordAbsent); `this` is its node then kWordThis;
// - a name: kWordName of its component count << 1 | global, then each
//   component (an identifier's Identifier pointer, an operator-function-id's
//   kWordName of its token << 1 | array form), each followed by kWordArguments
//   and its template arguments' canonical types when it has them; the first
//   component of a name looked up in its expression's scope is instead what
//   ordinary lookup found (kWordFreeName, kWordParameter, kWordEntity or
//   kWordType; a call's callee that is not dependent, kWordFunction value 1
//   and the function it resolves to when one is known), preceded by
//   kWordThis when it names a non-static member
//   where `this` may appear; a looked-up qualified name ends with what it
//   denotes: kWordType and its dependent qualifier's canonical type,
//   kWordFunction (value 1 and its function's entity after it when that is
//   one function and no template), kWordEntity and its entity, or
//   kWordAbsent.
namespace sema {

enum : uint64_t
{
	kWordNode = 1,
	kWordName = 2,
	// A name ordinary lookup does not find (for argument-dependent lookup),
	// or a dependent callee that finds functions (14.6.2p1): its Identifier
	// follows.
	kWordFreeName = 3,
	// A declaration's entity follows (a template parameter's canonical one).
	kWordEntity = 4,
	// A function parameter of a declarator: its position in its clause and
	// the parameter scopes between it and the reference (5.1.6's L) above
	// the tag (see ParameterWord); its canonical type follows.
	kWordParameter = 5,
	// A canonical type follows.
	kWordType = 6,
	// A component's template argument count; their canonical types follow.
	kWordArguments = 7,
	// A literal other than an integral constant: its spelling's length, the
	// spelling packed eight bytes to a word, then its canonical type (0 for
	// a literal the key does not type: a string, long double or
	// user-defined literal) and its value's bits (a floating literal's IEEE
	// bits at its type's width).
	kWordLiteral = 8,
	// `this` (value 1: its type in a member function's trailing return
	// type follows; 0: its class's entity follows).
	kWordThis = 9,
	kWordAbsent = 10,
	// An integral constant (a literal, true or false, a frame's constant
	// argument): its canonical type and value's bits follow.
	kWordConstant = 11,
	// A qualified name that names functions: an overload set or a template
	// (value 0: resolved where the placeholder is substituted, named by its
	// last component), or one function that is no template or the one
	// specialization its explicit template arguments give (value 1: its
	// entity follows). Value 1 also starts an unqualified callee that is not
	// dependent and resolves to such a function.
	kWordFunction = 12
};

const uint32_t kPlaceholderKeyPrefix = 3;

inline uint64_t Word(uint64_t tag, uint64_t value)
{
	return tag | value << 8;
}

inline uint64_t WordTag(uint64_t word)
{
	return word & 0xff;
}

inline uint64_t WordValue(uint64_t word)
{
	return word >> 8;
}

inline uint64_t ParameterWord(uint64_t index, uint64_t level)
{
	return Word(kWordParameter, index | level << 24);
}

inline uint32_t ParameterIndex(uint64_t word)
{
	return static_cast<uint32_t>(WordValue(word) & 0xffffff);
}

inline uint32_t ParameterLevel(uint64_t word)
{
	return static_cast<uint32_t>(WordValue(word) >> 24);
}

} // namespace sema
