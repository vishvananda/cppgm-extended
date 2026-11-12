#pragma once

#include <vector>

#include "native/mir/mir.h"

namespace native {

// O1+, before branch layout: orders the blocks so each one's likeliest
// successor follows it where it can (block_placement.cpp). Returns the
// blocks moved.
uint32_t place_blocks(Function& function);

// Stable cold separation using explicit branch hints and complete incoming
// edges. Keeps addressed entries and the final block in the hot order.
uint32_t place_cold_blocks(Function& function);

// Each block's estimated execution frequency (the entry's 2^20; loop
// headers' times 8, not a cycle entered twice's; branch odds as
// place_blocks weighs them); empty when a block does not end in a branch.
// `placing` takes the estimate place_blocks orders blocks by, with a cycle
// entered twice's header times 8 too, where there is one (else it stays
// empty).
void block_frequencies(Function& function, std::vector<uint64_t>& frequency, std::vector<uint64_t>* placing = nullptr);

// Puts the blocks in `order` (block indexes, the entry first), renaming
// every branch label, jump table entry and landing.
void reorder_blocks(Function& function, const std::vector<uint32_t>& order);

} // namespace native
