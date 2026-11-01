#include "lex/gnu_builtins.h"

#include <algorithm>
#include <cstring>

namespace {

// The builtin functions by spelling, sorted (strcmp order); the libm
// family, the __atomic and the __sync builtins are found by their names'
// parts (FindGnuBuiltin).
const struct
{
	const char* spelling;
	GnuBuiltinEntry entry;
} kGnuBuiltins[] = {
	{ "__builtin_COLUMN", { GnuBuiltin::SourceLocation, 3, nullptr, 0 } },
	{ "__builtin_FILE", { GnuBuiltin::SourceLocation, 0, nullptr, 0 } },
	{ "__builtin_FUNCTION", { GnuBuiltin::SourceLocation, 2, nullptr, 0 } },
	{ "__builtin_LINE", { GnuBuiltin::SourceLocation, 1, nullptr, 0 } },
	{ "__builtin_abort", { GnuBuiltin::Abort, 0, "v", 0 } },
	{ "__builtin_abs", { GnuBuiltin::Library, 0, "ii", 0 } },
	{ "__builtin_add_overflow", { GnuBuiltin::Overflow, 0, nullptr, 0 } },
	{ "__builtin_addressof", { GnuBuiltin::AddressOf, 0, nullptr, 0 } },
	{ "__builtin_alloca", { GnuBuiltin::Alloca, 0, nullptr, 0 } },
	{ "__builtin_assume_aligned", { GnuBuiltin::AssumeAligned, 0, nullptr, 0 } },
	{ "__builtin_bswap16", { GnuBuiltin::BitCount, 0, nullptr, 0 } },
	{ "__builtin_bswap32", { GnuBuiltin::BitCount, 1, nullptr, 0 } },
	{ "__builtin_bswap64", { GnuBuiltin::BitCount, 2, nullptr, 0 } },
	{ "__builtin_bzero", { GnuBuiltin::Library, 0, "vv*z", 0 } },
	{ "__builtin_clz", { GnuBuiltin::BitCount, 3, nullptr, 0 } },
	{ "__builtin_clzg", { GnuBuiltin::GenericBitCount, 0, nullptr, 0 } },
	{ "__builtin_clzl", { GnuBuiltin::BitCount, 4, nullptr, 0 } },
	{ "__builtin_clzll", { GnuBuiltin::BitCount, 5, nullptr, 0 } },
	{ "__builtin_complex", { GnuBuiltin::Complex, 0, nullptr, 0 } },
	{ "__builtin_constant_p", { GnuBuiltin::ConstantP, 0, nullptr, 0 } },
	{ "__builtin_ctz", { GnuBuiltin::BitCount, 6, nullptr, 0 } },
	{ "__builtin_ctzg", { GnuBuiltin::GenericBitCount, 1, nullptr, 0 } },
	{ "__builtin_ctzl", { GnuBuiltin::BitCount, 7, nullptr, 0 } },
	{ "__builtin_ctzll", { GnuBuiltin::BitCount, 8, nullptr, 0 } },
	{ "__builtin_dynamic_object_size", { GnuBuiltin::ObjectSize, 1, nullptr, 0 } },
	{ "__builtin_elementwise_abs",
		{ GnuBuiltin::VectorElementwise, static_cast<uint8_t>(VectorLanewiseOp::Abs), nullptr, 0 } },
	{ "__builtin_elementwise_add_sat",
		{ GnuBuiltin::VectorElementwise, static_cast<uint8_t>(VectorLanewiseOp::AddSaturate), nullptr, 0 } },
	{ "__builtin_elementwise_max", { GnuBuiltin::ElementwiseMinMax, 1, nullptr, 0 } },
	{ "__builtin_elementwise_min", { GnuBuiltin::ElementwiseMinMax, 0, nullptr, 0 } },
	{ "__builtin_elementwise_sqrt",
		{ GnuBuiltin::VectorElementwise, static_cast<uint8_t>(VectorLanewiseOp::Sqrt), nullptr, 0 } },
	{ "__builtin_elementwise_sub_sat",
		{ GnuBuiltin::VectorElementwise, static_cast<uint8_t>(VectorLanewiseOp::SubSaturate), nullptr, 0 } },
	{ "__builtin_expect", { GnuBuiltin::Expect, 0, "lll", 0 } },
	{ "__builtin_ffs", { GnuBuiltin::BitCount, 12, nullptr, 0 } },
	{ "__builtin_ffsl", { GnuBuiltin::BitCount, 13, nullptr, 0 } },
	{ "__builtin_ffsll", { GnuBuiltin::BitCount, 14, nullptr, 0 } },
	{ "__builtin_flt_rounds", { GnuBuiltin::FltRounds, 0, nullptr, 0 } },
	{ "__builtin_fpclassify", { GnuBuiltin::FloatClassify, 4, nullptr, 0 } },
	{ "__builtin_free", { GnuBuiltin::Library, 0, "vv*", 0 } },
	{ "__builtin_huge_val", { GnuBuiltin::FloatConstant, 6, nullptr, 0 } },
	{ "__builtin_huge_valf", { GnuBuiltin::FloatConstant, 7, nullptr, 0 } },
	{ "__builtin_huge_vall", { GnuBuiltin::FloatConstant, 8, nullptr, 0 } },
	{ "__builtin_ia32_addsd", { GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::Add), "DDD", 's' } },
	{ "__builtin_ia32_addss", { GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::Add), "FFF", 's' } },
	{ "__builtin_ia32_addsubpd",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::AddSub), "DDD", 0 } },
	{ "__builtin_ia32_addsubps",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::AddSub), "FFF", 0 } },
	{ "__builtin_ia32_andnpd",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::AndNot), "DDD", 0 } },
	{ "__builtin_ia32_andnps",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::AndNot), "FFF", 0 } },
	{ "__builtin_ia32_andpd", { GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::And), "DDD", 0 } },
	{ "__builtin_ia32_andps", { GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::And), "FFF", 0 } },
	{ "__builtin_ia32_blendpd",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::Blend), "DDDi", 0 } },
	{ "__builtin_ia32_blendps",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::Blend), "FFFi", 0 } },
	{ "__builtin_ia32_blendvpd",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::BlendVariable), "DDDD", 0 } },
	{ "__builtin_ia32_blendvps",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::BlendVariable), "FFFF", 0 } },
	{ "__builtin_ia32_clflush",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::Hint), "vv*", 0 } },
	{ "__builtin_ia32_cmpeqpd",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::CompareEq), "DDD", 0 } },
	{ "__builtin_ia32_cmpeqps",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::CompareEq), "FFF", 0 } },
	{ "__builtin_ia32_cmpeqsd",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::CompareEq), "DDD", 's' } },
	{ "__builtin_ia32_cmpeqss",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::CompareEq), "FFF", 's' } },
	{ "__builtin_ia32_cmpgepd",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::CompareGe), "DDD", 0 } },
	{ "__builtin_ia32_cmpgeps",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::CompareGe), "FFF", 0 } },
	{ "__builtin_ia32_cmpgtpd",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::CompareGt), "DDD", 0 } },
	{ "__builtin_ia32_cmpgtps",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::CompareGt), "FFF", 0 } },
	{ "__builtin_ia32_cmplepd",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::CompareLe), "DDD", 0 } },
	{ "__builtin_ia32_cmpleps",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::CompareLe), "FFF", 0 } },
	{ "__builtin_ia32_cmplesd",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::CompareLe), "DDD", 's' } },
	{ "__builtin_ia32_cmpless",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::CompareLe), "FFF", 's' } },
	{ "__builtin_ia32_cmpltpd",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::CompareLt), "DDD", 0 } },
	{ "__builtin_ia32_cmpltps",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::CompareLt), "FFF", 0 } },
	{ "__builtin_ia32_cmpltsd",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::CompareLt), "DDD", 's' } },
	{ "__builtin_ia32_cmpltss",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::CompareLt), "FFF", 's' } },
	{ "__builtin_ia32_cmpneqpd",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::CompareNe), "DDD", 0 } },
	{ "__builtin_ia32_cmpneqps",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::CompareNe), "FFF", 0 } },
	{ "__builtin_ia32_cmpneqsd",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::CompareNe), "DDD", 's' } },
	{ "__builtin_ia32_cmpneqss",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::CompareNe), "FFF", 's' } },
	{ "__builtin_ia32_cmpngepd",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::CompareNotGe), "DDD", 0 } },
	{ "__builtin_ia32_cmpngeps",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::CompareNotGe), "FFF", 0 } },
	{ "__builtin_ia32_cmpngtpd",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::CompareNotGt), "DDD", 0 } },
	{ "__builtin_ia32_cmpngtps",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::CompareNotGt), "FFF", 0 } },
	{ "__builtin_ia32_cmpnlepd",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::CompareNotLe), "DDD", 0 } },
	{ "__builtin_ia32_cmpnleps",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::CompareNotLe), "FFF", 0 } },
	{ "__builtin_ia32_cmpnlesd",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::CompareNotLe), "DDD", 's' } },
	{ "__builtin_ia32_cmpnless",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::CompareNotLe), "FFF", 's' } },
	{ "__builtin_ia32_cmpnltpd",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::CompareNotLt), "DDD", 0 } },
	{ "__builtin_ia32_cmpnltps",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::CompareNotLt), "FFF", 0 } },
	{ "__builtin_ia32_cmpnltsd",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::CompareNotLt), "DDD", 's' } },
	{ "__builtin_ia32_cmpnltss",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::CompareNotLt), "FFF", 's' } },
	{ "__builtin_ia32_cmpordpd",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::CompareOrdered), "DDD", 0 } },
	{ "__builtin_ia32_cmpordps",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::CompareOrdered), "FFF", 0 } },
	{ "__builtin_ia32_cmpordsd",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::CompareOrdered), "DDD", 's' } },
	{ "__builtin_ia32_cmpordss",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::CompareOrdered), "FFF", 's' } },
	{ "__builtin_ia32_cmpunordpd",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::CompareUnordered), "DDD", 0 } },
	{ "__builtin_ia32_cmpunordps",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::CompareUnordered), "FFF", 0 } },
	{ "__builtin_ia32_cmpunordsd",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::CompareUnordered), "DDD", 's' } },
	{ "__builtin_ia32_cmpunordss",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::CompareUnordered), "FFF", 's' } },
	{ "__builtin_ia32_comieq",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::OrderedEq), "iFF", 0 } },
	{ "__builtin_ia32_comige",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::OrderedGe), "iFF", 0 } },
	{ "__builtin_ia32_comigt",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::OrderedGt), "iFF", 0 } },
	{ "__builtin_ia32_comile",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::OrderedLe), "iFF", 0 } },
	{ "__builtin_ia32_comilt",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::OrderedLt), "iFF", 0 } },
	{ "__builtin_ia32_comineq",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::OrderedNe), "iFF", 0 } },
	{ "__builtin_ia32_comisdeq",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::OrderedEq), "iDD", 0 } },
	{ "__builtin_ia32_comisdge",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::OrderedGe), "iDD", 0 } },
	{ "__builtin_ia32_comisdgt",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::OrderedGt), "iDD", 0 } },
	{ "__builtin_ia32_comisdle",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::OrderedLe), "iDD", 0 } },
	{ "__builtin_ia32_comisdlt",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::OrderedLt), "iDD", 0 } },
	{ "__builtin_ia32_comisdneq",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::OrderedNe), "iDD", 0 } },
	{ "__builtin_ia32_cvtdq2pd",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::Convert), "DW", 0 } },
	{ "__builtin_ia32_cvtdq2ps",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::Convert), "FW", 0 } },
	{ "__builtin_ia32_cvtpd2dq",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::ConvertNearest), "WD", 0 } },
	{ "__builtin_ia32_cvtpd2ps",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::Convert), "FD", 0 } },
	{ "__builtin_ia32_cvtps2dq",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::ConvertNearest), "WF", 0 } },
	{ "__builtin_ia32_cvtps2pd",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::Convert), "DF", 0 } },
	{ "__builtin_ia32_cvtsd2si",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::ConvertNearest), "iD", 0 } },
	{ "__builtin_ia32_cvtsd2si64",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::ConvertNearest), "xD", 0 } },
	{ "__builtin_ia32_cvtsd2ss",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::ConvertLow), "FFD", 0 } },
	{ "__builtin_ia32_cvtsi2sd",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::ConvertLow), "DDi", 0 } },
	{ "__builtin_ia32_cvtsi2ss",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::ConvertLow), "FFi", 0 } },
	{ "__builtin_ia32_cvtsi642sd",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::ConvertLow), "DDx", 0 } },
	{ "__builtin_ia32_cvtsi642ss",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::ConvertLow), "FFx", 0 } },
	{ "__builtin_ia32_cvtss2sd",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::ConvertLow), "DDF", 0 } },
	{ "__builtin_ia32_cvtss2si",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::ConvertNearest), "iF", 0 } },
	{ "__builtin_ia32_cvtss2si64",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::ConvertNearest), "xF", 0 } },
	{ "__builtin_ia32_cvttpd2dq",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::Convert), "WD", 0 } },
	{ "__builtin_ia32_cvttps2dq", { GnuBuiltin::VectorTruncate, 0, nullptr, 0 } },
	{ "__builtin_ia32_cvttsd2si",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::Convert), "iD", 0 } },
	{ "__builtin_ia32_cvttsd2si64",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::Convert), "xD", 0 } },
	{ "__builtin_ia32_cvttss2si",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::Convert), "iF", 0 } },
	{ "__builtin_ia32_cvttss2si64",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::Convert), "xF", 0 } },
	{ "__builtin_ia32_divsd", { GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::Div), "DDD", 's' } },
	{ "__builtin_ia32_divss", { GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::Div), "FFF", 's' } },
	{ "__builtin_ia32_haddpd",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::HorizontalAdd), "DDD", 0 } },
	{ "__builtin_ia32_haddps",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::HorizontalAdd), "FFF", 0 } },
	{ "__builtin_ia32_hsubpd",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::HorizontalSub), "DDD", 0 } },
	{ "__builtin_ia32_hsubps",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::HorizontalSub), "FFF", 0 } },
	{ "__builtin_ia32_lddqu", { GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::Load), "Bc*", 0 } },
	{ "__builtin_ia32_lfence", { GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::Fence), "v", 0 } },
	{ "__builtin_ia32_loadhps",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::LoadHigh), "FFG*", 0 } },
	{ "__builtin_ia32_loadlps",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::LoadLow), "FFG*", 0 } },
	{ "__builtin_ia32_maxpd", { GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::Max), "DDD", 0 } },
	{ "__builtin_ia32_maxps", { GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::Max), "FFF", 0 } },
	{ "__builtin_ia32_maxsd", { GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::Max), "DDD", 's' } },
	{ "__builtin_ia32_maxss", { GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::Max), "FFF", 's' } },
	{ "__builtin_ia32_mfence", { GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::Fence), "v", 0 } },
	{ "__builtin_ia32_minpd", { GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::Min), "DDD", 0 } },
	{ "__builtin_ia32_minps", { GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::Min), "FFF", 0 } },
	{ "__builtin_ia32_minsd", { GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::Min), "DDD", 's' } },
	{ "__builtin_ia32_minss", { GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::Min), "FFF", 's' } },
	{ "__builtin_ia32_movhlps",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::MoveHighToLow), "FFF", 0 } },
	{ "__builtin_ia32_movlhps",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::MoveLowToHigh), "FFF", 0 } },
	{ "__builtin_ia32_movmskpd", { GnuBuiltin::VectorSignMask, 2, nullptr, 0 } },
	{ "__builtin_ia32_movmskps", { GnuBuiltin::VectorSignMask, 1, nullptr, 0 } },
	{ "__builtin_ia32_movntdq",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::Store), "vQ*Q", 0 } },
	{ "__builtin_ia32_movnti",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::Store), "vi*i", 0 } },
	{ "__builtin_ia32_movnti64",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::Store), "vx*x", 0 } },
	{ "__builtin_ia32_movntpd",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::Store), "vd*D", 0 } },
	{ "__builtin_ia32_movntps",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::Store), "vf*F", 0 } },
	{ "__builtin_ia32_movq128",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::MoveLow), "QQ", 0 } },
	{ "__builtin_ia32_movsd",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::MoveScalar), "DDD", 0 } },
	{ "__builtin_ia32_movshdup",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::DuplicateOdd), "FF", 0 } },
	{ "__builtin_ia32_movsldup",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::DuplicateEven), "FF", 0 } },
	{ "__builtin_ia32_movss",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::MoveScalar), "FFF", 0 } },
	{ "__builtin_ia32_mulsd", { GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::Mul), "DDD", 's' } },
	{ "__builtin_ia32_mulss", { GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::Mul), "FFF", 's' } },
	{ "__builtin_ia32_orpd", { GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::Or), "DDD", 0 } },
	{ "__builtin_ia32_orps", { GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::Or), "FFF", 0 } },
	{ "__builtin_ia32_pabsb128", { GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::Abs), "BB", 0 } },
	{ "__builtin_ia32_pabsd128", { GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::Abs), "WW", 0 } },
	{ "__builtin_ia32_pabsw128", { GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::Abs), "HH", 0 } },
	{ "__builtin_ia32_packssdw128", { GnuBuiltin::VectorPack, 2, nullptr, 0 } },
	{ "__builtin_ia32_packsswb128", { GnuBuiltin::VectorPack, 1, nullptr, 0 } },
	{ "__builtin_ia32_packuswb128", { GnuBuiltin::VectorPack, 0, nullptr, 0 } },
	{ "__builtin_ia32_paddsb128",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::AddSaturate), "BBB", 0 } },
	{ "__builtin_ia32_paddsw128",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::AddSaturate), "HHH", 0 } },
	{ "__builtin_ia32_paddusb128",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::AddSaturateUnsigned), "BBB", 0 } },
	{ "__builtin_ia32_paddusw128",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::AddSaturateUnsigned), "HHH", 0 } },
	{ "__builtin_ia32_palignr128",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::ByteAlign), "RRRi", 0 } },
	{ "__builtin_ia32_pandn128", { GnuBuiltin::VectorAndNot, 0, nullptr, 0 } },
	{ "__builtin_ia32_pause", { GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::Hint), "v", 0 } },
	{ "__builtin_ia32_pavgb128",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::Average), "BBB", 0 } },
	{ "__builtin_ia32_pavgw128",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::Average), "HHH", 0 } },
	{ "__builtin_ia32_pblendvb128",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::BlendVariable), "BBBB", 0 } },
	{ "__builtin_ia32_pblendw128",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::Blend), "HHHi", 0 } },
	{ "__builtin_ia32_phaddd128",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::HorizontalAdd), "WWW", 0 } },
	{ "__builtin_ia32_phaddsw128",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::HorizontalAddSaturate), "HHH", 0 } },
	{ "__builtin_ia32_phaddw128",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::HorizontalAdd), "HHH", 0 } },
	{ "__builtin_ia32_phsubd128",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::HorizontalSub), "WWW", 0 } },
	{ "__builtin_ia32_phsubsw128",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::HorizontalSubSaturate), "HHH", 0 } },
	{ "__builtin_ia32_phsubw128",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::HorizontalSub), "HHH", 0 } },
	{ "__builtin_ia32_pmaddubsw128",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::MulAddBytes), "HBB", 0 } },
	{ "__builtin_ia32_pmaddwd128",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::MulAdd), "WHH", 0 } },
	{ "__builtin_ia32_pmaxsb128",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::MaxSigned), "BBB", 0 } },
	{ "__builtin_ia32_pmaxsd128",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::MaxSigned), "WWW", 0 } },
	{ "__builtin_ia32_pmaxsw128",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::MaxSigned), "HHH", 0 } },
	{ "__builtin_ia32_pmaxub128", { GnuBuiltin::VectorByteMinMax, 1, nullptr, 0 } },
	{ "__builtin_ia32_pmaxud128",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::MaxUnsigned), "WWW", 0 } },
	{ "__builtin_ia32_pmaxuw128",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::MaxUnsigned), "HHH", 0 } },
	{ "__builtin_ia32_pminsb128",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::MinSigned), "BBB", 0 } },
	{ "__builtin_ia32_pminsd128",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::MinSigned), "WWW", 0 } },
	{ "__builtin_ia32_pminsw128",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::MinSigned), "HHH", 0 } },
	{ "__builtin_ia32_pminub128", { GnuBuiltin::VectorByteMinMax, 0, nullptr, 0 } },
	{ "__builtin_ia32_pminud128",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::MinUnsigned), "WWW", 0 } },
	{ "__builtin_ia32_pminuw128",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::MinUnsigned), "HHH", 0 } },
	{ "__builtin_ia32_pmovmskb128", { GnuBuiltin::VectorSignMask, 0, nullptr, 0 } },
	{ "__builtin_ia32_pmovsxbd128",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::Extend), "WB", 0 } },
	{ "__builtin_ia32_pmovsxbq128",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::Extend), "QB", 0 } },
	{ "__builtin_ia32_pmovsxbw128",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::Extend), "HB", 0 } },
	{ "__builtin_ia32_pmovsxdq128",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::Extend), "QW", 0 } },
	{ "__builtin_ia32_pmovsxwd128",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::Extend), "WH", 0 } },
	{ "__builtin_ia32_pmovsxwq128",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::Extend), "QH", 0 } },
	{ "__builtin_ia32_pmovzxbd128",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::ExtendUnsigned), "WB", 0 } },
	{ "__builtin_ia32_pmovzxbq128",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::ExtendUnsigned), "QB", 0 } },
	{ "__builtin_ia32_pmovzxbw128",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::ExtendUnsigned), "HB", 0 } },
	{ "__builtin_ia32_pmovzxdq128",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::ExtendUnsigned), "QW", 0 } },
	{ "__builtin_ia32_pmovzxwd128",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::ExtendUnsigned), "WH", 0 } },
	{ "__builtin_ia32_pmovzxwq128",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::ExtendUnsigned), "QH", 0 } },
	{ "__builtin_ia32_pmuldq128",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::MulWide), "QWW", 0 } },
	{ "__builtin_ia32_pmulhrsw128",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::MulHighRound), "HHH", 0 } },
	{ "__builtin_ia32_pmulhuw128",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::MulHighUnsigned), "HHH", 0 } },
	{ "__builtin_ia32_pmulhw128",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::MulHigh), "HHH", 0 } },
	{ "__builtin_ia32_pmuludq128",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::MulWideUnsigned), "QWW", 0 } },
	{ "__builtin_ia32_psadbw128",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::SumAbsoluteDifference), "QBB", 0 } },
	{ "__builtin_ia32_pshufb128",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::ByteShuffle), "BBB", 0 } },
	{ "__builtin_ia32_pshufd", { GnuBuiltin::VectorShuffle, 0, nullptr, 0 } },
	{ "__builtin_ia32_pshufhw", { GnuBuiltin::VectorShuffle, 3, nullptr, 0 } },
	{ "__builtin_ia32_pshuflw", { GnuBuiltin::VectorShuffle, 2, nullptr, 0 } },
	{ "__builtin_ia32_psignb128",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::Sign), "BBB", 0 } },
	{ "__builtin_ia32_psignd128",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::Sign), "WWW", 0 } },
	{ "__builtin_ia32_psignw128",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::Sign), "HHH", 0 } },
	{ "__builtin_ia32_pslld128",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::ShiftLeft), "WWW", 0 } },
	{ "__builtin_ia32_pslldi128", { GnuBuiltin::VectorShiftLanes, 3, nullptr, 0 } },
	{ "__builtin_ia32_pslldqi128", { GnuBuiltin::VectorShiftBytes, 0, nullptr, 0 } },
	{ "__builtin_ia32_pslldqi128_byteshift", { GnuBuiltin::VectorShiftBytes, 2, nullptr, 0 } },
	{ "__builtin_ia32_psllq128",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::ShiftLeft), "QQQ", 0 } },
	{ "__builtin_ia32_psllqi128", { GnuBuiltin::VectorShiftLanes, 6, nullptr, 0 } },
	{ "__builtin_ia32_psllw128",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::ShiftLeft), "HHH", 0 } },
	{ "__builtin_ia32_psllwi128", { GnuBuiltin::VectorShiftLanes, 0, nullptr, 0 } },
	{ "__builtin_ia32_psrad128",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::ShiftRightArithmetic), "WWW", 0 } },
	{ "__builtin_ia32_psradi128", { GnuBuiltin::VectorShiftLanes, 5, nullptr, 0 } },
	{ "__builtin_ia32_psraw128",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::ShiftRightArithmetic), "HHH", 0 } },
	{ "__builtin_ia32_psrawi128", { GnuBuiltin::VectorShiftLanes, 2, nullptr, 0 } },
	{ "__builtin_ia32_psrld128",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::ShiftRightLogical), "WWW", 0 } },
	{ "__builtin_ia32_psrldi128", { GnuBuiltin::VectorShiftLanes, 4, nullptr, 0 } },
	{ "__builtin_ia32_psrldqi128", { GnuBuiltin::VectorShiftBytes, 1, nullptr, 0 } },
	{ "__builtin_ia32_psrldqi128_byteshift", { GnuBuiltin::VectorShiftBytes, 3, nullptr, 0 } },
	{ "__builtin_ia32_psrlq128",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::ShiftRightLogical), "QQQ", 0 } },
	{ "__builtin_ia32_psrlqi128", { GnuBuiltin::VectorShiftLanes, 7, nullptr, 0 } },
	{ "__builtin_ia32_psrlw128",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::ShiftRightLogical), "HHH", 0 } },
	{ "__builtin_ia32_psrlwi128", { GnuBuiltin::VectorShiftLanes, 1, nullptr, 0 } },
	{ "__builtin_ia32_psubsb128",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::SubSaturate), "BBB", 0 } },
	{ "__builtin_ia32_psubsw128",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::SubSaturate), "HHH", 0 } },
	{ "__builtin_ia32_psubusb128",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::SubSaturateUnsigned), "BBB", 0 } },
	{ "__builtin_ia32_psubusw128",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::SubSaturateUnsigned), "HHH", 0 } },
	{ "__builtin_ia32_ptestc128",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::TestCarry), "iQQ", 0 } },
	{ "__builtin_ia32_ptestnzc128",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::TestMixed), "iQQ", 0 } },
	{ "__builtin_ia32_ptestz128",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::TestZero), "iQQ", 0 } },
	{ "__builtin_ia32_punpckhbw128", { GnuBuiltin::VectorUnpack, 1, nullptr, 0 } },
	{ "__builtin_ia32_punpckhdq128", { GnuBuiltin::VectorUnpack, 5, nullptr, 0 } },
	{ "__builtin_ia32_punpckhqdq128", { GnuBuiltin::VectorUnpack, 7, nullptr, 0 } },
	{ "__builtin_ia32_punpckhwd128", { GnuBuiltin::VectorUnpack, 3, nullptr, 0 } },
	{ "__builtin_ia32_punpcklbw128", { GnuBuiltin::VectorUnpack, 0, nullptr, 0 } },
	{ "__builtin_ia32_punpckldq128", { GnuBuiltin::VectorUnpack, 4, nullptr, 0 } },
	{ "__builtin_ia32_punpcklqdq128", { GnuBuiltin::VectorUnpack, 6, nullptr, 0 } },
	{ "__builtin_ia32_punpcklwd128", { GnuBuiltin::VectorUnpack, 2, nullptr, 0 } },
	{ "__builtin_ia32_rcpps",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::Reciprocal), "FF", 0 } },
	{ "__builtin_ia32_rcpss",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::Reciprocal), "FF", 's' } },
	{ "__builtin_ia32_rsqrtps",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::ReciprocalSqrt), "FF", 0 } },
	{ "__builtin_ia32_rsqrtss",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::ReciprocalSqrt), "FF", 's' } },
	{ "__builtin_ia32_sfence", { GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::Fence), "v", 0 } },
	{ "__builtin_ia32_shufpd", { GnuBuiltin::VectorShuffle, 1, nullptr, 0 } },
	{ "__builtin_ia32_shufps", { GnuBuiltin::VectorShuffle, 4, nullptr, 0 } },
	{ "__builtin_ia32_sqrtpd", { GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::Sqrt), "DD", 0 } },
	{ "__builtin_ia32_sqrtps", { GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::Sqrt), "FF", 0 } },
	{ "__builtin_ia32_sqrtsd",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::Sqrt), "DD", 's' } },
	{ "__builtin_ia32_sqrtss",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::Sqrt), "FF", 's' } },
	{ "__builtin_ia32_storehps",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::StoreHigh), "vG*F", 0 } },
	{ "__builtin_ia32_storelps",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::StoreLow), "vG*F", 0 } },
	{ "__builtin_ia32_subsd", { GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::Sub), "DDD", 's' } },
	{ "__builtin_ia32_subss", { GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::Sub), "FFF", 's' } },
	{ "__builtin_ia32_ucomieq",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::OrderedEq), "iFF", 0 } },
	{ "__builtin_ia32_ucomige",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::OrderedGe), "iFF", 0 } },
	{ "__builtin_ia32_ucomigt",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::OrderedGt), "iFF", 0 } },
	{ "__builtin_ia32_ucomile",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::OrderedLe), "iFF", 0 } },
	{ "__builtin_ia32_ucomilt",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::OrderedLt), "iFF", 0 } },
	{ "__builtin_ia32_ucomineq",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::OrderedNe), "iFF", 0 } },
	{ "__builtin_ia32_ucomisdeq",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::OrderedEq), "iDD", 0 } },
	{ "__builtin_ia32_ucomisdge",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::OrderedGe), "iDD", 0 } },
	{ "__builtin_ia32_ucomisdgt",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::OrderedGt), "iDD", 0 } },
	{ "__builtin_ia32_ucomisdle",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::OrderedLe), "iDD", 0 } },
	{ "__builtin_ia32_ucomisdlt",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::OrderedLt), "iDD", 0 } },
	{ "__builtin_ia32_ucomisdneq",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::OrderedNe), "iDD", 0 } },
	{ "__builtin_ia32_unpckhpd",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::UnpackHigh), "DDD", 0 } },
	{ "__builtin_ia32_unpckhps",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::UnpackHigh), "FFF", 0 } },
	{ "__builtin_ia32_unpcklpd",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::UnpackLow), "DDD", 0 } },
	{ "__builtin_ia32_unpcklps",
		{ GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::UnpackLow), "FFF", 0 } },
	{ "__builtin_ia32_vec_ext_v16qi", { GnuBuiltin::VectorLanes, 0, nullptr, 0 } },
	{ "__builtin_ia32_vec_ext_v2df", { GnuBuiltin::VectorLanes, 1, nullptr, 0 } },
	{ "__builtin_ia32_vec_ext_v2di", { GnuBuiltin::VectorLanes, 2, nullptr, 0 } },
	{ "__builtin_ia32_vec_ext_v2si", { GnuBuiltin::VectorLanes, 3, nullptr, 0 } },
	{ "__builtin_ia32_vec_ext_v4hi", { GnuBuiltin::VectorLanes, 4, nullptr, 0 } },
	{ "__builtin_ia32_vec_ext_v4sf", { GnuBuiltin::VectorLanes, 5, nullptr, 0 } },
	{ "__builtin_ia32_vec_ext_v4si", { GnuBuiltin::VectorLanes, 6, nullptr, 0 } },
	{ "__builtin_ia32_vec_ext_v8hi", { GnuBuiltin::VectorLanes, 7, nullptr, 0 } },
	{ "__builtin_ia32_vec_init_v2si", { GnuBuiltin::VectorLanes, 8, nullptr, 0 } },
	{ "__builtin_ia32_vec_init_v4hi", { GnuBuiltin::VectorLanes, 9, nullptr, 0 } },
	{ "__builtin_ia32_vec_init_v8qi", { GnuBuiltin::VectorLanes, 10, nullptr, 0 } },
	{ "__builtin_ia32_vec_set_v4hi", { GnuBuiltin::VectorLanes, 11, nullptr, 0 } },
	{ "__builtin_ia32_vec_set_v8hi", { GnuBuiltin::VectorLanes, 12, nullptr, 0 } },
	{ "__builtin_ia32_xorpd", { GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::Xor), "DDD", 0 } },
	{ "__builtin_ia32_xorps", { GnuBuiltin::VectorLanewise, static_cast<uint8_t>(VectorLanewiseOp::Xor), "FFF", 0 } },
	{ "__builtin_index", { GnuBuiltin::Library, 0, "c*Cc*i", 0 } },
	{ "__builtin_inf", { GnuBuiltin::FloatConstant, 6, nullptr, 0 } },
	{ "__builtin_inff", { GnuBuiltin::FloatConstant, 7, nullptr, 0 } },
	{ "__builtin_infl", { GnuBuiltin::FloatConstant, 8, nullptr, 0 } },
	{ "__builtin_invoke", { GnuBuiltin::Invoke, 0, nullptr, 0 } },
	{ "__builtin_is_constant_evaluated", { GnuBuiltin::IsConstantEvaluated, 0, nullptr, 0 } },
	{ "__builtin_isfinite", { GnuBuiltin::FloatClassify, 0, nullptr, 0 } },
	{ "__builtin_isgreater", { GnuBuiltin::FloatCompare, 0, nullptr, 0 } },
	{ "__builtin_isgreaterequal", { GnuBuiltin::FloatCompare, 1, nullptr, 0 } },
	{ "__builtin_isinf", { GnuBuiltin::FloatClassify, 1, nullptr, 0 } },
	{ "__builtin_isinf_sign", { GnuBuiltin::FloatClassify, 6, nullptr, 0 } },
	{ "__builtin_isless", { GnuBuiltin::FloatCompare, 2, nullptr, 0 } },
	{ "__builtin_islessequal", { GnuBuiltin::FloatCompare, 3, nullptr, 0 } },
	{ "__builtin_islessgreater", { GnuBuiltin::FloatCompare, 4, nullptr, 0 } },
	{ "__builtin_isnan", { GnuBuiltin::FloatClassify, 2, nullptr, 0 } },
	{ "__builtin_isnormal", { GnuBuiltin::FloatClassify, 3, nullptr, 0 } },
	{ "__builtin_issignaling", { GnuBuiltin::FloatClassify, 7, nullptr, 0 } },
	{ "__builtin_isunordered", { GnuBuiltin::FloatCompare, 5, nullptr, 0 } },
	{ "__builtin_labs", { GnuBuiltin::Library, 0, "ll", 0 } },
	{ "__builtin_launder", { GnuBuiltin::Launder, 0, nullptr, 0 } },
	{ "__builtin_llabs", { GnuBuiltin::Library, 0, "xx", 0 } },
	{ "__builtin_malloc", { GnuBuiltin::Library, 0, "v*z", 0 } },
	{ "__builtin_memchr", { GnuBuiltin::Library, 0, "v*Cv*iz", 0 } },
	{ "__builtin_memcmp", { GnuBuiltin::Library, 0, "iCv*Cv*z", 0 } },
	{ "__builtin_memcpy", { GnuBuiltin::Memcpy, 0, "v*v*Cv*z", 0 } },
	{ "__builtin_memmove", { GnuBuiltin::Memmove, 0, "v*v*Cv*z", 0 } },
	{ "__builtin_memset", { GnuBuiltin::Library, 0, "v*v*iz", 0 } },
	{ "__builtin_mul_overflow", { GnuBuiltin::Overflow, 2, nullptr, 0 } },
	{ "__builtin_nan", { GnuBuiltin::FloatConstant, 0, nullptr, 0 } },
	{ "__builtin_nanf", { GnuBuiltin::FloatConstant, 1, nullptr, 0 } },
	{ "__builtin_nanl", { GnuBuiltin::FloatConstant, 2, nullptr, 0 } },
	{ "__builtin_nans", { GnuBuiltin::FloatConstant, 3, nullptr, 0 } },
	{ "__builtin_nansf", { GnuBuiltin::FloatConstant, 4, nullptr, 0 } },
	{ "__builtin_nansl", { GnuBuiltin::FloatConstant, 5, nullptr, 0 } },
	{ "__builtin_nontemporal_load",
		{ GnuBuiltin::VectorElementwise, static_cast<uint8_t>(VectorLanewiseOp::Load), nullptr, 0 } },
	{ "__builtin_nontemporal_store",
		{ GnuBuiltin::VectorElementwise, static_cast<uint8_t>(VectorLanewiseOp::StoreValue), nullptr, 0 } },
	{ "__builtin_object_size", { GnuBuiltin::ObjectSize, 0, nullptr, 0 } },
	{ "__builtin_operator_delete", { GnuBuiltin::OperatorDelete, 0, nullptr, 0 } },
	{ "__builtin_operator_new", { GnuBuiltin::OperatorNew, 0, nullptr, 0 } },
	{ "__builtin_popcount", { GnuBuiltin::BitCount, 9, nullptr, 0 } },
	{ "__builtin_popcountg", { GnuBuiltin::GenericBitCount, 2, nullptr, 0 } },
	{ "__builtin_popcountl", { GnuBuiltin::BitCount, 10, nullptr, 0 } },
	{ "__builtin_popcountll", { GnuBuiltin::BitCount, 11, nullptr, 0 } },
	{ "__builtin_prefetch", { GnuBuiltin::Prefetch, 0, nullptr, 0 } },
	{ "__builtin_reduce_add", { GnuBuiltin::Reduce, 3, nullptr, 0 } },
	{ "__builtin_reduce_and", { GnuBuiltin::Reduce, 0, nullptr, 0 } },
	{ "__builtin_reduce_max", { GnuBuiltin::Reduce, 5, nullptr, 0 } },
	{ "__builtin_reduce_min", { GnuBuiltin::Reduce, 6, nullptr, 0 } },
	{ "__builtin_reduce_mul", { GnuBuiltin::Reduce, 4, nullptr, 0 } },
	{ "__builtin_reduce_or", { GnuBuiltin::Reduce, 1, nullptr, 0 } },
	{ "__builtin_reduce_xor", { GnuBuiltin::Reduce, 2, nullptr, 0 } },
	{ "__builtin_rindex", { GnuBuiltin::Library, 0, "c*Cc*i", 0 } },
	{ "__builtin_shuffle", { GnuBuiltin::VectorPermute, 0, nullptr, 0 } },
	{ "__builtin_shufflevector", { GnuBuiltin::VectorPermute, 1, nullptr, 0 } },
	{ "__builtin_signbit", { GnuBuiltin::FloatClassify, 5, nullptr, 0 } },
	{ "__builtin_strchr", { GnuBuiltin::Library, 0, "c*Cc*i", 0 } },
	{ "__builtin_strcmp", { GnuBuiltin::Library, 0, "iCc*Cc*", 0 } },
	{ "__builtin_strcpy", { GnuBuiltin::Library, 0, "c*c*Cc*", 0 } },
	{ "__builtin_strlen", { GnuBuiltin::Strlen, 0, "zCc*", 0 } },
	{ "__builtin_strncmp", { GnuBuiltin::Library, 0, "iCc*Cc*z", 0 } },
	{ "__builtin_strncpy", { GnuBuiltin::Library, 0, "c*c*Cc*z", 0 } },
	{ "__builtin_strpbrk", { GnuBuiltin::Library, 0, "c*Cc*Cc*", 0 } },
	{ "__builtin_strrchr", { GnuBuiltin::Library, 0, "c*Cc*i", 0 } },
	{ "__builtin_strstr", { GnuBuiltin::Library, 0, "c*Cc*Cc*", 0 } },
	{ "__builtin_sub_overflow", { GnuBuiltin::Overflow, 1, nullptr, 0 } },
	{ "__builtin_trap", { GnuBuiltin::Trap, 0, "v", 0 } },
	{ "__builtin_unreachable", { GnuBuiltin::Unreachable, 0, "v", 0 } },
	{ "__builtin_va_copy", { GnuBuiltin::VaList, 2, nullptr, 0 } },
	{ "__builtin_va_end", { GnuBuiltin::VaList, 1, nullptr, 0 } },
	{ "__builtin_va_start", { GnuBuiltin::VaList, 0, nullptr, 0 } },
	{ "__builtin_vsnprintf", { GnuBuiltin::Library, 0, "ic*zCc*V", 0 } },
};

