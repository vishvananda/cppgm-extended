#include "wasm/selector.h"

namespace wasm {

using namespace lowir;

void Selector::vector_shuffle(Value first, Value second, const uint8_t* lanes)
{
	for (Value operand : { first, second })
		if (operand.is(ValueKind::None))
			vector_constant(Literal{ 0, 0 });
		else
			value(operand, Type::scalar(TypeKind::V128));
	vector_shuffle_stack(lanes);
}

void Selector::vector_shuffle_stack(const uint8_t* lanes)
{
	out_.emit(Op::I8x16Shuffle,
		{ lanes[0], lanes[1], lanes[2], lanes[3], lanes[4], lanes[5], lanes[6], lanes[7], lanes[8], lanes[9], lanes[10],
			lanes[11], lanes[12], lanes[13], lanes[14], lanes[15] });
}

void Selector::vector_permute(const lowir::Instruction& inst)
{
	const VectorOp op = static_cast<VectorOp>(inst.sub);
	const VectorShape shape = shape_of(op);
	const uint32_t imm = shape == VectorShape::UnaryImmediate || shape == VectorShape::BinaryImmediate
		? vector_immediate(inst)
		: 0;
	Value a = inst.ops[0], b = shape == VectorShape::Unary || shape == VectorShape::UnaryImmediate ? a : inst.ops[1];
	uint8_t lanes[16];
	for (unsigned i = 0; i < 16; ++i)
		lanes[i] = static_cast<uint8_t>(i);
	unsigned width = 0, start = 0;
	if (op >= VectorOp::Punpcklbw && op <= VectorOp::Punpckhqdq)
	{
		const unsigned kind = static_cast<unsigned>(op) - static_cast<unsigned>(VectorOp::Punpcklbw);
		width = 1u << (kind & 3);
		start = kind >= 4 ? 8 : 0;
	}
	else if (op >= VectorOp::Unpcklps && op <= VectorOp::Unpckhpd)
	{
		const unsigned kind = static_cast<unsigned>(op) - static_cast<unsigned>(VectorOp::Unpcklps);
		width = (kind & 1) ? 8 : 4;
		start = kind >= 2 ? 8 : 0;
	}
	if (width != 0)
	{
		for (unsigned i = 0; i < 8 / width; ++i)
			for (unsigned byte = 0; byte < width; ++byte)
			{
				lanes[2 * i * width + byte] = static_cast<uint8_t>(start + i * width + byte);
				lanes[(2 * i + 1) * width + byte] = static_cast<uint8_t>(16 + start + i * width + byte);
			}
	}
	else if (op == VectorOp::Pshufd || op == VectorOp::Pshuflw || op == VectorOp::Pshufhw || op == VectorOp::Shufps ||
		op == VectorOp::Shufpd)
	{
		width = op == VectorOp::Shufpd ? 8 : (op == VectorOp::Pshuflw || op == VectorOp::Pshufhw ? 2 : 4);
		const unsigned count = width == 8 ? 2 : 4;
		const unsigned base = op == VectorOp::Pshufhw ? 8 : 0;
		for (unsigned i = 0; i < count; ++i)
		{
			const unsigned source = (imm >> (i * (width == 8 ? 1 : 2))) & (width == 8 ? 1 : 3);
			const unsigned input = (op == VectorOp::Shufps && i >= 2) || (op == VectorOp::Shufpd && i == 1) ? 16 : 0;
			for (unsigned byte = 0; byte < width; ++byte)
				lanes[base + i * width + byte] = static_cast<uint8_t>(input + base + source * width + byte);
		}
	}
	else if (op == VectorOp::Blendps || op == VectorOp::Blendpd || op == VectorOp::Pblendw)
	{
		width = op == VectorOp::Blendps ? 4 : op == VectorOp::Blendpd ? 8 : 2;
		for (unsigned i = 0; i < 16; ++i)
			lanes[i] = static_cast<uint8_t>(i + (((imm & (1u << (i / width))) != 0) ? 16 : 0));
	}
	else if (op == VectorOp::Insertps)
	{
		for (unsigned i = 0; i < 4; ++i)
			lanes[4 * ((imm >> 4) & 3) + i] = static_cast<uint8_t>(16 + 4 * (imm >> 6) + i);
	}
	else if (op == VectorOp::Palignr)
	{
		if (imm >= 32)
		{
			vector_constant(Literal{ 0, 0 });
			return;
		}
		a = imm < 16 ? inst.ops[1] : inst.ops[0];
		b = imm < 16 ? inst.ops[0] : Value::none();
		for (unsigned i = 0; i < 16; ++i)
			lanes[i] = static_cast<uint8_t>((imm & 15) + i);
	}
	else if (op == VectorOp::Pslldq || op == VectorOp::Psrldq)
	{
		b = Value::none();
		for (unsigned i = 0; i < 16; ++i)
		{
			const int source = op == VectorOp::Pslldq ? static_cast<int>(i) - static_cast<int>(imm) : i + imm;
			lanes[i] = static_cast<uint8_t>(source >= 0 && source < 16 ? source : 16);
		}
	}
	else if (op == VectorOp::Movss || op == VectorOp::Movsd)
	{
		for (unsigned i = 0; i < (op == VectorOp::Movss ? 4u : 8u); ++i)
			lanes[i] = static_cast<uint8_t>(16 + i);
	}
	else if (op == VectorOp::Movhlps || op == VectorOp::Movlhps)
	{
		for (unsigned i = 0; i < 8; ++i)
			lanes[op == VectorOp::Movhlps ? i : 8 + i] = static_cast<uint8_t>((op == VectorOp::Movhlps ? 24 : 16) + i);
	}
	else if (op == VectorOp::Movshdup || op == VectorOp::Movsldup || op == VectorOp::Movddup)
	{
		for (unsigned i = 0; i < 16; ++i)
			lanes[i] = static_cast<uint8_t>(
				op == VectorOp::Movddup ? i & 7u : (i & ~7u) + (op == VectorOp::Movshdup ? 4u : 0u) + (i & 3u));
	}
	else
		throw WasmError("unknown Wasm vector permutation");
	vector_shuffle(a, b, lanes);
	if (op == VectorOp::Insertps)
	{
		Literal mask{ 0, 0 };
		for (unsigned i = 0; i < 16; ++i)
			if ((imm & (1u << (i / 4))) == 0)
				(i < 8 ? mask.low : mask.high) |= uint64_t(255) << (8 * (i & 7));
		vector_constant(mask);
		out_.emit(Op::V128And);
	}
}

} // namespace wasm
