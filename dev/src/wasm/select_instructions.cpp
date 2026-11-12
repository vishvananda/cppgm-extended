#include "wasm/selector.h"

namespace wasm {

using namespace lowir;

void Selector::instruction(const lowir::Instruction& inst)
{
	if ((exception_reference_ != kNoId && exception_instruction(inst)) || object_instruction(inst) ||
		wide_instruction(inst))
		return;
	switch (inst.op)
	{
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
	case Opcode::Addr:
		address(inst.ops[0]);
		break;
	case Opcode::Load:
		address(inst.ops[0]);
		out_.emit(memory_op(inst.type, false), { 0, 0 });
		break;
	case Opcode::Store:
		address(inst.ops[1]);
		value(inst.ops[0], inst.type);
		out_.emit(memory_op(inst.type, true), { 0, 0 });
		return;
	case Opcode::Index:
	{
		address(inst.ops[0]);
		value(inst.ops[1], Type::scalar(TypeKind::U32));
		out_.emit(Op::I32Const, { storage_bytes(inst.type) });
		out_.emit(Op::I32Mul);
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
		address(inst.ops[1]);
		address(inst.ops[0]);
		out_.emit(Op::I32Const, { static_cast<uint64_t>(inst.immediate) });
		out_.emit(Op::MemoryCopy, { 0, 0 });
		return;
	case Opcode::ZeroInit:
		address(inst.ops[0]);
		out_.emit(Op::I32Const, { 0 });
		out_.emit(Op::I32Const, { static_cast<uint64_t>(inst.immediate) });
		out_.emit(Op::MemoryFill, { 0 });
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