// The libm functions whose builtin forms hosted <cmath> and <complex>
// forward to, by their double spellings (`f` and `l` add the float and long
// double ones), sorted, with their signatures over the floating type T (Z
// its complex type: C99 7.3's functions).
const struct
{
	const char* name;
	const char* signature;
} kLibmFunctions[] = {
	{ "acos", "TT" },
	{ "acosh", "TT" },
	{ "asin", "TT" },
	{ "asinh", "TT" },
	{ "atan", "TT" },
	{ "atan2", "TTT" },
	{ "atanh", "TT" },
	{ "cabs", "TZ" },
	{ "cacos", "ZZ" },
	{ "cacosh", "ZZ" },
	{ "carg", "TZ" },
	{ "casin", "ZZ" },
	{ "casinh", "ZZ" },
	{ "catan", "ZZ" },
	{ "catanh", "ZZ" },
	{ "cbrt", "TT" },
	{ "ccos", "ZZ" },
	{ "ccosh", "ZZ" },
	{ "ceil", "TT" },
	{ "cexp", "ZZ" },
	{ "cimag", "TZ" },
	{ "clog", "ZZ" },
	{ "conj", "ZZ" },
	{ "copysign", "TTT" },
	{ "cos", "TT" },
	{ "cosh", "TT" },
	{ "cpow", "ZZZ" },
	{ "cproj", "ZZ" },
	{ "creal", "TZ" },
	{ "csin", "ZZ" },
	{ "csinh", "ZZ" },
	{ "csqrt", "ZZ" },
	{ "ctan", "ZZ" },
	{ "ctanh", "ZZ" },
	{ "erf", "TT" },
	{ "erfc", "TT" },
	{ "exp", "TT" },
	{ "exp2", "TT" },
	{ "expm1", "TT" },
	{ "fabs", "TT" },
	{ "fdim", "TTT" },
	{ "floor", "TT" },
	{ "fma", "TTTT" },
	{ "fmax", "TTT" },
	{ "fmin", "TTT" },
	{ "fmod", "TTT" },
	{ "frexp", "TTi*" },
	{ "hypot", "TTT" },
	{ "ilogb", "iT" },
	{ "ldexp", "TTi" },
	{ "lgamma", "TT" },
	{ "llrint", "xT" },
	{ "llround", "xT" },
	{ "log", "TT" },
	{ "log10", "TT" },
	{ "log1p", "TT" },
	{ "log2", "TT" },
	{ "logb", "TT" },
	{ "lrint", "lT" },
	{ "lround", "lT" },
	{ "modf", "TTT*" },
	{ "nearbyint", "TT" },
	{ "nextafter", "TTT" },
	{ "nexttoward", "TTe" },
	{ "pow", "TTT" },
	{ "remainder", "TTT" },
	{ "remquo", "TTTi*" },
	{ "rint", "TT" },
	{ "round", "TT" },
	{ "scalbln", "TTl" },
	{ "scalbn", "TTi" },
	{ "sin", "TT" },
	{ "sinh", "TT" },
	{ "sqrt", "TT" },
	{ "tan", "TT" },
	{ "tanh", "TT" },
	{ "tgamma", "TT" },
	{ "trunc", "TT" },
};

