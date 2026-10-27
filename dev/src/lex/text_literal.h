#pragma once

#include <cstdint>

#include "types/fundamental_type.h"

// The encoding-prefix of a character or string literal ([lex.ccon] 2.14.3,
// [lex.string] 2.14.5).
enum class EncodingPrefix : uint8_t
{
	None,
	Utf8,
	Utf16,
	Utf32,
	Wide
};

// The parts of a character-literal or string-literal spelling from phase 3,
// with an optional ud-suffix. Offsets index the spelling.
struct TextLiteralParts
{
	EncodingPrefix prefix;
	bool raw;
	// `'` for a character literal, `"` for a string literal.
	char quote;
	// The c-chars, s-chars or r-chars.
	uint32_t body_begin;
	uint32_t body_end;
	// Just past the closing quote; the spelling length if there is no
	// ud-suffix.
	uint32_t suffix_begin;
};

// Splits a literal spelling. False if it is not a quoted literal.
bool SplitTextLiteral(const char* spelling, uint32_t length, TextLiteralParts& parts);

// The end of the literal starting at `begin` in a spelling of concatenated
// pieces joined by single spaces: past its closing quote (escaped quotes
// and a raw literal's r-chars skipped) and ud-suffix.
uint32_t TextLiteralPieceEnd(const char* spelling, uint32_t length, uint32_t begin);

// One c-char or s-char of a non-raw literal: a code point designated by a
// source character, simple-escape-sequence or universal-character-name, or
// the value of an octal or hexadecimal escape sequence, which designates one
// code unit ([lex.ccon] 2.14.3p4).
struct LiteralCharacter
{
	// Numeric escape values above 32 bits saturate at kNumericEscapeTooLarge.
	uint64_t value;
	bool numeric;
};

const uint64_t kNumericEscapeTooLarge = 0x100000000ull;

// Decodes the c-char or s-char at `pos` of a phase-3 literal whose body ends
// at `end`, and advances `pos` past it. False if the text is malformed.
bool DecodeLiteralCharacter(const char* text, uint32_t end, uint32_t& pos, LiteralCharacter& c);

// The code-unit width in bytes and the element type of a string literal
// with this prefix under the course ABI (wchar_t is UTF-32).
unsigned CodeUnitWidth(EncodingPrefix prefix);
FundamentalType StringElementType(EncodingPrefix prefix);

enum class CharacterDialect : uint8_t
{
	Course,
	Gnu
};

// A character literal ([lex.ccon] 2.14.3) under the course rules: exactly
// one c-char designating a Unicode scalar value. An unprefixed literal is a
// char when that value is ASCII and an int otherwise; `u` literals need a
// single UTF-16 code unit. A ud-suffix must be usable (IsUserDefinedSuffix).
// An unprefixed literal of several c-chars, each at most 0xFF, is a
// multicharacter literal: an int whose value packs the c-chars, first in
// the most significant byte (implementation-defined, 2.14.3p1; GCC's
// choice). PA2's post-token contract still reports it as invalid.
struct CharacterLiteralValue
{
	bool valid;
	bool multicharacter;
	FundamentalType type;
	uint32_t code_point;
	// The spelling length if the literal has no ud-suffix.
	uint32_t suffix_begin;
};

// GNU mode also gives ordinary numeric byte escapes type char. PA2's
// post-token contract keeps its Unicode-code-point classification.
CharacterLiteralValue EvaluateCharacterLiteral(
	const char* spelling, uint32_t length, CharacterDialect dialect = CharacterDialect::Gnu);
