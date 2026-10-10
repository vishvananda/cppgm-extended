#pragma once

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <new>
#include <type_traits>

// A bump allocator for objects that live as long as their owner, such as a
// translation unit's interned identifiers, macro definitions and produced
// token spellings. Memory is taken from geometrically growing blocks and
// released in bulk when the arena is destroyed; objects are never destroyed
// individually, so only trivially destructible types are allocated.
class Arena
{
public:
	Arena() : head_(nullptr), next_(nullptr), limit_(nullptr), bytes_reserved_(0) {}
	~Arena();

	Arena(const Arena&) = delete;
	Arena& operator=(const Arena&) = delete;

	void* allocate(std::size_t size, std::size_t alignment)
	{
		uintptr_t start = (reinterpret_cast<uintptr_t>(next_) + alignment - 1) & ~(alignment - 1);
		if (next_ == nullptr || start + size > reinterpret_cast<uintptr_t>(limit_))
			return allocate_slow(size, alignment);
		next_ = reinterpret_cast<char*>(start + size);
		return reinterpret_cast<void*>(start);
	}

	// Default-initialized storage for `count` objects of a trivially
	// destructible type.
	template<typename T>
	T* allocate_array(std::size_t count)
	{
		static_assert(std::is_trivially_destructible<T>::value, "arena objects are not destroyed");
		void* storage = allocate(sizeof(T) * (count == 0 ? 1 : count), alignof(T));
		return new (storage) T[count == 0 ? 1 : count];
	}

	template<typename T>
	T* copy_array(const T* source, std::size_t count)
	{
		static_assert(std::is_trivially_copyable<T>::value, "arena copies are bitwise");
		T* target = static_cast<T*>(allocate(sizeof(T) * (count == 0 ? 1 : count), alignof(T)));
		if (count != 0)
			std::memcpy(target, source, sizeof(T) * count);
		return target;
	}

	// A copy of `length` bytes followed by a null byte.
	const char* copy_string(const char* text, std::size_t length)
	{
		char* target = static_cast<char*>(allocate(length + 1, 1));
		std::memcpy(target, text, length);
		target[length] = '\0';
		return target;
	}

	// Bytes obtained from the system so far (telemetry).
	std::size_t bytes_reserved() const { return bytes_reserved_; }

private:
	struct Block
	{
		Block* next;
		std::size_t size;
	};

public:
	// A position in the arena. rewind() releases everything allocated
	// after mark() in bulk, so a parser checkpoint drops the nodes of an
	// abandoned alternative. Nothing allocated after the mark may be
	// referenced once the arena is rewound.
	struct Mark
	{
		Block* head;
		char* next;
		char* limit;
	};

	Mark mark() const { return Mark{ head_, next_, limit_ }; }
	void rewind(const Mark& mark);

private:
	void* allocate_slow(std::size_t size, std::size_t alignment);

	Block* head_;
	char* next_;
	char* limit_;
	std::size_t bytes_reserved_;
};