const char* const kTraitNames[] = { "__is_class", "__is_enum", "__is_union", "__is_void", "__is_nullptr",
	"__is_integral", "__is_floating_point", "__is_array", "__is_bounded_array", "__is_unbounded_array", "__is_pointer",
	"__is_lvalue_reference", "__is_rvalue_reference", "__is_reference", "__is_member_object_pointer",
	"__is_member_function_pointer", "__is_member_pointer", "__is_function", "__is_arithmetic", "__is_fundamental",
	"__is_object", "__is_scalar", "__is_compound", "__is_const", "__is_volatile", "__is_signed", "__is_unsigned",
	"__is_referenceable", "__is_scoped_enum", "__is_empty", "__is_polymorphic", "__is_abstract", "__is_final",
	"__is_aggregate", "__is_trivial", "__is_trivially_copyable", "__has_unique_object_representations",
	"__is_standard_layout", "__is_pod", "__is_literal_type", "__has_trivial_destructor", "__has_virtual_destructor",
	"__is_destructible", "__is_nothrow_destructible", "__is_trivially_destructible", "__has_trivial_constructor",
	"__has_trivial_copy", "__has_trivial_assign", "__has_nothrow_constructor", "__has_nothrow_copy",
	"__has_nothrow_assign", "__array_rank", "__builtin_offsetof", "__is_same", "__is_base_of", "__is_assignable",
	"__is_nothrow_assignable", "__is_trivially_assignable", "__is_convertible", "__is_convertible_to",
	"__is_nothrow_convertible", "__reference_binds_to_temporary", "__reference_constructs_from_temporary",
	"__reference_converts_from_temporary", "__is_constructible", "__is_nothrow_constructible",
	"__is_trivially_constructible", "__is_invocable", "__is_nothrow_invocable", "__underlying_type",
	"__add_lvalue_reference", "__add_pointer", "__add_rvalue_reference", "__decay", "__make_signed", "__make_unsigned",
	"__remove_all_extents", "__remove_const", "__remove_cv", "__remove_cvref", "__remove_extent", "__remove_pointer",
	"__remove_reference_t", "__remove_reference", "__remove_restrict", "__remove_volatile", "_Atomic", "_BitInt",
	"__integer_pack", "__builtin_va_arg", "__builtin_convertvector", "__builtin_bit_cast" };

