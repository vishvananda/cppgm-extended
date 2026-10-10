// Spelling table adapted from the CPPGM PA2 starter code
// (C) 2013 CPPGM Foundation www.cppgm.org. See NOTICE.

#include "lex/token_kind.h"

#include <cstring>

namespace {

const char* const kTokenKindNames[] = {
#define CPPGM_TOKEN_KIND_NAME(name) #name,
	CPPGM_TOKEN_KINDS(CPPGM_TOKEN_KIND_NAME)
#undef CPPGM_TOKEN_KIND_NAME
};

struct SimpleSpelling
{
	const char* text;
	TokenKind kind;
};

const SimpleSpelling kSimpleSpellings[] = {
	{ "alignas", TokenKind::KW_ALIGNAS },
	{ "alignof", TokenKind::KW_ALIGNOF },
	{ "asm", TokenKind::KW_ASM },
	{ "auto", TokenKind::KW_AUTO },
	{ "bool", TokenKind::KW_BOOL },
	{ "break", TokenKind::KW_BREAK },
	{ "case", TokenKind::KW_CASE },
	{ "catch", TokenKind::KW_CATCH },
	{ "char", TokenKind::KW_CHAR },
	{ "char16_t", TokenKind::KW_CHAR16_T },
	{ "char32_t", TokenKind::KW_CHAR32_T },
	{ "class", TokenKind::KW_CLASS },
	{ "const", TokenKind::KW_CONST },
	{ "constexpr", TokenKind::KW_CONSTEXPR },
	{ "const_cast", TokenKind::KW_CONST_CAST },
	{ "continue", TokenKind::KW_CONTINUE },
	{ "decltype", TokenKind::KW_DECLTYPE },
	{ "default", TokenKind::KW_DEFAULT },
	{ "delete", TokenKind::KW_DELETE },
	{ "do", TokenKind::KW_DO },
	{ "double", TokenKind::KW_DOUBLE },
	{ "dynamic_cast", TokenKind::KW_DYNAMIC_CAST },
	{ "else", TokenKind::KW_ELSE },
	{ "enum", TokenKind::KW_ENUM },
	{ "explicit", TokenKind::KW_EXPLICIT },
	{ "export", TokenKind::KW_EXPORT },
	{ "extern", TokenKind::KW_EXTERN },
	{ "false", TokenKind::KW_FALSE },
	{ "float", TokenKind::KW_FLOAT },
	{ "for", TokenKind::KW_FOR },
	{ "friend", TokenKind::KW_FRIEND },
	{ "goto", TokenKind::KW_GOTO },
	{ "if", TokenKind::KW_IF },
	{ "inline", TokenKind::KW_INLINE },
	{ "int", TokenKind::KW_INT },
	{ "long", TokenKind::KW_LONG },
	{ "mutable", TokenKind::KW_MUTABLE },
	{ "namespace", TokenKind::KW_NAMESPACE },
	{ "new", TokenKind::KW_NEW },
	{ "noexcept", TokenKind::KW_NOEXCEPT },
	{ "nullptr", TokenKind::KW_NULLPTR },
	{ "operator", TokenKind::KW_OPERATOR },
	{ "private", TokenKind::KW_PRIVATE },
	{ "protected", TokenKind::KW_PROTECTED },
	{ "public", TokenKind::KW_PUBLIC },
	{ "register", TokenKind::KW_REGISTER },
	{ "reinterpret_cast", TokenKind::KW_REINTERPET_CAST },
	{ "return", TokenKind::KW_RETURN },
	{ "short", TokenKind::KW_SHORT },
	{ "signed", TokenKind::KW_SIGNED },
	{ "sizeof", TokenKind::KW_SIZEOF },
	{ "static", TokenKind::KW_STATIC },
	{ "static_assert", TokenKind::KW_STATIC_ASSERT },
	{ "static_cast", TokenKind::KW_STATIC_CAST },
	{ "struct", TokenKind::KW_STRUCT },
	{ "switch", TokenKind::KW_SWITCH },
	{ "template", TokenKind::KW_TEMPLATE },
	{ "this", TokenKind::KW_THIS },
	{ "thread_local", TokenKind::KW_THREAD_LOCAL },
	{ "throw", TokenKind::KW_THROW },
	{ "true", TokenKind::KW_TRUE },
	{ "try", TokenKind::KW_TRY },
	{ "typedef", TokenKind::KW_TYPEDEF },
	{ "typeid", TokenKind::KW_TYPEID },
	{ "typename", TokenKind::KW_TYPENAME },
	{ "union", TokenKind::KW_UNION },
	{ "unsigned", TokenKind::KW_UNSIGNED },
	{ "using", TokenKind::KW_USING },
	{ "virtual", TokenKind::KW_VIRTUAL },
	{ "void", TokenKind::KW_VOID },
	{ "volatile", TokenKind::KW_VOLATILE },
	{ "wchar_t", TokenKind::KW_WCHAR_T },
	{ "while", TokenKind::KW_WHILE },

	{ "{", TokenKind::OP_LBRACE },
	{ "<%", TokenKind::OP_LBRACE },
	{ "}", TokenKind::OP_RBRACE },
	{ "%>", TokenKind::OP_RBRACE },
	{ "[", TokenKind::OP_LSQUARE },
	{ "<:", TokenKind::OP_LSQUARE },
	{ "]", TokenKind::OP_RSQUARE },
	{ ":>", TokenKind::OP_RSQUARE },
	{ "(", TokenKind::OP_LPAREN },
	{ ")", TokenKind::OP_RPAREN },
	{ "|", TokenKind::OP_BOR },
	{ "bitor", TokenKind::OP_BOR },
	{ "^", TokenKind::OP_XOR },
	{ "xor", TokenKind::OP_XOR },
	{ "~", TokenKind::OP_COMPL },
	{ "compl", TokenKind::OP_COMPL },
	{ "&", TokenKind::OP_AMP },
	{ "bitand", TokenKind::OP_AMP },
	{ "!", TokenKind::OP_LNOT },
	{ "not", TokenKind::OP_LNOT },
	{ ";", TokenKind::OP_SEMICOLON },
	{ ":", TokenKind::OP_COLON },
	{ "...", TokenKind::OP_DOTS },
	{ "?", TokenKind::OP_QMARK },
	{ "::", TokenKind::OP_COLON2 },
	{ ".", TokenKind::OP_DOT },
	{ ".*", TokenKind::OP_DOTSTAR },
	{ "+", TokenKind::OP_PLUS },
	{ "-", TokenKind::OP_MINUS },
	{ "*", TokenKind::OP_STAR },
	{ "/", TokenKind::OP_DIV },
	{ "%", TokenKind::OP_MOD },
	{ "=", TokenKind::OP_ASS },
	{ "<", TokenKind::OP_LT },
	{ ">", TokenKind::OP_GT },
	{ "+=", TokenKind::OP_PLUSASS },
	{ "-=", TokenKind::OP_MINUSASS },
	{ "*=", TokenKind::OP_STARASS },
	{ "/=", TokenKind::OP_DIVASS },
	{ "%=", TokenKind::OP_MODASS },
	{ "^=", TokenKind::OP_XORASS },
	{ "xor_eq", TokenKind::OP_XORASS },
	{ "&=", TokenKind::OP_BANDASS },
	{ "and_eq", TokenKind::OP_BANDASS },
	{ "|=", TokenKind::OP_BORASS },
	{ "or_eq", TokenKind::OP_BORASS },
	{ "<<", TokenKind::OP_LSHIFT },
	{ ">>", TokenKind::OP_RSHIFT },
	{ ">>=", TokenKind::OP_RSHIFTASS },
	{ "<<=", TokenKind::OP_LSHIFTASS },
	{ "==", TokenKind::OP_EQ },
	{ "!=", TokenKind::OP_NE },
	{ "not_eq", TokenKind::OP_NE },
	{ "<=", TokenKind::OP_LE },
	{ ">=", TokenKind::OP_GE },
	{ "&&", TokenKind::OP_LAND },
	{ "and", TokenKind::OP_LAND },
	{ "||", TokenKind::OP_LOR },
	{ "or", TokenKind::OP_LOR },
	{ "++", TokenKind::OP_INC },
	{ "--", TokenKind::OP_DEC },
	{ ",", TokenKind::OP_COMMA },
	{ "->*", TokenKind::OP_ARROWSTAR },
	{ "->", TokenKind::OP_ARROW },
};

// "reinterpret_cast" is the longest simple spelling.
const uint32_t kMaxSimpleSpellingLength = 16;

uint32_t HashSpelling(const char* spelling, uint32_t length)
{
	// FNV-1a.
	uint32_t hash = 2166136261u;
	for (uint32_t i = 0; i < length; ++i)
		hash = (hash ^ static_cast<unsigned char>(spelling[i])) * 16777619u;
	return hash;
}

const uint32_t kSimpleSpellingCount = sizeof(kSimpleSpellings) / sizeof(kSimpleSpellings[0]);

// Open-addressed index of kSimpleSpellings: filled once, read-only after.
// Slots hold entry numbers, so the table stays within a few cache lines.
class SimpleSpellingIndex
{
public:
	SimpleSpellingIndex()
	{
		for (Slot& slot : slots_)
			slot.entry = kEmpty;
		for (uint32_t entry = 0; entry < kSimpleSpellingCount; ++entry)
		{
			const char* text = kSimpleSpellings[entry].text;
			const uint32_t length = static_cast<uint32_t>(std::strlen(text));
			uint32_t index = HashSpelling(text, length) & kMask;
			while (slots_[index].entry != kEmpty)
				index = (index + 1) & kMask;
			slots_[index].entry = static_cast<uint8_t>(entry);
			slots_[index].length = static_cast<uint8_t>(length);
		}
	}

