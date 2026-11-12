#pragma once

#include "lowir/program.h"
#include <memory>

namespace machine {

// Per-function plans retain storage across functions and are computed only for
// calls that need them. Layout remains a target-defined typed representation.
template<typename Layout>
struct CallPlanCache
{
	std::vector<uint32_t> entry;
	std::vector<std::unique_ptr<Layout>> layouts;
	uint32_t used = 0;
	Layout scratch;
	void reset(size_t instructions)
	{
		entry.assign(instructions, lowir::kNoId);
		used = 0;
	}
	const Layout* known(uint32_t inst) const
	{
		return entry[inst] != lowir::kNoId ? layouts[entry[inst]].get() : nullptr;
	}
	Layout& add(uint32_t inst)
	{
		if (used == layouts.size())
			layouts.emplace_back(new Layout());
		entry[inst] = used;
		return *layouts[used++];
	}
};

void apply_argument_types(
	const lowir::FunctionBody& body, const lowir::Instruction& instruction, std::vector<lowir::Type>& types);

ir_model::AggregateShape argument_shape(
	const lowir::FunctionBody& body, const lowir::Instruction& instruction, uint32_t argument);

// Boundary formals determine named argument types; unnamed arguments retain
// their explicitly recorded scalar view or their LowIR operand's storage type.
void call_argument_types(const lowir::FunctionBody& body, const lowir::Instruction& inst,
	const lowir::Signature& signature, const lowir::Parameter* parameters, std::vector<lowir::Type>& output);

} // namespace machine
