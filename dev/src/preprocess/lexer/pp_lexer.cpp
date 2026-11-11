#include "preprocess/lexer/pp_lexer.h"

#include <cstring>

#include "preprocess/lexer/char_classes.h"
#include "support/unicode/utf8.h"

PPLexer::PPLexer(const SourceBuffer& source, const PPLexerOptions& options)
	: source_(source), reader_(source, !options.token_text, options.crlf_splices),
	  cur_(reader_.logical(source.begin_offset())), token_(), token_begin_(0), clean_end_(0), token_end_(0),
	  base_(options.location_base), dirty_(false), header_names_on_request_(options.header_names_on_request),
	  binary_exponents_(options.binary_exponents), digit_separators_(options.digit_separators),
	  legacy_comment_bytes_(options.legacy_comment_bytes), lone_quotes_(options.lone_quotes),
	  gnu_escapes_(options.gnu_escapes),
	  context_(options.header_names_on_request ? DirectiveContext::Other : DirectiveContext::LineStart)
{}

const PPToken& PPLexer::next()
{
	start_token();
	const int32_t c = cur_.code_point;
	if (c == kEndOfSource)
		return finish_empty(PPTokenKind::EndOfFile);
	if (c == '\n')
	{
		skip(cur_);
		return finish_empty(PPTokenKind::NewLine);
	}
	if ((IsHorizontalWhitespace(c) || c == '/') && skip_whitespace_sequence())
	{
		return finish_empty(PPTokenKind::WhitespaceSequence);
	}
	if (context_ == DirectiveContext::AfterInclude && (c == '<' || c == '"') && try_lex_header_name())
		return finish(PPTokenKind::HeaderName);
	if (IsIdentifierStart(c))
		return lex_identifier_or_prefixed_literal();
	if (IsAsciiDigit(c) || (c == '.' && IsAsciiDigit(peek_after(cur_).code_point)))
		return lex_pp_number();
	if (c == '\'')
		return lex_quoted_literal(PPTokenKind::CharacterLiteral, PPTokenKind::UserDefinedCharacterLiteral);
	if (c == '"')
		return lex_quoted_literal(PPTokenKind::StringLiteral, PPTokenKind::UserDefinedStringLiteral);
	if (c == '\\')
		return lex_backslash();
	if (const uint8_t punctuator = try_lex_punctuator())
		return finish(PPTokenKind::PreprocessingOpOrPunc, punctuator);

	// Any other single non-whitespace code point.
	take(cur_);
	return finish(PPTokenKind::NonWhitespaceCharacter);
}

// ---- token assembly -------------------------------------------------------

void PPLexer::start_token()
{
	token_begin_ = cur_.begin;
	clean_end_ = cur_.begin;
	token_end_ = cur_.begin;
	dirty_ = false;
}

void PPLexer::make_dirty()
{
	if (dirty_)
		return;
	dirty_ = true;
	scratch_.assign(source_.data() + token_begin_, clean_end_ - token_begin_);
}

void PPLexer::append_translated(const SourceChar& c)
{
	make_dirty();
	AppendUtf8(scratch_, static_cast<char32_t>(c.code_point));
}

void PPLexer::append_source_bytes(uint32_t begin, uint32_t end)
{
	// [begin, end) is already the UTF-8 spelling of its characters.
	if (!dirty_ && begin == clean_end_)
	{
		clean_end_ = end;
	}
	else
	{
		make_dirty();
		scratch_.append(source_.data() + begin, end - begin);
	}
	token_end_ = end;
}

bool PPLexer::spelling_is(const char* text) const
{
	const std::size_t length = std::strlen(text);
	return token_.length == length && std::memcmp(token_.spelling, text, length) == 0;
}

