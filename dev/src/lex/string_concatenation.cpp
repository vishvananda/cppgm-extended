#include "lex/string_concatenation.h"

#include <cstring>

#include "lex/ud_suffix.h"
#include "support/unicode/utf8.h"

void StringLiteralSequence::clear()
{
	spellings_.clear();
	pieces_.clear();
}

void StringLiteralSequence::add(const char* spelling, uint32_t length)
{
	if (!pieces_.empty())
		spellings_.push_back(' ');
	pieces_.push_back(Piece{ static_cast<uint32_t>(spellings_.size()), length });
	spellings_.append(spelling, length);
}

bool StringLiteralSequence::concatenate()
{
	units_.clear();
	if (!combine_prefixes_and_suffixes())
		return false;

	width_ = CodeUnitWidth(prefix_);
	for (const Piece& piece : pieces_)
	{
		// Splitting succeeded in combine_prefixes_and_suffixes().
		const char* text = spellings_.data() + piece.offset;
		TextLiteralParts parts;
		SplitTextLiteral(text, piece.length, parts);
		if (!encode_body(text, parts))
			return false;
	}
	append_unit(0);
	return true;
}

bool StringLiteralSequence::combine_prefixes_and_suffixes()
{
	prefix_ = EncodingPrefix::None;
	suffix_ = TokenSpelling{ nullptr, 0 };
	for (const Piece& piece : pieces_)
	{
		const char* text = spellings_.data() + piece.offset;
		TextLiteralParts parts;
		if (!SplitTextLiteral(text, piece.length, parts) || parts.quote != '"')
			return false;
		if (parts.prefix != EncodingPrefix::None)
		{
			if (prefix_ != EncodingPrefix::None && prefix_ != parts.prefix)
				return false;
			prefix_ = parts.prefix;
		}

		const char* suffix = text + parts.suffix_begin;
		const uint32_t suffix_length = piece.length - parts.suffix_begin;
		if (suffix_length == 0)
			continue;
		if (!IsUserDefinedSuffix(suffix, suffix_length, reserved_suffixes_))
			return false;
		if (suffix_.length == 0)
			suffix_ = TokenSpelling{ suffix, suffix_length };
		else if (suffix_.length != suffix_length || std::memcmp(suffix_.text, suffix, suffix_length) != 0)
			return false;
	}
	return true;
}

bool StringLiteralSequence::encode_body(const char* text, const TextLiteralParts& parts)
{
	// Source characters are already UTF-8, which is the encoding of char
	// arrays, so only escapes need decoding there.
	if (width_ == 1 && parts.raw)
	{
		units_.append(text + parts.body_begin, parts.body_end - parts.body_begin);
		return true;
	}

	const uint32_t max_unit = width_ == 1 ? 0xFF : width_ == 2 ? 0xFFFF : 0xFFFFFFFF;
	uint32_t pos = parts.body_begin;
	while (pos < parts.body_end)
	{
		if (width_ == 1 && text[pos] != '\\')
		{
			const void* escape = std::memchr(text + pos, '\\', parts.body_end - pos);
			const uint32_t run_end = escape == nullptr ? parts.body_end
													   : static_cast<uint32_t>(static_cast<const char*>(escape) - text);
			units_.append(text + pos, run_end - pos);
			pos = run_end;
			continue;
		}

		LiteralCharacter c;
		if (parts.raw)
		{
			const Utf8Decoded decoded = DecodeUtf8(
				reinterpret_cast<const unsigned char*>(text) + pos, parts.body_end - pos);
			if (decoded.status != Utf8DecodeStatus::Ok)
				return false;
			c.value = decoded.code_point;
			c.numeric = false;
			pos += decoded.length;
		}
		else if (!DecodeLiteralCharacter(text, parts.body_end, pos, c))
		{
			return false;
		}

		// A numeric escape is one code unit ([lex.string] 2.14.5p15) and
		// must fit in it (course rule).
		if (c.numeric)
		{
			if (c.value > max_unit)
				return false;
			append_unit(static_cast<uint32_t>(c.value));
		}
		else
		{
			append_code_point(static_cast<uint32_t>(c.value));
		}
	}
	return true;
}

void StringLiteralSequence::append_unit(uint32_t unit)
{
	for (unsigned i = 0; i < width_; ++i)
		units_.push_back(static_cast<char>(unit >> (8 * i)));
}

void StringLiteralSequence::append_code_point(uint32_t code_point)
{
	if (width_ == 1)
	{
		AppendUtf8(units_, code_point);
	}
	else if (width_ == 2 && code_point > 0xFFFF)
	{
		// A UTF-16 surrogate pair.
		const uint32_t offset = code_point - 0x10000;
		append_unit(0xD800 | (offset >> 10));
		append_unit(0xDC00 | (offset & 0x3FF));
	}
	else
	{
		append_unit(code_point);
	}
}
