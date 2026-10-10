#include "preprocess/lexer/pp_lexer.h"

#include <cstring>

namespace {

// A preprocessing-op-or-punc spelling and its PPToken::punctuator.
struct PunctuatorSpelling
{
	const char* text;
	uint8_t punctuator;
};

#define CPPGM_PUNCTUATOR(text, kind) { text, static_cast<uint8_t>(static_cast<uint8_t>(TokenKind::kind) + 1) }

// [lex.operators] 2.13 preprocessing-op-or-punc spellings made of
// punctuation, grouped by first character and ordered longest first so the
// first match is the longest one. Lists end with a null spelling.
const PunctuatorSpelling* PunctuatorCandidates(int32_t first)
{
	static const PunctuatorSpelling kLeftBrace[] = { CPPGM_PUNCTUATOR("{", OP_LBRACE), { nullptr, 0 } };
	static const PunctuatorSpelling kRightBrace[] = { CPPGM_PUNCTUATOR("}", OP_RBRACE), { nullptr, 0 } };
	static const PunctuatorSpelling kLeftBracket[] = { CPPGM_PUNCTUATOR("[", OP_LSQUARE), { nullptr, 0 } };
	static const PunctuatorSpelling kRightBracket[] = { CPPGM_PUNCTUATOR("]", OP_RSQUARE), { nullptr, 0 } };
	static const PunctuatorSpelling kLeftParen[] = { CPPGM_PUNCTUATOR("(", OP_LPAREN), { nullptr, 0 } };
	static const PunctuatorSpelling kRightParen[] = { CPPGM_PUNCTUATOR(")", OP_RPAREN), { nullptr, 0 } };
	static const PunctuatorSpelling kSemicolon[] = { CPPGM_PUNCTUATOR(";", OP_SEMICOLON), { nullptr, 0 } };
	static const PunctuatorSpelling kQuestion[] = { CPPGM_PUNCTUATOR("?", OP_QMARK), { nullptr, 0 } };
	static const PunctuatorSpelling kComma[] = { CPPGM_PUNCTUATOR(",", OP_COMMA), { nullptr, 0 } };
	static const PunctuatorSpelling kTilde[] = { CPPGM_PUNCTUATOR("~", OP_COMPL), { nullptr, 0 } };
	static const PunctuatorSpelling kHash[] = { { "##", kHashHashPunctuator }, { "#", kHashPunctuator },
		{ nullptr, 0 } };
	static const PunctuatorSpelling kColon[] = { CPPGM_PUNCTUATOR("::", OP_COLON2), CPPGM_PUNCTUATOR(":>", OP_RSQUARE),
		CPPGM_PUNCTUATOR(":", OP_COLON), { nullptr, 0 } };
	static const PunctuatorSpelling kDot[] = { CPPGM_PUNCTUATOR("...", OP_DOTS), CPPGM_PUNCTUATOR(".*", OP_DOTSTAR),
		CPPGM_PUNCTUATOR(".", OP_DOT), { nullptr, 0 } };
	static const PunctuatorSpelling kPlus[] = { CPPGM_PUNCTUATOR("++", OP_INC), CPPGM_PUNCTUATOR("+=", OP_PLUSASS),
		CPPGM_PUNCTUATOR("+", OP_PLUS), { nullptr, 0 } };
	static const PunctuatorSpelling kMinus[] = { CPPGM_PUNCTUATOR("->*", OP_ARROWSTAR), CPPGM_PUNCTUATOR("--", OP_DEC),
		CPPGM_PUNCTUATOR("-=", OP_MINUSASS), CPPGM_PUNCTUATOR("->", OP_ARROW), CPPGM_PUNCTUATOR("-", OP_MINUS),
		{ nullptr, 0 } };
	static const PunctuatorSpelling kStar[] = { CPPGM_PUNCTUATOR("*=", OP_STARASS), CPPGM_PUNCTUATOR("*", OP_STAR),
		{ nullptr, 0 } };
	static const PunctuatorSpelling kSlash[] = { CPPGM_PUNCTUATOR("/=", OP_DIVASS), CPPGM_PUNCTUATOR("/", OP_DIV),
		{ nullptr, 0 } };
	static const PunctuatorSpelling kPercent[] = { { "%:%:", kHashHashPunctuator }, CPPGM_PUNCTUATOR("%=", OP_MODASS),
		CPPGM_PUNCTUATOR("%>", OP_RBRACE), { "%:", kHashPunctuator }, CPPGM_PUNCTUATOR("%", OP_MOD), { nullptr, 0 } };
	static const PunctuatorSpelling kCaret[] = { CPPGM_PUNCTUATOR("^=", OP_XORASS), CPPGM_PUNCTUATOR("^", OP_XOR),
		{ nullptr, 0 } };
	static const PunctuatorSpelling kAmpersand[] = { CPPGM_PUNCTUATOR("&&", OP_LAND),
		CPPGM_PUNCTUATOR("&=", OP_BANDASS), CPPGM_PUNCTUATOR("&", OP_AMP), { nullptr, 0 } };
	static const PunctuatorSpelling kBar[] = { CPPGM_PUNCTUATOR("||", OP_LOR), CPPGM_PUNCTUATOR("|=", OP_BORASS),
		CPPGM_PUNCTUATOR("|", OP_BOR), { nullptr, 0 } };
	static const PunctuatorSpelling kExclamation[] = { CPPGM_PUNCTUATOR("!=", OP_NE), CPPGM_PUNCTUATOR("!", OP_LNOT),
		{ nullptr, 0 } };
	static const PunctuatorSpelling kEquals[] = { CPPGM_PUNCTUATOR("==", OP_EQ), CPPGM_PUNCTUATOR("=", OP_ASS),
		{ nullptr, 0 } };
	static const PunctuatorSpelling kLess[] = { CPPGM_PUNCTUATOR("<<=", OP_LSHIFTASS),
		CPPGM_PUNCTUATOR("<<", OP_LSHIFT), CPPGM_PUNCTUATOR("<=", OP_LE), CPPGM_PUNCTUATOR("<%", OP_LBRACE),
		CPPGM_PUNCTUATOR("<:", OP_LSQUARE), CPPGM_PUNCTUATOR("<", OP_LT), { nullptr, 0 } };
	static const PunctuatorSpelling kGreater[] = { CPPGM_PUNCTUATOR(">>=", OP_RSHIFTASS),
		CPPGM_PUNCTUATOR(">>", OP_RSHIFT), CPPGM_PUNCTUATOR(">=", OP_GE), CPPGM_PUNCTUATOR(">", OP_GT),
		{ nullptr, 0 } };

	switch (first)
	{
	case '{':
		return kLeftBrace;
	case '}':
		return kRightBrace;
	case '[':
		return kLeftBracket;
	case ']':
		return kRightBracket;
	case '(':
		return kLeftParen;
	case ')':
		return kRightParen;
	case ';':
		return kSemicolon;
	case '?':
		return kQuestion;
	case ',':
		return kComma;
	case '~':
		return kTilde;
	case '#':
		return kHash;
	case ':':
		return kColon;
	case '.':
		return kDot;
	case '+':
		return kPlus;
	case '-':
		return kMinus;
	case '*':
		return kStar;
	case '/':
		return kSlash;
	case '%':
		return kPercent;
	case '^':
		return kCaret;
	case '&':
		return kAmpersand;
	case '|':
		return kBar;
	case '!':
		return kExclamation;
	case '=':
		return kEquals;
	case '<':
		return kLess;
	case '>':
		return kGreater;
	default:
		return nullptr;
	}
}

// Up to four decoded characters starting at a punctuator's first character.
class PunctuatorLookahead
{
public:
	PunctuatorLookahead(const TranslationReader& reader, const SourceChar& first) : reader_(reader), count_(1)
	{
		chars_[0] = first;
	}

