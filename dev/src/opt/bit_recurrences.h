#pragma once

#include "lowir/program.h"
#include "support/flat_map.h"

namespace opt {

struct OptimizerStats;

struct BitTableKey
{
	uint64_t polynomial;
	uint32_t width, steps, inverted;
	bool operator==(const BitTableKey& other) const;
};

struct BitTableHash
{
	size_t operator()(const BitTableKey& key) const;
};

// One optimization unit's bounded table pool and reusable matching scratch.
// The cache is not a LowIR fact: its results are ordinary readonly globals.
struct BitTablePool
{
	lowir::SymbolId table(lowir::Program& program, const BitTableKey& key, OptimizerStats& stats);
	FlatMap<BitTableKey, lowir::SymbolId, BitTableHash> tables;
	uint32_t remaining = 4096;
	uint32_t names = 0;
	std::vector<lowir::InstId> candidates, owned, previous;
	std::vector<lowir::TempId> replacements;
	std::vector<uint32_t> internal;
	std::vector<uint8_t> covered;
};

} // namespace opt
