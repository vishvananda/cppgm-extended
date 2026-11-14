#include "wasm/selector.h"

namespace wasm {

using namespace lowir;

Selector::Place Selector::place_of(Value operand) const
{
	uint64_t offset = 0;
	// A Wasm memarg adds without wrapping at 32 bits. A displacement moves
	// into the memarg when its steps have the typed nonwrapping promise, or
	// when they stay in storage that is known: a plain step loses the promise
	// where it runs on a path that the source did not take.
	Value promised = operand;
	uint64_t promised_offset = 0;
	bool plain = false;
	for (unsigned depth = 0; depth < 16 && operand.is(ValueKind::Temp); ++depth)
	{
		const InstId id = body_.temps[operand.index].def;
		if (id == kNoId)
			break;
		const auto& def = body_.instructions[id];
		// A slot operand of copy is a load, not the slot's address.
		if (def.op == Opcode::Copy && def.type.is_pointer() && def.ops[0].is(ValueKind::Temp) &&
			body_.temps[def.ops[0].index].type.is_pointer())
		{
			operand = def.ops[0];
			continue;
		}
		if (def.op == Opcode::Addr && def.ops[0].is(ValueKind::Slot))
		{
			operand = def.ops[0];
			break;
		}
		if (def.op != Opcode::Index || !def.ops[1].is(ValueKind::Integer))
			break;
		const auto& literal = body_.literals[def.ops[1].index];
		const uint64_t scale = storage_bytes(def.type);
		if (scale == 0 || literal.high != 0 || literal.low > UINT32_MAX / scale)
			break;
		const uint64_t bytes = literal.low * scale;
		if (bytes > UINT32_MAX - offset)
			break;
		if (def.index_bounds() != IndexBounds::InBounds && !plain)
		{
			plain = true;
			promised = operand;
			promised_offset = offset;
		}
		offset += bytes;
		operand = def.ops[0];
	}
	// A slot and the object of a value in memory are in the frame: the
	// frame's address plus their offset does not wrap. A phi of such values
	// holds the address of one of its inputs.
	Place known{ Value::none(), kNoId, 0 };
	uint64_t storage = 0;
	if (operand.is(ValueKind::Slot))
	{
		known = slot_place(operand.index);
		storage = storage_bytes(body_.slots[operand.index].type);
	}
	else if (operand.is(ValueKind::Temp) && memory_value(body_.temps[operand.index].type) &&
		wide_locals_[operand.index] == kNoId)
	{
		const InstId id = body_.temps[operand.index].def;
		if (places_.slot_of[operand.index] != kNoId)
			known = slot_place(places_.slot_of[operand.index]);
		else if (objects_[operand.index] != kNoId && (id == kNoId || body_.instructions[id].op != Opcode::Phi))
			known = frame_place(objects_[operand.index]);
		else if (id == kNoId)
			// A parameter that passes through memory: its local is the address.
			known = Place{ Value::none(), locals_[operand.index], 0 };
		storage = storage_bytes(body_.temps[operand.index].type);
	}
	else if (operand.is(ValueKind::Temp) && operand.index < body_.parameter_count &&
		body_.temps[operand.index].type.is_pointer())
		// The bytes that are readable from a pointer parameter at entry.
		storage = program_.parameters_of(signature_)[operand.index].object_bytes;
	if (plain && offset > storage)
	{
		operand = promised;
		offset = promised_offset;
		known.local = kNoId;
	}
	if (known.local != kNoId && offset <= UINT32_MAX - known.offset)
		return Place{ Value::none(), known.local, static_cast<uint32_t>(offset + known.offset) };
	return Place{ operand, kNoId, static_cast<uint32_t>(offset) };
}

Selector::Place Selector::frame_place(uint32_t offset) const
{
	return Place{ Value::none(), frame_, offset };
}

Selector::Place Selector::slot_place(uint32_t slot) const
{
	return Place{ Value::none(), slot_locals_[slot] != kNoId ? slot_locals_[slot] : frame_, slots_[slot] };
}

// The slot that a by-value argument names when the callee takes the slot
// itself; kNoId when the callee takes a copy.
uint32_t Selector::argument_slot(Value source) const
{
	const auto address = opt::locate(body_, source);
	if (!address.root.is(ValueKind::Slot) || !address.known || address.offset != 0)
		return kNoId;
	return places_.argument[address.root.index] ? address.root.index : kNoId;
}

void Selector::place_base(const Place& place)
{
	if (place.local != kNoId)
		out_.emit(Op::LocalGet, { place.local });
	else
		address(place.base);
}

void Selector::place_address(const Place& place)
{
	place_base(place);
	if (place.offset != 0)
	{
		out_.emit(Op::I32Const, { place.offset });
		out_.emit(Op::I32Add);
	}
}

uint32_t Selector::memory_address(Value operand)
{
	const Place place = place_of(operand);
	place_base(place);
	return place.offset;
}

void Selector::instruction(const lowir::Instruction& inst)
{
	if ((exception_reference_ != kNoId && exception_instruction(inst)) || object_instruction(inst) ||
		wide_instruction(inst))
		return;
	switch (inst.op)
	{
	case Opcode::Asm:
		// Nonempty templates and operand/register effects are rejected before
		// selection. Empty compiler barriers retain their LowIR memory effects.
		return;
	case Opcode::Const:
	case Opcode::Copy:
	{
		value(inst.ops[0], inst.type);
		const Type actual = operand_type(inst.ops[0], inst.type);
		const ValueType source = value_type(memory_value(actual) ? inst.type : actual);
		const ValueType result = value_type(inst.type);
		if (inst.ops[0].is(ValueKind::Temp) && source != result)
		{
			if ((source == ValueType::I32 && result == ValueType::I64) ||
				(source == ValueType::I64 && result == ValueType::I32))
			{
				// Integer/pointer register-width views are normalized by value().
			}
			else if (source == ValueType::I32 && result == ValueType::F32)
				out_.emit(Op::F32ReinterpretI32);
			else if (source == ValueType::F32 && result == ValueType::I32)
				out_.emit(Op::I32ReinterpretF32);
			else if (source == ValueType::I64 && result == ValueType::F64)
				out_.emit(Op::F64ReinterpretI64);
			else if (source == ValueType::F64 && result == ValueType::I64)
				out_.emit(Op::I64ReinterpretF64);
			else
				throw WasmError("invalid Wasm bit-copy representation");
		}
		break;
	}
	case Opcode::Phi:
		return;
	case Opcode::Vector:
		vector_instruction(inst);
		return;
	case Opcode::AtomicLoad:
	case Opcode::AtomicStore:
	case Opcode::AtomicAddFetch:
	case Opcode::AtomicExchange:
	case Opcode::AtomicCompareExchange:
	case Opcode::AtomicThreadFence:
	case Opcode::AtomicSignalFence:
		atomic_instruction(inst);
		return;
	case Opcode::Addr:
		address(inst.ops[0]);
		break;
	case Opcode::Load:
	{
		const uint32_t offset = memory_address(inst.ops[0]);
		out_.emit(memory_op(inst.type, false), { 0, offset });
		// A load of 8 or 16 bits extends to the view of its type.
		const unsigned bits = integer_bits(inst.type.kind());
		if ((bits == 8 || bits == 16) && body_.temps[inst.result].type == inst.type)
		{
			out_.emit(Op::LocalSet, { locals_[inst.result] });
			return;
		}
		break;
	}
	case Opcode::Store:
	{
		const uint32_t offset = memory_address(inst.ops[1]);
		value(inst.ops[0], inst.type);
		out_.emit(memory_op(inst.type, true), { 0, offset });
		return;
	}
	case Opcode::Index:
	{
		address(inst.ops[0]);
		const uint64_t bytes = storage_bytes(inst.type);
		if (inst.ops[1].is(ValueKind::Integer))
		{
			// A literal step is one constant: the product wraps as the address does.
			const auto step = static_cast<int32_t>(
				static_cast<uint32_t>(body_.literals[inst.ops[1].index].low * bytes));
			if (step != 0)
			{
				out_.emit(Op::I32Const, { static_cast<uint64_t>(static_cast<int64_t>(step)) });
				out_.emit(Op::I32Add);
			}
			break;
		}
		value(inst.ops[1], Type::scalar(TypeKind::U32));
		if (bytes != 1)
		{
			out_.emit(Op::I32Const, { bytes });
			out_.emit(Op::I32Mul);
		}
		out_.emit(Op::I32Add);
		break;
	}
	case Opcode::Unary:
		unary(inst);
		break;
	case Opcode::Binary:
		binary(inst);
		break;
	case Opcode::Compare:
		compare(inst);
		break;
	case Opcode::Convert:
		convert(inst);
		break;
	case Opcode::Select:
		value(inst.ops[1], inst.type);
		value(inst.ops[2], inst.type);
		condition(inst.ops[0]);
		out_.emit(Op::Select);
		break;
	case Opcode::Call:
		call(inst);
		return;
	case Opcode::VaStart:
		variadic_start(inst);
		return;
	case Opcode::VaArg:
		variadic_argument(inst);
		return;
	case Opcode::Jump:
		edge(current_block_, inst.ops[0].index);
		return;
	case Opcode::Branch:
		condition(inst.ops[0]);
		out_.emit(Op::If, { block_type(ValueType::Void) });
		++nesting_;
		edge(current_block_, inst.ops[1].index);
		--nesting_;
		out_.emit(Op::End);
		edge(current_block_, inst.ops[2].index);
		return;
	case Opcode::Switch:
	{
		if (select_switch(inst))
			return;
		const Type type = operand_type(inst.ops[0], Type::scalar(TypeKind::I64));
		for (uint32_t i = 0; i < inst.extra_count; i += 2)
		{
			value(inst.ops[0], type);
			value(body_.operands[inst.extra_first + i], type);
			out_.emit(value_type(type) == ValueType::I64 ? Op::I64Eq : Op::I32Eq);
			out_.emit(Op::If, { block_type(ValueType::Void) });
			++nesting_;
			edge(current_block_, body_.operands[inst.extra_first + i + 1].index);
			--nesting_;
			out_.emit(Op::End);
		}
		edge(current_block_, inst.ops[1].index);
		return;
	}
	case Opcode::Return:
		return_value(inst);
		return;
	case Opcode::Unreachable:
		out_.emit(Op::Unreachable);
		return;
	case Opcode::CopyObject:
		copy_bytes(place_of(inst.ops[1]), place_of(inst.ops[0]), static_cast<uint64_t>(inst.immediate));
		return;
	case Opcode::ZeroInit:
		zero_bytes(place_of(inst.ops[0]), static_cast<uint64_t>(inst.immediate));
		return;
	case Opcode::StackSave:
		out_.emit(Op::GlobalGet, { 0 });
		out_.relocate(RelocationKind::GlobalIndex, module_.stack_symbol);
		break;
	case Opcode::StackRestore:
		value(inst.ops[0], program_.pointer_type());
		out_.emit(Op::GlobalSet, { 0 });
		out_.relocate(RelocationKind::GlobalIndex, module_.stack_symbol);
		return;
	case Opcode::StackAlloc:
	{
		out_.emit(Op::GlobalGet, { 0 });
		out_.relocate(RelocationKind::GlobalIndex, module_.stack_symbol);
		const Type size_type = operand_type(inst.ops[0], Type::scalar(TypeKind::I64));
		value(inst.ops[0], size_type);
		if (value_type(size_type) == ValueType::I64)
			out_.emit(Op::I32WrapI64);
		out_.emit(Op::I32Sub);
		out_.emit(Op::I32Const, { static_cast<uint64_t>(int64_t(-16)) });
		out_.emit(Op::I32And);
		out_.emit(Op::GlobalSet, { 0 });
		out_.relocate(RelocationKind::GlobalIndex, module_.stack_symbol);
		out_.emit(Op::GlobalGet, { 0 });
		out_.relocate(RelocationKind::GlobalIndex, module_.stack_symbol);
		break;
	}
	default:
		throw WasmError("unsupported operation in this Wasm profile: " + std::string(spelling(inst.op)));
	}
	const Type result = body_.temps[inst.result].type;
	normalize(result);
	out_.emit(Op::LocalSet, { locals_[inst.result] });
}

} // namespace wasm