	const SourceChar& at(unsigned index)
	{
		while (count_ <= index)
		{
			chars_[count_] = reader_.logical(chars_[count_ - 1].end);
			++count_;
		}
		return chars_[index];
	}

	bool spells(const char* text)
	{
		for (unsigned i = 0; text[i] != '\0'; ++i)
		{
			if (at(i).code_point != static_cast<unsigned char>(text[i]))
				return false;
		}
		return true;
	}

private:
	static const unsigned kCapacity = 4;

	const TranslationReader& reader_;
	unsigned count_;
	SourceChar chars_[kCapacity];
};

// A fixed spelling with its length.
struct Spelling
{
	const char* text;
	uint32_t length;
};

template<std::size_t N>
constexpr Spelling MakeSpelling(const char (&text)[N])
{
	return Spelling{ text, static_cast<uint32_t>(N - 1) };
}

const Spelling kEncodingPrefixes[] = { MakeSpelling("u8"), MakeSpelling("u"), MakeSpelling("U"), MakeSpelling("L") };

const Spelling kCharacterPrefixes[] = { MakeSpelling("u"), MakeSpelling("U"), MakeSpelling("L") };

const Spelling kRawStringPrefixes[] = { MakeSpelling("R"), MakeSpelling("u8R"), MakeSpelling("uR"), MakeSpelling("UR"),
	MakeSpelling("LR") };

// [lex.operators] 2.13 preprocessing-op-or-punc spellings that are lexed as
// identifiers, with their PPToken::punctuator.
struct IdentifierLikeOperator
{
	Spelling spelling;
	uint8_t punctuator;
};

#define CPPGM_IDENTIFIER_LIKE(text, kind) \
	{ MakeSpelling(text), static_cast<uint8_t>(static_cast<uint8_t>(TokenKind::kind) + 1) }

const IdentifierLikeOperator kIdentifierLikeOperators[] = { CPPGM_IDENTIFIER_LIKE("new", KW_NEW),
	CPPGM_IDENTIFIER_LIKE("delete", KW_DELETE), CPPGM_IDENTIFIER_LIKE("and", OP_LAND),
	CPPGM_IDENTIFIER_LIKE("and_eq", OP_BANDASS), CPPGM_IDENTIFIER_LIKE("bitand", OP_AMP),
	CPPGM_IDENTIFIER_LIKE("bitor", OP_BOR), CPPGM_IDENTIFIER_LIKE("compl", OP_COMPL),
	CPPGM_IDENTIFIER_LIKE("not", OP_LNOT), CPPGM_IDENTIFIER_LIKE("not_eq", OP_NE), CPPGM_IDENTIFIER_LIKE("or", OP_LOR),
	CPPGM_IDENTIFIER_LIKE("or_eq", OP_BORASS), CPPGM_IDENTIFIER_LIKE("xor", OP_XOR),
	CPPGM_IDENTIFIER_LIKE("xor_eq", OP_XORASS) };

template<std::size_t N>
bool SpellingIn(const char* spelling, uint32_t length, const Spelling (&list)[N])
{
	for (const Spelling& candidate : list)
	{
		if (candidate.length == length && std::memcmp(candidate.text, spelling, length) == 0)
			return true;
	}
	return false;
}

} // namespace

