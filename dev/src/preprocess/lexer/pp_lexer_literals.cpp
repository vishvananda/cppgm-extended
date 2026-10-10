#include "preprocess/lexer/pp_lexer.h"

#include "preprocess/lexer/char_classes.h"

namespace {

// [lex.ccon] 2.14.3 simple-escape-sequence characters after the backslash.
bool IsSimpleEscapeChar(int32_t c)
{
	switch (c)
	{
	case '\'':
	case '"':
	case '?':
	case '\\':
	case 'a':
	case 'b':
	case 'f':
	case 'n':
	case 'r':
	case 't':
	case 'v':
		return true;
	default:
		return false;
	}
}

// [lex.string] 2.14.5p2: a raw string delimiter has at most 16 characters.
const unsigned kMaxRawStringDelimiterChars = 16;

// The part of a raw string literal after its opening quote.
struct RawStringExtent
{
	// Offset just past the closing quote.
	uint32_t end;
	// Whether the physical bytes are the UTF-8 spelling of the characters,
	// that is, no Windows-1252 byte was mapped.
	bool verbatim;
};

// A raw-view character that phase 1 mapped from one Windows-1252 byte.
bool IsMappedByte(const SourceChar& c)
{
	return c.code_point >= 0x80 && c.end - c.begin == 1;
}

// Scans `d-char-sequence ( r-char-sequence ) d-char-sequence "` in the raw
// view from `begin`. The closing delimiter matches by character.
RawStringExtent ScanRawString(const TranslationReader& reader, uint32_t token_begin, uint32_t begin)
{
	int32_t delimiter[kMaxRawStringDelimiterChars];
	unsigned delimiter_chars = 0;
	bool verbatim = true;
	SourceChar c = reader.raw(begin);
	while (c.code_point != '(')
	{
		if (c.code_point == kEndOfSource)
			throw SourceError(token_begin, "unterminated raw string literal");
		if (!IsRawStringDelimiterChar(c.code_point))
			throw SourceError(c.begin, "invalid raw string delimiter");
		if (delimiter_chars == kMaxRawStringDelimiterChars)
			throw SourceError(c.begin, "raw string delimiter is too long");
		delimiter[delimiter_chars++] = c.code_point;
		verbatim = verbatim && !IsMappedByte(c);
		c = reader.raw(c.end);
	}

	for (;;)
	{
		c = reader.raw(c.end);
		if (c.code_point == kEndOfSource)
			throw SourceError(token_begin, "unterminated raw string literal");
		verbatim = verbatim && !IsMappedByte(c);
		if (c.code_point != ')')
			continue;
		// A mismatched character is still r-char and is rescanned above.
		SourceChar close = c;
		unsigned matched = 0;
		for (; matched <= delimiter_chars; ++matched)
		{
			close = reader.raw(close.end);
			if (close.code_point != (matched < delimiter_chars ? delimiter[matched] : '"'))
				break;
			verbatim = verbatim && !IsMappedByte(close);
		}
		if (matched > delimiter_chars)
			return RawStringExtent{ close.end, verbatim };
	}
}

} // namespace

const PPToken& PPLexer::lex_quoted_literal(PPTokenKind plain, PPTokenKind user_defined)
{
	// cur_ is the opening quote; any encoding prefix is already taken.
	// The c-char/s-char sequence may be empty: `''` lexes as a
	// character-literal and later phases reject it as a literal.
	const int32_t quote = cur_.code_point;
	take(cur_);
	for (;;)
	{
		const int32_t c = cur_.code_point;
		if (c == quote)
		{
			take(cur_);
			return finish_literal(plain, user_defined);
		}
		if (c == '\n' || c == kEndOfSource)
			throw SourceError(token_begin_, "unterminated quoted literal");
		if (c == '\\')
			lex_escape_sequence();
		else
			take(cur_);
	}
}