const PPToken& PPLexer::finish(PPTokenKind kind, uint8_t punctuator)
{
	token_.kind = kind;
	token_.punctuator = punctuator;
	token_.begin = base_ + token_begin_;
	token_.end = base_ + token_end_;
	if (dirty_)
	{
		token_.spelling = scratch_.data();
		token_.length = static_cast<uint32_t>(scratch_.size());
	}
	else
	{
		token_.spelling = source_.data() + token_begin_;
		token_.length = clean_end_ - token_begin_;
	}
	update_directive_context(kind);
	return token_;
}

const PPToken& PPLexer::finish_empty(PPTokenKind kind)
{
	token_.kind = kind;
	token_.punctuator = kNoPunctuator;
	token_.begin = base_ + token_begin_;
	token_.end = base_ + token_end_;
	token_.spelling = source_.data() + token_begin_;
	token_.length = 0;
	update_directive_context(kind);
	return token_;
}

void PPLexer::update_directive_context(PPTokenKind kind)
{
	if (header_names_on_request_)
	{
		if (kind != PPTokenKind::WhitespaceSequence)
			context_ = DirectiveContext::Other;
		return;
	}
	switch (kind)
	{
	case PPTokenKind::WhitespaceSequence:
		return;
	case PPTokenKind::NewLine:
		context_ = DirectiveContext::LineStart;
		return;
	case PPTokenKind::PreprocessingOpOrPunc:
		context_ = context_ == DirectiveContext::LineStart && token_.punctuator == kHashPunctuator
			? DirectiveContext::AfterHash
			: DirectiveContext::Other;
		return;
	case PPTokenKind::Identifier:
		context_ = context_ == DirectiveContext::AfterHash && (spelling_is("include") || spelling_is("include_next"))
			? DirectiveContext::AfterInclude
			: DirectiveContext::Other;
		return;
	default:
		context_ = DirectiveContext::Other;
		return;
	}
}

// ---- whitespace and comments ------------------------------------------------

namespace {

// Classes of a source byte that is its own character wherever it stands:
// ASCII other than `\\` and `?`, which may begin a splice or a trigraph.
enum ByteClass : uint8_t
{
	// A letter, a digit or an underscore.
	kIdentifierByte = 1,
	// Horizontal white space.
	kSpaceByte = 2,
	// Inside a `//` comment: any such byte but a new-line.
	kLineCommentByte = 4,
	// Inside a block comment: any such byte but `*`.
	kBlockCommentByte = 8,
	// In a line whose tokens are discarded: any such byte but the new-line,
	// the quotes, `/` and the null byte that ends the image.
	kPlainLineByte = 16
};

// (o: such a byte of no other class; i: an identifier byte; w: white space;
// q: a quote, `/` or the null byte; n: the new-line; s: `*`; x: `\\`, `?` and
// the bytes that are not ASCII.)
const uint8_t q = kLineCommentByte | kBlockCommentByte, o = q | kPlainLineByte;
const uint8_t i = o | kIdentifierByte, w = o | kSpaceByte;
const uint8_t n = kBlockCommentByte, s = kLineCommentByte | kPlainLineByte, x = 0;

const uint8_t kByteClasses[256] = { q, o, o, o, o, o, o, o, o, w, n, w, w, w, o, o, //
	o, o, o, o, o, o, o, o, o, o, o, o, o, o, o, o,                                 //
	w, o, q, o, o, o, o, q, o, o, s, o, o, o, o, q,                                 //
	i, i, i, i, i, i, i, i, i, i, o, o, o, o, o, x,                                 //
	o, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i,                                 //
	i, i, i, i, i, i, i, i, i, i, i, o, x, o, o, i,                                 //
	o, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i,                                 //
	i, i, i, i, i, i, i, i, i, i, i, o, o, o, o, o,                                 //
	x };

const uint32_t kNoOffset = ~0u;

} // namespace

