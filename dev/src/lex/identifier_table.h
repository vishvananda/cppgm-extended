#pragma once

#include <cstdint>
#include <vector>

#include "lex/token_kind.h"
#include "support/memory/arena.h"

struct MacroDefinition;

// An interned identifier. There is one record per distinct spelling in a
// translation unit, so identifiers compare by address. The spelling is
// null-terminated and lives as long as the table's arena.
struct Identifier
{
	const char* spelling;
	uint32_t length;
	uint32_t hash;
	// Preprocessor state of the translation unit: the current macro
	// definition, the identifier's preprocessor role (a directive name or an
	// operator such as `defined`, preprocess/preprocessor_names.h), and the
	// parameter being parsed in a #define (1 + index, 0 if none).
	MacroDefinition* macro;
	uint16_t parameter_slot;
	uint8_t role;
	// Phase 7: the keyword this identifier spells (TokenKind + 1), or 0.
	uint8_t keyword;
	// Dense, in order of interning, below 2^31 (for tables keyed by
	// identifier).
	uint32_t id;

	bool simple_token(TokenKind& kind) const
	{
		if (keyword == 0)
			return false;
		kind = static_cast<TokenKind>(keyword - 1);
		return true;
	}
};

// Interns identifier spellings: an open-addressed table of record pointers,
// kept at most half full and doubled as it grows. Records and spellings are
// allocated in the arena; each is created once, when its spelling is first
// seen, and its keyword kind is looked up then.
class IdentifierTable
{
public:
	explicit IdentifierTable(Arena& arena);

	Identifier* intern(const char* spelling, uint32_t length);

	uint32_t size() const { return count_; }

private:
	void grow();

	Arena& arena_;
	std::vector<Identifier*> slots_;
	uint32_t count_;
};
