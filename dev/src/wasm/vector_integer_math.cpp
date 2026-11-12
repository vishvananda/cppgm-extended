#include "wasm/selector.h"

namespace wasm {

using namespace lowir;

void Selector::vector_integer_math(const lowir::Instruction& inst)
{
	const auto op = static_cast<VectorOp>(inst.sub);
	const Type vector = Type::scalar(TypeKind::V128);
	if (op == VectorOp::Pmulhw || op == VectorOp::Pmulhuw)
	{
		const bool sign = op == VectorOp::Pmulhw;
		for (unsigned high = 0; high < 2; ++high)
		{
			value(inst.ops[0], vector);
			value(inst.ops[1], vector);
			out_.emit(high ? (sign ? Op::I32x4ExtmulHighI16x8S : Op::I32x4ExtmulHighI16x8U)
						   : (sign ? Op::I32x4ExtmulLowI16x8S : Op::I32x4ExtmulLowI16x8U));
			out_.emit(Op::I32Const, { 16 });
			out_.emit(sign ? Op::I32x4ShrS : Op::I32x4ShrU);
		}
		out_.emit(sign ? Op::I16x8NarrowI32x4S : Op::I16x8NarrowI32x4U);
	}
	else if (op == VectorOp::Pmulhrsw)
	{
		value(inst.ops[0], vector);
		value(inst.ops[1], vector);
		out_.emit(Op::I16x8Q15mulrSatS);
		// SSE's sole overflow wraps, while Wasm's q15 operation saturates.
		for (unsigned i = 0; i < 2; ++i)
		{
			value(inst.ops[i], vector);
			vector_constant(Literal{ UINT64_C(0x8000800080008000), UINT64_C(0x8000800080008000) });
			out_.emit(Op::I16x8Eq);
		}
		out_.emit(Op::V128And);
		out_.emit(Op::V128Xor);
	}
	else if (op == VectorOp::Pmuludq || op == VectorOp::Pmuldq)
	{
		const uint8_t even[] = { 0, 1, 2, 3, 8, 9, 10, 11, 0, 1, 2, 3, 4, 5, 6, 7 };
		for (unsigned i = 0; i < 2; ++i)
			vector_shuffle(inst.ops[i], inst.ops[i], even);
		out_.emit(op == VectorOp::Pmuldq ? Op::I64x2ExtmulLowI32x4S : Op::I64x2ExtmulLowI32x4U);
	}
	else if (op == VectorOp::Pmaddubsw)
	{
		for (unsigned high = 0; high < 2; ++high)
		{
			value(inst.ops[0], vector);
			out_.emit(high ? Op::I16x8ExtendHighI8x16U : Op::I16x8ExtendLowI8x16U);
			value(inst.ops[1], vector);
			out_.emit(high ? Op::I16x8ExtendHighI8x16S : Op::I16x8ExtendLowI8x16S);
			out_.emit(Op::I16x8Mul);
			out_.emit(Op::I32x4ExtaddPairwiseI16x8S);
		}
		out_.emit(Op::I16x8NarrowI32x4S);
	}
	else if (op == VectorOp::Psadbw)
	{
		for (unsigned swap = 0; swap < 2; ++swap)
		{
			value(inst.ops[swap], vector);
			value(inst.ops[1 - swap], vector);
			out_.emit(Op::I8x16SubSatU);
		}
		out_.emit(Op::V128Or);
		out_.emit(Op::I16x8ExtaddPairwiseI8x16U);
		out_.emit(Op::I32x4ExtaddPairwiseI16x8U);
		const uint32_t sums = add_local(ValueType::V128);
		out_.emit(Op::LocalSet, { sums });
		const uint8_t even[] = { 0, 1, 2, 3, 8, 9, 10, 11, 0, 1, 2, 3, 4, 5, 6, 7 };
		const uint8_t odd[] = { 4, 5, 6, 7, 12, 13, 14, 15, 0, 1, 2, 3, 4, 5, 6, 7 };
		for (unsigned i = 0; i < 2; ++i)
		{
			out_.emit(Op::LocalGet, { sums });
			out_.emit(Op::LocalGet, { sums });
			vector_shuffle_stack(i == 0 ? even : odd);
			out_.emit(Op::I64x2ExtendLowI32x4U);
		}
		out_.emit(Op::I64x2Add);
	}
	else
	{
		const unsigned width = static_cast<unsigned>(op) - static_cast<unsigned>(VectorOp::Psignb);
		const Op shifts[] = { Op::I8x16ShrS, Op::I16x8ShrS, Op::I32x4ShrS };
		const Op subs[] = { Op::I8x16Sub, Op::I16x8Sub, Op::I32x4Sub };
		const Op equals[] = { Op::I8x16Eq, Op::I16x8Eq, Op::I32x4Eq };
		const uint32_t mask = add_local(ValueType::V128);
		value(inst.ops[1], vector);
		out_.emit(Op::I32Const, { (8u << width) - 1 });
		out_.emit(shifts[width]);
		out_.emit(Op::LocalSet, { mask });
		value(inst.ops[0], vector);
		out_.emit(Op::LocalGet, { mask });
		out_.emit(Op::V128Xor);
		out_.emit(Op::LocalGet, { mask });
		out_.emit(subs[width]);
		value(inst.ops[1], vector);
		vector_constant(Literal{ 0, 0 });
		out_.emit(equals[width]);
		out_.emit(Op::V128Andnot);
	}
}

} // namespace wasm