void PPLexer::lex_escape_sequence()
{
	// Escapes are validated here but not decoded (that is phase 5); only
	// universal-character-names are translated into the spelling.
	const SourceChar backslash = cur_;
	const SourceChar introducer = peek_after(backslash);
	const int32_t c = introducer.code_point;
	if (IsSimpleEscapeChar(c))
	{
		take(backslash);
		take(introducer);
		return;
	}
	if (IsOctalDigit(c))
	{
		take(backslash);
		for (int digits = 0; digits < 3 && IsOctalDigit(cur_.code_point); ++digits)
			take(cur_);
		return;
	}
	if (c == 'x')
	{
		take(backslash);
		take(introducer);
		if (!IsHexDigit(cur_.code_point))
			throw SourceError(backslash.begin, "hex escape has no digits");
		while (IsHexDigit(cur_.code_point))
			take(cur_);
		return;
	}
	if (c == 'u' || c == 'U')
	{
		lex_literal_universal_character_name(backslash);
		return;
	}
	throw SourceError(backslash.begin, "invalid escape sequence");
}

void PPLexer::lex_literal_universal_character_name(const SourceChar& backslash)
{
	UniversalCharacterName ucn;
	if (!read_universal_character_name(backslash, ucn))
		throw SourceError(backslash.begin, "invalid escape sequence");
	if (!IsUniversalCharacterValue(ucn.value))
		throw SourceError(backslash.begin, "invalid universal character value");

	const int32_t value = static_cast<int32_t>(ucn.value);
	if (IsControlCharacter(value) || IsBasicSourceCharacter(value))
	{
		// Allowed inside literals ([lex.charset] 2.3p2), but decoding it
		// would change the literal's lexical structure (`"""`), so the
		// universal-character-name stays spelled for phase 5 to decode.
		take(backslash);
		while (cur_.begin < ucn.end)
			take(cur_);
		return;
	}
	take(SourceChar{ value, ucn.begin, ucn.end });
}

const PPToken& PPLexer::finish_literal(PPTokenKind plain, PPTokenKind user_defined)
{
	// A literal directly followed by an identifier has that ud-suffix.
	if (!take_identifier_start())
		return finish(plain);
	while (take_identifier_continue())
	{
	}
	return finish(user_defined);
}

const PPToken& PPLexer::lex_raw_string_literal()
{
	// The prefix and opening quote were read through phases 1 and 2. From
	// here to the closing quote trigraphs and splices are reverted, so the
	// rest of the literal is read from the raw view.
	append(cur_);
	const uint32_t begin = cur_.end;
	const RawStringExtent extent = ScanRawString(reader_, token_begin_, begin);
	if (extent.verbatim)
	{
		append_source_bytes(begin, extent.end);
	}
	else
	{
		for (uint32_t offset = begin; offset < extent.end;)
		{
			const SourceChar c = reader_.raw(offset);
			append(c);
			offset = c.end;
		}
	}
	cur_ = reader_.logical(extent.end);
	return finish_literal(PPTokenKind::StringLiteral, PPTokenKind::UserDefinedStringLiteral);
}

bool PPLexer::try_lex_header_name()
{
	// `<` h-char-sequence `>` or `"` q-char-sequence `"` on one line. If no
	// header-name can be formed, the characters lex as ordinary tokens.
	const int32_t close = cur_.code_point == '<' ? '>' : '"';
	SourceChar c = peek_after(cur_);
	if (c.code_point == close)
		return false;
	while (c.code_point != close)
	{
		if (c.code_point == '\n' || c.code_point == kEndOfSource)
			return false;
		c = peek_after(c);
	}

	// A backslash in a header-name is implementation-defined (2.9p2): one
	// that begins a universal-character-name designates that character, as
	// phase 1 would have spelled an extended character; any other backslash
	// is itself. The close character cannot be part of a hex-quad.
	const uint32_t end = c.end;
	while (cur_.begin < end)
	{
		UniversalCharacterName ucn;
		if (cur_.code_point == '\\' && read_universal_character_name(cur_, ucn))
			take(ucn_outside_literal(ucn));
		else
			take(cur_);
	}
	return true;
}
