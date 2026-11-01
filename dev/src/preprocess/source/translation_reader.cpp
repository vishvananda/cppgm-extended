#include "preprocess/source/translation_reader.h"

#include "support/unicode/utf8.h"

namespace {

// [lex.trigraph] 2.4: the character a `??x` trigraph stands for, or 0.
int TrigraphReplacement(unsigned char third)
{
	switch (third)
	{
	case '=':
		return '#';
	case '/':
		return '\\';
	case '\'':
		return '^';
	case '(':
		return '[';
	case ')':
		return ']';
	case '!':
		return '|';
	case '<':
		return '{';
	case '>':
		return '}';
	case '-':
		return '~';
	default:
		return 0;
	}
}

// Windows-1252 characters for bytes 0x80-0x9F; the five bytes that code page
// leaves unassigned keep their C1 control code point. Bytes 0xA0-0xBF are the
// same as ISO 8859-1.
const char16_t kWindows1252C1[32] = { 0x20AC, 0x0081, 0x201A, 0x0192, 0x201E, 0x2026, 0x2020, 0x2021, 0x02C6, 0x2030,
	0x0160, 0x2039, 0x0152, 0x008D, 0x017D, 0x008F, 0x0090, 0x2018, 0x2019, 0x201C, 0x201D, 0x2022, 0x2013, 0x2014,
	0x02DC, 0x2122, 0x0161, 0x203A, 0x0153, 0x009D, 0x017E, 0x0178 };

// The Windows-1252 character of a byte that is a UTF-8 continuation byte.
int32_t Windows1252ContinuationByte(unsigned char byte)
{
	return byte < 0xA0 ? kWindows1252C1[byte - 0x80] : byte;
}

} // namespace

uint32_t TranslationReader::skip_line_splices(uint32_t offset) const
{
	// Trigraphs are replaced (phase 1) before splicing (phase 2), so `??/`
	// followed by a new-line splices just like a backslash does.
	for (;;)
	{
		uint32_t end = 0;
		if (offset < size_ && bytes_[offset] == '\\')
			end = new_line_end(offset + 1);
		else if (offset + 2 < size_ && bytes_[offset] == '?' && bytes_[offset + 1] == '?' && bytes_[offset + 2] == '/')
			end = new_line_end(offset + 3);
		if (end == 0)
			return offset;
		offset = end;
	}
}

// The offset after the new-line at `offset`, or 0 where there is none.
uint32_t TranslationReader::new_line_end(uint32_t offset) const
{
	if (crlf_splices_ && offset < size_ && bytes_[offset] == '\r')
		++offset;
	return offset < size_ && bytes_[offset] == '\n' ? offset + 1 : 0;
}

SourceChar TranslationReader::logical_slow(uint32_t offset, bool comment) const
{
	if (!translate_)
		return raw(offset);
	offset = skip_line_splices(offset);
	if (offset >= size_)
		return SourceChar{ kEndOfSource, size_, size_ };

	const unsigned char byte = bytes_[offset];
	if (byte == '?' && offset + 2 < size_ && bytes_[offset + 1] == '?')
	{
		if (int replacement = TrigraphReplacement(bytes_[offset + 2]))
			return SourceChar{ replacement, offset, offset + 3 };
	}
	if (byte < 0x80 || comment)
		return SourceChar{ byte, offset, offset + 1 };
	return decode_multibyte(offset);
}

SourceChar TranslationReader::raw_slow(uint32_t offset) const
{
	if (offset >= size_)
		return SourceChar{ kEndOfSource, size_, size_ };
	return decode_multibyte(offset);
}

SourceChar TranslationReader::decode_multibyte(uint32_t offset) const
{
	Utf8Decoded decoded = DecodeUtf8(bytes_ + offset, size_ - offset);
	if (decoded.status == Utf8DecodeStatus::Ok)
		return SourceChar{ static_cast<int32_t>(decoded.code_point), offset, offset + decoded.length };

	// The phase 1 mapping of physical characters is implementation-defined
	// ([lex.phases] 2.2p1). A continuation byte where a character must begin
	// is taken as a Windows-1252 character, as legacy 8-bit text such as a
	// hosted header comment contains; every other malformed sequence is an
	// error.
	const unsigned char byte = bytes_[offset];
	if (decoded.status == Utf8DecodeStatus::InvalidLeadingByte && byte >= 0x80 && byte <= 0xBF)
		return SourceChar{ Windows1252ContinuationByte(byte), offset, offset + 1 };
	throw SourceError(offset, DescribeUtf8DecodeStatus(decoded.status));
}