const unsigned kSpelledTraits = sizeof kTraitNames / sizeof kTraitNames[0];
static_assert(kSpelledTraits == static_cast<unsigned>(GnuTrait::Declval), "one name per spelled trait");

// strcmp order between a table entry and a spelling of `length` bytes.
int Compare(const char* entry, const char* spelling, uint32_t length)
{
	const int order = std::strncmp(entry, spelling, length);
	if (order != 0)
		return order;
	return entry[length] == '\0' ? 0 : 1;
}

} // namespace

const GnuKeyword kGnuKeywords[] = { { "__inline", TokenKind::KW_INLINE, false },
	{ "__inline__", TokenKind::KW_INLINE, false }, { "__const", TokenKind::KW_CONST, false },
	{ "__const__", TokenKind::KW_CONST, false }, { "__volatile", TokenKind::KW_VOLATILE, false },
	{ "__volatile__", TokenKind::KW_VOLATILE, false }, { "__signed", TokenKind::KW_SIGNED, false },
	{ "__signed__", TokenKind::KW_SIGNED, false }, { "__decltype", TokenKind::KW_DECLTYPE, false },
	{ "__alignof", TokenKind::KW_ALIGNOF, false }, { "__alignof__", TokenKind::KW_ALIGNOF, false },
	{ "__asm", TokenKind::KW_ASM, false }, { "__asm__", TokenKind::KW_ASM, false },
	{ "__thread", TokenKind::KW_THREAD_LOCAL, false }, { "__restrict", TokenKind::KW_INLINE, true },
	{ "__restrict__", TokenKind::KW_INLINE, true }, { "__extension__", TokenKind::KW_INLINE, true } };
