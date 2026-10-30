// GNU CPUID consumes fixed register bindings lifted by analysis. The native
// runtime role returns the real CPU's four words through ordinary call ABI.
#include "lower/function_lowering.h"
#include "sema/asm_statements.h"

namespace lower {

lowir::SymbolId ProgramLowering::cpu_query_function()
{
	const lowir::SymbolId symbol = external_symbol("__cppgm_cpu_query", "__cppgm_cpu_query");
	if (program_.symbols[symbol].kind != lowir::SymbolKind::Unresolved)
		return symbol;
	lowir::Signature signature = lowir::Signature();
	signature.first_parameter = static_cast<uint32_t>(program_.parameters.size());
	signature.parameter_count = 3;
	signature.result = lowir::Type::scalar(lowir::TypeKind::Void);
	signature.boundary.unwind = ir_model::CUM_NO;
	for (uint32_t i = 0; i < 3; ++i)
	{
		lowir::Parameter parameter = lowir::Parameter();
		parameter.name = argument_name(i);
		parameter.type = lowir::Type::scalar(i == 2 ? lowir::TypeKind::Ptr : lowir::TypeKind::I32);
		program_.parameters.push_back(parameter);
	}
	program_.declare_function(symbol, lowir::SymbolKind::FunctionDeclaration, program_.add_signature(signature));
	lowir::SymbolMetadata& metadata = program_.symbols[symbol].metadata;
	metadata.role = ir_model::SR_CPU_QUERY;
	metadata.binding = ir_model::SBM_INTERNAL;
	++counters_.declarations;
	return symbol;
}

void FunctionLowering::asm_cpu_query(const sema::AsmInstruction& instruction, AsmState& state)
{
	const lowir::Type word = lowir::Type::scalar(lowir::TypeKind::I32);
	Place result = Place();
	result.storage = lowir::Value::slot(generated_slot(lowir::SlotKind::Asm, lowir::Type::object(16, 2)));
	const lowir::Value pointer = address_of(result);
	const size_t base = arguments_.size();
	for (uint32_t reg = 0; reg <= 2; reg += 2)
		arguments_.push_back(load(word, asm_place(state, (instruction.immediate >> (reg * 8)) & 0xff)).value);
	arguments_.push_back(pointer);
	emit_call(lowir::Type::scalar(lowir::TypeKind::Void), lowir::Value::symbol(lowering_.cpu_query_function()), base,
		lowir::kNoId, false);
	for (uint32_t reg = 0; reg < 4; ++reg)
	{
		Place value = Place();
		value.storage = index(lowir::Type::scalar(lowir::TypeKind::I8), ir_model::IPK_FIELD, pointer,
			integer(lowir::Type::scalar(lowir::TypeKind::I64), reg * 4))
							.value;
		value.untyped = true;
		asm_store(state, (instruction.immediate >> (reg * 8)) & 0xff, load(word, value));
	}
}

} // namespace lower
