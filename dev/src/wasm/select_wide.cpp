#include "wasm/selector.h"

namespace wasm {

using namespace lowir;

void Selector::wide_part(Value operand, unsigned part)
{
	if (operand.is(ValueKind::Integer))
	{
		const auto bits = body_.literals[operand.index];
		out_.emit(Op::I64Const, { part == 0 ? bits.low : bits.high });
		return;
	}
	address(operand);
	out_.emit(Op::I64Load, { 0, part * 8 });
}

void Selector::store_wide(Value destination)
{
	if (wide_low_ == kNoId)
	{
		wide_low_ = add_local(ValueType::I64);
		wide_high_ = add_local(ValueType::I64);
	}
	out_.emit(Op::LocalSet, { wide_high_ });
	out_.emit(Op::LocalSet, { wide_low_ });
	for (unsigned part = 0; part < 2; ++part)
	{
		address(destination);
		out_.emit(Op::LocalGet, { part == 0 ? wide_low_ : wide_high_ });
		out_.emit(Op::I64Store, { 0, part * 8 });
	}
}

void Selector::wide_convert(const lowir::Instruction& inst)
{
	const Type from = convert_source_type(inst);
	if (from.kind() == TypeKind::I128)
	{
		wide_part(inst.ops[0], 0);
		if (value_type(inst.type) == ValueType::I32)
			out_.emit(Op::I32WrapI64);
		normalize(inst.type);
		out_.emit(Op::LocalSet, { locals_[inst.result] });
		return;
	}
	const bool sign = static_cast<ConvertOp>(inst.sub) == ConvertOp::SExt;
	value(inst.ops[0], from);
	integer_view(from, sign);
	if (value_type(from) == ValueType::I32)
		out_.emit(sign ? Op::I64ExtendI32S : Op::I64ExtendI32U);
	if (sign)
	{
		value(inst.ops[0], from);
		integer_view(from, true);
		if (value_type(from) == ValueType::I32)
			out_.emit(Op::I64ExtendI32S);
		out_.emit(Op::I64Const, { 63 });
		out_.emit(Op::I64ShrS);
	}
	else
		out_.emit(Op::I64Const, { 0 });
	store_wide(Value::temp(inst.result));
}

bool Selector::wide_instruction(const lowir::Instruction& inst)
{
	const bool wide = inst.type.kind() == TypeKind::I128;
	if (inst.op == Opcode::Convert && (wide || convert_source_type(inst).kind() == TypeKind::I128))
	{
		wide_convert(inst);
		return true;
	}
	if (!wide)
		return false;
	switch (inst.op)
	{
	case Opcode::Const:
	case Opcode::Copy:
		wide_part(inst.ops[0], 0);
		wide_part(inst.ops[0], 1);
		store_wide(Value::temp(inst.result));
		return true;
	case Opcode::Binary:
		wide_binary(inst);
		return true;
	case Opcode::Unary:
	{
		const auto op = static_cast<UnaryOp>(inst.sub);
		if (op == UnaryOp::Not)
		{
			wide_part(inst.ops[0], 0);
			wide_part(inst.ops[0], 1);
			out_.emit(Op::I64Or);
			out_.emit(Op::I64Eqz);
			out_.emit(Op::I64ExtendI32U);
			out_.emit(Op::I64Const, { 0 });
		}
		else if (op == UnaryOp::BitNot)
			for (unsigned part = 0; part < 2; ++part)
			{
				wide_part(inst.ops[0], part);
				out_.emit(Op::I64Const, { UINT64_MAX });
				out_.emit(Op::I64Xor);
			}
		else if (op == UnaryOp::Neg)
		{
			out_.emit(Op::I64Const, { 0 });
			wide_part(inst.ops[0], 0);
			out_.emit(Op::I64Sub);
			out_.emit(Op::I64Const, { 0 });
			wide_part(inst.ops[0], 1);
			out_.emit(Op::I64Sub);
			wide_part(inst.ops[0], 0);
			out_.emit(Op::I64Eqz);
			out_.emit(Op::I32Eqz);
			out_.emit(Op::I64ExtendI32U);
			out_.emit(Op::I64Sub);
		}
		else
			throw WasmError("invalid wide integer unary operation");
		store_wide(Value::temp(inst.result));
		return true;
	}
	case Opcode::Compare:
		wide_compare(inst);
		out_.emit(Op::I64ExtendI32U);
		out_.emit(Op::LocalSet, { locals_[inst.result] });
		return true;
	default:
		return false;
	}
}

} // namespace wasm
