#include "backend/lowir_c_module.h"

#include "lowir/builder.h"

namespace {

bool valid_arrays(const CppgmBody& input)
{
	return (input.slot_count == 0 || input.slots != nullptr) && input.block_count != 0 && input.layout != nullptr &&
		(input.instruction_count == 0 || input.instructions != nullptr) &&
		(input.operand_count == 0 || input.operands != nullptr) &&
		(input.literal_count == 0 || input.literals != nullptr);
}

} // namespace

bool CppgmLowir::body(uint32_t symbol, const CppgmBody& input)
{
	using namespace backend::c_api;
	if (!ready())
		return false;
	if (symbol >= program.symbols.size() || program.symbols[symbol].kind != lowir::SymbolKind::FunctionDefinition ||
		!valid_arrays(input))
		return fail("invalid function body");
	lowir::FunctionBody& body = *program.functions[program.symbols[symbol].detail].body;
	if (!body.temps.empty() || !body.instructions.empty() || !body.blocks.empty())
		return fail("function body already supplied");
	lowir::FunctionBuilder builder(program, symbol);
	if (input.temp_count < body.parameter_count)
		return fail("function body omits parameters");
	body.temps.resize(input.temp_count, lowir::Temp{ 0, lowir::Type::scalar(lowir::TypeKind::Void), lowir::kNoId });
	for (uint32_t i = 0; i < input.slot_count; ++i)
	{
		if (!valid_type(input.slots[i]) || type(input.slots[i]).is_void())
			return fail("invalid slot type");
		builder.add_slot(0, type(input.slots[i]));
	}
	for (uint32_t i = 0; i < input.block_count; ++i)
		builder.add_block(0);
	std::vector<uint8_t> placed(input.block_count, 0);
	for (uint32_t i = 0; i < input.block_count; ++i)
	{
		const uint32_t id = input.layout[i];
		if (id >= input.block_count || placed[id])
			return fail("block layout is not a permutation");
		placed[id] = 1;
		builder.place_block(id);
	}
	body.instructions.reserve(input.instruction_count);
	body.operands.reserve(input.operand_count);
	for (uint32_t i = 0; i < input.operand_count; ++i)
	{
		if (!valid_value(input.operands[i]))
			return fail("invalid operand kind");
		body.operands.push_back(value(input.operands[i]));
	}
	body.literals.reserve(input.literal_count);
	for (uint32_t i = 0; i < input.literal_count; ++i)
		body.literals.push_back(lowir::Literal{ input.literals[i].low, input.literals[i].high });
	for (uint32_t i = 0; i < input.instruction_count; ++i)
	{
		const CppgmInstruction& in = input.instructions[i];
		if (in.block >= input.block_count || in.op >= lowir::kOpcodeCount || !valid_type(in.type) || in.reserved != 0)
			return fail("invalid instruction record");
		lowir::Instruction inst = lowir::make_instruction(static_cast<lowir::Opcode>(in.op), type(in.type));
		inst.sub = in.sub;
		inst.flags = in.flags;
		inst.order = static_cast<lowir::MemoryOrder>(in.order);
		inst.extra_first = in.extra_first;
		inst.extra_count = in.extra_count;
		inst.immediate = in.immediate;
		if (inst.op == lowir::Opcode::Convert)
		{
			if (!valid_type(static_cast<CppgmType>(in.immediate)))
				return fail("invalid conversion source type");
			lowir::set_convert_source_type(inst, type(static_cast<CppgmType>(in.immediate)));
		}
		for (uint32_t j = 0; j < 3; ++j)
		{
			if (!valid_value(in.operands[j]))
				return fail("invalid operand kind");
			inst.ops[j] = value(in.operands[j]);
		}
		builder.set_insertion_block(in.block);
		const lowir::InstId id = builder.append(inst);
		const bool defines = lowir::defines_value(inst.op) && (inst.op != lowir::Opcode::Call || !inst.type.is_void());
		if (defines)
		{
			if (in.result < body.parameter_count || in.result >= input.temp_count ||
				body.temps[in.result].def != lowir::kNoId)
				return fail("invalid instruction result");
			builder.bind(in.result, lowir::instruction_result_type(inst, program.pointer_type()), id);
		}
		else if (in.result != CPPGM_NO_ID)
			return fail("effect instruction has a result");
	}
	return true;
}
