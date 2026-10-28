#pragma once

#include <cstdint>

#include "lowir/name_table.h"

namespace lowir {

enum class TokenKind : uint8_t
{
	End,
	Word,   // keyword, type, identifier (`obj<16x8>` is one word)
	String, // quoted presentation name in optional debug records
	Number, // starts with a digit, or '.' and a digit
	Temp,   // %name
	Slot,   // $name
	Global, // @name
	Block,  // ^name
	LParen,
	RParen,
	LBracket,
	RBracket,
	LBrace,
	RBrace,
	Comma,
	Colon,
	Equals,
	Plus,
	Minus,
	Arrow,
	Debug, // !dbg(file, line, column)
	Invalid
};

struct Token
{
	TokenKind kind;
	uint32_t line;
	// The spelling; names without their sigil.
	const char* text;
	uint32_t length;
	// Word and name tokens.
	NameId name;
	// Debug tokens: the file and the line and column as written (0 if not a
	// positive decimal).
	NameId debug_file;
	uint64_t debug_line;
	uint64_t debug_column;
	uint64_t debug_scope;
	// Invalid tokens: what is wrong.
	const char* problem;
};

// Splits one immutable LowIR source buffer into tokens on demand: the
// parser sees the current token and advances, and no token list is built.
// Words and names are interned as they are scanned.
class Lexer
{
public:
	Lexer(NameTable& names, const char* begin, const char* end);

	const Token& current() const { return current_; }
	void advance() { scan(current_); }

private:
	void scan(Token& token);
	void skip_space();
	void scan_word(Token& token, TokenKind kind, const char* start);
	void scan_number(Token& token);
	void scan_string(Token& token);
	void scan_debug(Token& token);
	bool scan_decimal(uint64_t& value);

	NameTable& names_;
	const char* position_;
	const char* end_;
	uint32_t line_;
	Token current_;
};

} // namespace lowir
