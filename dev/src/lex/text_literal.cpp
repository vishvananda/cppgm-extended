#include "lex/text_literal.h"

#include <cstring>

#include "lex/ud_suffix.h"
#include "preprocess/lexer/char_classes.h"
#include "support/unicode/utf8.h"

namespace {

EncodingPrefix TakeEncodingPrefix(const char* text, uint32_t length, uint32_t& pos)
{
	if (length >= 2 && text[0] == 'u' && text[1] == '8')
	{
		pos = 2;
		return EncodingPrefix::Utf8;
	}
	pos = 1;
	switch (length == 0 ? '\0' : text[0])
	{
	case 'u':
		return EncodingPrefix::Utf16;
	case 'U':
		return EncodingPrefix::Utf32;
	case 'L':
		return EncodingPrefix::Wide;
	default:
		pos = 0;
		return EncodingPrefix::None;
	}
}

// The body of a raw string whose opening quote is at `quote` and closing
// quote at `close`: between `delimiter(` and `)delimiter`.
bool SplitRawBody(const char* text, uint32_t quote, uint32_t close, TextLiteralParts& parts)
{
	uint32_t open = quote + 1;
	while (open < close && text[open] != '(')
		++open;
	const uint32_t delimiter_length = open - (quote + 1);
	if (open == close || close - open < delimiter_length + 2)
		return false;
	parts.body_begin = open + 1;
	parts.body_end = close - delimiter_length - 1;
	return text[parts.body_end] == ')' &&
		std::memcmp(text + parts.body_end + 1, text + quote + 1, delimiter_length) == 0;
}

// A hex-quad sequence of `digits` digits at `pos`.
bool DecodeHexQuads(const char* text, uint32_t end, uint32_t& pos, unsigned digits, uint64_t& value)
{
	if (end - pos < digits)
		return false;
	value = 0;
	for (unsigned i = 0; i < digits; ++i, ++pos)
	{
		if (!IsHexDigit(text[pos]))
			return false;
		value = value << 4 | static_cast<unsigned>(HexDigitValue(text[pos]));
	}
	return true;
}

bool DecodeHexEscape(const char* text, uint32_t end, uint32_t& pos, uint64_t& value)
{
	const uint32_t first = pos;
	value = 0;
	for (; pos < end && IsHexDigit(text[pos]); ++pos)
	{
		value = value << 4 | static_cast<unsigned>(HexDigitValue(text[pos]));
		if (value >= kNumericEscapeTooLarge)
			value = kNumericEscapeTooLarge;
	}
	return pos != first;
}

// The value of a simple-escape-sequence character, or -1.
int SimpleEscapeValue(char c)
{
	switch (c)
	{
	case '\'':
	case '"':
	case '?':
	case '\\':
		return c;
	case 'a':
		return 0x07;
	case 'b':
		return 0x08;
	case 'f':
		return 0x0C;
	case 'n':
		return 0x0A;
	case 'r':
		return 0x0D;
	case 't':
		return 0x09;
	case 'v':
		return 0x0B;
	default:
		return -1;
	}
}

bool DecodeEscapeSequence(const char* text, uint32_t end, uint32_t& pos, LiteralCharacter& c)
{
	// text[pos] is the backslash.
	if (end - pos < 2)
		return false;
	const char introducer = text[pos + 1];
	pos += 2;
	c.numeric = false;
	const int simple = SimpleEscapeValue(introducer);
	if (simple >= 0)
	{
		c.value = static_cast<uint64_t>(simple);
		return true;
	}
	if (introducer == 'u' || introducer == 'U')
	{
		// Phase 3 decodes other universal-character-names into the
		// spelling; the ones left spelled designate basic or control
		// characters.
		return DecodeHexQuads(text, end, pos, introducer == 'u' ? 4 : 8, c.value) &&
			IsUniversalCharacterValue(static_cast<uint32_t>(c.value));
	}
	c.numeric = true;
	if (introducer == 'x')
		return DecodeHexEscape(text, end, pos, c.value);
	if (!IsOctalDigit(introducer))
		return false;
	c.value = static_cast<uint64_t>(introducer - '0');
	for (int digits = 1; digits < 3 && pos < end && IsOctalDigit(text[pos]); ++digits, ++pos)
		c.value = c.value * 8 + static_cast<uint64_t>(text[pos] - '0');
	return true;
}

} // namespace

bool SplitTextLiteral(const char* text, uint32_t length, TextLiteralParts& parts)
{
	uint32_t quote;
	parts.prefix = TakeEncodingPrefix(text, length, quote);
	parts.raw = quote < length && text[quote] == 'R';
	if (parts.raw)
		++quote;
	if (quote >= length || (text[quote] != '"' && text[quote] != '\''))
		return false;
	parts.quote = text[quote];
	if (parts.raw && parts.quote != '"')
		return false;

	// A ud-suffix is an identifier, so the closing quote is the last one.
	uint32_t close = length - 1;
	while (close > quote && text[close] != parts.quote)
		--close;
	if (close == quote)
		return false;
	parts.suffix_begin = close + 1;
	// A raw literal's r-chars end at its own `)delimiter"`, before the
	// pieces a concatenation's spelling may join after it.
	if (parts.raw)
	{
		uint32_t own = TextLiteralPieceEnd(text, length, 0);
		while (own > quote + 1 && text[own - 1] != '"')
			--own;
		return own > quote + 1 && SplitRawBody(text, quote, own - 1, parts);
	}
	parts.body_begin = quote + 1;
	parts.body_end = close;
	return true;
}

