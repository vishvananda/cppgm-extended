#pragma once

#include <cstdint>

#include "preprocess/source/source_buffer.h"

// SourceChar::code_point value past the last character.
constexpr int32_t kEndOfSource = -1;

// One character of the translated source together with the physical bytes
// it came from. `begin` is after any line splices deleted in front of the
// character, so `end - begin` differs from the character's UTF-8 length
// exactly when phase 1 replaced a trigraph or mapped a Windows-1252 byte.
struct SourceChar
{
	int32_t code_point;
	uint32_t begin;
	uint32_t end;
};

// Stateless, random-access view of translation phases 1 and 2 over a
// SourceBuffer. Decoding depends only on the offset, so a lexer can look
// ahead, backtrack, or switch to the raw view without buffered state.
class TranslationReader
{
public:
	// Without `translate`, logical() is raw(): the buffer holds token
	// spellings that are already past phases 1-2.
	explicit TranslationReader(const SourceBuffer& source, bool translate = true)
		: bytes_(reinterpret_cast<const unsigned char*>(source.data())), size_(source.size()), translate_(translate)
	{}

	// The next character at or after `offset` with physical characters
	// mapped (UTF-8, or Windows-1252 for a stray continuation byte),
	// trigraphs replaced and backslash-new-line splices deleted.
	SourceChar logical(uint32_t offset) const
	{
		if (offset < size_)
		{
			const unsigned char byte = bytes_[offset];
			if (byte < 0x80 && byte != '\\' && byte != '?')
				return SourceChar{ byte, offset, offset + 1 };
		}
		return logical_slow(offset);
	}

	// The character at `offset` with only the physical character mapping
	// applied: the view inside a raw string literal, where trigraphs and
	// splices are reverted ([lex.pptoken] 2.5p3).
	SourceChar raw(uint32_t offset) const
	{
		if (offset < size_ && bytes_[offset] < 0x80)
			return SourceChar{ bytes_[offset], offset, offset + 1 };
		return raw_slow(offset);
	}

private:
	SourceChar logical_slow(uint32_t offset) const;
	SourceChar raw_slow(uint32_t offset) const;
	SourceChar decode_multibyte(uint32_t offset) const;
	uint32_t skip_line_splices(uint32_t offset) const;

	const unsigned char* bytes_;
	uint32_t size_;
	bool translate_;
};
