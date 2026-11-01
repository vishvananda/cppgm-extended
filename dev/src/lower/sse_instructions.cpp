// The `vector` instruction of each SSE builtin (lex/gnu_builtins.h's
// VectorLanewise) on sixteen-byte vectors. A form this table does not give
// is computed lane by lane (lower/sse_lowering.cpp).

#include "lower/sse_instructions.h"

namespace lower {

namespace {

using lowir::VectorOp;

VectorOp Offset(VectorOp base, unsigned step)
{
	return static_cast<VectorOp>(static_cast<unsigned>(base) + step);
}

// 0, 1 and 2 for lanes of 1, 2 and 4 bytes.
unsigned WidthIndex(uint32_t bytes)
{
	return bytes == 1 ? 0 : bytes == 2 ? 1 : 2;
}

// Ps, pd, ss and sd follow one another in that order.
VectorOp FloatForm(VectorOp base, const SseForm& form)
{
	return Offset(base, (form.lane_bytes == 8 ? 1u : 0u) + (form.scalar ? 2u : 0u));
}

bool CompareForm(VectorLanewiseOp operation, const SseForm& form, SseInstruction& out)
{
	unsigned predicate = 0;
	switch (operation)
	{
	case VectorLanewiseOp::CompareEq:
		predicate = 0;
		break;
	case VectorLanewiseOp::CompareLt:
		predicate = 1;
		break;
	case VectorLanewiseOp::CompareLe:
		predicate = 2;
		break;
	case VectorLanewiseOp::CompareUnordered:
		predicate = 3;
		break;
	case VectorLanewiseOp::CompareNe:
		predicate = 4;
		break;
	case VectorLanewiseOp::CompareNotLt:
		predicate = 5;
		break;
	case VectorLanewiseOp::CompareNotLe:
		predicate = 6;
		break;
	case VectorLanewiseOp::CompareOrdered:
		predicate = 7;
		break;
	case VectorLanewiseOp::CompareGt:
		predicate = 1;
		out.swapped = true;
		break;
	case VectorLanewiseOp::CompareGe:
		predicate = 2;
		out.swapped = true;
		break;
	case VectorLanewiseOp::CompareNotGt:
		predicate = 5;
		out.swapped = true;
		break;
	case VectorLanewiseOp::CompareNotGe:
		predicate = 6;
		out.swapped = true;
		break;
	default:
		return false;
	}
	// The scalar forms keep the first operand's other lanes.
	if (form.scalar && out.swapped)
		return false;
	out.op = FloatForm(VectorOp::Cmpps, form);
	out.immediate = static_cast<int32_t>(predicate);
	return true;
}

bool ConvertForm(VectorLanewiseOp operation, const SseForm& form, SseInstruction& out)
{
	if (!form.vector_result)
		return false;
	const bool from_float = form.source_floating;
	if (operation == VectorLanewiseOp::ConvertLow)
	{
		if (!form.floating || !from_float || !form.second_vector)
			return false;
		out.op = form.lane_bytes == 4 ? VectorOp::Cvtsd2ss : VectorOp::Cvtss2sd;
		return true;
	}
	if (operation == VectorLanewiseOp::ConvertNearest)
	{
		out.op = form.source_bytes == 4 ? VectorOp::Cvtps2dq : VectorOp::Cvtpd2dq;
		return !form.floating && from_float;
	}
	if (form.floating && !from_float)
		out.op = form.lane_bytes == 4 ? VectorOp::Cvtdq2ps : VectorOp::Cvtdq2pd;
	else if (form.floating)
		out.op = form.lane_bytes == 4 ? VectorOp::Cvtpd2ps : VectorOp::Cvtps2pd;
	else
		out.op = VectorOp::Cvttpd2dq;
	return true;
}

bool ExtendForm(VectorLanewiseOp operation, const SseForm& form, SseInstruction& out)
{
	const VectorOp base = operation == VectorLanewiseOp::Extend ? VectorOp::Pmovsxbw : VectorOp::Pmovzxbw;
	if (form.source_bytes == 1)
		out.op = Offset(base, form.lane_bytes == 2 ? 0 : form.lane_bytes == 4 ? 1 : 2);
	else if (form.source_bytes == 2)
		out.op = Offset(base, form.lane_bytes == 4 ? 3 : 4);
	else
		out.op = Offset(base, 5);
	return true;
}

bool MoveForm(VectorLanewiseOp operation, const SseForm& form, SseInstruction& out)
{
	switch (operation)
	{
	case VectorLanewiseOp::MoveScalar:
		out.op = form.lane_bytes == 4 ? VectorOp::Movss : VectorOp::Movsd;
		return true;
	case VectorLanewiseOp::MoveHighToLow:
		out.op = VectorOp::Movhlps;
		return true;
	case VectorLanewiseOp::MoveLowToHigh:
		out.op = VectorOp::Movlhps;
		return true;
	case VectorLanewiseOp::UnpackLow:
		out.op = form.lane_bytes == 4 ? VectorOp::Unpcklps : VectorOp::Unpcklpd;
		return true;
	case VectorLanewiseOp::UnpackHigh:
		out.op = form.lane_bytes == 4 ? VectorOp::Unpckhps : VectorOp::Unpckhpd;
		return true;
	case VectorLanewiseOp::DuplicateOdd:
		out.op = VectorOp::Movshdup;
		return true;
	case VectorLanewiseOp::DuplicateEven:
		out.op = VectorOp::Movsldup;
		return true;
	default:
		return false;
	}
}

bool IntegerForm(VectorLanewiseOp operation, const SseForm& form, SseInstruction& out)
{
	const unsigned width = WidthIndex(form.lane_bytes);
	switch (operation)
	{
	case VectorLanewiseOp::AddSaturate:
		out.op = Offset(VectorOp::Paddsb, width);
		return true;
	case VectorLanewiseOp::AddSaturateUnsigned:
		out.op = Offset(VectorOp::Paddusb, width);
		return true;
	case VectorLanewiseOp::SubSaturate:
		out.op = Offset(VectorOp::Psubsb, width);
		return true;
	case VectorLanewiseOp::SubSaturateUnsigned:
		out.op = Offset(VectorOp::Psubusb, width);
		return true;
	case VectorLanewiseOp::Average:
		out.op = Offset(VectorOp::Pavgb, width);
		return true;
	case VectorLanewiseOp::MulHigh:
		out.op = VectorOp::Pmulhw;
		return true;
	case VectorLanewiseOp::MulHighUnsigned:
		out.op = VectorOp::Pmulhuw;
		return true;
	case VectorLanewiseOp::MulHighRound:
		out.op = VectorOp::Pmulhrsw;
		return true;
	case VectorLanewiseOp::MulWide:
		out.op = VectorOp::Pmuldq;
		return true;
	case VectorLanewiseOp::MulWideUnsigned:
		out.op = VectorOp::Pmuludq;
		return true;
	case VectorLanewiseOp::MulAdd:
		out.op = VectorOp::Pmaddwd;
		return true;
	case VectorLanewiseOp::MulAddBytes:
		out.op = VectorOp::Pmaddubsw;
		return true;
	case VectorLanewiseOp::SumAbsoluteDifference:
		out.op = VectorOp::Psadbw;
		return true;
	case VectorLanewiseOp::MaxSigned:
		out.op = Offset(VectorOp::Pmaxsb, width);
		return true;
	case VectorLanewiseOp::MaxUnsigned:
		out.op = Offset(VectorOp::Pmaxub, width);
		return true;
	case VectorLanewiseOp::MinSigned:
		out.op = Offset(VectorOp::Pminsb, width);
		return true;
	case VectorLanewiseOp::MinUnsigned:
		out.op = Offset(VectorOp::Pminub, width);
		return true;
	case VectorLanewiseOp::Abs:
		out.op = Offset(VectorOp::Pabsb, width);
		return true;
	case VectorLanewiseOp::Sign:
		out.op = Offset(VectorOp::Psignb, width);
		return true;
	case VectorLanewiseOp::ByteShuffle:
		out.op = VectorOp::Pshufb;
		return true;
	default:
		return false;
	}
}

bool ShiftForm(VectorLanewiseOp operation, const SseForm& form, SseInstruction& out)
{
	const unsigned width = form.lane_bytes == 2 ? 0 : form.lane_bytes == 4 ? 1 : 2;
	if (operation == VectorLanewiseOp::ShiftRightArithmetic && form.lane_bytes == 8)
		return false;
	out.op = Offset(operation == VectorLanewiseOp::ShiftLeft   ? VectorOp::Psllw
			: operation == VectorLanewiseOp::ShiftRightLogical ? VectorOp::Psrlw
															   : VectorOp::Psraw,
		width);
	return true;
}

bool HorizontalForm(VectorLanewiseOp operation, const SseForm& form, SseInstruction& out)
{
	const bool wide = form.lane_bytes == 4;
	switch (operation)
	{
	case VectorLanewiseOp::HorizontalAdd:
		out.op = form.floating ? (wide ? VectorOp::Haddps : VectorOp::Haddpd)
							   : (wide ? VectorOp::Phaddd : VectorOp::Phaddw);
		return true;
	case VectorLanewiseOp::HorizontalSub:
		out.op = form.floating ? (wide ? VectorOp::Hsubps : VectorOp::Hsubpd)
							   : (wide ? VectorOp::Phsubd : VectorOp::Phsubw);
		return true;
	case VectorLanewiseOp::HorizontalAddSaturate:
		out.op = VectorOp::Phaddsw;
		return true;
	default:
		out.op = VectorOp::Phsubsw;
		return true;
	}
}

} // namespace

bool SseInstructionOf(VectorLanewiseOp operation, const SseForm& form, SseInstruction& out)
{
	switch (operation)
	{
	case VectorLanewiseOp::Add:
		out.op = FloatForm(VectorOp::Addps, form);
		return form.floating;
	case VectorLanewiseOp::Sub:
		out.op = FloatForm(VectorOp::Subps, form);
		return form.floating;
	case VectorLanewiseOp::Mul:
		out.op = FloatForm(VectorOp::Mulps, form);
		return form.floating;
	case VectorLanewiseOp::Div:
		out.op = FloatForm(VectorOp::Divps, form);
		return form.floating;
	case VectorLanewiseOp::Min:
		out.op = FloatForm(VectorOp::Minps, form);
		return form.floating;
	case VectorLanewiseOp::Max:
		out.op = FloatForm(VectorOp::Maxps, form);
		return form.floating;
	case VectorLanewiseOp::Sqrt:
		out.op = form.scalar ? (form.lane_bytes == 8 ? VectorOp::Sqrtsd : VectorOp::Sqrtss)
							 : (form.lane_bytes == 8 ? VectorOp::Sqrtpd : VectorOp::Sqrtps);
		out.repeated = form.scalar;
		return true;
	case VectorLanewiseOp::Reciprocal:
		out.op = form.scalar ? VectorOp::Rcpss : VectorOp::Rcpps;
		out.repeated = form.scalar;
		return true;
	case VectorLanewiseOp::ReciprocalSqrt:
		out.op = form.scalar ? VectorOp::Rsqrtss : VectorOp::Rsqrtps;
		out.repeated = form.scalar;
		return true;
	case VectorLanewiseOp::And:
		out.op = Offset(VectorOp::Andps, form.lane_bytes == 8 ? 1 : 0);
		return form.floating;
	case VectorLanewiseOp::AndNot:
		out.op = Offset(VectorOp::Andnps, form.lane_bytes == 8 ? 1 : 0);
		return form.floating;
	case VectorLanewiseOp::Or:
		out.op = Offset(VectorOp::Orps, form.lane_bytes == 8 ? 1 : 0);
		return form.floating;
	case VectorLanewiseOp::Xor:
		out.op = Offset(VectorOp::Xorps, form.lane_bytes == 8 ? 1 : 0);
		return form.floating;
	case VectorLanewiseOp::CompareEq:
	case VectorLanewiseOp::CompareLt:
	case VectorLanewiseOp::CompareLe:
	case VectorLanewiseOp::CompareGt:
	case VectorLanewiseOp::CompareGe:
	case VectorLanewiseOp::CompareNe:
	case VectorLanewiseOp::CompareNotLt:
	case VectorLanewiseOp::CompareNotLe:
	case VectorLanewiseOp::CompareNotGt:
	case VectorLanewiseOp::CompareNotGe:
	case VectorLanewiseOp::CompareOrdered:
	case VectorLanewiseOp::CompareUnordered:
		return CompareForm(operation, form, out);
	case VectorLanewiseOp::TestZero:
		out.op = VectorOp::Ptestz;
		return true;
	case VectorLanewiseOp::TestCarry:
		out.op = VectorOp::Ptestc;
		return true;
	case VectorLanewiseOp::TestMixed:
		out.op = VectorOp::Ptestnzc;
		return true;
	case VectorLanewiseOp::Convert:
	case VectorLanewiseOp::ConvertNearest:
	case VectorLanewiseOp::ConvertLow:
		return ConvertForm(operation, form, out);
	case VectorLanewiseOp::Extend:
	case VectorLanewiseOp::ExtendUnsigned:
		return ExtendForm(operation, form, out);
	case VectorLanewiseOp::MoveScalar:
	case VectorLanewiseOp::MoveHighToLow:
	case VectorLanewiseOp::MoveLowToHigh:
	case VectorLanewiseOp::UnpackLow:
	case VectorLanewiseOp::UnpackHigh:
	case VectorLanewiseOp::DuplicateOdd:
	case VectorLanewiseOp::DuplicateEven:
		return MoveForm(operation, form, out);
	case VectorLanewiseOp::ShiftLeft:
	case VectorLanewiseOp::ShiftRightLogical:
	case VectorLanewiseOp::ShiftRightArithmetic:
		return ShiftForm(operation, form, out);
	case VectorLanewiseOp::HorizontalAdd:
	case VectorLanewiseOp::HorizontalSub:
	case VectorLanewiseOp::HorizontalAddSaturate:
	case VectorLanewiseOp::HorizontalSubSaturate:
		return HorizontalForm(operation, form, out);
	case VectorLanewiseOp::AddSub:
		out.op = form.lane_bytes == 4 ? VectorOp::Addsubps : VectorOp::Addsubpd;
		return true;
	case VectorLanewiseOp::ByteAlign:
		out.op = VectorOp::Palignr;
		out.literal_operand = 2;
		return true;
	case VectorLanewiseOp::Blend:
		out.op = form.lane_bytes == 2 ? VectorOp::Pblendw
			: form.lane_bytes == 4    ? VectorOp::Blendps
									  : VectorOp::Blendpd;
		out.literal_operand = 2;
		return true;
	case VectorLanewiseOp::BlendVariable:
		out.op = form.lane_bytes == 1 ? VectorOp::Pblendvb
			: form.lane_bytes == 4    ? VectorOp::Blendvps
									  : VectorOp::Blendvpd;
		return true;
	default:
		return IntegerForm(operation, form, out);
	}
}

} // namespace lower
