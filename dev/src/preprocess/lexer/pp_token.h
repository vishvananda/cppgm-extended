#pragma once

#include <cstdint>

#include "lex/token_kind.h"

// The preprocessing-token categories of [lex.pptoken] 2.5, plus the
// synthesized whitespace-sequence and new-line tokens and end-of-file.
// Placemarker is internal to macro replacement ([cpp.concat] 16.3.3p2) and
// never leaves it.
enum class PPTokenKind : uint8_t
{
	WhitespaceSequence,
	NewLine,
	HeaderName,
	Identifier,
	PPNumber,
	CharacterLiteral,
	UserDefinedCharacterLiteral,
	StringLiteral,
	UserDefinedStringLiteral,
	PreprocessingOpOrPunc,
	NonWhitespaceCharacter,
	EndOfFile,
	Placemarker
};

// PPToken::flags. The lexer leaves them clear and reports white space as
// separate tokens; the preprocessor folds that white space into flags.
enum PPTokenFlag : uint8_t
{
	// White space (a new-line included) precedes the token.
	kLeadingSpace = 1,
	// The token is the first on its source line.
	kStartOfLine = 2,
	// An identifier no longer available for macro replacement
	// ([cpp.rescan] 16.3.4p2).
	kPainted = 4,
	// A token substituted for a macro parameter (not pasted): an invocation
	// it names is not nested in the expansion it was substituted into, and
	// the names disabled for that expansion's own tokens do not apply to it
	// (preprocess/macro/macro_expander.h).
	kArgument = 8,
	// Substituted as a macro-replaced argument: the last name added to its
	// hide set is the macro it was substituted into.
	kReplacedArgument = 16,
	// A collected argument token read from the same expansion as the token
	// before it: whether an invocation in the argument began in the
	// expansion its name came from.
	kFollowsInContext = 32,
	// A collected argument token read from an argument being replaced: the
	// names disabled there stay disabled for it when the argument it was
	// collected into is replaced.
	kFromArgument = 64
};

// PPToken::punctuator of a preprocessing-op-or-punc: the phase-7 TokenKind
// plus one (alternative spellings share it), or `#`/`%:` and `##`/`%:%:`,
// which have no phase-7 token. Zero for every other token.
const uint8_t kNoPunctuator = 0;
const uint8_t kHashPunctuator = 0xFE;
const uint8_t kHashHashPunctuator = 0xFF;

inline uint8_t PunctuatorOf(TokenKind kind)
{
	return static_cast<uint8_t>(static_cast<uint8_t>(kind) + 1);
}

inline bool IsPunctuator(uint8_t punctuator, TokenKind kind)
{
	return punctuator == PunctuatorOf(kind);
}

struct Identifier;

// One preprocessing token.
//
// `begin`/`end` delimit the physical source bytes in the location space of
// the producer: buffer offsets for a lexer on its own, and translation-unit
// locations (a file inclusion's base plus the offset) in the preprocessor,
// where a token produced by macro replacement carries the location of the
// invocation that produced it. The spelling is the token's UTF-8 text after
// translation phases 1-3 (trigraphs replaced, splices removed,
// universal-character-names decoded); it is empty for whitespace-sequence,
// new-line and end-of-file. The lexer's spelling is borrowed: it points into
// the source buffer when the token needed no translation and into lexer
// scratch storage otherwise, and is valid until the next token. The
// preprocessor's spellings stay valid for its translation unit.
struct PPToken
{
	PPTokenKind kind;
	uint8_t flags;
	uint8_t punctuator;
	uint32_t begin;
	uint32_t end;
	uint32_t length;
	const char* spelling;
	// The interned identifier of an Identifier token from the preprocessor;
	// null from a bare lexer.
	Identifier* identifier;
	// The preprocessor's hide set: macro names this token may not invoke
	// (see preprocess/macro/hide_sets.h). Zero is the empty set.
	uint32_t hide_set;
	// The preprocessor's expansion context that produced a replacement-list
	// or pasted token; zero for other tokens.
	uint32_t origin;
};
