#include "lowir/builder.h"

namespace lowir {

Instruction make_instruction(Opcode op, Type type)
{
	Instruction inst;
	inst.op = op;
	inst.sub = 0;
	inst.flags = 0;
	inst.order = MemoryOrder::Relaxed;
	inst.result = kNoId;
	inst.next = kNoId;
	inst.type = type;
	inst.ops[0] = inst.ops[1] = inst.ops[2] = Value::none();
	inst.extra_first = 0;
	inst.extra_count = 0;
	inst.immediate = 0;
	inst.location = 0;
	return inst;
}

Literal integer_literal(int64_t value)
{
	Literal bits;
	bits.low = static_cast<uint64_t>(value);
	bits.high = value < 0 ? ~static_cast<uint64_t>(0) : 0;
	return bits;
}

FunctionBuilder::FunctionBuilder(Program& program, SymbolId function)
	: program_(program), body_(*program.functions[program.symbols[function].detail].body), insertion_(kNoId),
	  current_(DebugLocation{ 0, 0, 0 }), current_index_(0)
{
	const Function& definition = program.functions[program.symbols[function].detail];
	const Signature& signature = program.signatures[definition.signature];
	if (body_.temps.empty())
	{
		const Parameter* parameters = program.parameters_of(signature);
		body_.parameter_count = signature.parameter_count;
		for (uint32_t i = 0; i < signature.parameter_count; ++i)
			body_.temps.push_back(Temp{ parameters[i].name, parameters[i].type, kNoId });
	}
}

SlotId FunctionBuilder::add_slot(NameId name, Type type, SlotKind kind, uint16_t index)
{
	body_.slots.push_back(Slot{ name, kind, 0, index, type });
	return static_cast<SlotId>(body_.slots.size() - 1);
}

BlockId FunctionBuilder::add_block(NameId name, BlockKind kind)
{
	body_.blocks.push_back(Block{ name, kNoId, kNoId, kind });
	return static_cast<BlockId>(body_.blocks.size() - 1);
}

TempId FunctionBuilder::add_temp(NameId name)
{
	body_.temps.push_back(Temp{ name, Type::scalar(TypeKind::Void), kNoId });
	return static_cast<TempId>(body_.temps.size() - 1);
}

void FunctionBuilder::bind(TempId temp, Type type, InstId inst)
{
	body_.temps[temp].type = type;
	body_.temps[temp].def = inst;
	body_.instructions[inst].result = temp;
}

Value FunctionBuilder::literal(ValueKind kind, TypeKind float_kind, Literal bits)
{
	body_.literals.push_back(bits);
	Value value = Value::make(kind, static_cast<uint32_t>(body_.literals.size() - 1));
	value.float_kind = float_kind;
	return value;
}

uint32_t FunctionBuilder::add_operands(const Value* values, uint32_t count)
{
	const uint32_t first = static_cast<uint32_t>(body_.operands.size());
	body_.operands.insert(body_.operands.end(), values, values + count);
	return first;
}

InstId FunctionBuilder::append(const Instruction& inst)
{
	const InstId id = static_cast<InstId>(body_.instructions.size());
	body_.instructions.push_back(inst);
	if (current_.file != 0)
		locate(id);
	Block& block = body_.blocks[insertion_];
	if (block.last == kNoId)
		block.first = id;
	else
		body_.instructions[block.last].next = id;
	block.last = id;
	return id;
}

void FunctionBuilder::locate(InstId inst)
{
	if (body_.instructions[inst].location != 0)
		return;
	if (current_index_ == 0)
	{
		body_.locations.push_back(current_);
		current_index_ = static_cast<uint32_t>(body_.locations.size());
	}
	body_.instructions[inst].location = current_index_;
}

InstId FunctionBuilder::insert_after(BlockId block_id, InstId after, const Instruction& inst)
{
	const InstId id = static_cast<InstId>(body_.instructions.size());
	body_.instructions.push_back(inst);
	if (current_.file != 0)
		locate(id);
	Block& block = body_.blocks[block_id];
	if (after == kNoId)
	{
		body_.instructions[id].next = block.first;
		block.first = id;
		if (block.last == kNoId)
			block.last = id;
		return id;
	}
	body_.instructions[id].next = body_.instructions[after].next;
	body_.instructions[after].next = id;
	if (block.last == after)
		block.last = id;
	return id;
}

TempId FunctionBuilder::append_value(Instruction inst, Type result_type, NameId name)
{
	const InstId id = append(inst);
	const TempId temp = add_temp(name);
	bind(temp, result_type, id);
	return temp;
}

TempId FunctionBuilder::binary(BinaryOp op, Type type, Value lhs, Value rhs, NameId name)
{
	Instruction inst = make_instruction(Opcode::Binary, type);
	inst.sub = static_cast<uint8_t>(op);
	inst.ops[0] = lhs;
	inst.ops[1] = rhs;
	return append_value(inst, type, name);
}

TempId FunctionBuilder::load(Type type, Value storage, NameId name)
{
	Instruction inst = make_instruction(Opcode::Load, type);
	inst.ops[0] = storage;
	return append_value(inst, type, name);
}

void FunctionBuilder::store(Type type, Value value, Value storage)
{
	Instruction inst = make_instruction(Opcode::Store, type);
	inst.ops[0] = value;
	inst.ops[1] = storage;
	append(inst);
}

TempId FunctionBuilder::call(
	Type result, Value callee, const Value* args, uint32_t count, SignatureId signature, NameId name, uint16_t flags)
{
	Instruction inst = make_instruction(Opcode::Call, result);
	inst.flags |= flags;
	inst.ops[0] = callee;
	inst.extra_first = add_operands(args, count);
	inst.extra_count = count;
	if (signature != kNoId)
	{
		inst.flags |= IF_SIGNATURE;
		inst.immediate = signature;
	}
	if (result.is_void())
	{
		append(inst);
		return kNoId;
	}
	return append_value(inst, result, name);
}

void FunctionBuilder::jump(BlockId target)
{
	Instruction inst = make_instruction(Opcode::Jump, Type::scalar(TypeKind::Void));
	inst.ops[0] = Value::block(target);
	append(inst);
}

void FunctionBuilder::branch(Value condition, BlockId if_true, BlockId if_false)
{
	Instruction inst = make_instruction(Opcode::Branch, Type::scalar(TypeKind::Void));
	inst.ops[0] = condition;
	inst.ops[1] = Value::block(if_true);
	inst.ops[2] = Value::block(if_false);
	append(inst);
}

void FunctionBuilder::ret(Type type, Value value)
{
	Instruction inst = make_instruction(Opcode::Return, type);
	inst.ops[0] = value;
	append(inst);
}

void FunctionBuilder::set_location(InstId inst, DebugLocation location)
{
	body_.locations.push_back(location);
	body_.instructions[inst].location = static_cast<uint32_t>(body_.locations.size());
}

void FunctionBuilder::finish()
{
	body_.locations.shrink_to_fit();
	body_.temps.shrink_to_fit();
	body_.slots.shrink_to_fit();
	body_.blocks.shrink_to_fit();
	body_.layout.shrink_to_fit();
	body_.instructions.shrink_to_fit();
	body_.operands.shrink_to_fit();
	body_.literals.shrink_to_fit();
}

} // namespace lowir
