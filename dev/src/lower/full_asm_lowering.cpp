#include "lower/function_lowering.h"

#include "asm/packet.h"
#include "lowir/assembly_builder.h"
#include "lowir/assembly.h"
#include "sema/asm_statements.h"
#include "sema/expression_facts.h"
#include "sema/constant_values.h"

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
	std::vector<AsmOutputCapture> captures(expressions.size());
	std::vector<lowir::Type> types;
	std::vector<lowir::Value> values(packet.definition.argument_count, lowir::Value::none());
	for (uint32_t i = 0; i < expressions.size(); ++i)
	{
		const syntax::Expr& expr = *expressions[i];
		const asm_model::Operand& argument = packet.operands.data[i];
		const lowir::Type type = lowering_.storage_type(expr.facts->type->unqualified);
		types.push_back(type);
		lowir::Value destination = lowir::Value::none();
		if (argument.output != asm_model::kNoId && (argument.flags & asm_model::AO_NO_MEMORY) != 0)
		{
			AsmOutputCapture& capture = captures[i];
			capture.result_slot = lowir::Value::slot(generated_slot(lowir::SlotKind::Asm, type));
			capture.pointer_slot = lowir::Value::slot(
				generated_slot(lowir::SlotKind::Asm, lowir::Type::scalar(lowir::TypeKind::Ptr)));
			capture.selector_slot = lowir::Value::slot(
				generated_slot(lowir::SlotKind::Asm, lowir::Type::scalar(lowir::TypeKind::I32)));
			capture.empty = integer(type, 0);
			capture.read = (argument.flags & asm_model::AO_READ) != 0;
			BitFieldUpdate update = BitFieldUpdate();
			update.kind = BitFieldUpdate::Kind::Capture;
			update.capture = &capture;
			const RValue initial = update_bit_field(expr, update);
			Place result = Place();
			result.storage = capture.result_slot;
			values[argument.output] = address_of(result);
			if (argument.input != asm_model::kNoId)
				values[argument.input] = initial.value;
			continue;
		}
		if (argument.output != asm_model::kNoId || argument.input_storage)
			destination = address(expr);
		if (argument.output != asm_model::kNoId)
			values[argument.output] = destination;
		if (argument.input == asm_model::kNoId)
			continue;
		if (argument.input_storage)
			values[argument.input] = destination;
		else if (stmt.program->addresses != nullptr && stmt.program->addresses[i].kind == sema::ValueKind::Pointer)
		{
			if (!constant_address(stmt.program->addresses[i], values[argument.input], expr.location))
				throw LoweringError(expr.location, "assembly symbol requires a link-time address");
			types.back() = lowir::Type::scalar(lowir::TypeKind::Ptr);
		}
		else if (packet.constants.size != 0 && packet.constants.data[i].present)
		{
			const asm_model::Bits& bits = packet.constants.data[i].bits;
			values[argument.input] = integer(type, lowir::Literal{ bits.low, bits.high }).value;
		}
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
	const lowir::Value exits = full_asm_exit_arguments(stmt, types, values);
	lowir::Instruction inst = lowir::make_instruction(lowir::Opcode::Asm, lowir::Type::scalar(lowir::TypeKind::Void));
	inst.immediate = lowir::add_assembly(
		lowering_.program(), packet, types.data(), lowering_.debug_location(stmt.location));
	lowering_.program().assembly->templates[inst.immediate].environment = lowering_.assembly_environment(
		stmt.assembly_environment);
	inst.extra_count = static_cast<uint32_t>(values.size());
	inst.extra_first = builder_.add_operands(values.data(), inst.extra_count);
	builder_.append(inst);
	for (const AsmOutputCapture& capture : captures)
		if (!capture.cases.empty())
			finish_asm_output(capture);
	destroy_temporaries(cleanup_base);
	end_full_expression(region);
	if (stmt.is_goto)
		branch_asm_exits(stmt, exits);
}

} // namespace lower
