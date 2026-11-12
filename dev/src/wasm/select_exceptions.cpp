#include "wasm/selector.h"

#include "wasm/exceptions.h"

namespace wasm {

using namespace lowir;

void Selector::exception_address(uint32_t symbol, int64_t offset)
{
	out_.emit(Op::I32Const, { 0 });
	out_.relocate(RelocationKind::MemoryAddressSigned, symbol, 0, offset);
}

void Selector::exception_select(uint32_t index)
{
	const auto& tables = exceptions_->tables;
	const auto& landing = tables.landings[index];
	out_.emit(Op::I32Const, { 0 });
	out_.emit(Op::LocalSet, { exception_selector_ });
	for (uint32_t k = landing.map_count; k-- > 0;)
	{
		const uint32_t pair = landing.map_first + 2 * k;
		out_.emit(Op::I32Const, { static_cast<uint64_t>(static_cast<int64_t>(tables.map[pair + 1])) });
		out_.emit(Op::LocalGet, { exception_selector_ });
		out_.emit(Op::LocalGet, { exception_filter_ });
		out_.emit(Op::I32Const, { static_cast<uint64_t>(static_cast<int64_t>(tables.map[pair])) });
		out_.emit(Op::I32Eq);
		out_.emit(Op::Select);
		out_.emit(Op::LocalSet, { exception_selector_ });
	}
}

void Selector::exception_snapshot(uint32_t landing, bool restore)
{
	const uint32_t current[4] = { exception_reference_, exception_pointer_, exception_filter_, exception_selector_ };
	for (uint32_t k = 0; k < 4; ++k)
	{
		out_.emit(Op::LocalGet, { restore ? exception_contexts_[landing] + k : current[k] });
		out_.emit(Op::LocalSet, { restore ? current[k] : exception_contexts_[landing] + k });
	}
}

void Selector::restore_exception_context()
{
	const uint32_t context = exceptions_->analysis.eh_context_[current_inst_];
	if (context != kNoId)
	{
		const uint32_t landing = exceptions_->analysis.eh_landing_index_[context];
		if (landing != kNoId)
			exception_snapshot(landing, true);
	}
}

void Selector::exception_match(uint32_t landing)
{
	exception_address(module_.exception_context);
	out_.emit(Op::I32Const, { landing });
	out_.emit(Op::I32Store, { 2, 0 });
	exception_address(module_.exception_context, 4);
	exception_address(exceptions_->lsda_symbol);
	out_.emit(Op::I32Store, { 2, 0 });
	out_.emit(Op::LocalGet, { exception_pointer_ });
	out_.emit(Op::Call, { module_.function_indices[module_.personality_symbol] });
	out_.relocate(RelocationKind::FunctionIndex, module_.personality_symbol);
	out_.emit(Op::Drop);
	exception_address(module_.exception_context, 8);
	out_.emit(Op::I32Load, { 2, 0 });
	out_.emit(Op::LocalSet, { exception_filter_ });
	exception_select(landing);
	exception_snapshot(landing, false);
}

void Selector::exception_call(const lowir::Instruction& inst, const lowir::Signature& signature, uint32_t landing)
{
	out_.emit(Op::GlobalGet, { 0 });
	out_.relocate(RelocationKind::GlobalIndex, module_.stack_symbol);
	out_.emit(Op::LocalSet, { exception_stack_ });
	out_.emit(Op::Block, { block_type(ValueType::Void) });
	++nesting_;
	out_.emit(Op::Block, { module_.exception_pair_type });
	out_.relocate(RelocationKind::TypeIndex, module_.exception_pair_type);
	++nesting_;
	out_.emit(Op::TryTable, { block_type(ValueType::Void), 1, static_cast<uint64_t>(CatchKind::TagRef), 0, 0 });
	out_.relocate(RelocationKind::TagIndex, module_.exception_tag_symbol, 3);
	++nesting_;
	call_body(inst, signature);
	out_.emit(Op::End);
	--nesting_;
	out_.emit(Op::Br, { 1 });
	out_.emit(Op::End);
	--nesting_;
	out_.emit(Op::LocalSet, { exception_reference_ });
	out_.emit(Op::LocalSet, { exception_pointer_ });
	out_.emit(Op::LocalGet, { exception_stack_ });
	out_.emit(Op::GlobalSet, { 0 });
	out_.relocate(RelocationKind::GlobalIndex, module_.stack_symbol);
	exception_match(landing);
	edge(current_block_, exceptions_->landing_blocks[landing]);
	out_.emit(Op::End);
	--nesting_;
}

void Selector::exception_resume(uint32_t site)
{
	restore_exception_context();
	if (site != 0 && site != machine::ExceptionAnalysis::kResumeTerminal)
	{
		const uint32_t landing = exceptions_->analysis.eh_landing_index_[site - 1];
		exception_match(landing);
		edge(current_block_, body_.layout[site - 1]);
	}
	else
	{
		restore_stack();
		out_.emit(Op::LocalGet, { exception_reference_ });
		out_.emit(Op::ThrowRef);
	}
}

bool Selector::exception_instruction(const lowir::Instruction& inst)
{
	switch (inst.op)
	{
	case Opcode::EhTry:
	case Opcode::EhCleanup:
	case Opcode::EhCatch:
	case Opcode::EhFilter:
		return true;
	case Opcode::EhEnd:
		restore_exception_context();
		return true;
	case Opcode::EhCatchAll:
		if (exceptions_ != nullptr && exceptions_->active && exceptions_->analysis.eh_sites_[current_inst_] != 0)
		{
			out_.emit(Op::I32Const, { static_cast<uint64_t>(inst.immediate) });
			out_.emit(Op::LocalGet, { exception_selector_ });
			out_.emit(Op::LocalGet, { exception_selector_ });
			out_.emit(Op::I32Eqz);
			out_.emit(Op::Select);
			out_.emit(Op::LocalSet, { exception_selector_ });
		}
		return true;
	case Opcode::Exception:
	case Opcode::ExceptionSelector:
		restore_exception_context();
		out_.emit(Op::LocalGet, { inst.op == Opcode::Exception ? exception_pointer_ : exception_selector_ });
		if (value_type(body_.temps[inst.result].type) == ValueType::I64)
			out_.emit(inst.op == Opcode::ExceptionSelector ? Op::I64ExtendI32S : Op::I64ExtendI32U);
		out_.emit(Op::LocalSet, { locals_[inst.result] });
		return true;
	case Opcode::Resume:
		exception_resume(exceptions_->analysis.eh_sites_[current_inst_]);
		return true;
	case Opcode::Jump:
	{
		if (exceptions_ == nullptr || !exceptions_->active || exceptions_->analysis.eh_sites_[current_inst_] == 0)
			return false;
		exception_resume(exceptions_->analysis.eh_sites_[current_inst_]);
		return true;
	}
	default:
		return false;
	}
}

} // namespace wasm
