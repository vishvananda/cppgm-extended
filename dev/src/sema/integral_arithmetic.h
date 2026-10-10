#pragma once

#include <cstdint>

#include "lex/token_kind.h"
#include "sema/semantic_graph.h"

namespace sema {

class TypeTable;

// The value representation of an integral type (x86-64, LP64): width in
// bits and signedness. bool has width 1.
struct IntegralKind
{
	unsigned bits;
	bool is_signed;
};

// For an integral type, or an enumeration through its underlying type;
// false for other types and for an enumeration whose underlying type is not
// yet known.
bool IntegralKindOf(const Type* type, IntegralKind& kind);
bool IsIntegralType(const Type* type);
// A 128-bit integer type (or an enumeration with one underlying). The
// constant interpreter evaluates its values in 128 bits (sema::WideOf); a
// Constant of such a type (an enumerator, a template argument, an integral
// constant expression's value) holds its value's low 64 bits, extended by
// the type's signedness, and a value they cannot hold is no such constant.
bool IsWideIntegral(const Type* type);
// The value converted to `to` is held that way: false for a 128-bit type
// that would need more than its low 64 bits (an unsigned 64-bit value of
// 2^63 or more as a signed type, a negative one as an unsigned type).
bool HeldAsWideConstant(const Constant& value, IntegralKind to);
bool IsScopedEnumeration(const Type* type);
bool IsSignedConstant(const Constant& value);

// A value's bits (sign-extended when its type is signed) converted to an
// integral type (4.7p2, 4.7p3 as GCC defines it: modulo 2^N, then
// sign-extended for signed types); bool is `bits != 0`.
uint64_t ConvertIntegral(uint64_t bits, IntegralKind to);
// The value is unchanged by conversion to `to` (no narrowing, 8.5.4p7).
bool Representable(const Constant& value, IntegralKind to);

// A built-in multiplicative, additive or bitwise operator (5.6, 5.7,
// 5.11-5.13) on values already converted to the operation's type `kind`:
// false when the operation is undefined (5p4: a signed result outside the
// type's range; 5.6p4: a zero divisor), so the expression is not a
// constant; `out` then holds the wrapped value.
bool IntegralOperation(TokenKind op, IntegralKind kind, uint64_t a, uint64_t b, uint64_t& out);
// A shift (5.8) of `a` (of the promoted left operand's type `kind`) by
// `count` (of the promoted right operand's type): false when the count is
// negative or not less than the width, or a signed left shift overflows
// (5.8p2 as amended by CWG 1457).
bool IntegralShift(TokenKind op, IntegralKind kind, uint64_t a, IntegralKind count_kind, uint64_t count, uint64_t& out);
// A relational or equality operator (5.9, 5.10) on values of one type.
bool IntegralCompare(TokenKind op, bool is_signed, uint64_t a, uint64_t b);

// The three, of a 128-bit type's values (the constant interpreter's; the
// integral evaluator keeps 64-bit constants).
bool WideOperation(TokenKind op, bool is_signed, unsigned __int128 a, unsigned __int128 b, unsigned __int128& out);
bool WideShift(
	TokenKind op, bool is_signed, unsigned __int128 a, IntegralKind count_kind, uint64_t count, unsigned __int128& out);
bool WideCompare(TokenKind op, bool is_signed, unsigned __int128 a, unsigned __int128 b);

// Integral promotion (4.5) of an integral or unscoped enumeration type.
const Type* PromotedType(TypeTable& types, const Type* type);
// The usual arithmetic conversions (5p10) of two promoted integral types.
const Type* CommonType(TypeTable& types, const Type* a, const Type* b);
// The first of int, unsigned int, long, unsigned long, long long and
// unsigned long long that can represent `value` (7.2p5, 7.2p6).
const Type* SmallestTypeFor(TypeTable& types, uint64_t bits, bool is_signed);

} // namespace sema