bool PPLexer::skip_whitespace_sequence()
{
	// A maximal run of non-new-line whitespace and comments.
	for (;;)
	{
		if (IsHorizontalWhitespace(cur_.code_point))
		{
			const unsigned char* const text = bytes();
			const uint32_t size = source_.size();
			uint32_t end = cur_.end;
			while (end < size && (kByteClasses[text[end]] & kSpaceByte) != 0)
				++end;
			token_end_ = end;
			cur_ = reader_.logical(end);
			continue;
		}
		if (cur_.code_point != '/')
			return token_end_ != token_begin_;
		const SourceChar second = peek_after(cur_);
		if (second.code_point == '/')
		{
			skip_comment(second);
			while (cur_.code_point != '\n' && cur_.code_point != kEndOfSource)
				skip_comment_bytes(false);
		}
		else if (second.code_point == '*')
		{
			skip_comment(second);
			skip_block_comment();
		}
		else
		{
			return token_end_ != token_begin_;
		}
	}
}

// Whether a splice may delete the new-line byte at `new_line`: a backslash
// or `??/` stands before it in the bytes from `from` (a carriage return
// between them may be the splice's).
bool PPLexer::spliced_new_line(uint32_t from, uint32_t new_line) const
{
	const unsigned char* const text = bytes();
	uint32_t last = new_line;
	if (last > from && text[last - 1] == '\r')
		--last;
	return (last > from && text[last - 1] == '\\') ||
		(last >= from + 3 && text[last - 3] == '?' && text[last - 2] == '?' && text[last - 1] == '/');
}

// The new-line byte that ends the line from `at` where the line's tokens
// need no lexing to be discarded: they end on the line and none can be
// ill-formed. The line holds bytes that are their own characters, no quote
// and no block comment; a `//` comment runs to the new-line (in a hosted
// comment any byte may, short of a splice). kNoOffset for any other line.
uint32_t PPLexer::plain_line_end(uint32_t at) const
{
	const unsigned char* const text = bytes();
	for (;;)
	{
		while ((kByteClasses[text[at]] & kPlainLineByte) != 0)
			++at;
		if (text[at] == '\n')
			return at;
		if (text[at] != '/' || text[at + 1] == '*')
			return kNoOffset;
		if (text[at + 1] != '/')
		{
			++at;
			continue;
		}
		const unsigned char* const found = static_cast<const unsigned char*>(
			std::memchr(text + at, '\n', source_.size() - at));
		if (found == nullptr)
			return kNoOffset;
		const uint32_t last = static_cast<uint32_t>(found - text);
		if (legacy_comment_bytes_)
			return spliced_new_line(at + 2, last) ? kNoOffset : last;
		for (at += 2; at < last; ++at)
		{
			if (kByteClasses[text[at]] == 0)
				return kNoOffset;
		}
		return last;
	}
}

void PPLexer::skip_plain_lines()
{
	const unsigned char* const text = bytes();
	uint32_t at = cur_.begin;
	for (;;)
	{
		uint32_t first = at;
		while ((kByteClasses[text[first]] & kSpaceByte) != 0)
			++first;
		// (`#` or `%:` may begin a directive.)
		if (text[first] == '#' || text[first] == '%')
			break;
		const uint32_t end = plain_line_end(first);
		if (end == kNoOffset)
			break;
		at = end + 1;
	}
	if (at == cur_.begin)
		return;
	cur_ = reader_.logical(at);
	update_directive_context(PPTokenKind::NewLine);
}

bool PPLexer::skip_plain_line_rest()
{
	const uint32_t end = plain_line_end(cur_.begin);
	if (end == kNoOffset)
		return false;
	cur_ = reader_.logical(end);
	return true;
}

uint8_t PPLexer::skip_space()
{
	uint8_t flags = 0;
	for (;;)
	{
		const int32_t c = cur_.code_point;
		if (c == '\n')
		{
			cur_ = reader_.logical(cur_.end);
			flags |= kLeadingSpace | kStartOfLine;
			continue;
		}
		if (!IsHorizontalWhitespace(c) && c != '/')
			break;
		start_token();
		if (!skip_whitespace_sequence())
			break;
		flags |= kLeadingSpace;
	}
	if ((flags & kStartOfLine) != 0)
		update_directive_context(PPTokenKind::NewLine);
	return flags;
}

