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
	{ "__builtin_elementwise_max", { GnuBuiltin::ElementwiseMinMax, 1, nullptr, 0 } },
	{ "__builtin_elementwise_min", { GnuBuiltin::ElementwiseMinMax, 0, nullptr, 0 } },
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
	{ "__builtin_ia32_cvttps2dq", { GnuBuiltin::VectorTruncate, 0, nullptr, 0 } },
	{ "__builtin_ia32_movmskpd", { GnuBuiltin::VectorSignMask, 2, nullptr, 0 } },
	{ "__builtin_ia32_movmskps", { GnuBuiltin::VectorSignMask, 1, nullptr, 0 } },
	{ "__builtin_ia32_packssdw128", { GnuBuiltin::VectorPack, 2, nullptr, 0 } },
	{ "__builtin_ia32_packsswb128", { GnuBuiltin::VectorPack, 1, nullptr, 0 } },
	{ "__builtin_ia32_packuswb128", { GnuBuiltin::VectorPack, 0, nullptr, 0 } },
	{ "__builtin_ia32_pandn128", { GnuBuiltin::VectorAndNot, 0, nullptr, 0 } },
	{ "__builtin_ia32_pmaxub128", { GnuBuiltin::VectorByteMinMax, 1, nullptr, 0 } },
	{ "__builtin_ia32_pminub128", { GnuBuiltin::VectorByteMinMax, 0, nullptr, 0 } },
	{ "__builtin_ia32_pmovmskb128", { GnuBuiltin::VectorSignMask, 0, nullptr, 0 } },
	{ "__builtin_ia32_pshufd", { GnuBuiltin::VectorShuffle, 0, nullptr, 0 } },
	{ "__builtin_ia32_pshufhw", { GnuBuiltin::VectorShuffle, 3, nullptr, 0 } },
	{ "__builtin_ia32_pshuflw", { GnuBuiltin::VectorShuffle, 2, nullptr, 0 } },
	{ "__builtin_ia32_pslldi128", { GnuBuiltin::VectorShiftLanes, 3, nullptr, 0 } },
	{ "__builtin_ia32_pslldqi128", { GnuBuiltin::VectorShiftBytes, 0, nullptr, 0 } },
	{ "__builtin_ia32_pslldqi128_byteshift", { GnuBuiltin::VectorShiftBytes, 2, nullptr, 0 } },
	{ "__builtin_ia32_psllqi128", { GnuBuiltin::VectorShiftLanes, 6, nullptr, 0 } },
	{ "__builtin_ia32_psllwi128", { GnuBuiltin::VectorShiftLanes, 0, nullptr, 0 } },
	{ "__builtin_ia32_psradi128", { GnuBuiltin::VectorShiftLanes, 5, nullptr, 0 } },
	{ "__builtin_ia32_psrawi128", { GnuBuiltin::VectorShiftLanes, 2, nullptr, 0 } },
	{ "__builtin_ia32_psrldi128", { GnuBuiltin::VectorShiftLanes, 4, nullptr, 0 } },
	{ "__builtin_ia32_psrldqi128", { GnuBuiltin::VectorShiftBytes, 1, nullptr, 0 } },
	{ "__builtin_ia32_psrldqi128_byteshift", { GnuBuiltin::VectorShiftBytes, 3, nullptr, 0 } },
	{ "__builtin_ia32_psrlqi128", { GnuBuiltin::VectorShiftLanes, 7, nullptr, 0 } },
	{ "__builtin_ia32_psrlwi128", { GnuBuiltin::VectorShiftLanes, 1, nullptr, 0 } },
	{ "__builtin_ia32_punpckhbw128", { GnuBuiltin::VectorUnpack, 1, nullptr, 0 } },
	{ "__builtin_ia32_punpckhdq128", { GnuBuiltin::VectorUnpack, 5, nullptr, 0 } },
	{ "__builtin_ia32_punpckhqdq128", { GnuBuiltin::VectorUnpack, 7, nullptr, 0 } },
	{ "__builtin_ia32_punpckhwd128", { GnuBuiltin::VectorUnpack, 3, nullptr, 0 } },
	{ "__builtin_ia32_punpcklbw128", { GnuBuiltin::VectorUnpack, 0, nullptr, 0 } },
	{ "__builtin_ia32_punpckldq128", { GnuBuiltin::VectorUnpack, 4, nullptr, 0 } },
	{ "__builtin_ia32_punpcklqdq128", { GnuBuiltin::VectorUnpack, 6, nullptr, 0 } },
	{ "__builtin_ia32_punpcklwd128", { GnuBuiltin::VectorUnpack, 2, nullptr, 0 } },
	{ "__builtin_ia32_shufpd", { GnuBuiltin::VectorShuffle, 1, nullptr, 0 } },
	{ "__builtin_ia32_shufps", { GnuBuiltin::VectorShuffle, 4, nullptr, 0 } },
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
				entry = GnuBuiltinEntry{ GnuBuiltin::Library, 0, function.signature, suffix };
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