	const SimpleSpelling* find(const char* spelling, uint32_t length) const
	{
		for (uint32_t index = HashSpelling(spelling, length) & kMask;; index = (index + 1) & kMask)
		{
			const Slot& slot = slots_[index];
			if (slot.entry == kEmpty)
				return nullptr;
			const SimpleSpelling& entry = kSimpleSpellings[slot.entry];
			if (slot.length == length && std::memcmp(entry.text, spelling, length) == 0)
				return &entry;
		}
	}

private:
	// A power of two, at least twice the entry count.
	static const uint32_t kSlots = 512;
	static const uint32_t kMask = kSlots - 1;
	static const uint8_t kEmpty = 0xFF;

	struct Slot
	{
		uint8_t entry;
		uint8_t length;
	};

	Slot slots_[kSlots];
};

static_assert(sizeof(kTokenKindNames) / sizeof(kTokenKindNames[0]) == static_cast<unsigned>(TokenKind::OP_ARROW) + 1,
	"one name per token kind");
static_assert(
	kSimpleSpellingCount * 2 <= 512 && kSimpleSpellingCount < 0xFF, "SimpleSpellingIndex needs a larger table");

// Each kind's primary and alternative spelling, indexed by kind (built
// once from the spelling table, where a kind's primary spelling comes
// first).
class KindSpellings
{
public:
	KindSpellings()
	{
		for (uint32_t kind = 0; kind < kKindCount; ++kind)
		{
			primary_[kind] = "";
			alternative_[kind] = nullptr;
		}
		for (const SimpleSpelling& entry : kSimpleSpellings)
		{
			const uint32_t kind = static_cast<uint32_t>(entry.kind);
			if (primary_[kind][0] == '\0')
				primary_[kind] = entry.text;
			else if (alternative_[kind] == nullptr)
				alternative_[kind] = entry.text;
		}
	}