// Skips the comment character cur_ and the bytes after it that are their own
// characters and do not end the comment (a new-line ends a `//` comment, and
// a block comment's `*` is looked at by its caller).
void PPLexer::skip_comment_bytes(bool block)
{
	const unsigned char* const text = bytes();
	const uint32_t size = source_.size();
	uint32_t end = cur_.end;
	if (legacy_comment_bytes_)
	{
		// A hosted comment's bytes are opaque characters, so its end is
		// searched for as a byte. No trigraph or splice holds or makes a
		// `*`. A new-line ends the comment unless a splice deletes it, and
		// then the characters are read one by one.
		const unsigned char* const found = static_cast<const unsigned char*>(
			std::memchr(text + end, block ? '*' : '\n', size - end));
		const uint32_t last = found != nullptr ? static_cast<uint32_t>(found - text) : size;
		if (block || (found != nullptr && !spliced_new_line(end, last)))
		{
			token_end_ = last;
			cur_ = reader_.logical(last, true);
			return;
		}
	}
	const uint8_t wanted = block ? kBlockCommentByte : kLineCommentByte;
	while (end < size && ((kByteClasses[text[end]] & wanted) != 0 || (legacy_comment_bytes_ && text[end] >= 0x80)))
		++end;
	token_end_ = end;
	cur_ = reader_.logical(end, legacy_comment_bytes_);
}

void PPLexer::skip_block_comment()
{
	for (;;)
	{
		if (cur_.code_point == kEndOfSource)
			throw SourceError(token_begin_, "unterminated block comment");
		if (cur_.code_point != '*')
		{
			skip_comment_bytes(true);
			continue;
		}
		skip_comment(cur_);
		if (cur_.code_point == '/')
		{
			skip(cur_);
			return;
		}
	}
}

// ---- identifiers --------------------------------------------------------------

bool PPLexer::read_universal_character_name(const SourceChar& backslash, UniversalCharacterName& ucn) const
{
	SourceChar c = peek_after(backslash);
	int digits;
	if (c.code_point == 'u')
		digits = 4;
	else if (c.code_point == 'U')
		digits = 8;
	else
		return false;

	uint32_t value = 0;
	for (int i = 0; i < digits; ++i)
	{
		c = peek_after(c);
		if (!IsHexDigit(c.code_point))
			return false;
		value = (value << 4) | static_cast<uint32_t>(HexDigitValue(c.code_point));
	}
	ucn.value = value;
	ucn.begin = backslash.begin;
	ucn.end = c.end;
	return true;
}

SourceChar PPLexer::ucn_outside_literal(const UniversalCharacterName& ucn) const
{
	// [lex.charset] 2.3p2: outside literals a universal-character-name may
	// not designate a surrogate, a control character or a member of the
	// basic source character set.
	if (!IsUniversalCharacterValue(ucn.value) || IsControlCharacter(static_cast<int32_t>(ucn.value)) ||
		IsBasicSourceCharacter(static_cast<int32_t>(ucn.value)))
	{
		throw SourceError(ucn.begin, "invalid universal character value");
	}
	return SourceChar{ static_cast<int32_t>(ucn.value), ucn.begin, ucn.end };
}

bool PPLexer::take_identifier_start()
{
	if (cur_.code_point == '\\')
	{
		UniversalCharacterName ucn;
		if (!read_universal_character_name(cur_, ucn))
			return false;
		const SourceChar c = ucn_outside_literal(ucn);
		if (!IsIdentifierStart(c.code_point))
			return false;
		take(c);
		return true;
	}
	if (!IsIdentifierStart(cur_.code_point))
		return false;
	take(cur_);
	return true;
}

