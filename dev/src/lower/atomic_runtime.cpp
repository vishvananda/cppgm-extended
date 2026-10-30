#include "lower/function_lowering.h"

#include <cstring>

#include "sema/conversions.h"
#include "sema/expression_facts.h"

namespace lower {

lowir::SymbolId ProgramLowering::atomic_runtime_function(sema::Intrinsic kind)
{
	// The generic ABI uses size_t, pointers and int orders. Compare-exchange
	// has no weak argument: a strong operation also satisfies a weak request.
	const char* name = nullptr;
	const char* parameters = nullptr;
	switch (kind)
	{
	case sema::Intrinsic::AtomicLoad:
		name = "__atomic_load";
		parameters = "sppi";
		break;
	case sema::Intrinsic::AtomicStore:
		name = "__atomic_store";
		parameters = "sppi";
		break;
	case sema::Intrinsic::AtomicExchange:
		name = "__atomic_exchange";
		parameters = "spppi";
		break;
	case sema::Intrinsic::AtomicCompareExchange:
		name = "__atomic_compare_exchange";
		parameters = "spppii";
		break;
	default:
		throw LoweringError(0, "an atomic builtin without a generic runtime form");
	}
	const lowir::SymbolId symbol = external_symbol(name, std::string("__cppgm_generic_") + (name + 2));
	if (program_.symbols[symbol].kind != lowir::SymbolKind::Unresolved)
		return symbol;
	lowir::Signature signature = lowir::Signature();
	signature.first_parameter = static_cast<uint32_t>(program_.parameters.size());
	signature.parameter_count = static_cast<uint32_t>(std::strlen(parameters));
	for (uint32_t i = 0; i < signature.parameter_count; ++i)
	{
		lowir::Parameter parameter = lowir::Parameter();
		parameter.name = argument_name(i);
		parameter.type = lowir::Type::scalar(parameters[i] == 's' ? lowir::TypeKind::I64
				: parameters[i] == 'p'                            ? lowir::TypeKind::Ptr
																  : lowir::TypeKind::I32);
		program_.parameters.push_back(parameter);
	}
	signature.result = lowir::Type::scalar(
		kind == sema::Intrinsic::AtomicCompareExchange ? lowir::TypeKind::I1 : lowir::TypeKind::Void);
	signature.boundary.unwind = ir_model::CUM_NO;
	program_.declare_function(symbol, lowir::SymbolKind::FunctionDeclaration, program_.add_signature(signature));
	lowir::SymbolMetadata& metadata = program_.symbols[symbol].metadata;
	metadata.linkage = ir_model::LLM_C;
	metadata.binding = ir_model::SBM_STRONG;
	metadata.object = intern(name, static_cast<uint32_t>(std::strlen(name)));
	++counters_.declarations;
	return symbol;
}

RValue FunctionLowering::atomic_runtime_call(const syntax::CallExpr& expr, sema::Intrinsic kind)
{
	if (!lowering_.host_objects())
		throw LoweringError(expr.location, "a generic atomic runtime call requires host object mode");
	const bool compare = kind == sema::Intrinsic::AtomicCompareExchange;
	const syntax::Slice<syntax::Expr*>& operands = expr.arguments;
	const sema::Type* pointer = operands[0]->facts->type->unqualified;
	const size_t base = arguments_.size();
	arguments_.push_back(
		integer(lowir::Type::scalar(lowir::TypeKind::I64), lowering_.size_of(pointer->element, expr.location)).value);
	for (uint32_t i = 0; i < operands.size; ++i)
	{
		if (compare && i == 3)
			discard(*operands[i]);
		else if (i >= operands.size - (compare ? 2 : 1))
		{
			const lowir::MemoryOrder order = memory_order(*operands[i]);
			arguments_.push_back(
				integer(lowir::Type::scalar(lowir::TypeKind::I32), static_cast<uint64_t>(order)).value);
		}
		else
			arguments_.push_back(operand(*operands[i], ConversionContext::Argument).value);
	}
	RValue result;
	result.type = lowir::Type::scalar(compare ? lowir::TypeKind::I1 : lowir::TypeKind::Void);
	result.value = emit_call(
		result.type, lowir::Value::symbol(lowering_.atomic_runtime_function(kind)), base, lowir::kNoId, false);
	return result;
}

} // namespace lower
