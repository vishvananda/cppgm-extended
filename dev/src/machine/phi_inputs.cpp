#include "machine/phi_inputs.h"

#include <algorithm>

namespace machine {

void PhiInputs::reset(size_t instructions)
{
	first_.assign(instructions, lowir::kNoId);
	pairs_.clear();
}

lowir::Value PhiInputs::incoming(const lowir::FunctionBody& body, lowir::InstId id, lowir::BlockId predecessor)
{
	const auto& phi = body.instructions[id];
	lowir::Value value = lowir::Value::none();
	if (phi.extra_count <= 16)
	{
		for (uint32_t k = 0; k + 1 < phi.extra_count; k += 2)
			if (body.operands[phi.extra_first + k].index == predecessor)
				value = body.operands[phi.extra_first + k + 1];
		return value;
	}
	if (first_[id] == lowir::kNoId)
	{
		first_[id] = static_cast<uint32_t>(pairs_.size());
		for (uint32_t k = 0; k + 1 < phi.extra_count; k += 2)
			pairs_.push_back({ body.operands[phi.extra_first + k].index, k + 1 });
		std::sort(pairs_.begin() + first_[id], pairs_.end());
	}
	const auto first = pairs_.begin() + first_[id];
	const auto past = std::upper_bound(first, first + phi.extra_count / 2, std::make_pair(predecessor, UINT32_MAX));
	if (past != first && (past - 1)->first == predecessor)
		value = body.operands[phi.extra_first + (past - 1)->second];
	return value;
}

} // namespace machine
