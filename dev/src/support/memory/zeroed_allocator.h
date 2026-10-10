#pragma once

#include <cstddef>
#include <cstdlib>
#include <new>
#include <utility>

// An allocator for the slot vectors of open-addressed hash tables, whose
// empty slots are all-zero bytes: storage comes from calloc (fresh pages
// need no clearing) and a value-initialized element is left as those zero
// bytes. Only for trivial element types whose value-initialization is all
// zeros, in vectors that are sized when constructed (never resized within
// their capacity) and then only indexed or swapped.
template<typename T>
struct ZeroedAllocator
{
	typedef T value_type;

	ZeroedAllocator() {}
	template<typename U>
	ZeroedAllocator(const ZeroedAllocator<U>&)
	{}

	T* allocate(std::size_t count)
	{
		void* storage = std::calloc(count == 0 ? 1 : count, sizeof(T));
		if (storage == nullptr)
			throw std::bad_alloc();
		return static_cast<T*>(storage);
	}
	void deallocate(T* storage, std::size_t) { std::free(storage); }

	// Value-initialization: calloc's zeros already are the value.
	template<typename U>
	void construct(U*)
	{}
	template<typename U, typename First, typename... Rest>
	void construct(U* place, First&& first, Rest&&... rest)
	{
		::new (static_cast<void*>(place)) U(std::forward<First>(first), std::forward<Rest>(rest)...);
	}

	template<typename U>
	struct rebind
	{
		typedef ZeroedAllocator<U> other;
	};
};

template<typename T, typename U>
bool operator==(const ZeroedAllocator<T>&, const ZeroedAllocator<U>&)
{
	return true;
}

template<typename T, typename U>
bool operator!=(const ZeroedAllocator<T>&, const ZeroedAllocator<U>&)
{
	return false;
}
