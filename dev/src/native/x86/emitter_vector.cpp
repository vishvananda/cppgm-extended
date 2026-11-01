#include "lowir_model.h"
#include "native/x86/emitter.h"
#include "support/internal_error.h"

// The encodings of `vector`: legacy SSE forms through SSE4.1, with the first
// operand as destination.
namespace native {

using lowir::VectorOp;
using lowir::VectorShape;

namespace {

const Reg kX15 = static_cast<Reg>(XMM0 + 15);

// `digit` is 1 + the opcode extension of a form with the operand in r/m.
struct VectorEncoding
{
	uint8_t prefix;
	uint32_t opcode;
	uint8_t digit;
};

const VectorEncoding kVectorEncodings[] = {
	{ 0x00, 0x0f58, 0 },  // addps
	{ 0x66, 0x0f58, 0 },  // addpd
	{ 0xf3, 0x0f58, 0 },  // addss
	{ 0xf2, 0x0f58, 0 },  // addsd
	{ 0x00, 0x0f5c, 0 },  // subps
	{ 0x66, 0x0f5c, 0 },  // subpd
	{ 0xf3, 0x0f5c, 0 },  // subss
	{ 0xf2, 0x0f5c, 0 },  // subsd
	{ 0x00, 0x0f59, 0 },  // mulps
	{ 0x66, 0x0f59, 0 },  // mulpd
	{ 0xf3, 0x0f59, 0 },  // mulss
	{ 0xf2, 0x0f59, 0 },  // mulsd
	{ 0x00, 0x0f5e, 0 },  // divps
	{ 0x66, 0x0f5e, 0 },  // divpd
	{ 0xf3, 0x0f5e, 0 },  // divss
	{ 0xf2, 0x0f5e, 0 },  // divsd
	{ 0x00, 0x0f5d, 0 },  // minps
	{ 0x66, 0x0f5d, 0 },  // minpd
	{ 0xf3, 0x0f5d, 0 },  // minss
	{ 0xf2, 0x0f5d, 0 },  // minsd
	{ 0x00, 0x0f5f, 0 },  // maxps
	{ 0x66, 0x0f5f, 0 },  // maxpd
	{ 0xf3, 0x0f5f, 0 },  // maxss
	{ 0xf2, 0x0f5f, 0 },  // maxsd
	{ 0x00, 0x0f54, 0 },  // andps
	{ 0x66, 0x0f54, 0 },  // andpd
	{ 0x00, 0x0f55, 0 },  // andnps
	{ 0x66, 0x0f55, 0 },  // andnpd
	{ 0x00, 0x0f56, 0 },  // orps
	{ 0x66, 0x0f56, 0 },  // orpd
	{ 0x00, 0x0f57, 0 },  // xorps
	{ 0x66, 0x0f57, 0 },  // xorpd
	{ 0xf3, 0x0f51, 0 },  // sqrtss
	{ 0xf2, 0x0f51, 0 },  // sqrtsd
	{ 0xf3, 0x0f53, 0 },  // rcpss
	{ 0xf3, 0x0f52, 0 },  // rsqrtss
	{ 0xf3, 0x0f5a, 0 },  // cvtss2sd
	{ 0xf2, 0x0f5a, 0 },  // cvtsd2ss
	{ 0xf3, 0x0f10, 0 },  // movss
	{ 0xf2, 0x0f10, 0 },  // movsd
	{ 0x00, 0x0f12, 0 },  // movhlps
	{ 0x00, 0x0f16, 0 },  // movlhps
	{ 0x00, 0x0f14, 0 },  // unpcklps
	{ 0x66, 0x0f14, 0 },  // unpcklpd
	{ 0x00, 0x0f15, 0 },  // unpckhps
	{ 0x66, 0x0f15, 0 },  // unpckhpd
	{ 0xf2, 0x0fd0, 0 },  // addsubps
	{ 0x66, 0x0fd0, 0 },  // addsubpd
	{ 0xf2, 0x0f7c, 0 },  // haddps
	{ 0x66, 0x0f7c, 0 },  // haddpd
	{ 0xf2, 0x0f7d, 0 },  // hsubps
	{ 0x66, 0x0f7d, 0 },  // hsubpd
	{ 0x66, 0x0ffc, 0 },  // paddb
	{ 0x66, 0x0ffd, 0 },  // paddw
	{ 0x66, 0x0ffe, 0 },  // paddd
	{ 0x66, 0x0fd4, 0 },  // paddq
	{ 0x66, 0x0ff8, 0 },  // psubb
	{ 0x66, 0x0ff9, 0 },  // psubw
	{ 0x66, 0x0ffa, 0 },  // psubd
	{ 0x66, 0x0ffb, 0 },  // psubq
	{ 0x66, 0x0fec, 0 },  // paddsb
	{ 0x66, 0x0fed, 0 },  // paddsw
	{ 0x66, 0x0fdc, 0 },  // paddusb
	{ 0x66, 0x0fdd, 0 },  // paddusw
	{ 0x66, 0x0fe8, 0 },  // psubsb
	{ 0x66, 0x0fe9, 0 },  // psubsw
	{ 0x66, 0x0fd8, 0 },  // psubusb
	{ 0x66, 0x0fd9, 0 },  // psubusw
	{ 0x66, 0x0fe0, 0 },  // pavgb
	{ 0x66, 0x0fe3, 0 },  // pavgw
	{ 0x66, 0x0fd5, 0 },  // pmullw
	{ 0x66, 0xf3840, 0 }, // pmulld
	{ 0x66, 0x0fe5, 0 },  // pmulhw
	{ 0x66, 0x0fe4, 0 },  // pmulhuw
	{ 0x66, 0xf380b, 0 }, // pmulhrsw
	{ 0x66, 0x0ff4, 0 },  // pmuludq
	{ 0x66, 0xf3828, 0 }, // pmuldq
	{ 0x66, 0x0ff5, 0 },  // pmaddwd
	{ 0x66, 0xf3804, 0 }, // pmaddubsw
	{ 0x66, 0x0ff6, 0 },  // psadbw
	{ 0x66, 0xf383c, 0 }, // pmaxsb
	{ 0x66, 0x0fee, 0 },  // pmaxsw
	{ 0x66, 0xf383d, 0 }, // pmaxsd
	{ 0x66, 0x0fde, 0 },  // pmaxub
	{ 0x66, 0xf383e, 0 }, // pmaxuw
	{ 0x66, 0xf383f, 0 }, // pmaxud
	{ 0x66, 0xf3838, 0 }, // pminsb
	{ 0x66, 0x0fea, 0 },  // pminsw
	{ 0x66, 0xf3839, 0 }, // pminsd
	{ 0x66, 0x0fda, 0 },  // pminub
	{ 0x66, 0xf383a, 0 }, // pminuw
	{ 0x66, 0xf383b, 0 }, // pminud
	{ 0x66, 0x0fdb, 0 },  // pand
	{ 0x66, 0x0fdf, 0 },  // pandn
	{ 0x66, 0x0feb, 0 },  // por
	{ 0x66, 0x0fef, 0 },  // pxor
	{ 0x66, 0x0f74, 0 },  // pcmpeqb
	{ 0x66, 0x0f75, 0 },  // pcmpeqw
	{ 0x66, 0x0f76, 0 },  // pcmpeqd
	{ 0x66, 0xf3829, 0 }, // pcmpeqq
	{ 0x66, 0x0f64, 0 },  // pcmpgtb
	{ 0x66, 0x0f65, 0 },  // pcmpgtw
	{ 0x66, 0x0f66, 0 },  // pcmpgtd
	{ 0x66, 0xf3837, 0 }, // pcmpgtq
	{ 0x66, 0x0ff1, 0 },  // psllw
	{ 0x66, 0x0ff2, 0 },  // pslld
	{ 0x66, 0x0ff3, 0 },  // psllq
	{ 0x66, 0x0fd1, 0 },  // psrlw
	{ 0x66, 0x0fd2, 0 },  // psrld
	{ 0x66, 0x0fd3, 0 },  // psrlq
	{ 0x66, 0x0fe1, 0 },  // psraw
	{ 0x66, 0x0fe2, 0 },  // psrad
	{ 0x66, 0x0f63, 0 },  // packsswb
	{ 0x66, 0x0f6b, 0 },  // packssdw
	{ 0x66, 0x0f67, 0 },  // packuswb
	{ 0x66, 0xf382b, 0 }, // packusdw
	{ 0x66, 0x0f60, 0 },  // punpcklbw
	{ 0x66, 0x0f61, 0 },  // punpcklwd
	{ 0x66, 0x0f62, 0 },  // punpckldq
	{ 0x66, 0x0f6c, 0 },  // punpcklqdq
	{ 0x66, 0x0f68, 0 },  // punpckhbw
	{ 0x66, 0x0f69, 0 },  // punpckhwd
	{ 0x66, 0x0f6a, 0 },  // punpckhdq
	{ 0x66, 0x0f6d, 0 },  // punpckhqdq
	{ 0x66, 0xf3800, 0 }, // pshufb
	{ 0x66, 0xf3808, 0 }, // psignb
	{ 0x66, 0xf3809, 0 }, // psignw
	{ 0x66, 0xf380a, 0 }, // psignd
	{ 0x66, 0xf3801, 0 }, // phaddw
	{ 0x66, 0xf3802, 0 }, // phaddd
	{ 0x66, 0xf3803, 0 }, // phaddsw
	{ 0x66, 0xf3805, 0 }, // phsubw
	{ 0x66, 0xf3806, 0 }, // phsubd
	{ 0x66, 0xf3807, 0 }, // phsubsw
	{ 0x00, 0x0f51, 0 },  // sqrtps
	{ 0x66, 0x0f51, 0 },  // sqrtpd
	{ 0x00, 0x0f53, 0 },  // rcpps
	{ 0x00, 0x0f52, 0 },  // rsqrtps
	{ 0xf3, 0x0f16, 0 },  // movshdup
	{ 0xf3, 0x0f12, 0 },  // movsldup
	{ 0xf2, 0x0f12, 0 },  // movddup
	{ 0x66, 0xf381c, 0 }, // pabsb
	{ 0x66, 0xf381d, 0 }, // pabsw
	{ 0x66, 0xf381e, 0 }, // pabsd
	{ 0x66, 0xf3820, 0 }, // pmovsxbw
	{ 0x66, 0xf3821, 0 }, // pmovsxbd
	{ 0x66, 0xf3822, 0 }, // pmovsxbq
	{ 0x66, 0xf3823, 0 }, // pmovsxwd
	{ 0x66, 0xf3824, 0 }, // pmovsxwq
	{ 0x66, 0xf3825, 0 }, // pmovsxdq
	{ 0x66, 0xf3830, 0 }, // pmovzxbw
	{ 0x66, 0xf3831, 0 }, // pmovzxbd
	{ 0x66, 0xf3832, 0 }, // pmovzxbq
	{ 0x66, 0xf3833, 0 }, // pmovzxwd
	{ 0x66, 0xf3834, 0 }, // pmovzxwq
	{ 0x66, 0xf3835, 0 }, // pmovzxdq
	{ 0x00, 0x0f5b, 0 },  // cvtdq2ps
	{ 0xf3, 0x0fe6, 0 },  // cvtdq2pd
	{ 0x66, 0x0f5b, 0 },  // cvtps2dq
	{ 0xf3, 0x0f5b, 0 },  // cvttps2dq
	{ 0xf2, 0x0fe6, 0 },  // cvtpd2dq
	{ 0x66, 0x0fe6, 0 },  // cvttpd2dq
	{ 0x00, 0x0f5a, 0 },  // cvtps2pd
	{ 0x66, 0x0f5a, 0 },  // cvtpd2ps
	{ 0x00, 0x0fc2, 0 },  // cmpps
	{ 0x66, 0x0fc2, 0 },  // cmppd
	{ 0xf3, 0x0fc2, 0 },  // cmpss
	{ 0xf2, 0x0fc2, 0 },  // cmpsd
	{ 0x00, 0x0fc6, 0 },  // shufps
	{ 0x66, 0x0fc6, 0 },  // shufpd
	{ 0x66, 0xf3a0f, 0 }, // palignr
	{ 0x66, 0xf3a0c, 0 }, // blendps
	{ 0x66, 0xf3a0d, 0 }, // blendpd
	{ 0x66, 0xf3a0e, 0 }, // pblendw
	{ 0x66, 0x0f70, 0 },  // pshufd
	{ 0xf2, 0x0f70, 0 },  // pshuflw
	{ 0xf3, 0x0f70, 0 },  // pshufhw
	{ 0x66, 0x0f71, 7 },  // psllwi
	{ 0x66, 0x0f72, 7 },  // pslldi
	{ 0x66, 0x0f73, 7 },  // psllqi
	{ 0x66, 0x0f71, 3 },  // psrlwi
	{ 0x66, 0x0f72, 3 },  // psrldi
	{ 0x66, 0x0f73, 3 },  // psrlqi
	{ 0x66, 0x0f71, 5 },  // psrawi
	{ 0x66, 0x0f72, 5 },  // psradi
	{ 0x66, 0x0f73, 8 },  // pslldq
	{ 0x66, 0x0f73, 4 },  // psrldq
	{ 0x66, 0xf3814, 0 }, // blendvps
	{ 0x66, 0xf3815, 0 }, // blendvpd
	{ 0x66, 0xf3810, 0 }, // pblendvb
	{ 0x00, 0x0f50, 0 },  // movmskps
	{ 0x66, 0x0f50, 0 },  // movmskpd
	{ 0x66, 0x0fd7, 0 },  // pmovmskb
	{ 0x66, 0xf3817, 0 }, // ptestz
	{ 0x66, 0xf3817, 0 }, // ptestc
	{ 0x66, 0xf3817, 0 }, // ptestnzc
	{ 0x00, 0x0000, 0 },  // pzero
	{ 0x00, 0x0000, 0 },  // fromf32
	{ 0x00, 0x0000, 0 },  // fromf64
	{ 0x00, 0x0000, 0 },  // fromi32
	{ 0x00, 0x0000, 0 },  // fromi64
	{ 0x00, 0x0000, 0 },  // tof32
	{ 0x00, 0x0000, 0 },  // tof64
	{ 0x00, 0x0000, 0 },  // toi32
	{ 0x00, 0x0000, 0 },  // toi64
};
static_assert(sizeof(kVectorEncodings) / sizeof(kVectorEncodings[0]) == lowir::kVectorOpCount,
	"one encoding per vector operation");

} // namespace

// The whole of `src`, register or home, into d.
void FunctionEmitter::vector_move(Reg d, const Operand& src)
{
	if (src.kind != OperandKind::Reg)
		asm_.sse(0, 0x10, d, rm(src)); // movups
	else if (src.reg != d)
		asm_.sse(0, 0x28, d, RM::r(src.reg)); // movaps
}

// d <- a op b for the destructive forms. A destination that is also the
// second operand swaps with the first if the operation commutes, else keeps
// the second operand in the vector scratch.
Reg FunctionEmitter::vector_operands(const Inst& inst)
{
	const VectorOp op = static_cast<VectorOp>(inst.sub);
	const Reg d = inst.ops[0].reg;
	const Operand& a = inst.ops[1];
	Reg src = inst.ops[2].reg;
	if (a.kind == OperandKind::Reg && a.reg == d)
		return src;
	if (src != d)
	{
		vector_move(d, a);
		return src;
	}
	if (lowir::vector_commutes(op) && a.kind == OperandKind::Reg)
		return a.reg;
	asm_.sse(0, 0x28, kX15, RM::r(src));
	vector_move(d, a);
	return kX15;
}

// The operations with no encoding row of their own.
void FunctionEmitter::vector_special(const Inst& inst)
{
	const Reg d = inst.ops[0].reg;
	const Reg a = inst.nops > 1 ? inst.ops[1].reg : kNoReg;
	switch (static_cast<VectorOp>(inst.sub))
	{
	case VectorOp::Pzero:
		asm_.sse(0x66, 0xef, d, RM::r(d));
		break;
	case VectorOp::Fromf32:
		// movss between registers keeps the destination's other lanes.
		if (a == d)
		{
			asm_.sse(0x66, 0xef, kX15, RM::r(kX15));
			asm_.sse(0xf3, 0x10, kX15, RM::r(a));
			asm_.sse(0, 0x28, d, RM::r(kX15));
			break;
		}
		asm_.sse(0x66, 0xef, d, RM::r(d));
		asm_.sse(0xf3, 0x10, d, RM::r(a));
		break;
	case VectorOp::Fromf64:
		asm_.sse(0xf3, 0x7e, d, RM::r(a)); // movq
		break;
	case VectorOp::Fromi32:
		asm_.sse(0x66, 0x6e, d, RM::r(a)); // movd
		break;
	case VectorOp::Fromi64:
		asm_.sse(0x66, 0x6e, d, RM::r(a), true); // movq
		break;
	case VectorOp::Toi32:
		asm_.sse_store(0x66, 0x7e, RM::r(d), a); // movd
		break;
	case VectorOp::Toi64:
		asm_.op(0x66, true, 0x0f7e, 2, a & 15, RM::r(d)); // movq
		break;
	default:
		throw InternalError("a vector operation with no encoding");
	}
}

void FunctionEmitter::vector_op(const Inst& inst)
{
	const VectorOp op = static_cast<VectorOp>(inst.sub);
	const VectorEncoding& e = kVectorEncodings[inst.sub];
	const unsigned bytes = e.opcode > 0xffff ? 3 : 2;
	auto encode = [&](unsigned reg, Reg src) { asm_.op(e.prefix, false, e.opcode, bytes, reg & 15, RM::r(src)); };
	switch (lowir::shape_of(op))
	{
	case VectorShape::Binary:
	case VectorShape::Blend:
		encode(inst.ops[0].reg, vector_operands(inst));
		break;
	case VectorShape::BinaryImmediate:
		encode(inst.ops[0].reg, vector_operands(inst));
		asm_.byte(static_cast<uint8_t>(inst.aux));
		break;
	case VectorShape::Unary:
	case VectorShape::Mask:
		encode(inst.ops[0].reg, inst.ops[1].reg);
		break;
	case VectorShape::Test:
		encode(inst.ops[0].reg, inst.ops[1].reg);
		break;
	case VectorShape::UnaryImmediate:
		if (e.digit == 0)
			encode(inst.ops[0].reg, inst.ops[1].reg);
		else
		{
			vector_move(inst.ops[0].reg, inst.ops[1]);
			encode(e.digit - 1u, inst.ops[0].reg);
		}
		asm_.byte(static_cast<uint8_t>(inst.aux));
		break;
	case VectorShape::Zero:
	case VectorShape::FromScalar:
	case VectorShape::ToScalar:
		vector_special(inst);
		break;
	}
}

} // namespace native
