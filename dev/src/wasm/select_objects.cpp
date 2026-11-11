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
	if (!inst.type.is_object())
		return false;
	switch (inst.op)
	{
	case Opcode::Load:
	case Opcode::Copy:
		copy_object(inst.ops[0], Value::temp(inst.result), inst.type.bytes);
		return true;
	case Opcode::Store:
		copy_object(inst.ops[0], inst.ops[1], inst.type.bytes);
		return true;
	default:
		return false;
	}
}

} // namespace wasm
