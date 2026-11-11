#include "lex/identifier_table.h"

#include <cstring>

#include "support/tool_errors.h"

namespace {

const uint32_t kInitialSlots = 1024;

// A multiplicative hash of the spelling, eight bytes at a time; the last
// step takes the bytes left (the first and last four, which overlap below
// eight, or the first, middle and last of fewer than four).
uint32_t HashSpelling(const char* spelling, uint32_t length)
{
	const uint64_t kMultiplier = 0x9E3779B97F4A7C15ull;
	uint64_t hash = (length + 1) * kMultiplier;
	for (; length > 8; spelling += 8, length -= 8)
	{
		uint64_t word;
		std::memcpy(&word, spelling, 8);
		hash = (hash ^ word) * kMultiplier;
		hash ^= hash >> 32;
	}
	uint64_t rest = 0;
	if (length >= 4)
	{
		uint32_t low, high;
		std::memcpy(&low, spelling, 4);
		std::memcpy(&high, spelling + length - 4, 4);
		rest = (static_cast<uint64_t>(high) << 32) | low;
	}
	else if (length != 0)
	{
		rest = static_cast<unsigned char>(spelling[0]) |
			(static_cast<uint64_t>(static_cast<unsigned char>(spelling[length >> 1])) << 8) |
			(static_cast<uint64_t>(static_cast<unsigned char>(spelling[length - 1])) << 16);
	}
	hash = (hash ^ rest) * kMultiplier;
	return static_cast<uint32_t>(hash >> 32);
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
	identifier->macro_open_serial = 0;
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
