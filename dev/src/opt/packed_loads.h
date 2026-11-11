#pragma once

#include "opt/function_ir.h"

namespace opt {

// A byte-aligned floating bit projection from a single-use integer load. Reuse
// the value walk's use counts and block map; inspect only a bounded chain.
bool simplify_packed_load(lowir::FunctionBody& body, const std::vector<uint32_t>& uses,
	const std::vector<lowir::BlockId>& block_of, lowir::BlockId block, lowir::InstId& previous,
	lowir::InstId instruction);

} // namespace opt