	const char* spelling(TokenKind kind, bool alternative) const
	{
		const uint32_t index = static_cast<uint32_t>(kind);
		return alternative && alternative_[index] != nullptr ? alternative_[index] : primary_[index];
	}

private:
	static const uint32_t kKindCount = sizeof(kTokenKindNames) / sizeof(kTokenKindNames[0]);
	const char* primary_[kKindCount];
	const char* alternative_[kKindCount];
};

} // namespace

const char* TokenKindName(TokenKind kind)
{
	return kTokenKindNames[static_cast<unsigned>(kind)];
}

const char* TokenKindSpelling(TokenKind kind, bool alternative)
{
	static const KindSpellings spellings;
	return spellings.spelling(kind, alternative);
}

const char* TokenKindSpelling(TokenKind kind)
{
	return TokenKindSpelling(kind, false);
}

bool FindSimpleToken(const char* spelling, uint32_t length, TokenKind& kind)
{
	static const SimpleSpellingIndex index;
	if (length > kMaxSimpleSpellingLength)
		return false;
	const SimpleSpelling* entry = index.find(spelling, length);
	if (entry == nullptr)
		return false;
	kind = entry->kind;
	return true;
}

bool IsAssignmentOperator(TokenKind kind)
{
	switch (kind)
	{
	case TokenKind::OP_ASS:
	case TokenKind::OP_PLUSASS:
	case TokenKind::OP_MINUSASS:
	case TokenKind::OP_STARASS:
	case TokenKind::OP_DIVASS:
	case TokenKind::OP_MODASS:
	case TokenKind::OP_XORASS:
	case TokenKind::OP_BANDASS:
	case TokenKind::OP_BORASS:
	case TokenKind::OP_LSHIFTASS:
	case TokenKind::OP_RSHIFTASS:
		return true;
	default:
		return false;
	}
}