const unsigned kGnuKeywordCount = sizeof kGnuKeywords / sizeof kGnuKeywords[0];

bool IsGnuKeyword(const char* spelling, uint32_t length)
{
	for (const GnuKeyword& keyword : kGnuKeywords)
	{
		if (Compare(keyword.spelling, spelling, length) == 0)
			return true;
	}
	return false;
}

// kGnuBuiltins' VectorLanes variants index these.
const GnuVectorLaneOperation kGnuVectorLaneOperations[] = {
	{ 'e', 16, 'c' },
	{ 'e', 2, 'd' },
	{ 'e', 2, 'x' },
	{ 'e', 2, 'i' },
	{ 'e', 4, 's' },
	{ 'e', 4, 'f' },
	{ 'e', 4, 'i' },
	{ 'e', 8, 's' },
	{ 'i', 2, 'i' },
	{ 'i', 4, 's' },
	{ 'i', 8, 'a' },
	{ 's', 4, 's' },
	{ 's', 8, 's' },
};

const GnuAtomicOperation kGnuAtomicOperations[] = { { "load_n", "po", 'T' }, { "load", "pro", '-' },
	{ "store_n", "pvo", '-' }, { "store", "pro", '-' }, { "exchange_n", "pvo", 'T' }, { "exchange", "prro", '-' },
	{ "compare_exchange_n", "prvboo", 'B' }, { "compare_exchange", "prrboo", 'B' }, { "add_fetch", "pvo", 'T' },
	{ "sub_fetch", "pvo", 'T' }, { "and_fetch", "pvo", 'T' }, { "or_fetch", "pvo", 'T' }, { "xor_fetch", "pvo", 'T' },
	{ "nand_fetch", "pvo", 'T' }, { "fetch_add", "pvo", 'T' }, { "fetch_sub", "pvo", 'T' }, { "fetch_and", "pvo", 'T' },
	{ "fetch_or", "pvo", 'T' }, { "fetch_xor", "pvo", 'T' }, { "fetch_nand", "pvo", 'T' },
	{ "test_and_set", "po", 'B' }, { "clear", "po", '-' }, { "thread_fence", "o", '-' }, { "signal_fence", "o", '-' },
	{ "always_lock_free", "sr", 'C' }, { "is_lock_free", "sr", 'C' } };
