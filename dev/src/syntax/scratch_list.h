#pragma once

#include <cstdint>
#include <vector>

#include "support/memory/arena.h"
#include "syntax/ast_names.h"

namespace syntax {

// A reusable stack of list elements under construction. Nested lists of
// the same element type push above each other; ListFrame copies a
// finished list into the arena and truncates, and truncates on any other
// exit, so failed alternatives leave nothing behind.
template<typename T>
class ScratchStack
{
public:
	std::vector<T> items;
};

template<typename T>
class ListFrame
{
public:
	explicit ListFrame(ScratchStack<T>& stack) : stack_(stack), start_(stack.items.size()) {}
	~ListFrame() { stack_.items.resize(start_); }
	ListFrame(const ListFrame&) = delete;
	ListFrame& operator=(const ListFrame&) = delete;

	void push(const T& item) { stack_.items.push_back(item); }
	uint32_t size() const { return static_cast<uint32_t>(stack_.items.size() - start_); }
	T& back() { return stack_.items.back(); }

	Slice<T> take(Arena& arena)
	{
		const uint32_t count = size();
		if (count == 0)
			return Slice<T>{ nullptr, 0 };
		Slice<T> slice{ arena.copy_array(stack_.items.data() + start_, count), count };
		stack_.items.resize(start_);
		return slice;
	}

private:
	ScratchStack<T>& stack_;
	size_t start_;
};

} // namespace syntax
