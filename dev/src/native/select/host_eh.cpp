#include "native/select/function_selector.h"
#include "native/select/for_each_inst.h"
#include "native/backend_error.h"

namespace native {

using lowir::kNoId;

void FunctionSelector::analyze_host_eh()
{
	const lowir::FunctionBody& body = *body_;
	const uint32_t n = static_cast<uint32_t>(blocks_.size());
	pad_mir_.assign(n, kNoId);
	select_mir_.assign(n, kNoId);
	bool any = false;
	for_each_inst([&](uint32_t id, uint32_t) {
		const lowir::Opcode op = body.instructions[id].op;
		any = any || (op >= lowir::Opcode::EhTry && op <= lowir::Opcode::EhEnd) || op == lowir::Opcode::Throw ||
			op == lowir::Opcode::Exception || op == lowir::Opcode::ExceptionSelector || op == lowir::Opcode::Resume;
	});
	if (!any)
	{
		eh_.reset(n, static_cast<uint32_t>(body.instructions.size()));
		return;
	}
	eh_.blocks_.resize(n);
	for (uint32_t b = 0; b < n; ++b)
		eh_.blocks_[b] = machine::ExceptionBlock{ blocks_[b].id, blocks_[b].reachable, blocks_[b].handler };
	const machine::ExceptionGraph graph{ &layout_of_, &succ_first_, &succs_, &preds_first_, &preds_, &handler_first_,
		&handlers_ };
	try
	{
		eh_.analyze(program_, symbol_, body, graph, out_->eh);
	}
	catch (const machine::ExceptionError& error)
	{
		throw BackendError(std::string("host EH: ") + error.what());
	}
	stats_.eh_walks += eh_.eh_walks_;
	stats_.eh_states += eh_.eh_states_;
	bool pruned = false;
	for (uint32_t b = 0; b < n; ++b)
	{
		if (!blocks_[b].reachable || eh_.blocks_[b].reachable)
			continue;
		blocks_[b].reachable = false;
		pruned = true;
		for (lowir::InstId i = body.blocks[blocks_[b].id].first; i != kNoId; i = body.instructions[i].next)
			marks_[i] &= ~IM_REACHABLE;
	}
	if (pruned)
		reachable_preds();
	if (!out_->eh.landings.empty())
		out_->eh.slot = frame_alloc(eh_.eh_raw_ ? 24 : 16, 8);
}

int64_t FunctionSelector::eh_entry_selector(uint32_t b) const
{
	return machine::exception_entry_selector(*body_, blocks_[b].id, blocks_[b].handler);
}

bool FunctionSelector::eh_entry_edge(uint32_t b) const
{
	return eh_model_ == EhModel::Host && eh_entry_selector(b) != 0;
}

bool FunctionSelector::eh_selects(uint32_t b, uint32_t index) const
{
	return eh_.eh_selects(b, index);
}

} // namespace native
