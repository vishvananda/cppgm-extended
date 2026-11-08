#include "lower/function_lowering.h"

#include "asm/packet.h"
#include "lowir/assembly_builder.h"
#include "sema/asm_statements.h"
#include "sema/expression_facts.h"

namespace lower {

void FunctionLowering::full_asm_statement(const syntax::AsmStmt& stmt)
{
	const assembly::Packet& packet = *stmt.program->full;
	std::vector<const syntax::Expr*> expressions;
	for (const syntax::AsmOperand& source : stmt.outputs)
		expressions.push_back(source.expression);
	for (const syntax::AsmOperand& source : stmt.inputs)
		expressions.push_back(source.expression);
	const size_t cleanup_base = cleanups_.size();
	bool temporaries = false, throwing = false;
	for (const syntax::Expr* expr : expressions)
	{
		temporaries = temporaries || creates_temporaries(expr);
		throwing = throwing || may_throw(expr);
		hidden_context_ = hidden_context_ || branch_temporaries(expr, 0);
	}
	const bool forced = forced_temporaries_;
	forced_temporaries_ = forced || temporaries;
	const bool region = begin_full_expression(expressions.empty() ? nullptr : expressions[0], true, throwing);
	forced_temporaries_ = forced;
	if (region && temporaries)
	{
		end_tracking();
		begin_tracking(expressions.data(), expressions.size(), true, true);
	}
	std::vector<lowir::Type> types;
	std::vector<lowir::Value> values(packet.definition.argument_count, lowir::Value::none());
	for (uint32_t i = 0; i < expressions.size(); ++i)
	{
		const syntax::Expr& expr = *expressions[i];
		const asm_model::Operand& argument = packet.operands.data[i];
		const lowir::Type type = lowering_.storage_type(expr.facts->type->unqualified);
		types.push_back(type);
		lowir::Value destination = lowir::Value::none();
		if (argument.output != asm_model::kNoId || argument.input_storage)
			destination = address(expr);
		if (argument.output != asm_model::kNoId)
			values[argument.output] = destination;
		if (argument.input == asm_model::kNoId)
			continue;
		if (argument.input_storage)
			values[argument.input] = destination;
		else if (argument.output != asm_model::kNoId)
		{
			Place place = Place();
			place.storage = destination;
			place.is_volatile = (expr.facts->type->cv & sema::kVolatile) != 0;
			values[argument.input] = load(type, place).value;
		}
		else
			values[argument.input] = operand(expr, ConversionContext::Argument).value;
	}
	lowir::Instruction inst = lowir::make_instruction(lowir::Opcode::Asm, lowir::Type::scalar(lowir::TypeKind::Void));
	inst.immediate = lowir::add_assembly(
		lowering_.program(), packet, types.data(), lowering_.debug_location(stmt.location));
	inst.extra_count = static_cast<uint32_t>(values.size());
	inst.extra_first = builder_.add_operands(values.data(), inst.extra_count);
	builder_.append(inst);
	destroy_temporaries(cleanup_base);
	end_full_expression(region);
}

} // namespace lower
