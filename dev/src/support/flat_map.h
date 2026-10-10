#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <type_traits>
#include <utility>
#include <vector>

// An open-addressed map from a small trivially copyable key (compact
// identities: interned pointers, counts) to a value, its slots in one
// array at most half full that doubles as it grows: an insertion
// allocates nothing but that growth. Each slot keeps its key's hash, so a
// probe compares keys only on a hash match. Entries are never removed one
// by one (clear() drops them all).
template<typename Key, typename Value, typename Hash>
class FlatMap
{
	static_assert(std::is_trivially_copyable<Key>::value && std::is_trivially_copyable<Value>::value,
		"flat map slots copy bitwise");

public:
	FlatMap() : capacity_(0), count_(0) {}

	// The value of `key`, or null when absent.
	Value* find(const Key& key)
	{
		if (capacity_ == 0)
			return nullptr;
		const size_t position = position_of(key, hash_of(key));
		return full_[position] ? &slots_[position].value : nullptr;
	}

	// Inserts `key`, which is absent.
	void insert(const Key& key, const Value& value)
	{
		if ((count_ + 1) * 2 > capacity_)
			grow();
		const uint32_t hash = hash_of(key);
		const size_t position = position_of(key, hash);
		slots_[position] = Slot{ key, value, hash };
		full_[position] = 1;
		++count_;
	}

	size_t size() const { return count_; }

	void swap(FlatMap& other)
	{
		slots_.swap(other.slots_);
		full_.swap(other.full_);
		std::swap(capacity_, other.capacity_);
		std::swap(count_, other.count_);
	}

	// Removes every entry, keeping the slots for the next ones (an owner
	// whose entries die together).
	void clear()
	{
		if (count_ == 0)
			return;
		full_.assign(capacity_, 0);
		count_ = 0;
	}

	// clear() for an owner reused across inputs of different sizes: slots
	// far beyond what `expected` entries need are released rather than
	// cleared, so a large input does not make every later clear() cost its
	// size.
	void clear(size_t expected)
	{
		if (capacity_ > 64 && capacity_ > 8 * expected)
		{
			slots_.reset();
			std::vector<uint8_t>().swap(full_);
			capacity_ = 0;
			count_ = 0;
			return;
		}
		clear();
	}

private:
	// A slot's contents mean something only where full_ is set (slots are
	// never initialized as a whole: clearing resets the flags alone).
	struct Slot
	{
		Key key;
		Value value;
		uint32_t hash;
	};

	static uint32_t hash_of(const Key& key)
	{
		uint64_t bits = static_cast<uint64_t>(Hash()(key));
		bits ^= bits >> 33;
		bits *= 0xff51afd7ed558ccdull;
		bits ^= bits >> 33;
		return static_cast<uint32_t>(bits);
	}

	// The slot holding `key`, or the empty slot where it would go.
	size_t position_of(const Key& key, uint32_t hash) const
	{
		const size_t mask = capacity_ - 1;
		size_t position = hash & mask;
		while (full_[position] && !(slots_[position].hash == hash && slots_[position].key == key))
			position = (position + 1) & mask;
		return position;
	}

	void grow()
	{
		std::unique_ptr<Slot[]> old(slots_.release());
		std::vector<uint8_t> old_full;
		old_full.swap(full_);
		const size_t old_capacity = capacity_;
		capacity_ = old_capacity == 0 ? 64 : old_capacity * 2;
		slots_.reset(new Slot[capacity_]);
		full_.assign(capacity_, 0);
		const size_t mask = capacity_ - 1;
		for (size_t k = 0; k < old_capacity; ++k)
		{
			if (!old_full[k])
				continue;
			size_t position = old[k].hash & mask;
			while (full_[position])
				position = (position + 1) & mask;
			slots_[position] = old[k];
			full_[position] = 1;
		}
	}

	std::unique_ptr<Slot[]> slots_;
	std::vector<uint8_t> full_; // per slot: holds an entry
	size_t capacity_;
	size_t count_;
};
