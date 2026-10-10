#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

// A flat open-addressed map from object addresses to 32-bit values: the
// identity maps of one phase (an entity's symbol, a variable's slot) over
// records another phase owns. Slots live in one vector that is at most half
// full and doubles as it grows, so an insertion allocates nothing but that
// growth. clear() resets only the slots written since the last clear, so a
// map reused for many small functions costs what they use, not the largest.
class PointerMap
{
public:
	// An enumerator rather than a static member, so that binding it to a
	// reference (vector::assign) needs no out-of-class definition.
	enum : uint32_t
	{
		kMissing = 0xffffffffu
	};

	PointerMap() : count_(0) {}

	uint32_t find(const void* key) const
	{
		if (slots_.empty())
			return kMissing;
		const Slot& slot = slots_[position_of(key)];
		return slot.key == key ? slot.value : kMissing;
	}

	// Sets the value of `key`, inserting it if absent.
	void set(const void* key, uint32_t value)
	{
		if ((count_ + 1) * 2 > slots_.size())
			grow();
		const size_t position = position_of(key);
		Slot& slot = slots_[position];
		if (slot.key == nullptr)
		{
			slot.key = key;
			++count_;
			written_.push_back(static_cast<uint32_t>(position));
		}
		slot.value = value;
	}

	void clear()
	{
		for (uint32_t position : written_)
			slots_[position] = Slot{ nullptr, 0 };
		written_.clear();
		count_ = 0;
	}

	size_t size() const { return count_; }

private:
	struct Slot
	{
		const void* key;
		uint32_t value;
	};

	static size_t hash_of(const void* key)
	{
		uint64_t bits = static_cast<uint64_t>(reinterpret_cast<uintptr_t>(key));
		bits ^= bits >> 33;
		bits *= 0xff51afd7ed558ccdull;
		bits ^= bits >> 33;
		return static_cast<size_t>(bits);
	}

	// The slot holding `key`, or the empty slot where it would go.
	size_t position_of(const void* key) const
	{
		const size_t mask = slots_.size() - 1;
		size_t position = hash_of(key) & mask;
		while (slots_[position].key != nullptr && slots_[position].key != key)
			position = (position + 1) & mask;
		return position;
	}

	void grow()
	{
		std::vector<Slot> old;
		old.swap(slots_);
		slots_.assign(old.empty() ? 64 : old.size() * 2, Slot{ nullptr, 0 });
		written_.clear();
		for (const Slot& slot : old)
		{
			if (slot.key == nullptr)
				continue;
			const size_t position = position_of(slot.key);
			slots_[position] = slot;
			written_.push_back(static_cast<uint32_t>(position));
		}
	}

	std::vector<Slot> slots_;
	std::vector<uint32_t> written_;
	size_t count_;
};
