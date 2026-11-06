#include "lowir/builder.h"
#include "opt/passes.h"

namespace opt {

using namespace lowir;

namespace {

// Storage operands name the slot itself; value operands read its current
// contents. Make the latter explicit before passes treat values as SSA.
unsigned value_slots(Opcode op)
{
	switch (op)
	{
	case Opcode::Copy:
	case Opcode::Unary:
	case Opcode::Convert:
	case Opcode::Store:
	case Opcode::AtomicStore:
	case Opcode::StackAlloc:
	case Opcode::Throw:
	case Opcode::Branch:
	case Opcode::Switch:
	case Opcode::Return:
		return 1;
	case Opcode::Binary:
	case Opcode::Compare:
		return 3;
	case Opcode::Select:
	case Opcode::Vector:
		return 7;
	case Opcode::Index:
	case Opcode::AtomicAddFetch:
	case Opcode::AtomicExchange:
		return 2;
	default:
		return 0;
	}
}

Type read_type(const FunctionBody& body, const Instruction& inst, unsigned operand)
{
	if (inst.op == Opcode::Convert)
		return convert_source_type(inst);
	if (inst.op == Opcode::Branch || (inst.op == Opcode::Select && operand == 0) || inst.op == Opcode::Vector)
		return body.slots[inst.ops[operand].index].type;
	if (inst.op == Opcode::Binary && operand == 1 &&
		(inst.sub == static_cast<unsigned>(BinaryOp::Shl) || inst.sub == static_cast<unsigned>(BinaryOp::Shr) ||
			inst.sub == static_cast<unsigned>(BinaryOp::UShr)))
		return Type::scalar(TypeKind::I64);
	if (inst.op == Opcode::Index || inst.op == Opcode::StackAlloc || inst.op == Opcode::Switch)
		return Type::scalar(TypeKind::I64);
	return inst.type;
}

} // namespace

Value materialize_slot_read(FunctionBody& body, BlockId block, InstId& prev, Value slot, Type type, uint32_t location)
{
	Instruction load = make_instruction(Opcode::Load, type);
	load.ops[0] = slot;
	load.location = location;
	const InstId id = append_instruction(body, load);
	const TempId result = append_temp(body, type, id);
	body.instructions[id].result = result;
	insert_after(body, block, prev, id);
	prev = id;
	return Value::temp(result);
}

void materialize_slot_values(FunctionBody& body, BlockId block, InstId& prev, InstId id)
{
	const unsigned mask = value_slots(body.instructions[id].op);
	for (unsigned k = 0; k < 3; ++k)
	{
		const Value value = body.instructions[id].ops[k];
		if ((mask & (1u << k)) == 0 || !value.is(ValueKind::Slot))
			continue;
		const Type type = read_type(body, body.instructions[id], k);
		// Object operands already denote addressable aggregate storage.
		if (type.is_object())
			continue;
		const uint32_t location = body.instructions[id].location;
		const Value loaded = materialize_slot_read(body, block, prev, value, type, location);
		// Appending the load can move the instruction arena.
		body.instructions[id].ops[k] = loaded;
	}
}

} // namespace opt
