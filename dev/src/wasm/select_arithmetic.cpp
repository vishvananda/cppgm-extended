#include "wasm/selector.h"

namespace wasm {

using namespace lowir;

void Selector::binary(const lowir::Instruction& inst)
{
	const auto operation = static_cast<BinaryOp>(inst.sub);
	const ValueType type = value_type(inst.type);
	const bool unsigned_view = operation == BinaryOp::UDiv || operation == BinaryOp::UMod ||
		operation == BinaryOp::UShr;
	// A quotient, a remainder and a right shift read the high bits of their
	// operands. The low bits of the other results depend on the low bits of
	// the operands only, and the result is normalized.
	const bool reads_view = !inst.type.is_float() &&
		(unsigned_view || operation == BinaryOp::Div || operation == BinaryOp::Mod || operation == BinaryOp::Shr);
	if (reads_view)
	{
		viewed(inst.ops[0], inst.type, !unsigned_view);
		viewed(inst.ops[1], inst.type, !unsigned_view);
	}
	else
	{
		value(inst.ops[0], inst.type);
		if (operation == BinaryOp::Shl)
			viewed(inst.ops[1], inst.type, true);
		else
			value(inst.ops[1], inst.type);
	}
	if (type == ValueType::F32 || type == ValueType::F64)
	{
		unsigned offset;
		switch (operation)
		{
		case BinaryOp::Add:
			offset = 0;
			break;
		case BinaryOp::Sub:
			offset = 1;
			break;
		case BinaryOp::Mul:
			offset = 2;
			break;
		case BinaryOp::Div:
			offset = 3;
			break;
		default:
			throw WasmError("unsupported Wasm floating binary operation");
		}
		out_.emit(static_cast<Op>((type == ValueType::F32 ? 0x92 : 0xa0) + offset));
		return;
	}
	unsigned offset;
	switch (operation)
	{
	case BinaryOp::Add:
		offset = 0;
		break;
	case BinaryOp::Sub:
		offset = 1;
		break;
	case BinaryOp::Mul:
		offset = 2;
		break;
	case BinaryOp::Div:
		offset = 3;
		break;
	case BinaryOp::UDiv:
		offset = 4;
		break;
	case BinaryOp::Mod:
		offset = 5;
		break;
	case BinaryOp::UMod:
		offset = 6;
		break;
	case BinaryOp::And:
		offset = 7;
		break;
	case BinaryOp::Or:
		offset = 8;
		break;
	case BinaryOp::Xor:
		offset = 9;
		break;
	case BinaryOp::Shl:
		offset = 10;
		break;
	case BinaryOp::Shr:
		offset = 11;
		break;
	case BinaryOp::UShr:
		offset = 12;
		break;
	default:
		throw WasmError("invalid Wasm integer binary operation");
	}
	out_.emit(static_cast<Op>((type == ValueType::I64 ? 0x7c : 0x6a) + offset));
}

void Selector::unary(const lowir::Instruction& inst)
{
	const auto operation = static_cast<UnaryOp>(inst.sub);
	const ValueType type = value_type(inst.type);
	const bool wide = type == ValueType::I64;
	const bool floating = type == ValueType::F32 || type == ValueType::F64;
	if (operation == UnaryOp::Neg && !floating)
		constant(Literal{ 0, 0 }, inst.type);
	value(inst.ops[0], inst.type);
	switch (operation)
	{
	case UnaryOp::Neg:
		out_.emit(floating ? (type == ValueType::F32 ? Op::F32Neg : Op::F64Neg) : wide ? Op::I64Sub : Op::I32Sub);
		break;
	case UnaryOp::Not:
		out_.emit(wide ? Op::I64Eqz : Op::I32Eqz);
		if (wide)
			out_.emit(Op::I64ExtendI32U);
		break;
	case UnaryOp::BitNot:
		constant(Literal{ ~uint64_t(0), ~uint64_t(0) }, inst.type);
		out_.emit(wide ? Op::I64Xor : Op::I32Xor);
		break;
	case UnaryOp::CountLeadingZeros:
		out_.emit(wide ? Op::I64Clz : Op::I32Clz);
		break;
	case UnaryOp::CountTrailingZeros:
		out_.emit(wide ? Op::I64Ctz : Op::I32Ctz);
		break;
	case UnaryOp::CountOnes:
		out_.emit(wide ? Op::I64Popcnt : Op::I32Popcnt);
		break;
	case UnaryOp::Sqrt:
		out_.emit(type == ValueType::F32 ? Op::F32Sqrt : Op::F64Sqrt);
		break;
	case UnaryOp::ByteSwap:
	{
		out_.emit(Op::Drop);
		const unsigned bytes = static_cast<unsigned>(storage_bytes(inst.type));
		for (unsigned i = 0; i < bytes; ++i)
		{
			value(inst.ops[0], inst.type);
			constant(Literal{ i * 8u, 0 }, inst.type);
			out_.emit(wide ? Op::I64ShrU : Op::I32ShrU);
			constant(Literal{ 255, 0 }, inst.type);
			out_.emit(wide ? Op::I64And : Op::I32And);
			constant(Literal{ (bytes - 1 - i) * 8u, 0 }, inst.type);
			out_.emit(wide ? Op::I64Shl : Op::I32Shl);
			if (i != 0)
				out_.emit(wide ? Op::I64Or : Op::I32Or);
		}
		break;
	}
	}
}

void Selector::compare(const lowir::Instruction& inst)
{
	const auto predicate = static_cast<ComparePredicate>(inst.sub);
	const ValueType type = value_type(inst.type);
	const bool floating = type == ValueType::F32 || type == ValueType::F64;
	const bool unordered = predicate == ComparePredicate::Ult || predicate == ComparePredicate::Ule ||
		predicate == ComparePredicate::Ugt || predicate == ComparePredicate::Uge;
	if (floating)
	{
		value(inst.ops[0], inst.type);
		value(inst.ops[1], inst.type);
	}
	else
	{
		bool signed_view = !unordered && inst.type.kind() != TypeKind::I1;
		// Equality holds in either view: the operands keep the unsigned
		// view when one of them has it.
		if ((predicate == ComparePredicate::Eq || predicate == ComparePredicate::Ne) &&
			(in_view(inst.ops[0], inst.type, false) || in_view(inst.ops[1], inst.type, false)))
			signed_view = false;
		viewed(inst.ops[0], inst.type, signed_view);
		viewed(inst.ops[1], inst.type, signed_view);
	}
	unsigned offset;
	switch (predicate)
	{
	case ComparePredicate::Eq:
		offset = 0;
		break;
	case ComparePredicate::Ne:
		offset = 1;
		break;
	case ComparePredicate::Lt:
		offset = 2;
		break;
	case ComparePredicate::Ult:
		offset = floating ? 5 : 3;
		break;
	case ComparePredicate::Gt:
		offset = floating ? 3 : 4;
		break;
	case ComparePredicate::Ugt:
		offset = floating ? 4 : 5;
		break;
	case ComparePredicate::Le:
		offset = floating ? 4 : 6;
		break;
	case ComparePredicate::Ule:
		offset = floating ? 3 : 7;
		break;
	case ComparePredicate::Ge:
		offset = floating ? 5 : 8;
		break;
	case ComparePredicate::Uge:
		offset = floating ? 2 : 9;
		break;
	default:
		throw WasmError("invalid Wasm comparison");
	}
	const unsigned first = type == ValueType::F32 ? 0x5b
		: type == ValueType::F64                  ? 0x61
		: type == ValueType::I64                  ? 0x51
												  : 0x46;
	out_.emit(static_cast<Op>(first + offset));
	if (floating && unordered)
		out_.emit(Op::I32Eqz);
	// LowIR comparisons produce i64 truth values on every target.
	out_.emit(Op::I64ExtendI32U);
}

void Selector::integer_view(Type type, bool signed_view)
{
	const unsigned bits = integer_bits(type.kind());
	if (bits == 0 || bits >= 32)
		return;
	if (!signed_view)
	{
		out_.emit(Op::I32Const, { (uint64_t(1) << bits) - 1 });
		out_.emit(Op::I32And);
	}
	else if (bits == 8 || bits == 16)
		out_.emit(bits == 8 ? Op::I32Extend8S : Op::I32Extend16S);
	else
	{
		out_.emit(Op::I32Const, { 32 - bits });
		out_.emit(Op::I32Shl);
		out_.emit(Op::I32Const, { 32 - bits });
		out_.emit(Op::I32ShrS);
	}
}

// Whether `operand`, as value() leaves it, already is the signed or the
// unsigned view of the narrow `type`. The local of a temporary holds the view
// of that temporary's type when one of these operations made it: the selector
// normalizes their results.
bool Selector::in_view(Value operand, Type type, bool signed_view) const
{
	if (!operand.is(ValueKind::Temp) || wide_locals_[operand.index] != kNoId)
		return false;
	const auto& temp = body_.temps[operand.index];
	const unsigned bits = integer_bits(temp.type.kind());
	if (bits >= 32 || bits != integer_bits(type.kind()) || temp.def == kNoId)
		return false;
	switch (body_.instructions[temp.def].op)
	{
	case Opcode::Const:
	case Opcode::Copy:
	case Opcode::Load:
	case Opcode::Unary:
	case Opcode::Binary:
	case Opcode::Convert:
	case Opcode::Select:
		return temp.type.kind() == TypeKind::I1 ? !signed_view : is_unsigned(temp.type.kind()) != signed_view;
	default:
		return false;
	}
}

// `operand` in the signed or the unsigned view of `type`.
void Selector::viewed(Value operand, Type type, bool signed_view)
{
	const unsigned bits = integer_bits(type.kind());
	if (operand.is(ValueKind::Integer) && bits != 0 && bits < 32)
	{
		const uint64_t mask = (uint64_t(1) << bits) - 1;
		uint64_t low = body_.literals[operand.index].low & mask;
		if (signed_view && (low >> (bits - 1)) != 0)
			low |= ~mask;
		out_.emit(Op::I32Const, { static_cast<uint64_t>(static_cast<int64_t>(static_cast<int32_t>(low))) });
		return;
	}
	value(operand, type);
	if (!in_view(operand, type, signed_view))
		integer_view(type, signed_view);
}

void Selector::convert(const lowir::Instruction& inst)
{
	const auto operation = static_cast<ConvertOp>(inst.sub);
	const Type from = convert_source_type(inst);
	const ValueType source = value_type(from), destination = value_type(inst.type);
	if (operation == ConvertOp::SExt || operation == ConvertOp::ZExt)
		viewed(inst.ops[0], from, operation == ConvertOp::SExt);
	else
		value(inst.ops[0], from);
	if (source == destination && operation != ConvertOp::SExt && operation != ConvertOp::ZExt)
		return;
	switch (operation)
	{
	case ConvertOp::SExt:
	case ConvertOp::ZExt:
	{
		if (destination == ValueType::I64 && source == ValueType::I32)
			out_.emit(operation == ConvertOp::SExt ? Op::I64ExtendI32S : Op::I64ExtendI32U);
		break;
	}
	case ConvertOp::Trunc:
		if (source == ValueType::I64 && destination == ValueType::I32)
			out_.emit(Op::I32WrapI64);
		break;
	case ConvertOp::SIToFP:
	case ConvertOp::UIToFP:
	{
		if (!in_view(inst.ops[0], from, operation == ConvertOp::SIToFP))
			integer_view(from, operation == ConvertOp::SIToFP);
		const unsigned first = destination == ValueType::F32 ? 0xb2 : 0xb7;
		const unsigned offset = (source == ValueType::I64 ? 2 : 0) + (operation == ConvertOp::UIToFP ? 1 : 0);
		out_.emit(static_cast<Op>(first + offset));
		break;
	}
	case ConvertOp::FPToSI:
	case ConvertOp::FPToUI:
	{
		const unsigned offset = (destination == ValueType::I64 ? 4 : 0) + (source == ValueType::F64 ? 2 : 0) +
			(operation == ConvertOp::FPToUI ? 1 : 0);
		out_.emit(static_cast<Op>(0xfc00 + offset));
		break;
	}
	case ConvertOp::FPExt:
		out_.emit(Op::F64PromoteF32);
		break;
	case ConvertOp::FPTrunc:
		out_.emit(Op::F32DemoteF64);
		break;
	}
}

} // namespace wasm