bool PPLexer::take_identifier_continue()
{
	if (cur_.code_point == '\\')
	{
		UniversalCharacterName ucn;
		if (!read_universal_character_name(cur_, ucn))
			return false;
		const SourceChar c = ucn_outside_literal(ucn);
		if (!IsIdentifierContinue(c.code_point))
			return false;
		take(c);
		return true;
	}
	if (!IsIdentifierContinue(cur_.code_point))
		return false;
	take(cur_);
	return true;
}

// Extends a spelling that is still a view of the source over the letters,
// digits and underscores that directly follow it (the image ends with a
// byte that is none of them). Whether the identifier is then complete: it
// is where the next byte is its own character.
bool PPLexer::take_identifier_bytes()
{
	if (dirty_ || cur_.begin != clean_end_)
		return false;
	const unsigned char* const text = bytes();
	uint32_t end = clean_end_;
	while ((kByteClasses[text[end]] & kIdentifierByte) != 0)
		++end;
	if (end != clean_end_)
	{
		clean_end_ = end;
		token_end_ = end;
		cur_ = reader_.logical(end);
	}
	return kByteClasses[text[end]] != 0;
}

const PPToken& PPLexer::lex_identifier_or_prefixed_literal()
{
	take(cur_);
	if (!take_identifier_bytes())
	{
		while (take_identifier_continue())
		{
		}
	}

	// An identifier that spells an encoding prefix, directly followed by a
	// quote, begins a literal instead ([lex.pptoken] 2.5p3 for raw strings).
	if (cur_.code_point == '"')
	{
		if (is_encoding_prefix())
			return lex_quoted_literal(PPTokenKind::StringLiteral, PPTokenKind::UserDefinedStringLiteral);
		if (is_raw_string_prefix())
			return lex_raw_string_literal();
	}
	else if (cur_.code_point == '\'' && is_character_prefix())
	{
		return lex_quoted_literal(PPTokenKind::CharacterLiteral, PPTokenKind::UserDefinedCharacterLiteral);
	}

	if (const uint8_t punctuator = identifier_like_operator())
		return finish(PPTokenKind::PreprocessingOpOrPunc, punctuator);
	return finish(PPTokenKind::Identifier);
}

const PPToken& PPLexer::lex_backslash()
{
	UniversalCharacterName ucn;
	if (!read_universal_character_name(cur_, ucn))
	{
		take(cur_);
		return finish(PPTokenKind::NonWhitespaceCharacter);
	}

	const SourceChar c = ucn_outside_literal(ucn);
	take(c);
	if (!IsIdentifierStart(c.code_point))
		return finish(PPTokenKind::NonWhitespaceCharacter);
	while (take_identifier_continue())
	{
	}
	return finish(PPTokenKind::Identifier);
}

// ---- pp-numbers -----------------------------------------------------------------

// Extends a spelling that is still a view of the source over the decimal
// digits that directly follow it.
void PPLexer::take_digit_bytes()
{
	if (dirty_ || cur_.begin != clean_end_)
		return;
	const unsigned char* const text = bytes();
	uint32_t end = clean_end_;
	while (text[end] >= '0' && text[end] <= '9')
		++end;
	if (end == clean_end_)
		return;
	clean_end_ = end;
	token_end_ = end;
	cur_ = reader_.logical(end);
}

const PPToken& PPLexer::lex_pp_number()
{
	// digit | `.` digit
	if (cur_.code_point == '.')
		take(cur_);
	take(cur_);

	for (;;)
	{
		take_digit_bytes();
		const int32_t c = cur_.code_point;
		if (c == 'e' || c == 'E' || (binary_exponents_ && (c == 'p' || c == 'P')))
		{
			// pp-number `e` sign / pp-number `E` sign
			const SourceChar sign = peek_after(cur_);
			take(cur_);
			if (sign.code_point == '+' || sign.code_point == '-')
				take(sign);
		}
		else if (c == '.' || (digit_separators_ && c == '\'' && IsIdentifierContinue(peek_after(cur_).code_point)))
		{
			take(cur_);
		}
		else if (!take_identifier_continue())
		{
			return finish(PPTokenKind::PPNumber);
		}
	}
}
