#pragma once

#include "lowir/program.h"

namespace machine {

// Logical predecessor/value lookup, independent of register, local or stack
// placement. Large joins are indexed once and reused by all incoming edges.
class PhiInputs
{
public:
	void reset(size_t instructions);
	lowir::Value incoming(const lowir::FunctionBody& body, lowir::InstId phi, lowir::BlockId predecessor);

private:
	std::vector<uint32_t> first_;
	std::vector<std::pair<uint32_t, uint32_t>> pairs_;
};

} // namespace machine
