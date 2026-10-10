#include "lex/identifier_table.h"

#include <cstring>

#include "support/tool_errors.h"

namespace {

const uint32_t kInitialSlots = 1024;

// FNV-1a over the spelling's bytes.
uint32_t HashSpelling(const char* spelling, uint32_t length)
{
	uint32_t hash = 2166136261u;
	for (uint32_t i = 0; i < length; ++i)
	{
		hash ^= static_cast<unsigned char>(spelling[i]);
		hash *= 16777619u;
	}
	return hash;
}

} // namespace

IdentifierTable::IdentifierTable(Arena& arena) : arena_(arena), slots_(kInitialSlots, nullptr), count_(0)
{}

Identifier* IdentifierTable::intern(const char* spelling, uint32_t length)
{
	const uint32_t hash = HashSpelling(spelling, length);
	const uint32_t mask = static_cast<uint32_t>(slots_.size()) - 1;
	for (uint32_t index = hash & mask;; index = (index + 1) & mask)
	{
		Identifier* found = slots_[index];
		if (found == nullptr)
			break;
		if (found->hash == hash && found->length == length && std::memcmp(found->spelling, spelling, length) == 0)
			return found;
	}

	// Ids stay below 2^31 (a tag bit is free in tables of ids).
	if (count_ == 0x7FFFFFFFu)
		throw LimitError("too many identifiers in one translation unit");
	Identifier* identifier = arena_.allocate_array<Identifier>(1);
	identifier->spelling = arena_.copy_string(spelling, length);
	identifier->length = length;
	identifier->hash = hash;
	identifier->id = count_;
	identifier->macro = nullptr;
	identifier->parameter_slot = 0;
	identifier->role = 0;
	TokenKind kind;
	identifier->keyword = FindSimpleToken(spelling, length, kind) ? static_cast<uint8_t>(static_cast<uint8_t>(kind) + 1)
																  : 0;

	if (2 * (count_ + 1) > slots_.size())
		grow();
	const uint32_t new_mask = static_cast<uint32_t>(slots_.size()) - 1;
	uint32_t index = hash & new_mask;
	while (slots_[index] != nullptr)
		index = (index + 1) & new_mask;
	slots_[index] = identifier;
	++count_;
	return identifier;
}

void IdentifierTable::grow()
{
	std::vector<Identifier*> old(slots_.size() * 2, nullptr);
	old.swap(slots_);
	const uint32_t mask = static_cast<uint32_t>(slots_.size()) - 1;
	for (Identifier* identifier : old)
	{
		if (identifier == nullptr)
			continue;
		uint32_t index = identifier->hash & mask;
		while (slots_[index] != nullptr)
			index = (index + 1) & mask;
		slots_[index] = identifier;
	}
}
