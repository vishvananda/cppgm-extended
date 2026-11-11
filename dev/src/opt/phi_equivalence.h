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
// A phi whose every input is a retyping copy from one other type of the same
// width (a pointer's bits as i64, an integer in the other signedness), a
// zero or the phi itself becomes that copy of a phi of the copies' sources;
// one whose every input steps a pointer by the same constant (and is no
// loop's pointer) becomes that step from a phi of the pointers.
// Inputs are read through `replacements`, which grows with `uses` for the
// new phis; a new phi precedes the block's other phis.
bool retype_copy_phis(FunctionContext& context, lowir::BlockId block, std::vector<lowir::Value>& replacements,
	std::vector<uint32_t>& uses);
} // namespace opt
