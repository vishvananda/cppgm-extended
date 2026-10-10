#pragma once

#include <cstdint>
#include <vector>

#include "support/memory/arena.h"

namespace lowir {

// A dense name identity; 0 is "no name".
typedef uint32_t NameId;

// Interns the spellings of one LowIR program: symbol, temporary, slot and
// block names (without their sigils), object spellings, sections and debug
// file tokens. Each distinct spelling is stored once in the arena, null
// terminated, and gets the next dense id, so names compare by id and
// per-name tables are plain arrays. The index is open addressed, at most
// half full, and doubles as it grows.
class NameTable
{
public:
	explicit NameTable(Arena& arena);

	NameId intern(const char* text, uint32_t length);
	// The id of an interned spelling, or 0.
	NameId find(const char* text, uint32_t length) const;

	const char* spelling(NameId name) const { return entries_[name].spelling; }
	uint32_t length(NameId name) const { return entries_[name].length; }

	// Ids are below size(); id 0 is the empty "no name" entry.
	uint32_t size() const { return static_cast<uint32_t>(entries_.size()); }

private:
	struct Entry
	{
		const char* spelling;
		uint32_t length;
		uint32_t hash;
	};

	static uint32_t hash_of(const char* text, uint32_t length);
	// The index position holding the spelling, or the empty position where
	// it would go.
	uint32_t position_of(const char* text, uint32_t length, uint32_t hash) const;
	void grow();

	Arena& arena_;
	std::vector<Entry> entries_;
	// Name ids by hash position; 0 marks an empty position.
	std::vector<NameId> index_;
};

} // namespace lowir
