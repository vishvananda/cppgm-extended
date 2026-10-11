#include "sema/string_literals.h"

#include "lex/text_literal.h"
#include "support/unicode/utf8.h"
#include "syntax/ast.h"
#include "sema/semantic_types.h"

#include <cstring>
#include <string>

namespace sema {

bool IsCharacterArrayElement(const Type* type)
{
	type = type->unqualified;
	if (type->kind != TypeKind::Fundamental)
		return false;
	switch (type->fundamental)
	{
	case FundamentalType::Char:
	case FundamentalType::SignedChar:
	case FundamentalType::UnsignedChar:
	case FundamentalType::Char16T:
	case FundamentalType::Char32T:
	case FundamentalType::WcharT:
		return true;
	default:
		return false;
	}
}

namespace {

// Appends the code units of one code point in an encoding.
void AppendUnits(uint64_t code_point, EncodingPrefix prefix, std::vector<uint64_t>& units)
{
	switch (prefix)
	{
	case EncodingPrefix::Utf16:
		if (code_point > 0xFFFF)
		{
			const uint64_t offset = code_point - 0x10000;
			units.push_back(0xD800 + (offset >> 10));
			units.push_back(0xDC00 + (offset & 0x3FF));
		}
		else
			units.push_back(code_point);
		return;
	case EncodingPrefix::Utf32:
	case EncodingPrefix::Wide:
		units.push_back(code_point);
		return;
	default:
	{
		std::string bytes;
		AppendUtf8(bytes, static_cast<char32_t>(code_point));
		for (char byte : bytes)
			units.push_back(static_cast<unsigned char>(byte));
		return;
	}
	}
}

// Code units of one code point in an encoding (UTF-8 for no prefix and u8,
// UTF-16 for u, UTF-32 for U and L under the course ABI).
uint64_t UnitsOf(uint64_t code_point, EncodingPrefix prefix)
{
	switch (prefix)
	{
	case EncodingPrefix::Utf16:
		return code_point > 0xFFFF ? 2 : 1;
	case EncodingPrefix::Utf32:
	case EncodingPrefix::Wide:
		return 1;
	default:
		return Utf8EncodedLength(static_cast<char32_t>(code_point));
	}
}

} // namespace

bool DecodeStringLiteral(const TokenSpelling& spelling, StringLiteralFacts& out, std::vector<uint64_t>* values);

bool AnalyzeStringLiteral(const TokenSpelling& spelling, StringLiteralFacts& out)
{
	return DecodeStringLiteral(spelling, out, nullptr);
}

bool StringLiteralUnits(const TokenSpelling& spelling, StringLiteralFacts& out, std::vector<uint64_t>& units)
{
	units.clear();
	return DecodeStringLiteral(spelling, out, &units);
}

long double BuiltinNan(FundamentalType type, const syntax::Expr& text)
{
	const uint64_t payload = BuiltinNanPayload(text);
	// The x87 format: exponent all ones, the integer and quiet bits, then
	// the payload where a narrower type's fraction bits (22 below float's
	// quiet bit, 51 below double's) end up on conversion.
	const unsigned bits = type == FundamentalType::Float ? 22 : type == FundamentalType::Double ? 51 : 62;
	const uint64_t significand = 0xC000000000000000ull | ((payload & ((uint64_t(1) << bits) - 1)) << (62 - bits));
	unsigned char bytes[sizeof(long double)] = {};
	std::memcpy(bytes, &significand, 8);
	const uint16_t sign_exponent = 0x7fff;
	std::memcpy(bytes + 8, &sign_exponent, 2);
	long double value;
	std::memcpy(&value, bytes, sizeof value);
	return value;
}

long double BuiltinSignalingNan(FundamentalType type, const syntax::Expr& text)
{
	const unsigned bits = type == FundamentalType::Float ? 22 : type == FundamentalType::Double ? 51 : 62;
	const uint64_t payload = BuiltinNanPayload(text) & ((uint64_t(1) << bits) - 1);
	const uint64_t significand = 0x8000000000000000ull | (payload != 0 ? payload << (62 - bits) : uint64_t(1) << 61);
	unsigned char bytes[sizeof(long double)] = {};
	std::memcpy(bytes, &significand, 8);
	const uint16_t sign_exponent = 0x7fff;
	std::memcpy(bytes + 8, &sign_exponent, 2);
	long double value;
	std::memcpy(&value, bytes, sizeof value);
	return value;
}

uint64_t BuiltinNanPayload(const syntax::Expr& text)
{
	uint64_t payload = 0;
	return BuiltinNanText(text, payload) ? payload : 0;
}

bool BuiltinNanText(const syntax::Expr& text, uint64_t& payload)
{
	// GCC's reading (real_nan): white space and a sign skipped (the sign
	// ignored), a 0x (hexadecimal) or 0 (octal) prefix, then digits of the
	// base to the end of the string, accumulated modulo 2^64.
	const syntax::Expr* e = &text;
	while (e->kind == syntax::ExprKind::Paren)
		e = static_cast<const syntax::ParenExpr*>(e)->inner;
	payload = 0;
	StringLiteralFacts facts;
	std::vector<uint64_t> units;
	if (e->kind != syntax::ExprKind::Literal ||
		!StringLiteralUnits(static_cast<const syntax::LiteralExpr*>(e)->spelling, facts, units) || units.empty() ||
		facts.element != FundamentalType::Char)
		return false;
	const size_t length = units.size() - 1;
	size_t at = 0;
	while (at < length && (units[at] == ' ' || (units[at] >= '\t' && units[at] <= '\r')))
		++at;
	if (at < length && (units[at] == '-' || units[at] == '+'))
		++at;
	unsigned base = 10;
	if (at < length && units[at] == '0')
	{
		++at;
		base = 8;
		if (at < length && (units[at] == 'x' || units[at] == 'X'))
		{
			++at;
			base = 16;
		}
	}
	for (; at < length; ++at)
	{
		const uint64_t c = units[at];
		const unsigned digit = c >= '0' && c <= '9' ? static_cast<unsigned>(c - '0')
			: c >= 'a' && c <= 'f'                  ? static_cast<unsigned>(c - 'a' + 10)
			: c >= 'A' && c <= 'F'                  ? static_cast<unsigned>(c - 'A' + 10)
													: 16u;
		if (digit >= base)
		{
			payload = 0;
			return false;
		}
		payload = payload * base + digit;
	}
	return true;
}

bool DecodeStringLiteral(const TokenSpelling& spelling, StringLiteralFacts& out, std::vector<uint64_t>* values)
{
	const char* text = spelling.text;
	const uint32_t length = spelling.length;
	EncodingPrefix prefix = EncodingPrefix::None;
	// Code points per piece, counted in the combined encoding at the end.
	uint64_t units = 1;
	uint32_t begin = 0;
	out.suffix = nullptr;
	out.suffix_length = 0;
	struct Piece
	{
		uint32_t begin;
		uint32_t end;
	};
	// Two passes: the combined prefix first (2.14.5p13), then the units.
	for (int pass = 0; pass < 2; ++pass)
	{
		begin = 0;
		while (begin < length)
		{
			const uint32_t end = TextLiteralPieceEnd(text, length, begin);
			const Piece piece = Piece{ begin, end };
			begin = end + 1;
			TextLiteralParts parts;
			if (!SplitTextLiteral(text + piece.begin, piece.end - piece.begin, parts) || parts.quote != '"')
				return false;
			if (pass == 0 && parts.suffix_begin != piece.end - piece.begin)
			{
				const char* suffix = text + piece.begin + parts.suffix_begin;
				const uint32_t suffix_length = piece.end - piece.begin - parts.suffix_begin;
				if (out.suffix != nullptr &&
					(out.suffix_length != suffix_length ||
						std::string(out.suffix, suffix_length) != std::string(suffix, suffix_length)))
					return false;
				out.suffix = suffix;
				out.suffix_length = suffix_length;
			}
			if (pass == 0)
			{
				if (parts.prefix != EncodingPrefix::None && prefix != EncodingPrefix::None && parts.prefix != prefix)
					return false;
				if (parts.prefix != EncodingPrefix::None)
					prefix = parts.prefix;
				continue;
			}
			const char* body = text + piece.begin;
			uint32_t position = parts.body_begin;
			while (position < parts.body_end)
			{
				if (parts.raw)
				{
					const Utf8Decoded decoded = DecodeUtf8(
						reinterpret_cast<const unsigned char*>(body) + position, parts.body_end - position);
					if (decoded.status != Utf8DecodeStatus::Ok)
						return false;
					units += UnitsOf(decoded.code_point, prefix);
					if (values != nullptr)
						AppendUnits(decoded.code_point, prefix, *values);
					position += decoded.length;
					continue;
				}
				LiteralCharacter character;
				if (!DecodeLiteralCharacter(body, parts.body_end, position, character))
					return false;
				// A numeric escape designates one code unit (2.14.3p4).
				units += character.numeric ? 1 : UnitsOf(character.value, prefix);
				if (values != nullptr)
				{
					if (character.numeric)
						values->push_back(character.value);
					else
						AppendUnits(character.value, prefix, *values);
				}
			}
		}
	}
	out.element = StringElementType(prefix);
	out.units = units;
	if (values != nullptr)
		values->push_back(0);
	return true;
}

} // namespace sema
