#include "lowir/name_table.h"

#include <cstring>

namespace lowir {

namespace {

const uint32_t kInitialIndexSize = 1024;

} // namespace

NameTable::NameTable(Arena& arena) : arena_(arena), index_(kInitialIndexSize, 0)
{
	entries_.reserve(kInitialIndexSize / 2);
	entries_.push_back(Entry{ arena_.copy_string("", 0), 0, hash_of("", 0) });
}

uint32_t NameTable::hash_of(const char* text, uint32_t length)
{
	// FNV-1a.
	uint32_t hash = 2166136261u;
	for (uint32_t i = 0; i < length; ++i)
	{
		hash ^= static_cast<unsigned char>(text[i]);
		hash *= 16777619u;
	}
	return hash;
}

uint32_t NameTable::position_of(const char* text, uint32_t length, uint32_t hash) const
{
	const uint32_t mask = static_cast<uint32_t>(index_.size()) - 1;
	for (uint32_t position = hash & mask;; position = (position + 1) & mask)
	{
		const NameId id = index_[position];
		if (id == 0)
			return position;
		const Entry& entry = entries_[id];
		if (entry.hash == hash && entry.length == length && std::memcmp(entry.spelling, text, length) == 0)
			return position;
	}
}

NameId NameTable::find(const char* text, uint32_t length) const
{
	return length == 0 ? 0 : index_[position_of(text, length, hash_of(text, length))];
}

NameId NameTable::intern(const char* text, uint32_t length)
{
	if (length == 0)
		return 0;
	const uint32_t hash = hash_of(text, length);
	const uint32_t position = position_of(text, length, hash);
	if (index_[position] != 0)
		return index_[position];
	const NameId created = static_cast<NameId>(entries_.size());
	entries_.push_back(Entry{ arena_.copy_string(text, length), length, hash });
	index_[position] = created;
	if (entries_.size() * 2 > index_.size())
		grow();
	return created;
}

void NameTable::grow()
{
	std::vector<NameId> index(index_.size() * 2, 0);
	const uint32_t mask = static_cast<uint32_t>(index.size()) - 1;
	for (NameId id = 1; id < entries_.size(); ++id)
	{
		uint32_t position = entries_[id].hash & mask;
		while (index[position] != 0)
			position = (position + 1) & mask;
		index[position] = id;
	}
	index_.swap(index);
}

} // namespace lowir
