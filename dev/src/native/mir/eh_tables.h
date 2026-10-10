#pragma once

#include <cstdint>
#include <vector>

// The exception models of the native backend and the host model's
// per-function tables.
//
// EhModel::Course is the course's handler-stack runtime ABI (eh_push and
// eh_pop records, native/runtime/runtime.h): lowir2native and link mode.
// EhModel::Host is the host's Itanium tables (host objects): a landing pad
// stores the exception object and selector the personality delivers in a
// frame slot pair, each potentially throwing call names its landing
// (Inst::eh), and the emitter records the call-site ranges that the LSDA
// and the unwind table describe (native/object/host_unwind.h).
namespace native {

enum class EhModel : uint8_t
{
	Course,
	Host
};

// A host landing: its pad's MIR block, its selector block's (kNoId when
// the pad stores the delivered filter as the selector), its LSDA action
// chain, a run of EhTables::filters (in matching order: a positive filter
// is a catch type's table index, 0 a cleanup), and its selector
// translation, pairs (filter, selector) in EhTables::map: the selector of
// the first pair with the delivered filter, else 0.
struct EhLanding
{
	uint32_t pad;
	uint32_t select;
	uint32_t first;
	uint32_t count;
	uint32_t map_first;
	uint32_t map_count;
};

// A function's host EH tables: its landings, their chains and
// translations, the type table (index k + 1 -> type-information symbol;
// lowir::kNoId for a catch-all or an unused index), the exception
// specification lists that follow the type table's base (ULEB128 type
// indexes, each list ended by 0; a negative filter -n names the list at
// byte n - 1, Itanium's LSDA as __gxx_personality_v0 reads it) and the
// frame slot of the pads' exception object (the selector at slot + 8, a
// translating pad's raw filter at slot + 16); slot 0 without landings.
struct EhTables
{
	std::vector<EhLanding> landings;
	std::vector<int32_t> filters;
	std::vector<int32_t> map;
	std::vector<uint32_t> types;
	std::vector<uint8_t> specs;
	int32_t slot = 0;

	void clear()
	{
		landings.clear();
		filters.clear();
		map.clear();
		types.clear();
		specs.clear();
		slot = 0;
	}
};

} // namespace native
