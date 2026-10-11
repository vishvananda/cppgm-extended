#pragma once

#include <cstdint>
#include <vector>

#include "lex/token_spelling.h"
#include "types/fundamental_type.h"

namespace syntax {
struct Expr;
}

namespace sema {

struct Type;

// Whether an array of this element type is a character array, one a
// string literal initializes by its code units (8.5.2p1).
bool IsCharacterArrayElement(const Type* type);

// The array a string literal denotes (2.14.5p8, p13-p15): its element type
// from the encoding prefix, and its length in code units of that encoding
// with the terminating null. A literal of concatenated pieces is spelled
// with single spaces between them.
struct StringLiteralFacts
{
	FundamentalType element;
	uint64_t units;
	// A user-defined string literal's ud-suffix (2.14.8p8: the pieces'
	// suffixes agree), else empty.
	const char* suffix;
	uint32_t suffix_length;
};

// False for a malformed literal, pieces with different encoding prefixes
// or different ud-suffixes.
bool AnalyzeStringLiteral(const TokenSpelling& spelling, StringLiteralFacts& out);

// Also the code units' values, the terminating null included.
bool StringLiteralUnits(const TokenSpelling& spelling, StringLiteralFacts& out, std::vector<uint64_t>& units);

// GCC's __builtin_nan[f|l](text) of floating type `type`: a quiet NaN
// whose significand holds, below its quiet bit, the number a string
// literal `text` spells as strtoull reads it (a 0x or 0 prefix selecting
// the base); 0 for other text. As a long double whose conversion to the
// type keeps those bits.
long double BuiltinNan(FundamentalType type, const syntax::Expr& text);
// The payload number `text` spells that way (0 for other text).
uint64_t BuiltinNanPayload(const syntax::Expr& text);
// Whether `text` is a string literal GCC folds a NaN of (its payload
// read as above): an ordinary literal of the number's digits, optionally
// after white space and a sign.
bool BuiltinNanText(const syntax::Expr& text, uint64_t& payload);
// GCC's __builtin_nans[f|l](text): a signaling NaN, its quiet bit clear and
// the payload below it (the bit below the quiet bit for none), as a long
// double whose bit-preserving narrowing (lowir::float_literal) keeps them.
long double BuiltinSignalingNan(FundamentalType type, const syntax::Expr& text);

} // namespace sema
