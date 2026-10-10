#pragma once

#include <cstdint>
#include <vector>

#include "lowir/program.h"

// The bytes of a LowIR global definition, shared by the executable image
// and the object compiler: typed items at their natural alignment (capped
// at 16), `zero` items as raw bytes, address items as fixups, repeat items
// as copies of the bytes before them.
namespace native {

// An address item: `target` plus `addend`, stored as 8 bytes at `offset`.
struct GlobalAddress
{
	uint64_t offset;
	lowir::SymbolId target;
	int64_t addend;
};

// The global's size in bytes (at least 1) and alignment.
uint64_t global_size(const lowir::Program& program, const lowir::Global& global, uint32_t& align);

// Writes the global's constants at `base` in `out`, which already holds
// zero bytes there, and appends its address items (offsets from `base`).
void write_global(const lowir::Program& program, const lowir::Global& global, std::vector<uint8_t>& out, uint64_t base,
	std::vector<GlobalAddress>& addresses);

// Whether every byte of the global is zero (zero-filled storage suffices).
bool global_is_zero(const lowir::Program& program, const lowir::Global& global);

// Whether the global holds an address (an item a loader relocates).
bool global_has_addresses(const lowir::Program& program, const lowir::Global& global);

} // namespace native
