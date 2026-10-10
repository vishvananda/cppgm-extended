#pragma once

#include "native/select/function_selector.h"

namespace native {

// Visits the instructions of reachable blocks in layout order.
template<typename F>
void FunctionSelector::for_each_inst(F f)
{
	const lowir::FunctionBody& body = *body_;
	for (uint32_t b = 0; b < blocks_.size(); ++b)
	{
		if (!blocks_[b].reachable)
			continue;
		for (lowir::InstId i = body.blocks[blocks_[b].id].first; i != lowir::kNoId; i = body.instructions[i].next)
			f(i, b);
	}
}

} // namespace native
