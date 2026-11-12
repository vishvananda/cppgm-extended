#include "wasm/selector.h"

namespace wasm {

using namespace lowir;

void Selector::vector_compare(const lowir::Instruction& inst)
{
	const auto op = static_cast<VectorOp>(inst.sub);
	const bool doubles = op == VectorOp::Cmppd || op == VectorOp::Cmpsd;
	const bool scalar = op == VectorOp::Cmpss || op == VectorOp::Cmpsd;
	const unsigned predicate = vector_immediate(inst) & 7;
	const Type vector = Type::scalar(TypeKind::V128);
	if (predicate == 3 || predicate == 7)
	{
		for (unsigned i = 0; i < 2; ++i)
		{
			value(inst.ops[i], vector);
			value(inst.ops[i], vector);
			out_.emit(doubles ? Op::F64x2Eq : Op::F32x4Eq);
		}
		out_.emit(Op::V128And);
		if (predicate == 3)
			out_.emit(Op::V128Not);
	}
	else
	{
		const unsigned kind = predicate >= 5 ? predicate - 4 : predicate;
		const Op narrow[] = { Op::F32x4Eq, Op::F32x4Lt, Op::F32x4Le, Op::Nop, Op::F32x4Ne };
		const Op wide[] = { Op::F64x2Eq, Op::F64x2Lt, Op::F64x2Le, Op::Nop, Op::F64x2Ne };
		value(inst.ops[0], vector);
		value(inst.ops[1], vector);
		out_.emit(doubles ? wide[kind] : narrow[kind]);
		if (predicate >= 5)
			out_.emit(Op::V128Not);
	}
	if (scalar)
	{
		value(inst.ops[0], vector);
		vector_constant(Literal{ doubles ? UINT64_MAX : UINT64_C(0xffffffff), 0 });
		out_.emit(Op::V128Bitselect);
	}
}

void Selector::vector_blend_test(const lowir::Instruction& inst)
{
	const auto op = static_cast<VectorOp>(inst.sub);
	const Type vector = Type::scalar(TypeKind::V128);
	if (shape_of(op) == VectorShape::Blend)
	{
		value(inst.ops[1], vector);
		value(inst.ops[0], vector);
		value(inst.ops[2], vector);
		vector_constant(Literal{ 0, 0 });
		out_.emit(op == VectorOp::Blendvps ? Op::I32x4LtS : op == VectorOp::Blendvpd ? Op::I64x2LtS : Op::I8x16LtS);
		out_.emit(Op::V128Bitselect);
		return;
	}
	if (op == VectorOp::Ptestz || op == VectorOp::Ptestnzc)
	{
		value(inst.ops[0], vector);
		value(inst.ops[1], vector);
		out_.emit(Op::V128And);
		out_.emit(Op::V128AnyTrue);
	}
	if (op == VectorOp::Ptestc || op == VectorOp::Ptestnzc)
	{
		value(inst.ops[1], vector);
		value(inst.ops[0], vector);
		out_.emit(Op::V128Andnot);
		out_.emit(Op::V128AnyTrue);
	}
	out_.emit(op == VectorOp::Ptestnzc ? Op::I32And : Op::I32Eqz);
}

} // namespace wasm