uint8_t PPLexer::try_lex_punctuator()
{
	const PunctuatorSpelling* candidates = PunctuatorCandidates(cur_.code_point);
	if (candidates == nullptr)
		return kNoPunctuator;

	PunctuatorLookahead ahead(reader_, cur_);

	// [lex.pptoken] 2.5p3: `<::` not followed by `:` or `>` lexes `<` alone
	// rather than the alternative token `<:`.
	if (ahead.spells("<::") && ahead.at(3).code_point != ':' && ahead.at(3).code_point != '>')
	{
		take(cur_);
		return PunctuatorOf(TokenKind::OP_LT);
	}

	for (; candidates->text != nullptr; ++candidates)
	{
		if (!ahead.spells(candidates->text))
			continue;
		const unsigned length = static_cast<unsigned>(std::strlen(candidates->text));
		for (unsigned i = 0; i + 1 < length; ++i)
			append(ahead.at(i));
		take(ahead.at(length - 1));
		return candidates->punctuator;
	}
	return kNoPunctuator;
}

void PPLexer::current_spelling(const char*& spelling, uint32_t& length) const
{
	spelling = dirty_ ? scratch_.data() : source_.data() + token_begin_;
	length = dirty_ ? static_cast<uint32_t>(scratch_.size()) : clean_end_ - token_begin_;
}

bool PPLexer::is_encoding_prefix() const
{
	const char* spelling;
	uint32_t length;
	current_spelling(spelling, length);
	return SpellingIn(spelling, length, kEncodingPrefixes);
}

bool PPLexer::is_character_prefix() const
{
	const char* spelling;
	uint32_t length;
	current_spelling(spelling, length);
	return SpellingIn(spelling, length, kCharacterPrefixes);
}

bool PPLexer::is_raw_string_prefix() const
{
	const char* spelling;
	uint32_t length;
	current_spelling(spelling, length);
	return SpellingIn(spelling, length, kRawStringPrefixes);
}

uint8_t PPLexer::identifier_like_operator() const
{
	const char* spelling;
	uint32_t length;
	current_spelling(spelling, length);
	for (const IdentifierLikeOperator& candidate : kIdentifierLikeOperators)
	{
		if (candidate.spelling.length == length && std::memcmp(candidate.spelling.text, spelling, length) == 0)
			return candidate.punctuator;
	}
	return kNoPunctuator;
}
