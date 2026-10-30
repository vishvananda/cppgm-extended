// GNU asm statements consume the instructions analysis lifted once, with
// register operands held in slots and memory operands addressed at each use.
#include "lower/function_lowering.h"
#include "sema/asm_statements.h"
#include "sema/expression_facts.h"
#include "support/internal_error.h"

namespace lower {
namespace {
lowir::Type AsmType(uint32_t bytes)
{
	return lowir::Type::scalar(bytes == 16 ? lowir::TypeKind::I128
			: bytes == 1                   ? lowir::TypeKind::I8
			: bytes == 2                   ? lowir::TypeKind::I16
			: bytes == 4                   ? lowir::TypeKind::I32
										   : lowir::TypeKind::I64);
}
}

void FunctionLowering::asm_bind_operands(const syntax::AsmStmt& stmt, AsmState& state)
{
	const sema::AsmProgram* program = stmt.program;
	const uint32_t count = stmt.outputs.size + stmt.inputs.size;
	state.addresses.resize(count);
	state.homes.resize(count);
	state.values.resize(count);
	state.types.resize(count);
	state.widths.resize(count);
	state.volatiles.resize(count);
	state.outputs = stmt.outputs.size;
	state.registers = program->registers;
	for (uint32_t i = 0; i < count; ++i)
	{
		const bool output = i < stmt.outputs.size;
		const syntax::Expr& expr = *(output ? stmt.outputs[i] : stmt.inputs[i - stmt.outputs.size]).expression;
		if (!output && (program->immediates & (1u << i)))
			continue;
		state.widths[i] = static_cast<uint32_t>(lowering_.size_of(expr.facts->type, stmt.location));
		state.volatiles[i] = stmt.is_volatile || (expr.facts->type->cv & sema::kVolatile) != 0;
		state.types[i] = lowering_.storage_type(expr.facts->type->unqualified);
		const size_t first = cleanups_.size();
		const bool region = begin_full_expression(&expr, true);
		if (output || expr.facts->category == sema::ValueCategory::LValue)
			state.addresses[i] = address(expr);
		else
			state.values[i] = operand(expr, ConversionContext::Argument);
		if (!output && expr.facts->category == sema::ValueCategory::LValue &&
			state.types[i].kind() != lowir::TypeKind::Obj && (program->registers & (1u << i)))
		{
			Place place = Place();
			place.storage = state.addresses[i];
			state.values[i] = load(state.types[i], place);
		}
		destroy_temporaries(first);
		end_full_expression(region);
	}
	for (uint32_t i = 0; i < stmt.outputs.size; ++i)
	{
		if (!(program->registers & (1u << i)))
			continue;
		Place object = Place();
		object.storage = state.addresses[i];
		Place home = Place();
		home.storage = lowir::Value::slot(generated_slot(lowir::SlotKind::Asm, state.types[i]));
		// Its object's value, or a matching input's (its bits: same size).
		const uint32_t tied = program->tied != nullptr ? program->tied[i] : 0xff;
		if (tied == 0xff && (program->reads & (1u << i)))
			store(state.types[i], load(state.types[i], object), home);
		else if (tied != 0xff && state.values[tied].value.kind != lowir::ValueKind::None)
			store(state.values[tied].type, state.values[tied], home);
		else if (tied != 0xff)
		{
			Place input = Place();
			input.storage = state.addresses[tied];
			store(state.types[tied], load(state.types[tied], input), home);
		}
		state.homes[i] = address_of(home);
	}
}

Place FunctionLowering::asm_place(const AsmState& state, uint32_t index)
{
	Place place = Place();
	const bool reg = state.homes[index].kind != lowir::ValueKind::None;
	place.storage = reg ? state.homes[index] : state.addresses[index];
	place.untyped = true;
	place.is_volatile = !reg && state.volatiles[index];
	return place;
}

void FunctionLowering::asm_store(const AsmState& state, uint32_t index, RValue value)
{
	// A 32-bit GPR write clears the high half of its 64-bit register.
	if ((state.registers & (1u << index)) && state.widths[index] >= 8 && value.type.kind() == lowir::TypeKind::I32)
		value = convert_op(lowir::ConvertOp::ZExt, AsmType(8), value);
	store(value.type, value, asm_place(state, index));
}

RValue FunctionLowering::asm_source(const AsmState& state, const sema::AsmInstruction& instruction)
{
	const lowir::Type type = AsmType(instruction.source_bytes != 0 ? instruction.source_bytes : instruction.bytes);
	if (instruction.source == sema::kAsmImmediateSource)
		return integer(type, instruction.immediate);
	const uint32_t source = instruction.source;
	if (source < state.outputs || !(state.registers & (1u << source)) ||
		state.values[source].value.kind == lowir::ValueKind::None)
		return load(type, asm_place(state, source));
	if (state.values[source].type == type)
		return state.values[source];
	Place bits = Place();
	bits.storage = lowir::Value::slot(generated_slot(lowir::SlotKind::Asm, state.values[source].type));
	bits.untyped = true;
	store(state.values[source].type, state.values[source], bits);
	bits.storage = address_of(bits);
	return load(type, bits);
}

void FunctionLowering::asm_capture_flags(const sema::AsmInstruction& instruction, AsmState& state, const RValue& before,
	const RValue& after, const RValue& source)
{
	// Analysis marks only flags with a consumer. Arithmetic flags are facts
	// about these values, independent of the native instruction selection.
	if ((instruction.flags & sema::kAsmCaptureZero) != 0)
		state.zero = compare(lowir::ComparePredicate::Eq, after.type, after, integer(after.type, 0));
	if ((instruction.flags & sema::kAsmCaptureCarry) == 0)
		return;
	if (instruction.operation == sema::AsmOperation::Add)
		state.carry = compare(lowir::ComparePredicate::Ult, after.type, after, before);
	else if (instruction.operation == sema::AsmOperation::Sub)
		state.carry = compare(lowir::ComparePredicate::Ult, before.type, before, source);
	else if (instruction.operation == sema::AsmOperation::Neg)
		state.carry = compare(lowir::ComparePredicate::Ne, before.type, before, integer(before.type, 0));
	else
		state.carry = integer(lowir::Type::scalar(lowir::TypeKind::I64), 0);
}

void FunctionLowering::asm_set_flag(const sema::AsmInstruction& instruction, AsmState& state)
{
	const bool carry = instruction.operation == sema::AsmOperation::SetCarry ||
		instruction.operation == sema::AsmOperation::SetNotCarry;
	RValue value = carry ? state.carry : state.zero;
	if (value.value.kind == lowir::ValueKind::None)
		throw InternalError("an asm condition without a lowered producer");
	if (instruction.operation == sema::AsmOperation::SetNotCarry ||
		instruction.operation == sema::AsmOperation::SetNotZero)
		value = compare(lowir::ComparePredicate::Eq, value.type, value, integer(value.type, 0));
	const lowir::Type type = AsmType(instruction.bytes);
	if (type != value.type)
		value = convert_op(instruction.bytes > 8 ? lowir::ConvertOp::ZExt : lowir::ConvertOp::Trunc, type, value);
	asm_store(state, instruction.destination, value);
}

void FunctionLowering::asm_bit_instruction(
	const sema::AsmInstruction& instruction, AsmState& state, const RValue& source)
{
	const lowir::Type type = AsmType(instruction.bytes);
	Place target = asm_place(state, instruction.destination);
	const bool memory = (state.registers & (1u << instruction.destination)) == 0;
	const uint32_t bits = instruction.bytes * 8;
	if (memory && instruction.source != sema::kAsmImmediateSource)
	{
		// Register indices address a signed bit string. An immediate instead
		// selects a bit in this word. No C++ in-bounds fact is asserted here.
		const lowir::Type i64 = AsmType(8);
		const RValue signed_index = source.type == i64 ? source : convert_op(lowir::ConvertOp::SExt, i64, source);
		const uint32_t shift = instruction.bytes == 2 ? 4 : instruction.bytes == 4 ? 5 : 6;
		const RValue word = binary(lowir::BinaryOp::Shr, i64, signed_index, integer(i64, shift));
		target.storage = index(type, ir_model::IPK_NONE, target.storage, word).value;
	}
	const RValue bit = binary(lowir::BinaryOp::And, type, source, integer(type, bits - 1));
	const RValue mask = binary(lowir::BinaryOp::Shl, type, integer(type, 1), bit);
	const bool reset = instruction.operation == sema::AsmOperation::BitReset;
	const RValue update = reset ? unary(lowir::UnaryOp::BitNot, mask) : mask;
	RValue previous;
	if (instruction.locked)
	{
		const sema::Intrinsic kind = instruction.operation == sema::AsmOperation::BitSet
			? sema::Intrinsic::AtomicFetchOr
			: reset ? sema::Intrinsic::AtomicFetchAnd
					: sema::Intrinsic::AtomicFetchXor;
		previous = atomic_update(kind, type, target.storage, update, lowir::MemoryOrder::SeqCst);
	}
	else
	{
		previous = load(type, target);
		const lowir::BinaryOp op = instruction.operation == sema::AsmOperation::BitSet ? lowir::BinaryOp::Or
			: reset                                                                    ? lowir::BinaryOp::And
																					   : lowir::BinaryOp::Xor;
		const RValue updated = binary(op, type, previous, update);
		if (memory)
			store(type, updated, target);
		else
			asm_store(state, instruction.destination, updated);
	}
	if ((instruction.flags & sema::kAsmCaptureCarry) != 0)
		state.carry = compare(
			lowir::ComparePredicate::Ne, type, binary(lowir::BinaryOp::And, type, previous, mask), integer(type, 0));
}

void FunctionLowering::asm_instruction(const sema::AsmInstruction& instruction, AsmState& state)
{
	if (instruction.operation >= sema::AsmOperation::SetCarry &&
		instruction.operation <= sema::AsmOperation::SetNotZero)
	{
		asm_set_flag(instruction, state);
		return;
	}
	const lowir::Type type = AsmType(instruction.bytes);
	const Place target = asm_place(state, instruction.destination);
	RValue source = asm_source(state, instruction);
	if (instruction.operation == sema::AsmOperation::BitSet || instruction.operation == sema::AsmOperation::BitReset ||
		instruction.operation == sema::AsmOperation::BitComplement)
	{
		asm_bit_instruction(instruction, state, source);
		return;
	}
	if (instruction.operation == sema::AsmOperation::Move)
	{
		if (source.type != type)
			source = convert_op(lowir::ConvertOp::ZExt, type, source);
		asm_store(state, instruction.destination, source);
		return;
	}
	if (instruction.operation == sema::AsmOperation::CompareExchange)
	{
		RValue expected = RValue();
		if ((instruction.flags & sema::kAsmCaptureCarry) != 0)
			expected = load(type, asm_place(state, instruction.accumulator));
		lowir::Instruction inst = lowir::make_instruction(lowir::Opcode::AtomicCompareExchange, type);
		inst.ops[0] = state.addresses[instruction.destination];
		inst.ops[1] = state.homes[instruction.accumulator];
		inst.ops[2] = source.value;
		inst.order = lowir::MemoryOrder::SeqCst;
		inst.sub = static_cast<uint8_t>(lowir::MemoryOrder::SeqCst);
		RValue exchanged;
		exchanged.type = lowir::Type::scalar(lowir::TypeKind::I64);
		exchanged.value = emit_value(inst, exchanged.type);
		if ((instruction.flags & sema::kAsmCaptureZero) != 0)
			state.zero = exchanged;
		if ((instruction.flags & sema::kAsmCaptureCarry) != 0)
			state.carry = compare(
				lowir::ComparePredicate::Ult, type, expected, load(type, asm_place(state, instruction.accumulator)));
		return;
	}
	if (instruction.operation == sema::AsmOperation::Not)
		source = integer(type, ~uint64_t(0));
	if (instruction.operation == sema::AsmOperation::Exchange)
	{
		RValue previous;
		if (instruction.locked)
		{
			lowir::Instruction inst = lowir::make_instruction(lowir::Opcode::AtomicExchange, type);
			inst.ops[0] = state.addresses[instruction.destination];
			inst.ops[1] = source.value;
			inst.order = lowir::MemoryOrder::SeqCst;
			previous.value = emit_value(inst, type);
			previous.type = type;
		}
		else
		{
			previous = load(type, target);
			asm_store(state, instruction.destination, source);
		}
		asm_store(state, instruction.source, previous);
		return;
	}
	if (instruction.locked && instruction.operation == sema::AsmOperation::Neg)
	{
		// A locked negation: a compare-exchange loop (no single LowIR
		// operation negates memory).
		AtomicLoop loop;
		loop.bits = type;
		const RValue old = begin_atomic_loop(target, type, loop);
		const RValue updated = unary(lowir::UnaryOp::Neg, old);
		end_atomic_loop(target, type, loop, updated);
		asm_capture_flags(instruction, state, old, updated, source);
		return;
	}
	if (instruction.locked)
	{
		static const sema::Intrinsic kUpdates[] = { sema::Intrinsic::AtomicAddFetch, sema::Intrinsic::AtomicSubFetch,
			sema::Intrinsic::AtomicAndFetch, sema::Intrinsic::AtomicOrFetch, sema::Intrinsic::AtomicXorFetch,
			sema::Intrinsic::AtomicXorFetch };
		const RValue updated = atomic_update(
			kUpdates[static_cast<uint8_t>(instruction.operation) - static_cast<uint8_t>(sema::AsmOperation::Add)], type,
			state.addresses[instruction.destination], source, lowir::MemoryOrder::SeqCst);
		RValue before = updated;
		if ((instruction.flags & sema::kAsmCaptureCarry) != 0 &&
			(instruction.operation == sema::AsmOperation::Add || instruction.operation == sema::AsmOperation::Sub))
			before = binary(
				instruction.operation == sema::AsmOperation::Add ? lowir::BinaryOp::Sub : lowir::BinaryOp::Add, type,
				updated, source);
		asm_capture_flags(instruction, state, before, updated, source);
		return;
	}
	const RValue value = load(type, target);
	RValue result;
	switch (instruction.operation)
	{
	case sema::AsmOperation::Add:
		result = binary(lowir::BinaryOp::Add, type, value, source);
		break;
	case sema::AsmOperation::Sub:
		result = binary(lowir::BinaryOp::Sub, type, value, source);
		break;
	case sema::AsmOperation::And:
	case sema::AsmOperation::PackedAnd:
		result = binary(lowir::BinaryOp::And, type, value, source);
		break;
	case sema::AsmOperation::Or:
		result = binary(lowir::BinaryOp::Or, type, value, source);
		break;
	case sema::AsmOperation::Xor:
	case sema::AsmOperation::Not:
		result = binary(lowir::BinaryOp::Xor, type, value, source);
		break;
	case sema::AsmOperation::Neg:
		result = unary(lowir::UnaryOp::Neg, value);
		break;
	default:
		result = unary(lowir::UnaryOp::ByteSwap, value);
		break;
	}
	asm_store(state, instruction.destination, result);
	asm_capture_flags(instruction, state, value, result, source);
}

void FunctionLowering::asm_statement(const syntax::AsmStmt& stmt)
{
	const sema::AsmProgram* program = stmt.program;
	if (program == nullptr)
		throw LoweringError(stmt.location, "an asm statement the analysis did not lift");
	if (program->unlifted != nullptr)
		throw LoweringError(stmt.location, program->unlifted);
	AsmState state = AsmState();
	asm_bind_operands(stmt, state);
	std::vector<lowir::BlockId> labels(program->count, lowir::kNoId);
	for (uint32_t n = 0; n < program->count; ++n)
	{
		const sema::AsmInstruction& instruction = program->instructions[n];
		if (instruction.operation == sema::AsmOperation::JumpZero ||
			instruction.operation == sema::AsmOperation::JumpNotZero)
		{
			if (instruction.immediate > n)
				throw InternalError("an asm branch without a backward target", stmt.location);
			if (labels[instruction.immediate] == lowir::kNoId)
				labels[instruction.immediate] = new_block(lowir::BlockKind::Plain);
		}
	}
	for (uint32_t n = 0; n < program->count; ++n)
	{
		if (labels[n] != lowir::kNoId)
		{
			jump(labels[n]);
			place(labels[n]);
		}
		const sema::AsmInstruction& instruction = program->instructions[n];
		if (instruction.operation == sema::AsmOperation::Fence)
		{
			lowir::Instruction inst = lowir::make_instruction(
				lowir::Opcode::AtomicThreadFence, lowir::Type::scalar(lowir::TypeKind::Void));
			inst.order = lowir::MemoryOrder::SeqCst;
			emit(inst);
		}
		else if (instruction.operation == sema::AsmOperation::JumpZero ||
			instruction.operation == sema::AsmOperation::JumpNotZero)
		{
			const lowir::BlockId next = new_block(lowir::BlockKind::Plain);
			if (instruction.operation == sema::AsmOperation::JumpZero)
				branch(state.zero, labels[instruction.immediate], next);
			else
				branch(state.zero, next, labels[instruction.immediate]);
			place(next);
		}
		else
			asm_instruction(instruction, state);
	}
	for (uint32_t i = 0; i < stmt.outputs.size; ++i)
	{
		if (state.homes[i].kind == lowir::ValueKind::None)
			continue;
		Place home = Place();
		home.storage = state.homes[i];
		Place object = Place();
		object.storage = state.addresses[i];
		store(state.types[i], load(state.types[i], home), object);
	}
	if (program->barrier)
	{
		lowir::Instruction inst = lowir::make_instruction(
			lowir::Opcode::AtomicSignalFence, lowir::Type::scalar(lowir::TypeKind::Void));
		inst.order = lowir::MemoryOrder::SeqCst;
		emit(inst);
	}
}

} // namespace lower
