#include "wasm/selector.h"

namespace wasm {

using namespace lowir;

void Selector::vector_shift(const lowir::Instruction& inst)
{
	const auto op = static_cast<VectorOp>(inst.sub);
	const bool immediate = shape_of(op) == VectorShape::UnaryImmediate;
	const unsigned kind = static_cast<unsigned>(op) -
		static_cast<unsigned>(immediate ? VectorOp::Psllwi : VectorOp::Psllw);
	const unsigned width = 16u << (kind % 3);
	const bool arithmetic = kind >= 6;
	const Op operations[] = { Op::I16x8Shl, Op::I32x4Shl, Op::I64x2Shl, Op::I16x8ShrU, Op::I32x4ShrU, Op::I64x2ShrU,
		Op::I16x8ShrS, Op::I32x4ShrS };
	auto shift = [&](uint32_t amount) {
		if (amount >= width && !arithmetic)
			vector_constant(Literal{ 0, 0 });
		else
		{
			value(inst.ops[0], Type::scalar(TypeKind::V128));
			out_.emit(Op::I32Const, { amount >= width ? width - 1 : amount });
			out_.emit(operations[kind]);
		}
	};
	if (immediate)
	{
		shift(vector_immediate(inst));
		return;
	}
	const uint32_t count = add_local(ValueType::I64);
	value(inst.ops[1], Type::scalar(TypeKind::V128));
	out_.emit(Op::I64x2ExtractLane, { 0 });
	out_.emit(Op::LocalTee, { count });
	out_.emit(Op::I64Const, { width });
	out_.emit(Op::I64LtU);
	out_.emit(Op::If, { block_type(ValueType::V128) });
	++nesting_;
	value(inst.ops[0], Type::scalar(TypeKind::V128));
	out_.emit(Op::LocalGet, { count });
	out_.emit(Op::I32WrapI64);
	out_.emit(operations[kind]);
	out_.emit(Op::Else);
	shift(width);
	out_.emit(Op::End);
	--nesting_;
}

} // namespace wasm