const unsigned kGnuAtomicOperationCount = sizeof kGnuAtomicOperations / sizeof kGnuAtomicOperations[0];

const GnuSyncOperation kGnuSyncOperations[] = { { "fetch_and_add", 14, "pv", 'T', 5 },
	{ "fetch_and_sub", 15, "pv", 'T', 5 }, { "fetch_and_and", 16, "pv", 'T', 5 }, { "fetch_and_or", 17, "pv", 'T', 5 },
	{ "fetch_and_xor", 18, "pv", 'T', 5 }, { "fetch_and_nand", 19, "pv", 'T', 5 }, { "add_and_fetch", 8, "pv", 'T', 5 },
	{ "sub_and_fetch", 9, "pv", 'T', 5 }, { "and_and_fetch", 10, "pv", 'T', 5 }, { "or_and_fetch", 11, "pv", 'T', 5 },
	{ "xor_and_fetch", 12, "pv", 'T', 5 }, { "nand_and_fetch", 13, "pv", 'T', 5 },
	{ "bool_compare_and_swap", kGnuSyncCompareSwap, "pvv", 'B', 5 },
	{ "val_compare_and_swap", kGnuSyncCompareSwap, "pvv", 'T', 5 }, { "lock_test_and_set", 4, "pv", 'T', 2 },
	{ "lock_release", 2, "p", '-', 3 }, { "synchronize", 22, "", '-', 5 } };
