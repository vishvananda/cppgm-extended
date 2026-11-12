#include "wasm/selector.h"

namespace wasm {

void Selector::wide_shift(const lowir::Instruction& inst)
{
	using namespace lowir;
	const auto operation = static_cast<BinaryOp>(inst.sub);
	const bool left = operation == BinaryOp::Shl;
	const bool sign = operation == BinaryOp::Shr;
	const uint32_t count = add_local(ValueType::I64);
	value(inst.ops[1], Type::scalar(TypeKind::I64));
	out_.emit(Op::I64Const, { 127 });
	out_.emit(Op::I64And);
	out_.emit(Op::LocalSet, { count });
	// Wasm's scalar shifts mask at 64 bits. Handle the full-width boundary
	// explicitly; a shift of zero must not OR in the other complete half.
	for (unsigned part = 0; part < 2; ++part)
	{
		out_.emit(Op::LocalGet, { count });
		out_.emit(Op::I64Const, { 64 });
		out_.emit(Op::I64LtU);
		out_.emit(Op::If, { block_type(ValueType::I64) });
		wide_part(inst.ops[0], part);
		out_.emit(Op::LocalGet, { count });
		out_.emit(left ? Op::I64Shl : sign && part == 1 ? Op::I64ShrS : Op::I64ShrU);
		if ((left && part == 1) || (!left && part == 0))
		{
			wide_part(inst.ops[0], 1 - part);
			out_.emit(Op::I64Const, { 64 });
			out_.emit(Op::LocalGet, { count });
			out_.emit(Op::I64Sub);
			out_.emit(left ? Op::I64ShrU : Op::I64Shl);
			out_.emit(Op::I64Const, { 0 });
			out_.emit(Op::LocalGet, { count });
			out_.emit(Op::I64Eqz);
			out_.emit(Op::I32Eqz);
			out_.emit(Op::Select);
			out_.emit(Op::I64Or);
		}
		out_.emit(Op::Else);
		if ((left && part == 1) || (!left && part == 0))
		{
			wide_part(inst.ops[0], 1 - part);
			out_.emit(Op::LocalGet, { count });
			out_.emit(left ? Op::I64Shl : sign ? Op::I64ShrS : Op::I64ShrU);
		}
		else if (sign)
		{
			wide_part(inst.ops[0], 1);
			out_.emit(Op::I64Const, { 63 });
			out_.emit(Op::I64ShrS);
		}
		else
			out_.emit(Op::I64Const, { 0 });
		out_.emit(Op::End);
	}
	store_wide(Value::temp(inst.result));
}

} // namespace wasm
