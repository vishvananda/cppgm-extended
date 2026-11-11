#pragma once
#include "opt/passes.h"
namespace opt {
// Reusable predecessor scratch for a bounded proof of equal cyclic phi graphs.
class PhiComparison
{
public:
	bool equal(FunctionContext& context, const lowir::Instruction& a, const lowir::Instruction& b,
		const std::vector<lowir::Value>& replacements);

private:
	std::vector<lowir::Value> pred_input_;
	std::vector<uint32_t> pred_stamp_;
	uint32_t stamp_ = 0;
};
uint64_t phi_signature(
	const lowir::FunctionBody& body, const std::vector<lowir::BlockId>& block_of, const lowir::Instruction& phi);
} // namespace opt
