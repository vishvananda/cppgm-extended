#include "opt/packed_loads.h"

#include "lowir/builder.h"

namespace opt {

using namespace lowir;

bool simplify_packed_load(FunctionBody& body, const std::vector<uint32_t>& uses, const std::vector<BlockId>& block_of,
	BlockId block, InstId& previous, InstId id)
{
	const Instruction original = body.instructions[id];
	const Type result = original.type;
	const bool floating = original.op == Opcode::Copy &&
		(result.kind() == TypeKind::F32 || result.kind() == TypeKind::F64);
	if (!floating)
		return false;
	const auto single = [&](Value value) -> const Instruction* {
		if (!value.is(ValueKind::Temp) || value.index < body.parameter_count || value.index >= uses.size() ||
			uses[value.index] != 1)
			return nullptr;
		const InstId def = body.temps[value.index].def;
		return def != kNoId && def < block_of.size() && block_of[def] == block ? &body.instructions[def] : nullptr;
	};
	Value value = original.ops[0];
	if (!value.is(ValueKind::Temp))
		return false;
	Type carrier = body.temps[value.index].type;
	if (!float_twins(result, carrier))
		return false;
	const Instruction* source = single(value);
	if (source == nullptr || source->op != Opcode::Convert || static_cast<ConvertOp>(source->sub) != ConvertOp::Trunc)
		return false;
	carrier = convert_source_type(*source);
	value = source->ops[0];
	source = single(value);
	if (!carrier.is_integer() || carrier.kind() == TypeKind::I1 || storage_bytes(carrier) <= storage_bytes(result))
		return false;
	uint64_t offset = 0;
	if (source != nullptr && source->op == Opcode::Binary && source->type == carrier &&
		(static_cast<BinaryOp>(source->sub) == BinaryOp::UShr || static_cast<BinaryOp>(source->sub) == BinaryOp::Shr))
	{
		const Value count = source->ops[1];
		if (!count.is(ValueKind::Integer))
			return false;
		const Literal bits = body.literals[count.index];
		if (bits.high != 0 || bits.low >= 8 * storage_bytes(carrier) || bits.low % 8 != 0)
			return false;
		offset = bits.low / 8;
		value = source->ops[0];
		source = single(value);
	}
	if (source == nullptr || source->op != Opcode::Load || source->type != carrier ||
		(source->flags & IF_VOLATILE) != 0 || offset + storage_bytes(result) > storage_bytes(carrier))
		return false;
	const Instruction load = *source;
	// Move the read only across a short sequence of pure computations, in
	// the same block. Stores, calls, volatile/atomic accesses, asm and EH
	// boundaries retain the original snapshot, even if they cannot alias.
	InstId next = load.next;
	for (unsigned steps = 0; next != id; ++steps)
	{
		if (next == kNoId || steps == 16 || !is_pure(body.instructions[next]))
			return false;
		next = body.instructions[next].next;
	}
	Value address = load.ops[0];
	const auto emit_address = [&](Instruction inst) {
		inst.location = load.location;
		const InstId added = append_instruction(body, inst);
		const TempId temp = append_temp(body, Type::scalar(TypeKind::Ptr), added);
		body.instructions[added].result = temp;
		insert_after(body, block, previous, added);
		previous = added;
		return Value::temp(temp);
	};
	if (offset != 0)
	{
		if (address.is(ValueKind::Slot))
		{
			Instruction addr = make_instruction(Opcode::Addr, Type::scalar(TypeKind::Ptr));
			addr.ops[0] = address;
			address = emit_address(addr);
		}
		Instruction index = make_instruction(Opcode::Index, Type::scalar(TypeKind::I8));
		index.ops[0] = address;
		index.ops[1] = integer_value(body, integer_literal(static_cast<int64_t>(offset)));
		address = emit_address(index);
	}
	Instruction narrowed = make_instruction(Opcode::Load, result);
	narrowed.result = original.result;
	narrowed.next = original.next;
	narrowed.location = original.location;
	narrowed.ops[0] = address;
	// A bit projection establishes no source-language alias type. Retaining
	// the wide access's `typed` promise for the new type would be unsound.
	body.instructions[id] = narrowed;
	return true;
}

} // namespace opt
