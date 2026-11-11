#include "wasm/selector.h"

namespace wasm {

using namespace lowir;

void Selector::copy_object(Value source, Value destination, uint64_t bytes)
{
	address(destination);
	address(source);
	out_.emit(Op::I32Const, { bytes });
	out_.emit(Op::MemoryCopy, { 0, 0 });
}

bool Selector::object_instruction(const lowir::Instruction& inst)
{
	if (!memory_value(inst.type))
		return false;
	switch (inst.op)
	{
	case Opcode::Load:
	case Opcode::Copy:
		if (inst.type.kind() == TypeKind::I128 && inst.op == Opcode::Copy)
			return false;
		copy_object(inst.ops[0], Value::temp(inst.result), storage_bytes(inst.type));
		return true;
	case Opcode::Store:
		if (inst.type.kind() == TypeKind::I128)
		{
			wide_part(inst.ops[0], 0);
			wide_part(inst.ops[0], 1);
			store_wide(inst.ops[1]);
			return true;
		}
		copy_object(inst.ops[0], inst.ops[1], storage_bytes(inst.type));
		return true;
	default:
		return false;
	}
}

} // namespace wasm
