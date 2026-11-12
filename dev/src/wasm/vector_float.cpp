#include "wasm/selector.h"

namespace wasm {

using namespace lowir;

void Selector::vector_float(const lowir::Instruction& inst)
{
	const VectorOp op = static_cast<VectorOp>(inst.sub);
	const Type vector = Type::scalar(TypeKind::V128);
	bool doubles = false, scalar = false;
	if (op <= VectorOp::Maxsd)
	{
		const unsigned code = static_cast<unsigned>(op);
		const unsigned group = code / 4;
		doubles = (code & 1) != 0;
		scalar = (code & 2) != 0;
		if (group < 4)
		{
			const Op narrow[] = { Op::F32x4Add, Op::F32x4Sub, Op::F32x4Mul, Op::F32x4Div };
			const Op wide[] = { Op::F64x2Add, Op::F64x2Sub, Op::F64x2Mul, Op::F64x2Div };
			value(inst.ops[0], vector);
			value(inst.ops[1], vector);
			out_.emit(doubles ? wide[group] : narrow[group]);
		}
		else
		{
			// SSE keeps the second operand for a NaN or equal zero. A comparison
			// and bit selection preserve its bits instead of canonicalizing a NaN.
			value(inst.ops[0], vector);
			value(inst.ops[1], vector);
			value(inst.ops[0], vector);
			value(inst.ops[1], vector);
			out_.emit(group == 4 ? (doubles ? Op::F64x2Lt : Op::F32x4Lt) : (doubles ? Op::F64x2Gt : Op::F32x4Gt));
			out_.emit(Op::V128Bitselect);
		}
	}
	else if (op == VectorOp::Sqrtss || op == VectorOp::Sqrtsd || op == VectorOp::Cvtss2sd || op == VectorOp::Cvtsd2ss)
	{
		scalar = true;
		doubles = op == VectorOp::Sqrtsd || op == VectorOp::Cvtss2sd;
		value(inst.ops[1], vector);
		out_.emit(op == VectorOp::Cvtss2sd ? Op::F64x2PromoteLowF32x4
				: op == VectorOp::Cvtsd2ss ? Op::F32x4DemoteF64x2Zero
				: doubles                  ? Op::F64x2Sqrt
										   : Op::F32x4Sqrt);
	}
	else
		throw WasmError(
			"Wasm floating vector operation requires legalization: " + std::string(lowir_model::spelling(op)));
	if (scalar)
	{
		value(inst.ops[0], vector);
		vector_constant(Literal{ doubles ? UINT64_MAX : UINT64_C(0xffffffff), 0 });
		out_.emit(Op::V128Bitselect);
	}
}

} // namespace wasm
