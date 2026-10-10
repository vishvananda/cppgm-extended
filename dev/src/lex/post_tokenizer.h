#pragma once

#include <cstdint>
#include <string>

#include "lex/number_literal.h"
#include "lex/post_token.h"
#include "lex/string_concatenation.h"
#include "preprocess/tokens/pp_token_cursor.h"

class IdentifierTable;

// Translation phases 5-7 as a streaming cursor over preprocessing tokens:
// keywords, operators and punctuators become simple tokens (an interned
// identifier's keyword and a punctuator's kind come with the pp-token, so
// only a bare lexer's identifiers are looked up), literals are
// classified and evaluated, and adjacent string literals are concatenated
// (phase 6). Whitespace and new-lines are dropped. A pp-token that forms no
// token is reported as Invalid and conversion continues.
//
// The cursor holds at most one pp-token of lookahead (the one ending a
// string literal sequence), reusable scratch buffers and the string
// sequence being concatenated.
class PostTokenizer
{
public:
	// With `identifiers`, the identifier split off `operator ""identifier`
	// is interned there like the input's identifiers. `literals`: the
	// literal forms read (see ClassifyNumber).
	explicit PostTokenizer(
		PPTokenCursor& input, IdentifierTable* identifiers = nullptr, NumberDialect literals = NumberDialect::Cxx11);

	// The next token; EndOfFile repeats at the end. Propagates the input's
	// exceptions (SourceError from the lexer).
	const PostToken& next();

private:
	const PPToken& pull();
	const PPToken& pull_input();
	void convert(const PPToken& pp);
	void convert_identifier(const Identifier* identifier);
	void convert_operator(uint8_t punctuator);
	void convert_number(const PPToken& pp);
	void convert_character(const PPToken& pp);
	void convert_string_sequence(const PPToken& first);
	bool split_literal_operator_suffix(const PPToken& first);
	void finish_string_sequence(uint32_t begin, uint32_t end);
	void start_token(PostTokenKind kind, const TokenSpelling& spelling, uint32_t begin, uint32_t end);
	void set_scalar(LiteralForm form, FundamentalType type, uint64_t value);
	void set_ud_suffix(uint32_t suffix_begin);

	PPTokenCursor& input_;
	IdentifierTable* identifiers_;
	NumberDialect literals_;
	// A pp-token pulled past the end of a string literal sequence.
	const PPToken* lookahead_;
	PostToken token_;
	// The identifier split off `operator ""identifier` (see
	// split_literal_operator_suffix), emitted by the next call.
	TokenSpelling split_suffix_;
	uint32_t split_suffix_begin_;
	uint32_t split_suffix_end_;
	bool after_operator_;
	// Scalar literal values; 16 bytes hold a long double.
	unsigned char scalar_[16];
	std::string floating_text_;
	StringLiteralSequence strings_;
};
