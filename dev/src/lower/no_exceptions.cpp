#include "lower/program_lowering.h"

#include "lowir/builder.h"
#include "opt/passes.h"

namespace lower {

void ProgramLowering::remove_exception_regions(lowir::SymbolId symbol)
{
	lowir::FunctionBody& body = *program_.functions[program_.symbols[symbol].detail].body;
	bool changed = false;
	for (lowir::BlockId block : body.layout)
	{
		lowir::InstId previous = lowir::kNoId;
		for (lowir::InstId i = body.blocks[block].first; i != lowir::kNoId;)
		{
			const lowir::InstId next = body.instructions[i].next;
			if (lowir::is_eh_marker(body.instructions[i].op))
			{
				opt::unlink_after(body, block, previous, i);
				changed = true;
			}
			else
				previous = i;
			i = next;
		}
	}
	// Normal destruction and branch-dependent temporary tracking are retained.
	// Generated unwind-only paths lose their region edges and can be removed.
	// Addressed labels retain their own placement and reachability contract.
	if (changed && body.code_labels.empty())
	{
		opt::OptimizerStats stats;
		opt::FunctionContext context(program_, symbol, body, stats, 0);
		opt::cleanup_cfg(context);
	}
}

} // namespace lower
