#pragma once

#include <cstdint>
#include <utility>
#include <vector>

#include "native/select/selector_facts.h"

// O1+ function-level registers: spilling a value around the loops it does
// not use. A candidate no register fits for its whole life
// (register_allocation.cpp) may still take one outside the outermost loops
// it is live through without a use or its definition: inside them it
// stays in its frame home, stored on each edge entering such a loop and
// reloaded on each edge leaving one into a block it is live into. The
// loops are the natural loops loop_depths records (complete walks), less
// those holding the entry block or a block on a host EH edge, so each is
// entered only at its header. A reload writes the register at its
// predecessor's edge transfer: a range on that block's transfer write
// slot, as a phi's. The split stands when the transfers' loop weight is
// below the value's.
//
// Splitting around blocks (plan_homes): a candidate no register fits may
// also keep its frame home only in the blocks where its register is
// clobbered (a call), taken or over pressure, and the register elsewhere:
// reloaded on each edge out of those blocks into a block it is live
// into, and stored on each edge into them (none when one of them defines
// it: its definition writes the home, which never changes once written).
// Inside them every read is the home's, as a homed value's. Host EH edges carry no transfers: a call
// block and a landing it enters are homed together. A phi keeps its
// register in its own block and its predecessors, whose transfers write it.
namespace native {

struct SelectorState;

class LoopSplitter
{
public:
	// Per function, after loop_depths filled the loops: which may hold a
	// split value and their order (largest first).
	void reset(const SelectorState& s);
	// The outermost loops candidate c is live into without a use or its
	// definition; false when none.
	bool candidates(const SelectorState& s, uint32_t c);
	// Whether block b lies in one of them (after candidates).
	bool inside(uint32_t b) const { return inside_at_[b] == epoch_; }
	// Candidate c's split around those of them holding a block of
	// `conflicts` (all of them when null): its ranges (by block) outside
	// the loops plus the reloads', its transfers and their loop weight;
	// false when the transfers cost the value's weight.
	bool plan(const SelectorState& s, uint32_t c, const std::vector<uint32_t>* conflicts);
	// Candidate c homed in the blocks `conflicts` (repeats allowed) and
	// the host EH blocks they imply: its ranges outside them, its
	// transfers and their cost with the home writes and reads inside
	// (homed: the blocks, in no particular order); false when that weighs
	// the value's weight or `bound`.
	bool plan_homes(const SelectorState& s, uint32_t c, const std::vector<uint32_t>& conflicts,
		uint64_t bound = ~static_cast<uint64_t>(0));
	uint64_t cost = 0;

	// The natural loops (CSR, header first), as loop_depths records them,
	// and each block's estimated executions (kWeightScale per entry, none
	// when cold), as block_weights does.
	std::vector<uint32_t> loop_first;
	std::vector<uint32_t> loop_blocks;
	std::vector<uint64_t> weights;
	// plan's result.
	std::vector<std::pair<int32_t, int32_t>> ranges;
	std::vector<uint32_t> blocks;
	std::vector<SplitMove> moves;
	std::vector<uint32_t> homed;

private:
	std::vector<uint32_t> order_;          // qualifying loops, largest first
	std::vector<uint32_t> range_at_;       // per block: epoch when the value is live into it
	std::vector<uint32_t> used_at_;        // per block: epoch when it uses or defines the value
	std::vector<uint32_t> inside_at_;      // per block: epoch when inside a chosen loop
	std::vector<uint32_t> reload_at_;      // per block: epoch when a reload's range was added
	std::vector<uint32_t> loop_at_;        // per block: its chosen loop (when inside)
	std::vector<uint32_t> chosen_;         // candidates' loops
	std::vector<uint32_t> split_at_;       // per block: split stamp when inside a loop plan keeps
	std::vector<uint32_t> exc_succ_first_; // per block: the landings it enters (CSR)
	std::vector<uint32_t> exc_succs_;
	uint32_t epoch_ = 0;
	uint32_t split_epoch_ = 0;
};

} // namespace native