uint32_t TextLiteralPieceEnd(const char* text, uint32_t length, uint32_t begin)
{
	// The encoding prefix, then the quoted body (2.14.3p1, 2.14.5p1).
	uint32_t at = begin;
	while (at < length && text[at] != '"' && text[at] != '\'' && text[at] != ' ')
		++at;
	if (at >= length || text[at] == ' ')
		return at;
	const char quote = text[at];
	const bool raw = at > begin && text[at - 1] == 'R';
	++at;
	if (raw)
	{
		// r-chars end at `)delimiter"`.
		const uint32_t delimiter = at;
		while (at < length && text[at] != '(')
			++at;
		const uint32_t size = at - delimiter;
		for (; at < length; ++at)
		{
			if (text[at] == ')' && at + size + 1 < length && text[at + size + 1] == '"' &&
				std::memcmp(text + at + 1, text + delimiter, size) == 0)
			{
				at += size + 2;
				break;
			}
		}
	}
	else
	{
		while (at < length && text[at] != quote)
			at += text[at] == '\\' ? 2 : 1;
		++at;
	}
	// A ud-suffix is an identifier.
	while (at < length && text[at] != ' ')
		++at;
	return at < length ? at : length;
}

bool DecodeLiteralCharacter(const char* text, uint32_t end, uint32_t& pos, LiteralCharacter& c)
{
	const unsigned char lead = static_cast<unsigned char>(text[pos]);
	if (lead == '\\')
		return DecodeEscapeSequence(text, end, pos, c);
	c.numeric = false;
	if (lead < 0x80)
	{
		c.value = lead;
		++pos;
		return true;
	}
	const Utf8Decoded decoded = DecodeUtf8(reinterpret_cast<const unsigned char*>(text) + pos, end - pos);
	if (decoded.status != Utf8DecodeStatus::Ok)
		return false;
	c.value = decoded.code_point;
	pos += decoded.length;
	return true;
}

unsigned CodeUnitWidth(EncodingPrefix prefix)
{
	switch (prefix)
	{
	case EncodingPrefix::Utf16:
		return 2;
	case EncodingPrefix::Utf32:
	case EncodingPrefix::Wide:
		return 4;
	default:
		return 1;
	}
}

FundamentalType StringElementType(EncodingPrefix prefix)
{
	switch (prefix)
	{
	case EncodingPrefix::Utf16:
		return FundamentalType::Char16T;
	case EncodingPrefix::Utf32:
		return FundamentalType::Char32T;
	case EncodingPrefix::Wide:
		return FundamentalType::WcharT;
	default:
		return FundamentalType::Char;
	}
}

namespace {

CharacterLiteralValue EvaluateMulticharacterLiteral(
	const char* text, const TextLiteralParts& parts, LiteralCharacter c, uint32_t pos, CharacterLiteralValue result)
{
	if (parts.prefix != EncodingPrefix::None || c.value > 0xFF)
		return result;
	uint32_t value = static_cast<uint32_t>(c.value);
	while (pos != parts.body_end)
	{
		if (!DecodeLiteralCharacter(text, parts.body_end, pos, c) || c.value > 0xFF)
			return result;
		value = (value << 8) | static_cast<uint32_t>(c.value);
	}
	result.code_point = value;
	result.type = FundamentalType::Int;
	result.multicharacter = true;
	result.valid = true;
	return result;
}

} // namespace

CharacterLiteralValue EvaluateCharacterLiteral(const char* text, uint32_t length, CharacterDialect dialect)
{
	CharacterLiteralValue result = { false, false, FundamentalType::Int, 0, length };
	TextLiteralParts parts;
	if (!SplitTextLiteral(text, length, parts) || parts.quote != '\'' || parts.prefix == EncodingPrefix::Utf8)
		return result;
	result.suffix_begin = parts.suffix_begin;
	if (parts.suffix_begin != length && !IsUserDefinedSuffix(text + parts.suffix_begin, length - parts.suffix_begin))
		return result;

	// Exactly one c-char, designating a Unicode scalar value (numeric
	// escapes included).
	uint32_t pos = parts.body_begin;
	LiteralCharacter c;
	if (pos == parts.body_end || !DecodeLiteralCharacter(text, parts.body_end, pos, c))
		return result;
	if (pos != parts.body_end)
		return parts.suffix_begin == length ? EvaluateMulticharacterLiteral(text, parts, c, pos, result) : result;
	if (c.value > 0x10FFFF || !IsUniversalCharacterValue(static_cast<uint32_t>(c.value)))
		return result;
	result.code_point = static_cast<uint32_t>(c.value);
	switch (parts.prefix)
	{
	case EncodingPrefix::None:
		result.type = result.code_point <= 0x7F ||
				(dialect == CharacterDialect::Gnu && c.numeric && result.code_point <= 0xFF)
			? FundamentalType::Char
			: FundamentalType::Int;
		break;
	case EncodingPrefix::Utf16:
		if (result.code_point > 0xFFFF)
			return result;
		result.type = FundamentalType::Char16T;
		break;
	default:
		result.type = StringElementType(parts.prefix);
		break;
	}
	result.valid = true;
	return result;
}
