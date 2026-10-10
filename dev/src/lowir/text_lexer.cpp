#include "lowir/text_lexer.h"

#include <cstring>

namespace lowir {

namespace {

bool is_digit(char c)
{
	return c >= '0' && c <= '9';
}

bool is_alnum(char c)
{
	return is_digit(c) || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

// Characters of names and words after the first.
bool is_name_char(char c)
{
	return is_alnum(c) || c == '.' || c == '$';
}

bool is_space(char c)
{
	return c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\f' || c == '\v';
}

// Characters of a debug location's file token.
bool is_file_char(char c)
{
	return !is_space(c) && c != ',' && c != '(' && c != ')';
}

TokenKind punctuator(char c)
{
	switch (c)
	{
	case '(':
		return TokenKind::LParen;
	case ')':
		return TokenKind::RParen;
	case '[':
		return TokenKind::LBracket;
	case ']':
		return TokenKind::RBracket;
	case '{':
		return TokenKind::LBrace;
	case '}':
		return TokenKind::RBrace;
	case ',':
		return TokenKind::Comma;
	case ':':
		return TokenKind::Colon;
	case '=':
		return TokenKind::Equals;
	case '+':
		return TokenKind::Plus;
	default:
		return TokenKind::Invalid;
	}
}

TokenKind sigil(char c)
{
	switch (c)
	{
	case '%':
		return TokenKind::Temp;
	case '$':
		return TokenKind::Slot;
	case '@':
		return TokenKind::Global;
	case '^':
		return TokenKind::Block;
	default:
		return TokenKind::Invalid;
	}
}

} // namespace

Lexer::Lexer(NameTable& names, const char* begin, const char* end)
	: names_(names), position_(begin), end_(end), line_(1)
{
	scan(current_);
}

void Lexer::skip_space()
{
	// Whitespace and `#` comments, which run to the end of the line.
	while (position_ < end_)
	{
		if (*position_ == '#')
		{
			while (position_ < end_ && *position_ != '\n')
				++position_;
		}
		else if (!is_space(*position_))
			return;
		else
		{
			if (*position_ == '\n')
				++line_;
			++position_;
		}
	}
}

void Lexer::scan(Token& token)
{
	skip_space();
	std::memset(&token, 0, sizeof token);
	token.line = line_;
	token.text = position_;
	if (position_ == end_)
	{
		token.kind = TokenKind::End;
		return;
	}
	const char c = *position_;
	const TokenKind name_kind = sigil(c);
	if (name_kind != TokenKind::Invalid)
	{
		++position_;
		scan_word(token, name_kind, position_);
		if (token.length == 0)
		{
			token.kind = TokenKind::Invalid;
			token.problem = "expected a name after its sigil";
		}
		return;
	}
	if (c == '-')
	{
		const bool arrow = position_ + 1 < end_ && position_[1] == '>';
		token.kind = arrow ? TokenKind::Arrow : TokenKind::Minus;
		token.length = arrow ? 2 : 1;
		position_ += token.length;
		return;
	}
	if (c == '!')
	{
		scan_debug(token);
		token.length = static_cast<uint32_t>(position_ - token.text);
		return;
	}
	if (is_digit(c) || (c == '.' && position_ + 1 < end_ && is_digit(position_[1])))
	{
		scan_number(token);
		return;
	}
	if (is_name_char(c))
	{
		scan_word(token, TokenKind::Word, position_);
		return;
	}
	token.kind = punctuator(c);
	token.length = 1;
	if (token.kind == TokenKind::Invalid)
		token.problem = "unexpected character";
	++position_;
}

void Lexer::scan_word(Token& token, TokenKind kind, const char* start)
{
	while (position_ < end_ && is_name_char(*position_))
		++position_;
	// `obj<bytesxalign>` is one type word.
	if (kind == TokenKind::Word && position_ - start == 3 && std::memcmp(start, "obj", 3) == 0 && position_ < end_ &&
		*position_ == '<')
	{
		while (position_ < end_ && *position_ != '>' && !is_space(*position_))
			++position_;
		if (position_ < end_ && *position_ == '>')
			++position_;
	}
	token.kind = kind;
	token.text = start;
	token.length = static_cast<uint32_t>(position_ - start);
	token.name = names_.intern(start, token.length);
}

void Lexer::scan_number(Token& token)
{
	const char* start = position_;
	const bool hex = end_ - position_ > 1 && position_[0] == '0' && (position_[1] == 'x' || position_[1] == 'X');
	while (position_ < end_)
	{
		const char c = *position_;
		const bool exponent = hex ? (c == 'p' || c == 'P') : (c == 'e' || c == 'E');
		if (exponent && position_ + 1 < end_ && (position_[1] == '+' || position_[1] == '-'))
			position_ += 2;
		else if (is_alnum(c) || c == '.')
			++position_;
		else
			break;
	}
	token.kind = TokenKind::Number;
	token.text = start;
	token.length = static_cast<uint32_t>(position_ - start);
}

bool Lexer::scan_decimal(uint64_t& value)
{
	// A positive decimal without leading zeros; anything else reads as 0.
	skip_space();
	const char* start = position_;
	value = 0;
	bool overflow = false;
	while (position_ < end_ && is_digit(*position_))
	{
		if (value > (~static_cast<uint64_t>(0) - 9) / 10)
			overflow = true;
		value = value * 10 + static_cast<uint64_t>(*position_ - '0');
		++position_;
	}
	if (position_ == start)
		return false;
	if (overflow || *start == '0')
		value = 0;
	return true;
}

void Lexer::scan_debug(Token& token)
{
	token.kind = TokenKind::Invalid;
	token.problem = "malformed !dbg(file, line, column) location";
	if (end_ - position_ < 4 || std::memcmp(position_, "!dbg", 4) != 0)
	{
		++position_;
		token.problem = "unexpected character";
		return;
	}
	position_ += 4;
	skip_space();
	if (position_ == end_ || *position_ != '(')
		return;
	++position_;
	skip_space();
	const char* file = position_;
	while (position_ < end_ && is_file_char(*position_))
		++position_;
	if (position_ == file)
		return;
	token.debug_file = names_.intern(file, static_cast<uint32_t>(position_ - file));
	for (int field = 0; field < 2; ++field)
	{
		skip_space();
		if (position_ == end_ || *position_ != ',')
			return;
		++position_;
		if (!scan_decimal(field == 0 ? token.debug_line : token.debug_column))
			return;
	}
	skip_space();
	if (position_ == end_ || *position_ != ')')
		return;
	++position_;
	token.kind = TokenKind::Debug;
	token.problem = nullptr;
}

} // namespace lowir
