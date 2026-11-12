#include "wasm/selector.h"

namespace wasm {

using namespace lowir;

namespace {

Op primitive(VectorOp op)
{
	switch (op)
	{
	case VectorOp::Addps:
		return Op::F32x4Add;
	case VectorOp::Subps:
		return Op::F32x4Sub;
	case VectorOp::Mulps:
		return Op::F32x4Mul;
	case VectorOp::Divps:
		return Op::F32x4Div;
	case VectorOp::Andps:
		return Op::V128And;
	case VectorOp::Orps:
		return Op::V128Or;
	case VectorOp::Xorps:
		return Op::V128Xor;
	case VectorOp::Addpd:
		return Op::F64x2Add;
	case VectorOp::Subpd:
		return Op::F64x2Sub;
	case VectorOp::Mulpd:
		return Op::F64x2Mul;
	case VectorOp::Divpd:
		return Op::F64x2Div;
	case VectorOp::Andpd:
		return Op::V128And;
	case VectorOp::Orpd:
		return Op::V128Or;
	case VectorOp::Xorpd:
		return Op::V128Xor;
	case VectorOp::Paddb:
		return Op::I8x16Add;
	case VectorOp::Psubb:
		return Op::I8x16Sub;
	case VectorOp::Pcmpeqb:
		return Op::I8x16Eq;
	case VectorOp::Pcmpgtb:
		return Op::I8x16GtS;
	case VectorOp::Paddw:
		return Op::I16x8Add;
	case VectorOp::Psubw:
		return Op::I16x8Sub;
	case VectorOp::Pcmpeqw:
		return Op::I16x8Eq;
	case VectorOp::Pcmpgtw:
		return Op::I16x8GtS;
	case VectorOp::Paddd:
		return Op::I32x4Add;
	case VectorOp::Psubd:
		return Op::I32x4Sub;
	case VectorOp::Pcmpeqd:
		return Op::I32x4Eq;
	case VectorOp::Pcmpgtd:
		return Op::I32x4GtS;
	case VectorOp::Paddq:
		return Op::I64x2Add;
	case VectorOp::Psubq:
		return Op::I64x2Sub;
	case VectorOp::Pcmpeqq:
		return Op::I64x2Eq;
	case VectorOp::Pcmpgtq:
		return Op::I64x2GtS;
	case VectorOp::Paddsb:
		return Op::I8x16AddSatS;
	case VectorOp::Paddsw:
		return Op::I16x8AddSatS;
	case VectorOp::Paddusb:
		return Op::I8x16AddSatU;
	case VectorOp::Paddusw:
		return Op::I16x8AddSatU;
	case VectorOp::Psubsb:
		return Op::I8x16SubSatS;
	case VectorOp::Psubsw:
		return Op::I16x8SubSatS;
	case VectorOp::Psubusb:
		return Op::I8x16SubSatU;
	case VectorOp::Psubusw:
		return Op::I16x8SubSatU;
	case VectorOp::Pavgb:
		return Op::I8x16AvgrU;
	case VectorOp::Pavgw:
		return Op::I16x8AvgrU;
	case VectorOp::Pmullw:
		return Op::I16x8Mul;
	case VectorOp::Pmulld:
		return Op::I32x4Mul;
	case VectorOp::Pmaddwd:
		return Op::I32x4DotI16x8S;
	case VectorOp::Pand:
		return Op::V128And;
	case VectorOp::Por:
		return Op::V128Or;
	case VectorOp::Pxor:
		return Op::V128Xor;
	case VectorOp::Packsswb:
		return Op::I8x16NarrowI16x8S;
	case VectorOp::Packuswb:
		return Op::I8x16NarrowI16x8U;
	case VectorOp::Packssdw:
		return Op::I16x8NarrowI32x4S;
	case VectorOp::Packusdw:
		return Op::I16x8NarrowI32x4U;
	case VectorOp::Pmaxsb:
		return Op::I8x16MaxS;
	case VectorOp::Pmaxsw:
		return Op::I16x8MaxS;
	case VectorOp::Pmaxsd:
		return Op::I32x4MaxS;
	case VectorOp::Pmaxub:
		return Op::I8x16MaxU;
	case VectorOp::Pmaxuw:
		return Op::I16x8MaxU;
	case VectorOp::Pmaxud:
		return Op::I32x4MaxU;
	case VectorOp::Pminsb:
		return Op::I8x16MinS;
	case VectorOp::Pminsw:
		return Op::I16x8MinS;
	case VectorOp::Pminsd:
		return Op::I32x4MinS;
	case VectorOp::Pminub:
		return Op::I8x16MinU;
	case VectorOp::Pminuw:
		return Op::I16x8MinU;
	case VectorOp::Pminud:
		return Op::I32x4MinU;
	case VectorOp::Pabsb:
		return Op::I8x16Abs;
	case VectorOp::Pabsw:
		return Op::I16x8Abs;
	case VectorOp::Pabsd:
		return Op::I32x4Abs;
	case VectorOp::Sqrtps:
		return Op::F32x4Sqrt;
	case VectorOp::Sqrtpd:
		return Op::F64x2Sqrt;
	case VectorOp::Cvtdq2ps:
		return Op::F32x4ConvertI32x4S;
	case VectorOp::Cvtdq2pd:
		return Op::F64x2ConvertLowI32x4S;
	case VectorOp::Cvtps2pd:
		return Op::F64x2PromoteLowF32x4;
	case VectorOp::Cvtpd2ps:
		return Op::F32x4DemoteF64x2Zero;
	case VectorOp::Pmovsxbw:
		return Op::I16x8ExtendLowI8x16S;
	case VectorOp::Pmovsxwd:
		return Op::I32x4ExtendLowI16x8S;
	case VectorOp::Pmovsxdq:
		return Op::I64x2ExtendLowI32x4S;
	case VectorOp::Pmovzxbw:
		return Op::I16x8ExtendLowI8x16U;
	case VectorOp::Pmovzxwd:
		return Op::I32x4ExtendLowI16x8U;
	case VectorOp::Pmovzxdq:
		return Op::I64x2ExtendLowI32x4U;
	default:
		return Op::Nop;
	}
}

} // namespace

void Selector::vector_constant(Literal bits)
{
	out_.emit(Op::V128Const,
		{ bits.low & 255, (bits.low >> 8) & 255, (bits.low >> 16) & 255, (bits.low >> 24) & 255, (bits.low >> 32) & 255,
			(bits.low >> 40) & 255, (bits.low >> 48) & 255, bits.low >> 56, bits.high & 255, (bits.high >> 8) & 255,
			(bits.high >> 16) & 255, (bits.high >> 24) & 255, (bits.high >> 32) & 255, (bits.high >> 40) & 255,
			(bits.high >> 48) & 255, bits.high >> 56 });
}

uint32_t Selector::vector_immediate(const lowir::Instruction& inst) const
{
	if (!inst.ops[2].is(ValueKind::Integer) || body_.literals[inst.ops[2].index].high != 0 ||
		body_.literals[inst.ops[2].index].low > 255)
		throw WasmError("Wasm vector operation needs an eight-bit literal control");
	return static_cast<uint32_t>(body_.literals[inst.ops[2].index].low);
}

void Selector::vector_scalar(const lowir::Instruction& inst)
{
	const VectorOp op = static_cast<VectorOp>(inst.sub);
	const auto shape = shape_of(op);
	const Type type = Type::scalar(vector_scalar_kind(op));
	const Op extracts[] = { Op::F32x4ExtractLane, Op::F64x2ExtractLane, Op::I32x4ExtractLane, Op::I64x2ExtractLane };
	const Op replaces[] = { Op::F32x4ReplaceLane, Op::F64x2ReplaceLane, Op::I32x4ReplaceLane, Op::I64x2ReplaceLane };
	if (shape == VectorShape::FromScalar)
	{
		vector_constant(Literal{ 0, 0 });
		value(inst.ops[0], type);
		out_.emit(replaces[static_cast<unsigned>(op) - static_cast<unsigned>(VectorOp::Fromf32)], { 0 });
	}
	else
	{
		value(inst.ops[0], Type::scalar(TypeKind::V128));
		out_.emit(extracts[static_cast<unsigned>(op) - static_cast<unsigned>(VectorOp::Tof32)], { 0 });
	}
}

void Selector::vector_instruction(const lowir::Instruction& inst)
{
	const VectorOp op = static_cast<VectorOp>(inst.sub);
	const auto shape = shape_of(op);
	const Type vector = Type::scalar(TypeKind::V128);
	const Op direct = primitive(op);
	if (shape == VectorShape::Zero)
		vector_constant(Literal{ 0, 0 });
	else if (shape == VectorShape::FromScalar || shape == VectorShape::ToScalar)
		vector_scalar(inst);
	else if (direct != Op::Nop)
	{
		value(inst.ops[0], vector);
		if (shape == VectorShape::Binary)
			value(inst.ops[1], vector);
		out_.emit(direct);
	}
	else if (op == VectorOp::Pandn || op == VectorOp::Andnps || op == VectorOp::Andnpd)
	{
		value(inst.ops[1], vector);
		value(inst.ops[0], vector);
		out_.emit(Op::V128Andnot);
	}
	else if (op == VectorOp::Pshufb)
	{
		value(inst.ops[0], vector);
		value(inst.ops[1], vector);
		vector_constant(Literal{ 0x8f8f8f8f8f8f8f8fULL, 0x8f8f8f8f8f8f8f8fULL });
		out_.emit(Op::V128And);
		out_.emit(Op::I8x16Swizzle);
	}
	else if (op == VectorOp::Movmskps || op == VectorOp::Movmskpd || op == VectorOp::Pmovmskb)
	{
		value(inst.ops[0], vector);
		out_.emit(op == VectorOp::Movmskps ? Op::I32x4Bitmask
				: op == VectorOp::Movmskpd ? Op::I64x2Bitmask
										   : Op::I8x16Bitmask);
	}
	else if ((op >= VectorOp::Psllw && op <= VectorOp::Psrad) || (op >= VectorOp::Psllwi && op <= VectorOp::Psradi))
		vector_shift(inst);
	else if ((op >= VectorOp::Movss && op <= VectorOp::Unpckhpd) ||
		(op >= VectorOp::Punpcklbw && op <= VectorOp::Punpckhqdq) || op == VectorOp::Pshufd ||
		op == VectorOp::Pshuflw || op == VectorOp::Pshufhw || op == VectorOp::Shufps || op == VectorOp::Shufpd ||
		op == VectorOp::Palignr || op == VectorOp::Movshdup || op == VectorOp::Movsldup || op == VectorOp::Movddup ||
		op == VectorOp::Pslldq || op == VectorOp::Psrldq || op == VectorOp::Blendps || op == VectorOp::Blendpd ||
		op == VectorOp::Pblendw || op == VectorOp::Insertps)
		vector_permute(inst);
	else if (op >= VectorOp::Cmpps && op <= VectorOp::Cmpsd)
		vector_compare(inst);
	else if (shape == VectorShape::Blend || shape == VectorShape::Test)
		vector_blend_test(inst);
	else if ((op >= VectorOp::Pmovsxbw && op <= VectorOp::Pmovzxdq) || op == VectorOp::Cvtps2dq ||
		op == VectorOp::Cvttps2dq || op == VectorOp::Cvtpd2dq || op == VectorOp::Cvttpd2dq ||
		(op >= VectorOp::Roundps && op <= VectorOp::Roundsd))
		vector_convert(inst);
	else if ((op >= VectorOp::Addsubps && op <= VectorOp::Hsubpd) ||
		(op >= VectorOp::Phaddw && op <= VectorOp::Phsubsw))
		vector_horizontal(inst);
	else if (op == VectorOp::Pmulhw || op == VectorOp::Pmulhuw || op == VectorOp::Pmulhrsw || op == VectorOp::Pmuludq ||
		op == VectorOp::Pmuldq || op == VectorOp::Pmaddubsw || op == VectorOp::Psadbw ||
		(op >= VectorOp::Psignb && op <= VectorOp::Psignd))
		vector_integer_math(inst);
	else if (op == VectorOp::Rcpss || op == VectorOp::Rsqrtss || op == VectorOp::Rcpps || op == VectorOp::Rsqrtps)
		vector_reciprocal(inst);
	else if (op == VectorOp::Phminposuw)
		vector_minpos(inst);
	else if (op == VectorOp::Pclmulqdq)
		vector_carryless(inst);
	else if (op == VectorOp::Dpps || op == VectorOp::Dppd)
		vector_dot(inst);
	else if (op <= VectorOp::Cvtsd2ss)
		vector_float(inst);
	else
		throw WasmError("Wasm vector operation requires legalization: " + std::string(lowir_model::spelling(op)));
	out_.emit(Op::LocalSet, { locals_[inst.result] });
}

} // namespace wasm