const unsigned kGnuSyncOperationCount = sizeof kGnuSyncOperations / sizeof kGnuSyncOperations[0];

const GnuC11Operation kGnuC11Operations[] = { { "init", 2, "pv", '-', 1, 0xff }, { "load", 0, "po", 'T', 0, 0xff },
	{ "store", 2, "pvo", '-', 0, 0xff }, { "exchange", 4, "pvo", 'T', 0, 0xff },
	{ "compare_exchange_strong", 6, "prvoo", 'B', 0, 0 }, { "compare_exchange_weak", 6, "prvoo", 'B', 0, 1 },
	{ "fetch_add", 14, "pvo", 'T', 0, 0xff }, { "fetch_sub", 15, "pvo", 'T', 0, 0xff },
	{ "fetch_and", 16, "pvo", 'T', 0, 0xff }, { "fetch_or", 17, "pvo", 'T', 0, 0xff },
	{ "fetch_xor", 18, "pvo", 'T', 0, 0xff }, { "fetch_nand", 19, "pvo", 'T', 0, 0xff },
	{ "thread_fence", 22, "o", '-', 0, 0xff }, { "signal_fence", 23, "o", '-', 0, 0xff },
	{ "is_lock_free", 25, "s", 'C', 0, 0xff } };
const unsigned kGnuC11OperationCount = sizeof kGnuC11Operations / sizeof kGnuC11Operations[0];

