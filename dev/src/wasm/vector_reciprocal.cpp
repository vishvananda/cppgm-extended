#include "wasm/selector.h"

namespace wasm {

using namespace lowir;

void Selector::vector_reciprocal(const lowir::Instruction& inst)
{
	const auto op = static_cast<VectorOp>(inst.sub);
	const bool scalar = op == VectorOp::Rcpss || op == VectorOp::Rsqrtss;
	const bool root = op == VectorOp::Rsqrtss || op == VectorOp::Rsqrtps;
	const uint32_t original = add_local(ValueType::V128);
	const uint32_t input = add_local(ValueType::V128);
	const uint32_t result = add_local(ValueType::V128);
	const Literal sign{ UINT64_C(0x8000000080000000), UINT64_C(0x8000000080000000) };
	const Literal exponent{ UINT64_C(0x7f8000007f800000), UINT64_C(0x7f8000007f800000) };
	value(inst.ops[scalar ? 1 : 0], Type::scalar(TypeKind::V128));
	out_.emit(Op::LocalSet, { original });
	// The approximate SSE operations treat subnormal inputs as signed zero.
	out_.emit(Op::LocalGet, { original });
	vector_constant(sign);
	out_.emit(Op::V128And);
	out_.emit(Op::LocalGet, { original });
	out_.emit(Op::LocalGet, { original });
	vector_constant(exponent);
	out_.emit(Op::V128And);
	vector_constant(Literal{ 0, 0 });
	out_.emit(Op::I32x4Eq);
	out_.emit(Op::V128Bitselect);
	out_.emit(Op::LocalSet, { input });
	vector_constant(Literal{ UINT64_C(0x3f8000003f800000), UINT64_C(0x3f8000003f800000) });
	out_.emit(Op::LocalGet, { input });
	if (root)
		out_.emit(Op::F32x4Sqrt);
	out_.emit(Op::F32x4Div);
	out_.emit(Op::LocalSet, { result });
	// Exact division is within the approximation bound. Flush tiny outputs.
	out_.emit(Op::LocalGet, { result });
	vector_constant(sign);
	out_.emit(Op::V128And);
	out_.emit(Op::LocalGet, { result });
	out_.emit(Op::LocalGet, { result });
	vector_constant(exponent);
	out_.emit(Op::V128And);
	vector_constant(Literal{ 0, 0 });
	out_.emit(Op::I32x4Eq);
	out_.emit(Op::V128Bitselect);
	out_.emit(Op::LocalSet, { result });
	if (root)
	{
		vector_constant(Literal{ UINT64_C(0xffc00000ffc00000), UINT64_C(0xffc00000ffc00000) });
		out_.emit(Op::LocalGet, { result });
		out_.emit(Op::LocalGet, { input });
		vector_constant(Literal{ 0, 0 });
		out_.emit(Op::F32x4Lt);
		out_.emit(Op::V128Bitselect);
		out_.emit(Op::LocalSet, { result });
	}
	out_.emit(Op::LocalGet, { original });
	vector_constant(Literal{ UINT64_C(0x0040000000400000), UINT64_C(0x0040000000400000) });
	out_.emit(Op::V128Or);
	out_.emit(Op::LocalGet, { result });
	out_.emit(Op::LocalGet, { original });
	out_.emit(Op::LocalGet, { original });
	out_.emit(Op::F32x4Ne);
	out_.emit(Op::V128Bitselect);
	if (scalar)
	{
		value(inst.ops[0], Type::scalar(TypeKind::V128));
		vector_constant(Literal{ UINT64_C(0xffffffff), 0 });
		out_.emit(Op::V128Bitselect);
	}
}

} // namespace wasm
