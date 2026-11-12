#pragma once

#include <cstdint>
#include <vector>

namespace machine {

// Typed landing/action facts. The emitter assigns pad/select machine-block
// identities; first/count identify action filters and map_first/map_count
// identify (filter, LowIR selector) pairs. No register or frame location is
// part of this shared representation.
struct ExceptionLanding
{
	uint32_t pad;
	uint32_t select;
	uint32_t first;
	uint32_t count;
	uint32_t map_first;
	uint32_t map_count;
};

// Positive filters index the type-information symbols, zero denotes cleanup,
// and negative filters identify exception-specification lists in specs.
struct ExceptionTables
{
	std::vector<ExceptionLanding> landings;
	std::vector<int32_t> filters;
	std::vector<int32_t> map;
	std::vector<uint32_t> types;
	std::vector<uint8_t> specs;

	void clear()
	{
		landings.clear();
		filters.clear();
		map.clear();
		types.clear();
		specs.clear();
	}
};

} // namespace machine
