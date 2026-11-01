#pragma once

#include <cstdint>
#include <string>

#include "preprocess/lexer/pp_token.h"
#include "preprocess/source/source_buffer.h"
#include "preprocess/source/translation_reader.h"
#include "preprocess/tokens/pp_token_cursor.h"
#include "support/unicode/utf8.h"

struct PPLexerOptions
{
	// Added to token offsets: the buffer's base in a translation unit's
	// location space. SourceError offsets stay buffer offsets.
	uint32_t location_base = 0;
	// Header-names are lexed only after request_header_name(), when the
	// directive layer knows one may follow. Otherwise the lexer tracks
	// `#include` and `#include_next` at the start of a line itself.
	bool header_names_on_request = false;
	// The buffer holds token spellings that are past phases 1-2, so no
	// trigraph is replaced and no splice deleted (token pasting re-lexes
	// spellings, [cpp.concat] 16.3.3p3).
	bool token_text = false;
	// A pp-number continues with `p` or `P` and a sign, as in C++17 (hosted
	// hexadecimal floating literals such as `0x1p+4`).
	bool binary_exponents = false;
	// Hosted source comments may contain legacy bytes, never token spellings.
	bool legacy_comment_bytes = false;
	// A quote with no partner on its line is a token of its own (hosted).
	bool lone_quotes = false;
	// Hosted sources may end lines in CRLF: a splice deletes the CR too.
	bool crlf_splices = false;
};

// Streaming preprocessing tokenizer: translation phase 3 over the phase 1-2
// view of one source buffer. Tokens are produced on demand; the lexer keeps
// only the current position, the header-name context and one scratch
// spelling buffer.
class PPLexer final : public PPTokenCursor
{
public:
	explicit PPLexer(const SourceBuffer& source, const PPLexerOptions& options = PPLexerOptions());

	// Lexes the next token. Once EndOfFile is returned, every later call
	// returns EndOfFile again. Throws SourceError for ill-formed input.
	const PPToken& next() override;

	// The next token that is not white space may be a header-name
	// (header_names_on_request mode).
	void request_header_name() { context_ = DirectiveContext::AfterInclude; }

private:
	// Where header-name tokens may appear: after `#include` or
	// `#include_next` at the start of a line ([lex.header] 2.9,
	// [cpp.include] 16.2), or on request. Whitespace does not change the
	// context.
	enum class DirectiveContext : uint8_t
	{
		LineStart,
		AfterHash,
		AfterInclude,
		Other
	};

	// A syntactically complete universal-character-name (value unchecked).
	struct UniversalCharacterName
	{
		uint32_t value;
		uint32_t begin;
		uint32_t end;
	};

	// Token assembly (pp_lexer.cpp).
	void start_token();
	void make_dirty();
	void append(const SourceChar& c)
	{
		// A character whose bytes are contiguous with the clean prefix and
		// are exactly its UTF-8 encoding leaves the spelling a view of the
		// source. A deleted splice breaks contiguity. Trigraphs and
		// universal-character-names shrink and Windows-1252 bytes grow when
		// translated, so a length match rules them out.
		if (!dirty_ && c.begin == clean_end_ && c.end - c.begin == Utf8EncodedLength(c.code_point))
			clean_end_ = c.end;
		else
			append_translated(c);
		token_end_ = c.end;
	}
	void append_translated(const SourceChar& c);
	void append_source_bytes(uint32_t begin, uint32_t end);
	void take(const SourceChar& c)
	{
		append(c);
		cur_ = reader_.logical(c.end);
	}
	void skip(const SourceChar& c)
	{
		token_end_ = c.end;
		cur_ = reader_.logical(c.end);
	}
	void skip_comment(const SourceChar& c)
	{
		token_end_ = c.end;
		cur_ = reader_.logical(c.end, legacy_comment_bytes_);
	}
	SourceChar peek_after(const SourceChar& c) const { return reader_.logical(c.end); }
	bool spelling_is(const char* text) const;
	const PPToken& finish(PPTokenKind kind, uint8_t punctuator = kNoPunctuator);
	const PPToken& finish_empty(PPTokenKind kind);
	void update_directive_context(PPTokenKind kind);

	// Whitespace, identifiers and numbers (pp_lexer.cpp).
	bool skip_whitespace_sequence();
	void skip_block_comment();
	bool read_universal_character_name(const SourceChar& backslash, UniversalCharacterName& ucn) const;
	SourceChar ucn_outside_literal(const UniversalCharacterName& ucn) const;
	bool take_identifier_start();
	bool take_identifier_continue();
	const PPToken& lex_identifier_or_prefixed_literal();
	const PPToken& lex_backslash();
	const PPToken& lex_pp_number();

	// Literals and header names (pp_lexer_literals.cpp).
	const PPToken& lex_quoted_literal(PPTokenKind plain, PPTokenKind user_defined);
	const PPToken& lex_raw_string_literal();
	const PPToken& finish_literal(PPTokenKind plain, PPTokenKind user_defined);
	void lex_escape_sequence();
	void lex_literal_universal_character_name(const SourceChar& backslash);
	bool try_lex_header_name();

	// Operators, punctuators and spelling classes (pp_lexer_punctuators.cpp).
	void current_spelling(const char*& spelling, uint32_t& length) const;
	uint8_t try_lex_punctuator();
	bool is_encoding_prefix() const;
	bool is_character_prefix() const;
	bool is_raw_string_prefix() const;
	uint8_t identifier_like_operator() const;

	const SourceBuffer& source_;
	TranslationReader reader_;
	SourceChar cur_;
	PPToken token_;
	// While !dirty_, the spelling is source bytes [token_begin_, clean_end_);
	// the first translated character copies that prefix into scratch_.
	std::string scratch_;
	uint32_t token_begin_;
	uint32_t clean_end_;
	uint32_t token_end_;
	uint32_t base_;
	bool dirty_;
	bool header_names_on_request_;
	bool binary_exponents_;
	bool legacy_comment_bytes_;
	bool lone_quotes_;
	DirectiveContext context_;
};
