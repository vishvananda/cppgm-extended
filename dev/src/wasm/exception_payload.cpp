#include "wasm/selector.h"

namespace wasm {

using namespace lowir;

void Selector::exception_throw(const lowir::Instruction& inst)
{
	if (module_.exception_abi != ExceptionAbi::Opaque)
		throw WasmError("opaque throw requires the opaque exception ABI");
	if (inst.type.kind() == TypeKind::I128)
	{
		wide_part(inst.ops[0], 0);
		wide_part(inst.ops[0], 1);
	}
	else
	{
		value(inst.ops[0], inst.type);
		const ValueType type = value_type(inst.type);
		if (type == ValueType::F32)
			out_.emit(Op::I32ReinterpretF32);
		else if (type == ValueType::F64)
			out_.emit(Op::I64ReinterpretF64);
		else if (type != ValueType::I32 && type != ValueType::I64)
			throw WasmError("opaque exception requires a scalar or pointer payload");
		if (type == ValueType::I32 || type == ValueType::F32)
			out_.emit(Op::I64ExtendI32U);
		out_.emit(Op::I64Const, { 0 });
	}
	restore_stack();
	out_.emit(Op::Throw, { 0 });
	out_.relocate(RelocationKind::TagIndex, module_.exception_tag_symbol);
}

void Selector::exception_value(Type type)
{
	if (type.kind() == TypeKind::I128)
	{
		if (exception_high_ == kNoId)
			throw WasmError("C++ exception carries an unwind pointer");
		out_.emit(Op::LocalGet, { exception_pointer_ });
		out_.emit(Op::LocalGet, { exception_high_ });
		return;
	}
	const ValueType to = value_type(type);
	if (to == ValueType::V128 || to == ValueType::Void)
		throw WasmError("exception read requires a scalar or pointer type");
	out_.emit(Op::LocalGet, { exception_pointer_ });
	const bool cxx = module_.exception_abi == ExceptionAbi::Cxx;
	if (cxx && (to == ValueType::I64 || to == ValueType::F64))
		out_.emit(Op::I64ExtendI32U);
	if (!cxx && (to == ValueType::I32 || to == ValueType::F32))
		out_.emit(Op::I32WrapI64);
	if (to == ValueType::F32)
		out_.emit(Op::F32ReinterpretI32);
	else if (to == ValueType::F64)
		out_.emit(Op::F64ReinterpretI64);
	normalize(type);
}

} // namespace wasm
