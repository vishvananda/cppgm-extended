#include "wasm/selector.h"

namespace wasm {

namespace {

Op atomic_op(lowir::Opcode opcode, lowir::Type type)
{
	using lowir::Opcode;
	const unsigned bytes = static_cast<unsigned>(lowir::storage_bytes(type));
	const unsigned index = bytes == 1 ? 0 : bytes == 2 ? 1 : bytes == 4 ? 2 : 3;
	if ((!type.is_integer() && !type.is_pointer()) || bytes > 8)
		throw WasmError("Wasm atomics require an integer or pointer of at most 64 bits");
	static const Op load[] = { Op::I32AtomicLoad8U, Op::I32AtomicLoad16U, Op::I32AtomicLoad, Op::I64AtomicLoad };
	static const Op store[] = { Op::I32AtomicStore8, Op::I32AtomicStore16, Op::I32AtomicStore, Op::I64AtomicStore };
	static const Op add[] = { Op::I32AtomicRmw8AddU, Op::I32AtomicRmw16AddU, Op::I32AtomicRmwAdd, Op::I64AtomicRmwAdd };
	static const Op exchange[] = { Op::I32AtomicRmw8XchgU, Op::I32AtomicRmw16XchgU, Op::I32AtomicRmwXchg,
		Op::I64AtomicRmwXchg };
	static const Op compare[] = { Op::I32AtomicRmw8CmpxchgU, Op::I32AtomicRmw16CmpxchgU, Op::I32AtomicRmwCmpxchg,
		Op::I64AtomicRmwCmpxchg };
	return opcode == Opcode::AtomicLoad    ? load[index]
		: opcode == Opcode::AtomicStore    ? store[index]
		: opcode == Opcode::AtomicAddFetch ? add[index]
		: opcode == Opcode::AtomicExchange ? exchange[index]
										   : compare[index];
}

} // namespace

void Selector::atomic_instruction(const lowir::Instruction& inst)
{
	using namespace lowir;
	if (inst.op == Opcode::AtomicSignalFence ||
		(inst.op == Opcode::AtomicThreadFence && inst.order == MemoryOrder::Relaxed))
		return;
	if (!module_.atomics)
		throw WasmError("unsupported operation in this Wasm profile: " + std::string(spelling(inst.op)) +
			" (atomics disabled by -mno-atomics)");
	if (inst.op == Opcode::AtomicThreadFence)
	{
		out_.emit(Op::AtomicFence, { 0 });
		return;
	}
	const Op op = atomic_op(inst.op, inst.type);
	const uint8_t align = wasm_model::atomic_info(op).alignment;
	if (inst.op == Opcode::AtomicCompareExchange)
	{
		const ValueType type = value_type(inst.type);
		const uint32_t expected = add_local(type), previous = add_local(type);
		address(inst.ops[1]);
		out_.emit(memory_op(inst.type, false), { 0, 0 });
		out_.emit(Op::LocalSet, { expected });
		address(inst.ops[0]);
		out_.emit(Op::LocalGet, { expected });
		value(inst.ops[2], inst.type);
		out_.emit(op, { align, 0 });
		normalize(inst.type);
		out_.emit(Op::LocalTee, { previous });
		out_.emit(Op::LocalGet, { expected });
		out_.emit(type == ValueType::I64 ? Op::I64Eq : Op::I32Eq);
		// LowIR compare-exchange produces an i64 boolean, independent of its stored type.
		out_.emit(Op::I64ExtendI32U);
		out_.emit(Op::LocalSet, { locals_[inst.result] });
		out_.emit(Op::LocalGet, { locals_[inst.result] });
		out_.emit(Op::I64Eqz);
		out_.emit(Op::If, { block_type(ValueType::Void) });
		address(inst.ops[1]);
		out_.emit(Op::LocalGet, { previous });
		out_.emit(memory_op(inst.type, true), { 0, 0 });
		out_.emit(Op::End);
		return;
	}
	address(inst.ops[inst.op == Opcode::AtomicStore ? 1 : 0]);
	if (inst.op == Opcode::AtomicStore)
	{
		value(inst.ops[0], inst.type);
		out_.emit(op, { align, 0 });
		return;
	}
	uint32_t delta = kNoId;
	if (inst.op != Opcode::AtomicLoad)
	{
		value(inst.ops[1], inst.type);
		if (inst.op == Opcode::AtomicAddFetch)
		{
			delta = add_local(value_type(inst.type));
			out_.emit(Op::LocalTee, { delta });
		}
	}
	out_.emit(op, { align, 0 });
	if (delta != kNoId)
	{
		out_.emit(Op::LocalGet, { delta });
		out_.emit(value_type(inst.type) == ValueType::I64 ? Op::I64Add : Op::I32Add);
	}
	normalize(inst.type);
	if (inst.result != kNoId)
		out_.emit(Op::LocalSet, { locals_[inst.result] });
	else
		out_.emit(Op::Drop);
}

} // namespace wasm
