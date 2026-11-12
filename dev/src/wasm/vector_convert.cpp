#include "wasm/selector.h"

namespace wasm {

using namespace lowir;

void Selector::vector_convert(const lowir::Instruction& inst)
{
	const auto op = static_cast<VectorOp>(inst.sub);
	const Type vector = Type::scalar(TypeKind::V128);
	if (op >= VectorOp::Pmovsxbw && op <= VectorOp::Pmovzxdq)
	{
		const unsigned kind = static_cast<unsigned>(op) - static_cast<unsigned>(VectorOp::Pmovsxbw);
		const bool sign = kind < 6;
		const unsigned from[] = { 0, 0, 0, 1, 1, 2 };
		const unsigned to[] = { 1, 2, 3, 2, 3, 3 };
		const Op signed_ops[] = { Op::I16x8ExtendLowI8x16S, Op::I32x4ExtendLowI16x8S, Op::I64x2ExtendLowI32x4S };
		const Op unsigned_ops[] = { Op::I16x8ExtendLowI8x16U, Op::I32x4ExtendLowI16x8U, Op::I64x2ExtendLowI32x4U };
		value(inst.ops[0], vector);
		for (unsigned width = from[kind % 6]; width < to[kind % 6]; ++width)
			out_.emit(sign ? signed_ops[width] : unsigned_ops[width]);
		return;
	}
	if (op >= VectorOp::Roundps && op <= VectorOp::Roundsd)
	{
		const bool doubles = op == VectorOp::Roundpd || op == VectorOp::Roundsd;
		const bool scalar = op == VectorOp::Roundss || op == VectorOp::Roundsd;
		const unsigned control = vector_immediate(inst);
		const unsigned direction = (control & 4) ? 0 : control & 3;
		const Op narrow[] = { Op::F32x4Nearest, Op::F32x4Floor, Op::F32x4Ceil, Op::F32x4Trunc };
		const Op wide[] = { Op::F64x2Nearest, Op::F64x2Floor, Op::F64x2Ceil, Op::F64x2Trunc };
		value(inst.ops[scalar ? 1 : 0], vector);
		out_.emit(doubles ? wide[direction] : narrow[direction]);
		if (scalar)
		{
			value(inst.ops[0], vector);
			vector_constant(Literal{ doubles ? UINT64_MAX : UINT64_C(0xffffffff), 0 });
			out_.emit(Op::V128Bitselect);
		}
		return;
	}
	const bool doubles = op == VectorOp::Cvtpd2dq || op == VectorOp::Cvttpd2dq;
	const bool rounded = op == VectorOp::Cvtps2dq || op == VectorOp::Cvtpd2dq;
	const uint32_t input = add_local(ValueType::V128);
	value(inst.ops[0], vector);
	if (rounded)
		out_.emit(doubles ? Op::F64x2Nearest : Op::F32x4Nearest);
	out_.emit(Op::LocalSet, { input });
	out_.emit(Op::LocalGet, { input });
	out_.emit(doubles ? Op::I32x4TruncSatF64x2SZero : Op::I32x4TruncSatF32x4S);
	const uint64_t indefinite = UINT64_C(0x8000000080000000);
	vector_constant(Literal{ indefinite, doubles ? 0 : indefinite });
	for (unsigned upper = 0; upper < 2; ++upper)
	{
		out_.emit(Op::LocalGet, { input });
		const uint64_t bound = doubles ? (upper ? UINT64_C(0x41e0000000000000) : UINT64_C(0xc1e0000000000000))
									   : (upper ? UINT64_C(0x4f0000004f000000) : UINT64_C(0xcf000000cf000000));
		vector_constant(Literal{ bound, bound });
		out_.emit(upper ? (doubles ? Op::F64x2Lt : Op::F32x4Lt) : (doubles ? Op::F64x2Ge : Op::F32x4Ge));
	}
	out_.emit(Op::V128And);
	if (doubles)
	{
		vector_constant(Literal{ 0, 0 });
		out_.emit(Op::I8x16Shuffle, { 0, 1, 2, 3, 8, 9, 10, 11, 16, 17, 18, 19, 20, 21, 22, 23 });
	}
	out_.emit(Op::V128Bitselect);
}

} // namespace wasm
