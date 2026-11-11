#include "wasm/selector.h"

namespace wasm {

using namespace lowir;

void Selector::wide_binary(const lowir::Instruction& inst)
{
	const auto operation = static_cast<BinaryOp>(inst.sub);
	if (operation == BinaryOp::Shl || operation == BinaryOp::Shr || operation == BinaryOp::UShr)
	{
		wide_shift(inst);
		return;
	}
	Op op;
	switch (operation)
	{
	case BinaryOp::Add:
		op = Op::I64Add;
		break;
	case BinaryOp::Sub:
		op = Op::I64Sub;
		break;
	case BinaryOp::And:
		op = Op::I64And;
		break;
	case BinaryOp::Or:
		op = Op::I64Or;
		break;
	case BinaryOp::Xor:
		op = Op::I64Xor;
		break;
	default:
		throw WasmError("wide integer operation requires runtime legalization");
	}
	for (unsigned part = 0; part < 2; ++part)
	{
		wide_part(inst.ops[0], part);
		wide_part(inst.ops[1], part);
		out_.emit(op);
		if (part == 1 && (operation == BinaryOp::Add || operation == BinaryOp::Sub))
		{
			if (operation == BinaryOp::Add)
			{
				wide_part(inst.ops[0], 0);
				wide_part(inst.ops[1], 0);
				out_.emit(Op::I64Add);
				wide_part(inst.ops[0], 0);
			}
			else
			{
				wide_part(inst.ops[0], 0);
				wide_part(inst.ops[1], 0);
			}
			out_.emit(Op::I64LtU);
			out_.emit(Op::I64ExtendI32U);
			out_.emit(op);
		}
	}
	store_wide(Value::temp(inst.result));
}

void Selector::wide_compare(const lowir::Instruction& inst)
{
	const auto predicate = static_cast<ComparePredicate>(inst.sub);
	if (predicate == ComparePredicate::Eq || predicate == ComparePredicate::Ne)
	{
		for (unsigned part = 0; part < 2; ++part)
		{
			wide_part(inst.ops[0], part);
			wide_part(inst.ops[1], part);
			out_.emit(predicate == ComparePredicate::Eq ? Op::I64Eq : Op::I64Ne);
		}
		out_.emit(predicate == ComparePredicate::Eq ? Op::I32And : Op::I32Or);
		return;
	}
	Op high, low;
	switch (predicate)
	{
	case ComparePredicate::Lt:
		high = Op::I64LtS;
		low = Op::I64LtU;
		break;
	case ComparePredicate::Le:
		high = Op::I64LtS;
		low = Op::I64LeU;
		break;
	case ComparePredicate::Gt:
		high = Op::I64GtS;
		low = Op::I64GtU;
		break;
	case ComparePredicate::Ge:
		high = Op::I64GtS;
		low = Op::I64GeU;
		break;
	case ComparePredicate::Ult:
		high = Op::I64LtU;
		low = Op::I64LtU;
		break;
	case ComparePredicate::Ule:
		high = Op::I64LtU;
		low = Op::I64LeU;
		break;
	case ComparePredicate::Ugt:
		high = Op::I64GtU;
		low = Op::I64GtU;
		break;
	case ComparePredicate::Uge:
		high = Op::I64GtU;
		low = Op::I64GeU;
		break;
	default:
		throw WasmError("invalid wide integer predicate");
	}
	wide_part(inst.ops[0], 1);
	wide_part(inst.ops[1], 1);
	out_.emit(high);
	wide_part(inst.ops[0], 1);
	wide_part(inst.ops[1], 1);
	out_.emit(Op::I64Eq);
	wide_part(inst.ops[0], 0);
	wide_part(inst.ops[1], 0);
	out_.emit(low);
	out_.emit(Op::I32And);
	out_.emit(Op::I32Or);
}

} // namespace wasm
