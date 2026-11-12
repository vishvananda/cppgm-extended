#include "wasm/selector.h"

namespace wasm {

using namespace lowir;

void Selector::vector_minpos(const lowir::Instruction& inst)
{
	const uint32_t minimum = add_local(ValueType::V128);
	value(inst.ops[0], Type::scalar(TypeKind::V128));
	out_.emit(Op::LocalSet, { minimum });
	for (unsigned distance = 4; distance != 0; distance /= 2)
	{
		uint8_t lanes[16];
		for (unsigned i = 0; i < 8; ++i)
			for (unsigned byte = 0; byte < 2; ++byte)
				lanes[2 * i + byte] = static_cast<uint8_t>(2 * (i ^ distance) + byte);
		out_.emit(Op::LocalGet, { minimum });
		out_.emit(Op::LocalGet, { minimum });
		out_.emit(Op::LocalGet, { minimum });
		vector_shuffle_stack(lanes);
		out_.emit(Op::I16x8MinU);
		out_.emit(Op::LocalSet, { minimum });
	}
	vector_constant(Literal{ 0, 0 });
	out_.emit(Op::LocalGet, { minimum });
	out_.emit(Op::I16x8ExtractLaneU, { 0 });
	value(inst.ops[0], Type::scalar(TypeKind::V128));
	out_.emit(Op::LocalGet, { minimum });
	out_.emit(Op::I16x8Eq);
	out_.emit(Op::I16x8Bitmask);
	out_.emit(Op::I32Ctz);
	out_.emit(Op::I32Const, { 16 });
	out_.emit(Op::I32Shl);
	out_.emit(Op::I32Or);
	out_.emit(Op::I32x4ReplaceLane, { 0 });
}

void Selector::vector_dot(const lowir::Instruction& inst)
{
	const bool doubles = static_cast<VectorOp>(inst.sub) == VectorOp::Dppd;
	const unsigned width = doubles ? 8 : 4;
	const unsigned control = vector_immediate(inst);
	if ((control & (doubles ? 3u : 15u)) == 0)
	{
		vector_constant(Literal{ 0, 0 });
		return;
	}
	const auto mask = [&](unsigned bits) {
		Literal value{ 0, 0 };
		for (unsigned i = 0; i < 16; ++i)
			if ((bits & (1u << (i / width))) != 0)
				(i < 8 ? value.low : value.high) |= UINT64_C(255) << (8 * (i & 7));
		vector_constant(value);
	};
	value(inst.ops[0], Type::scalar(TypeKind::V128));
	value(inst.ops[1], Type::scalar(TypeKind::V128));
	out_.emit(doubles ? Op::F64x2Mul : Op::F32x4Mul);
	mask(control >> 4);
	out_.emit(Op::V128And);
	const uint32_t products = add_local(ValueType::V128);
	out_.emit(Op::LocalSet, { products });
	const unsigned count = 16 / width;
	for (unsigned i = 0; i < count; ++i)
	{
		out_.emit(Op::LocalGet, { products });
		out_.emit(doubles ? Op::F64x2ExtractLane : Op::F32x4ExtractLane, { i });
		if (i == 1 || i == 3)
			out_.emit(doubles ? Op::F64Add : Op::F32Add);
	}
	if (!doubles)
		out_.emit(Op::F32Add);
	out_.emit(doubles ? Op::F64x2Splat : Op::F32x4Splat);
	mask(control & 15);
	out_.emit(Op::V128And);
}

} // namespace wasm
