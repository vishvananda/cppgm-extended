#include "native/select/function_selector.h"

namespace native {

using lowir::kNoId;
using lowir::TypeKind;

// A landing's pad (the exception object; the filter as the selector, the
// raw filter, or both) when a call names it, and its selector block, ahead
// of its block.
void FunctionSelector::emit_landing_pad(uint32_t b)
{
	location_ = 0;
	const uint32_t index = eh_.eh_landing_index_[b];
	const int32_t slot = out_->eh.slot;
	const bool translate = eh_.eh_translate_[index] != 0;
	if (pad_mir_[b] != kNoId)
	{
		begin_mir_block(lowir::own_name(body_->blocks[blocks_[b].id]), kNoId, kNoId);
		out_->blocks.back().landing = true;
		Inst& pad = emit2(
			Op::Landing, TypeKind::Void, Operand::make_frame(slot), Operand::make_frame(slot + (eh_.eh_raw_ ? 16 : 8)));
		if (eh_.eh_raw_ && !translate)
		{
			pad.ops[2] = Operand::make_frame(slot + 8);
			pad.nops = 3;
		}
		emit1(Op::Jmp, TypeKind::Void, Operand::make_label(translate ? select_mir_[b] : blocks_[b].mir));
		end_mir_block();
	}
	if (select_mir_[b] == kNoId)
		return;
	begin_mir_block(lowir::own_name(body_->blocks[blocks_[b].id]), kNoId, kNoId);
	out_->blocks.back().landing = true;
	Inst& select = emit2(Op::EhSelect, TypeKind::Void, Operand::make_frame(slot + 8), Operand::make_frame(slot + 16));
	select.aux = index;
	emit1(Op::Jmp, TypeKind::Void, Operand::make_label(blocks_[b].mir));
	end_mir_block();
}

} // namespace native