bool IsProvidedBuiltin(const char* spelling, uint32_t length)
{
	// Clang's builtin templates, which the hosted builtin declarations
	// declare (driver/host_environment.cpp).
	static const char* const kBuiltinTemplates[] = { "__make_integer_seq", "__type_pack_element" };
	for (const char* name : kBuiltinTemplates)
	{
		if (Compare(name, spelling, length) == 0)
			return true;
	}
	GnuTrait trait;
	GnuBuiltinEntry entry;
	return FindGnuTrait(spelling, length, trait) || FindGnuBuiltin(spelling, length, entry);
}

bool FindGnuBuiltin(const char* spelling, uint32_t length, GnuBuiltinEntry& entry)
{
	if (length < 7 || spelling[0] != '_' || spelling[1] != '_')
		return false;
	const auto has_prefix = [&](const char* prefix, uint32_t size) {
		return length > size && std::memcmp(spelling, prefix, size) == 0;
	};
	if (has_prefix("__c11_atomic_", 13))
	{
		for (unsigned i = 0; i < kGnuC11OperationCount; ++i)
		{
			if (Compare(kGnuC11Operations[i].suffix, spelling + 13, length - 13) == 0)
			{
				entry = GnuBuiltinEntry{ GnuBuiltin::C11Atomic, static_cast<uint8_t>(i), nullptr, 0 };
				return true;
			}
		}
		return false;
	}
	if (has_prefix("__atomic_", 9) || has_prefix("__sync_", 7))
	{
		const bool atomic = spelling[2] == 'a';
		const uint32_t size = atomic ? 9 : 7;
		const unsigned count = atomic ? kGnuAtomicOperationCount : kGnuSyncOperationCount;
		for (unsigned i = 0; i < count; ++i)
		{
			if (Compare(atomic ? kGnuAtomicOperations[i].suffix : kGnuSyncOperations[i].suffix, spelling + size,
					length - size) == 0)
			{
				entry = GnuBuiltinEntry{ atomic ? GnuBuiltin::Atomic : GnuBuiltin::Sync, static_cast<uint8_t>(i),
					nullptr, 0 };
				return true;
			}
		}
		return false;
	}
	const auto* end = kGnuBuiltins + sizeof kGnuBuiltins / sizeof kGnuBuiltins[0];
	const auto* found = std::lower_bound(
		kGnuBuiltins, end, spelling, [length](const decltype(kGnuBuiltins[0])& item, const char* key) {
			return Compare(item.spelling, key, length) < 0;
		});
	if (found != end && Compare(found->spelling, spelling, length) == 0)
	{
		entry = found->entry;
		return true;
	}
	// `__builtin_` + a libm function, its double spelling or with `f` or
	// `l` (an exact name first: modf, erf, logb are functions themselves).
	if (!has_prefix("__builtin_", 10))
		return false;
	const char* name = spelling + 10;
	const uint32_t size = length - 10;
	for (uint32_t trim = 0; trim < 2 && trim < size; ++trim)
	{
		const char suffix = trim == 0 ? 'd' : name[size - 1] == 'f' ? 'f' : name[size - 1] == 'l' ? 'e' : 0;
		if (suffix == 0)
			break;
		for (const auto& function : kLibmFunctions)
		{
			if (Compare(function.name, name, size - trim) == 0)
			{
				entry = GnuBuiltinEntry{ Compare(function.name, "fabs", 4) == 0 ? GnuBuiltin::FloatAbs
																				: GnuBuiltin::Library,
					0, function.signature, suffix };
				return true;
			}
		}
	}
	return false;
}

bool FindGnuTrait(const char* spelling, uint32_t length, GnuTrait& trait)
{
	if (length < 6 || spelling[0] != '_' || spelling[1] != '_')
		return false;
	for (unsigned i = 0; i < kSpelledTraits; ++i)
	{
		if (Compare(kTraitNames[i], spelling, length) == 0)
		{
			trait = static_cast<GnuTrait>(i);
			return true;
		}
	}
	return false;
}

uint32_t GnuTraitArity(GnuTrait trait)
{
	if (trait >= GnuTrait::IsConstructible && trait <= GnuTrait::IsNothrowInvocable)
		return 0;
	if (trait >= GnuTrait::IsSame && trait <= GnuTrait::ReferenceConvertsFromTemporary)
		return 2;
	return 1;
}

const char* GnuTraitName(GnuTrait trait)
{
	if (trait == GnuTrait::RealPart || trait == GnuTrait::ImagPart)
		return trait == GnuTrait::RealPart ? "__real__" : "__imag__";
	if (trait == GnuTrait::Declval || trait == GnuTrait::DeclvalPrvalue)
		return "__declval";
	if (trait == GnuTrait::OffsetofBase)
		return "__builtin_offsetof";
	if (trait == GnuTrait::TypeofOperand)
		return "__typeof__";
	return kTraitNames[static_cast<unsigned>(trait)];
}
