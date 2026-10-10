#pragma once

#include <cstdint>

// Character classification for preprocessing tokenization ([lex] clause 2).
// Code points are int32_t so the end-of-source marker (-1) classifies as
// nothing.

inline bool IsAsciiDigit(int32_t c)
{
	return c >= '0' && c <= '9';
}

inline bool IsOctalDigit(int32_t c)
{
	return c >= '0' && c <= '7';
}

inline bool IsHexDigit(int32_t c)
{
	return IsAsciiDigit(c) || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
}

inline int HexDigitValue(int32_t c)
{
	if (IsAsciiDigit(c))
		return c - '0';
	if (c >= 'a' && c <= 'f')
		return c - 'a' + 10;
	return c - 'A' + 10;
}

// `nondigit`: the ASCII letters and underscore.
inline bool IsAsciiNondigit(int32_t c)
{
	return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

// Whitespace other than new-line that separates preprocessing tokens.
// Carriage return is treated as horizontal whitespace.
inline bool IsHorizontalWhitespace(int32_t c)
{
	return c == ' ' || c == '\t' || c == '\v' || c == '\f' || c == '\r';
}

// Annex E.1: code points allowed in identifiers.
bool IsAnnexE1Allowed(int32_t c);

// Annex E.2: code points not allowed as the first identifier character.
bool IsAnnexE2DisallowedInitially(int32_t c);

inline bool IsIdentifierStart(int32_t c)
{
	if (c < 0x80)
		return IsAsciiNondigit(c);
	return IsAnnexE1Allowed(c) && !IsAnnexE2DisallowedInitially(c);
}

// `identifier-nondigit` or `digit`.
inline bool IsIdentifierContinue(int32_t c)
{
	if (c < 0x80)
		return IsAsciiNondigit(c) || IsAsciiDigit(c);
	return IsAnnexE1Allowed(c);
}

// [lex.charset] 2.3p1: the basic source character set.
bool IsBasicSourceCharacter(int32_t c);

// [lex.charset] 2.3p2: the control character ranges.
inline bool IsControlCharacter(int32_t c)
{
	return (c >= 0x00 && c <= 0x1F) || (c >= 0x7F && c <= 0x9F);
}

// A value that a universal-character-name may designate anywhere: an ISO
// 10646 code point that is not a surrogate.
inline bool IsUniversalCharacterValue(uint32_t value)
{
	return value <= 0x10FFFF && (value < 0xD800 || value > 0xDFFF);
}

// [lex.string] 2.14.5p1 `d-char` as course-defined: any code point except
// space, parentheses, backslash, horizontal tab, vertical tab, form feed
// and new-line.
inline bool IsRawStringDelimiterChar(int32_t c)
{
	switch (c)
	{
	case ' ':
	case '(':
	case ')':
	case '\\':
	case '\t':
	case '\v':
	case '\f':
	case '\n':
		return false;
	default:
		return c >= 0;
	}
}
