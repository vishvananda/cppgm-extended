// Token kinds adapted from the CPPGM PA2 starter code
// (C) 2013 CPPGM Foundation www.cppgm.org. See NOTICE.

#pragma once

#include <cstdint>

// The keywords ([lex.key] 2.12) and operators and punctuators
// ([lex.operators] 2.13) that phase 7 converts to `simple` tokens. `true`,
// `false`, `nullptr`, `new` and `delete` are keywords here. Alternative
// spellings share a kind (`<%` and `{` are OP_LBRACE, `and` is OP_LAND).
// clang-format off: clang-format reflows this list differently on every run.
#define CPPGM_TOKEN_KINDS(X) \
	X(KW_ALIGNAS) \
	X(KW_ALIGNOF) \
	X(KW_ASM) \
	X(KW_AUTO) \
	X(KW_BOOL) \
	X(KW_BREAK) \
	X(KW_CASE) \
	X(KW_CATCH) \
	X(KW_CHAR) \
	X(KW_CHAR16_T) \
	X(KW_CHAR32_T) \
	X(KW_CLASS) \
	X(KW_CONST) \
	X(KW_CONSTEXPR) \
	X(KW_CONST_CAST) \
	X(KW_CONTINUE) \
	X(KW_DECLTYPE) \
	X(KW_DEFAULT) \
	X(KW_DELETE) \
	X(KW_DO) \
	X(KW_DOUBLE) \
	X(KW_DYNAMIC_CAST) \
	X(KW_ELSE) \
	X(KW_ENUM) \
	X(KW_EXPLICIT) \
	X(KW_EXPORT) \
	X(KW_EXTERN) \
	X(KW_FALSE) \
	X(KW_FLOAT) \
	X(KW_FOR) \
	X(KW_FRIEND) \
	X(KW_GOTO) \
	X(KW_IF) \
	X(KW_INLINE) \
	X(KW_INT) \
	X(KW_LONG) \
	X(KW_MUTABLE) \
	X(KW_NAMESPACE) \
	X(KW_NEW) \
	X(KW_NOEXCEPT) \
	X(KW_NULLPTR) \
	X(KW_OPERATOR) \
	X(KW_PRIVATE) \
	X(KW_PROTECTED) \
	X(KW_PUBLIC) \
	X(KW_REGISTER) \
	X(KW_REINTERPET_CAST) \
	X(KW_RETURN) \
	X(KW_SHORT) \
	X(KW_SIGNED) \
	X(KW_SIZEOF) \
	X(KW_STATIC) \
	X(KW_STATIC_ASSERT) \
	X(KW_STATIC_CAST) \
	X(KW_STRUCT) \
	X(KW_SWITCH) \
	X(KW_TEMPLATE) \
	X(KW_THIS) \
	X(KW_THREAD_LOCAL) \
	X(KW_THROW) \
	X(KW_TRUE) \
	X(KW_TRY) \
	X(KW_TYPEDEF) \
	X(KW_TYPEID) \
	X(KW_TYPENAME) \
	X(KW_UNION) \
	X(KW_UNSIGNED) \
	X(KW_USING) \
	X(KW_VIRTUAL) \
	X(KW_VOID) \
	X(KW_VOLATILE) \
	X(KW_WCHAR_T) \
	X(KW_WHILE) \
	X(OP_LBRACE) \
	X(OP_RBRACE) \
	X(OP_LSQUARE) \
	X(OP_RSQUARE) \
	X(OP_LPAREN) \
	X(OP_RPAREN) \
	X(OP_BOR) \
	X(OP_XOR) \
	X(OP_COMPL) \
	X(OP_AMP) \
	X(OP_LNOT) \
	X(OP_SEMICOLON) \
	X(OP_COLON) \
	X(OP_DOTS) \
	X(OP_QMARK) \
	X(OP_COLON2) \
	X(OP_DOT) \
	X(OP_DOTSTAR) \
	X(OP_PLUS) \
	X(OP_MINUS) \
	X(OP_STAR) \
	X(OP_DIV) \
	X(OP_MOD) \
	X(OP_ASS) \
	X(OP_LT) \
	X(OP_GT) \
	X(OP_PLUSASS) \
	X(OP_MINUSASS) \
	X(OP_STARASS) \
	X(OP_DIVASS) \
	X(OP_MODASS) \
	X(OP_XORASS) \
	X(OP_BANDASS) \
	X(OP_BORASS) \
	X(OP_LSHIFT) \
	X(OP_RSHIFT) \
	X(OP_RSHIFTASS) \
	X(OP_LSHIFTASS) \
	X(OP_EQ) \
	X(OP_NE) \
	X(OP_LE) \
	X(OP_GE) \
	X(OP_LAND) \
	X(OP_LOR) \
	X(OP_INC) \
	X(OP_DEC) \
	X(OP_COMMA) \
	X(OP_ARROWSTAR) \
	X(OP_ARROW)
// clang-format on

enum class TokenKind : uint8_t
{
#define CPPGM_TOKEN_KIND_ENUMERATOR(name) name,
	CPPGM_TOKEN_KINDS(CPPGM_TOKEN_KIND_ENUMERATOR)
#undef CPPGM_TOKEN_KIND_ENUMERATOR
};

// The course's term name, e.g. "KW_AUTO" or "OP_LAND".
const char* TokenKindName(TokenKind kind);

// The primary spelling, e.g. "auto" or "&&" (not an alternative token).
const char* TokenKindSpelling(TokenKind kind);

// The spelling with or without the alternative token (2.6): "and" for
// OP_LAND when `alternative`.
const char* TokenKindSpelling(TokenKind kind, bool alternative);

// An operator spelled by an alternative token such as `and` or `bitor` (a
// word where the primary spelling is punctuation; digraphs only spell
// brackets and `#`).
inline bool IsAlternativeSpelling(const char* spelling, uint32_t length)
{
	return length != 0 && spelling[0] >= 'a' && spelling[0] <= 'z';
}

// Finds the simple token spelled by an identifier or preprocessing-op-or-punc.
// `#`, `##`, `%:` and `%:%:` have no simple token.
bool FindSimpleToken(const char* spelling, uint32_t length, TokenKind& kind);

// `=` and the compound assignment operators (5.17).
bool IsAssignmentOperator(TokenKind kind);
