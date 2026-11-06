#pragma once

#include <vector>

#include "sema/constant_values.h"
#include "support/memory/arena.h"
#include "support/pointer_map.h"

namespace sema {

// Allocated only when an evaluation actually needs shared class/part
// templates or storage that a callee writes into a caller's object.
struct ConstantStorage
{
	Arena immutable_arena;
	Arena mutable_arena;
	PointerMap classes[2];
	std::vector<ConstValue> values;
	PointerMap complete;
	struct Allocation
	{
		uint32_t object;
		uint32_t generation;
		Arena::Mark mark;
	};
	std::vector<Allocation> allocations;
};

} // namespace sema
