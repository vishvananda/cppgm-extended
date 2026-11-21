#pragma once

#include "lowir/program.h"

namespace opt {
struct Cfg;
}

namespace machine {
struct ExceptionLayout;
class PhiWebs;
}

namespace aarch64 {

struct LiveRange
{
	uint32_t id, first = lowir::kNoId, last = 0;
	uint32_t segments = 0, count = 0, affinity = lowir::kNoId;
	bool vector_address = false;
};

// A phi transfer two homes can share: the incoming value's representative,
// the phi, the predecessor that supplies it and the phi's block.
struct PhiCopy
{
	uint32_t source, phi;
	lowir::BlockId from, to;
};

// What the allocator reads beside the ranges: the phi transfers two homes
// can share, and the first position of each landing pad, in order.
struct RangeFacts
{
	std::vector<PhiCopy> copies;
	std::vector<uint32_t> landings;
};

// The narrow integer values whose readers take only their low bits (narrow.cpp).
std::vector<uint8_t> loose_values(
	const lowir::FunctionBody& body, const std::vector<uint8_t>& word_accesses, bool before_allocation);
std::vector<uint32_t> scalar_aliases(const lowir::FunctionBody& body);
inline uint32_t representative(const std::vector<uint32_t>& aliases, uint32_t id)
{
	return aliases.empty() ? id : aliases[id];
}

std::vector<LiveRange> live_ranges(const lowir::Program& program, const lowir::FunctionBody& body,
	const machine::ExceptionLayout* exceptions, std::vector<uint32_t>& uses, std::vector<uint32_t>& calls,
	std::vector<std::pair<uint32_t, uint32_t>>& segments, opt::Cfg& cfg, machine::PhiWebs& webs,
	std::vector<uint32_t>& entries, const std::vector<uint32_t>& aliases, bool separate_shared_inputs,
	RangeFacts& facts);

void block_frequencies(
	const lowir::Program& program, const lowir::FunctionBody& body, opt::Cfg& cfg, std::vector<uint64_t>& weights);
std::vector<uint64_t> spill_weights(const lowir::Program& program, const lowir::FunctionBody& body, opt::Cfg& cfg,
	const std::vector<uint32_t>& aliases, std::vector<uint64_t>& weights);

} // namespace aarch64
