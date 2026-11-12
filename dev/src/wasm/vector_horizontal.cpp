#include "wasm/selector.h"

namespace wasm {

using namespace lowir;

void Selector::vector_horizontal(const lowir::Instruction& inst)
{
	const auto op = static_cast<VectorOp>(inst.sub);
	const bool floats = op >= VectorOp::Addsubps && op <= VectorOp::Hsubpd;
	unsigned width;
	Op operation;
	if (floats)
	{
		const unsigned kind = static_cast<unsigned>(op) - static_cast<unsigned>(VectorOp::Addsubps);
		width = (kind & 1) ? 8 : 4;
		const bool subtract = kind >= 4;
		operation = subtract ? (width == 8 ? Op::F64x2Sub : Op::F32x4Sub) : (width == 8 ? Op::F64x2Add : Op::F32x4Add);
	}
	else
	{
		const unsigned kind = static_cast<unsigned>(op) - static_cast<unsigned>(VectorOp::Phaddw);
		width = kind % 3 == 1 ? 4 : 2;
		const bool saturated = kind % 3 == 2;
		operation = kind < 3 ? (saturated           ? Op::I16x8AddSatS
									   : width == 4 ? Op::I32x4Add
													: Op::I16x8Add)
							 : (saturated           ? Op::I16x8SubSatS
									   : width == 4 ? Op::I32x4Sub
													: Op::I16x8Sub);
	}
	uint8_t even[16], odd[16];
	if (op == VectorOp::Addsubps || op == VectorOp::Addsubpd)
	{
		const Type vector = Type::scalar(TypeKind::V128);
		for (unsigned which = 0; which < 2; ++which)
		{
			value(inst.ops[0], vector);
			value(inst.ops[1], vector);
			out_.emit(
				which == 0 ? (width == 8 ? Op::F64x2Sub : Op::F32x4Sub) : (width == 8 ? Op::F64x2Add : Op::F32x4Add));
		}
		for (unsigned i = 0; i < 16; ++i)
			even[i] = static_cast<uint8_t>(i + ((i / width) & 1) * 16);
		vector_shuffle_stack(even);
		return;
	}
	const unsigned half = 8 / width;
	for (unsigned i = 0; i < 16 / width; ++i)
		for (unsigned byte = 0; byte < width; ++byte)
		{
			const unsigned base = (i >= half ? 16 : 0) + 2 * (i % half) * width + byte;
			even[i * width + byte] = static_cast<uint8_t>(base);
			odd[i * width + byte] = static_cast<uint8_t>(base + width);
		}
	vector_shuffle(inst.ops[0], inst.ops[1], even);
	vector_shuffle(inst.ops[0], inst.ops[1], odd);
	out_.emit(operation);
}

} // namespace wasm
