#pragma once

#include "machine/exception_analysis.h"
#include "opt/function_ir.h"
#include "wasm/object.h"

namespace wasm {

struct ExceptionPlan
{
	machine::ExceptionAnalysis analysis;
	machine::ExceptionTables tables;
	std::vector<uint32_t> layout_of, succ_first, succs, pred_first, preds, handler_first, handlers;
	std::vector<int64_t> entry_selectors;
	std::vector<lowir::BlockId> landing_blocks;
	opt::Cfg control;
	uint32_t lsda_symbol = lowir::kNoId;
	bool active = false;
};

void prepare_exception_runtime(lowir::Program& program, Module& module);
void prepare_exception_plan(const lowir::Program& program, lowir::SymbolId symbol, Object& object,
	std::vector<uint32_t>& symbols, uint32_t group, ExceptionPlan& plan);

} // namespace wasm
