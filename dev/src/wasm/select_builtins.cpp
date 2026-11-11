#include "wasm/selector.h"

namespace wasm {

bool Selector::builtin_call(const lowir::Instruction& inst, const lowir::Signature& signature)
{
	using namespace lowir;
	if (!inst.ops[0].is(ValueKind::Symbol))
		return false;
	const SymbolId symbol = inst.ops[0].index;
	if (program_.trap_function(symbol))
	{
		if (inst.extra_count != 0 || !signature.result.is_void())
			throw WasmError("invalid Wasm trap builtin boundary");
		out_.emit(Op::Unreachable);
		return true;
	}
	const bool copy = program_.memcpy_function(symbol) || program_.memmove_function(symbol);
	const bool fill = program_.memset_function(symbol);
	if (!copy && !fill)
		return false;
	const Parameter* parameters = program_.parameters_of(signature);
	if (inst.extra_count != 3 || !signature.result.is_pointer() || !parameters[0].type.is_pointer() ||
		(copy ? !parameters[1].type.is_pointer() : value_type(parameters[1].type) != ValueType::I32) ||
		value_type(parameters[2].type) != ValueType::I32)
		throw WasmError("invalid Wasm memory builtin boundary");
	const Value* arguments = body_.operands.data() + inst.extra_first;
	value(arguments[0], program_.pointer_type());
	if (!inst.type.is_void())
		out_.emit(Op::LocalTee, { locals_[inst.result] });
	value(arguments[1], parameters[1].type);
	value(arguments[2], parameters[2].type);
	out_.emit(copy ? Op::MemoryCopy : Op::MemoryFill,
		copy ? std::initializer_list<uint64_t>{ 0, 0 } : std::initializer_list<uint64_t>{ 0 });
	return true;
}

} // namespace wasm
