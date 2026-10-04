#pragma once

#include "support/containers/flat_hash_map.h"

namespace cppgm
{
namespace detail
{

// Query-local identities: small walks stay inline and larger walks allocate
// geometrically, without storage proportional to the whole program.
template <typename Key, typename Hash = std::hash<Key> >
class FlatHashSet
{
public:
	FlatHashSet() : size_(0), inline_slots_() {}

	bool Insert(const Key& key)
	{
		if ((size_ + 1) * 10 >= Capacity() * 7) Grow();
		if (!InsertInto(Slots(), Capacity(), key)) return false;
		++size_;
		return true;
	}

private:
	enum { INLINE_CAPACITY = 16 };

	struct Slot
	{
		Key key;
		bool occupied;

		Slot() : key(), occupied(false) {}
	};

	Slot* Slots()
	{
		return slots_.empty() ? inline_slots_ : slots_.data();
	}

	std::size_t Capacity() const
	{
		return slots_.empty() ? INLINE_CAPACITY : slots_.size();
	}

	bool InsertInto(Slot* slots, std::size_t capacity, const Key& key)
	{
		std::size_t position = MixedHash(Hash()(key)) & (capacity - 1);
		while (slots[position].occupied)
		{
			if (slots[position].key == key) return false;
			position = (position + 1) & (capacity - 1);
		}
		slots[position].key = key;
		slots[position].occupied = true;
		return true;
	}

	void Grow()
	{
		const std::size_t old_capacity = Capacity();
		std::vector<Slot> grown(old_capacity * 2);
		Slot* old = Slots();
		for (std::size_t i = 0; i < old_capacity; ++i)
			if (old[i].occupied)
				InsertInto(grown.data(), grown.size(), old[i].key);
		slots_.swap(grown);
	}

	std::size_t size_;
	Slot inline_slots_[INLINE_CAPACITY];
	std::vector<Slot> slots_;
};

}
}
