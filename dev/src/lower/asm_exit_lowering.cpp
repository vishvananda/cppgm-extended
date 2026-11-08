#include "lower/function_lowering.h"
#include "asm/packet.h"
#include "sema/asm_statements.h"

namespace lower {
lowir::Value FunctionLowering::full_asm_exit_arguments(
	const syntax::AsmStmt& stmt, std::vector<lowir::Type>& types, std::vector<lowir::Value>& values)
{
	if (!stmt.is_goto)
		return lowir::Value::none();
	const lowir::Type type = lowir::Type::scalar(lowir::TypeKind::I32);
	const assembly::Packet& packet = *stmt.program->full;
	const uint32_t first = stmt.outputs.size + stmt.inputs.size;
	for (uint32_t i = 0; i < stmt.labels.size; ++i)
	{
		types.push_back(type);
		values[packet.operands.data[first + i].input] = integer(type, i + 1).value;
	}
	types.push_back(type);
	Place result = Place();
	result.storage = lowir::Value::slot(generated_slot(lowir::SlotKind::Asm, type));
	values[packet.operands.data[first + stmt.labels.size].output] = address_of(result);
	return result.storage;
}

void FunctionLowering::branch_asm_exits(const syntax::AsmStmt& stmt, lowir::Value selector)
{
	const lowir::Type type = lowir::Type::scalar(lowir::TypeKind::I32);
	Place result = Place();
	result.storage = selector;
	const RValue selected = load(type, result);
	const lowir::BlockId end = new_block(lowir::BlockKind::CondEnd);
	std::vector<lowir::BlockId> blocks;
	std::vector<lowir::Value> cases;
	for (uint32_t i = 0; i < stmt.labels.size; ++i)
	{
		blocks.push_back(new_block(lowir::BlockKind::SwitchCase));
		cases.push_back(integer(type, i + 1).value);
		cases.push_back(lowir::Value::block(blocks.back()));
	}
	lowir::Instruction dispatch = lowir::make_instruction(
		lowir::Opcode::Switch, lowir::Type::scalar(lowir::TypeKind::Void));
	dispatch.ops[0] = selected.value;
	dispatch.ops[1] = lowir::Value::block(end);
	dispatch.extra_first = builder_.add_operands(cases.data(), static_cast<uint32_t>(cases.size()));
	dispatch.extra_count = static_cast<uint32_t>(cases.size());
	terminate(dispatch);
	for (uint32_t i = 0; i < stmt.labels.size; ++i)
	{
		place(blocks[i]);
		goto_statement(*stmt.labels[i]);
	}
	place(end);
}
} // namespace lower
