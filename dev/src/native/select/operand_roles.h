#pragma once

#include "lowir/program.h"

namespace native {

// How an instruction consumes an operand: its value, an address it
// dereferences (or forms, for an index base), or the storage of a direct
// load/store (where a slot operand is accessed rather than escaping).
enum class OperandRole : uint8_t
{
	Value,
	Address,
	Storage
};

// Calls f(value, role) for each value operand of `inst` (not block labels,
// type-info symbols or phi pairs, which callers handle).
template<typename F>
void for_each_operand(const lowir::FunctionBody& body, const lowir::Instruction& inst, F f)
{
	using lowir::Opcode;
	switch (inst.op)
	{
	case Opcode::Const:
	case Opcode::Phi:
	case Opcode::Exception:
	case Opcode::ExceptionSelector:
	case Opcode::AtomicThreadFence:
	case Opcode::AtomicSignalFence:
	case Opcode::EhTry:
	case Opcode::EhCleanup:
	case Opcode::EhCatch:
	case Opcode::EhFilter:
	case Opcode::EhCatchAll:
	case Opcode::EhEnd:
	case Opcode::Resume:
	case Opcode::Jump:
	case Opcode::Unreachable:
		return;
	case Opcode::Addr:
		f(inst.ops[0], OperandRole::Value);
		return;
	case Opcode::Load:
		f(inst.ops[0], OperandRole::Storage);
		return;
	case Opcode::Store:
		f(inst.ops[0], OperandRole::Value);
		f(inst.ops[1], OperandRole::Storage);
		return;
	case Opcode::AtomicLoad:
	case Opcode::VaStart:
	case Opcode::VaArg:
	case Opcode::ZeroInit:
		f(inst.ops[0], OperandRole::Address);
		return;
	case Opcode::Index:
		f(inst.ops[0], OperandRole::Address);
		f(inst.ops[1], OperandRole::Value);
		return;
	case Opcode::AtomicAddFetch:
	case Opcode::AtomicExchange:
		f(inst.ops[0], OperandRole::Address);
		f(inst.ops[1], OperandRole::Value);
		return;
	case Opcode::AtomicCompareExchange:
		f(inst.ops[0], OperandRole::Address);
		f(inst.ops[1], OperandRole::Address);
		f(inst.ops[2], OperandRole::Value);
		return;
	case Opcode::AtomicStore:
		f(inst.ops[0], OperandRole::Value);
		f(inst.ops[1], OperandRole::Address);
		return;
	case Opcode::CopyObject:
		f(inst.ops[0], OperandRole::Address);
		f(inst.ops[1], OperandRole::Address);
		return;
	case Opcode::Call:
		if (inst.ops[0].kind != lowir::ValueKind::Symbol)
			f(inst.ops[0], OperandRole::Value);
		for (uint32_t k = 0; k < inst.extra_count; ++k)
			f(body.operands[inst.extra_first + k], OperandRole::Value);
		return;
	case Opcode::Branch:
		f(inst.ops[0], OperandRole::Value);
		return;
	case Opcode::Switch:
		f(inst.ops[0], OperandRole::Value);
		for (uint32_t k = 0; k < inst.extra_count; k += 2)
			f(body.operands[inst.extra_first + k], OperandRole::Value);
		return;
	case Opcode::Return:
		if (inst.ops[0].kind != lowir::ValueKind::None)
			f(inst.ops[0], OperandRole::Value);
		return;
	default:
		// Copy, Unary, Binary, Compare, Convert, StackAlloc, Throw.
		for (unsigned k = 0; k < 3; ++k)
			if (inst.ops[k].kind != lowir::ValueKind::None && inst.ops[k].kind != lowir::ValueKind::Block)
				f(inst.ops[k], OperandRole::Value);
		return;
	}
}

} // namespace native
