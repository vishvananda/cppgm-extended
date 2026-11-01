#include "preprocess/lexer/pp_lexer.h"

#include <cstring>

#include "preprocess/lexer/char_classes.h"
#include "support/unicode/utf8.h"

PPLexer::PPLexer(const SourceBuffer& source, const PPLexerOptions& options)
	: source_(source), reader_(source, !options.token_text, options.crlf_splices),
	  cur_(reader_.logical(source.begin_offset())), token_(), token_begin_(0), clean_end_(0), token_end_(0),
	  base_(options.location_base), dirty_(false), header_names_on_request_(options.header_names_on_request),
	  binary_exponents_(options.binary_exponents), legacy_comment_bytes_(options.legacy_comment_bytes),
	  lone_quotes_(options.lone_quotes),
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

bool PPLexer::skip_whitespace_sequence()
{
	// A maximal run of non-new-line whitespace and comments.
	for (;;)
	{
		if (IsHorizontalWhitespace(cur_.code_point))
		{
			skip(cur_);
			continue;
		}
		if (cur_.code_point != '/')
			return token_end_ != token_begin_;
		const SourceChar second = peek_after(cur_);
		if (second.code_point == '/')
		{
			skip_comment(second);
			while (cur_.code_point != '\n' && cur_.code_point != kEndOfSource)
				skip_comment(cur_);
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

void PPLexer::skip_block_comment()
{
	for (;;)
	{
		if (cur_.code_point == kEndOfSource)
			throw SourceError(token_begin_, "unterminated block comment");
		const bool star = cur_.code_point == '*';
		skip_comment(cur_);
		if (star && cur_.code_point == '/')
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

const PPToken& PPLexer::lex_identifier_or_prefixed_literal()
{
	take(cur_);
	while (take_identifier_continue())
	{
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

const PPToken& PPLexer::lex_pp_number()
{
	// digit | `.` digit
	if (cur_.code_point == '.')
		take(cur_);
	take(cur_);

	for (;;)
	{
		const int32_t c = cur_.code_point;
		if (c == 'e' || c == 'E' || (binary_exponents_ && (c == 'p' || c == 'P')))
		{
			// pp-number `e` sign / pp-number `E` sign
			const SourceChar sign = peek_after(cur_);
			take(cur_);
			if (sign.code_point == '+' || sign.code_point == '-')
				take(sign);
		}
		else if (c == '.')
		{
			take(cur_);
		}
		else if (!take_identifier_continue())
		{
			return finish(PPTokenKind::PPNumber);
		}
	}
}
