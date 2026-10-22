#include "lex/number_literal.h"

#include <cfloat>
#include <cmath>
#include <cstdlib>
#include <cstring>

#include "lex/interchange_float.h"
#include "lex/ud_suffix.h"
#include "preprocess/lexer/char_classes.h"

// The compiler runs on its target, so the host's floating types are the
// x86-64 ones and the C library scans their representation directly.
static_assert(sizeof(float) == 4 && sizeof(double) == 8, "IEEE single and double");
static_assert(sizeof(long double) == 16 && LDBL_MANT_DIG == 64, "x87 extended long double");

namespace {

// The x87 extended format occupies the first 10 of its 16 bytes.
const unsigned kLongDoubleValueBytes = 10;

// An integer-suffix ([lex.icon] 2.14.2): an optional unsigned-suffix and an
// optional long-suffix or long-long-suffix, in either order.
struct IntegerSuffix
{
	bool is_unsigned;
	unsigned longs;
};

bool TakeUnsignedSuffix(const char* text, uint32_t length, uint32_t& pos)
{
	if (pos < length && (text[pos] == 'u' || text[pos] == 'U'))
	{
		++pos;
		return true;
	}
	return false;
}

unsigned TakeLongSuffix(const char* text, uint32_t length, uint32_t& pos)
{
	if (pos >= length || (text[pos] != 'l' && text[pos] != 'L'))
		return 0;
	// `ll` and `LL` are long-long-suffixes; `lL` and `Ll` are not.
	if (pos + 1 < length && text[pos + 1] == text[pos])
	{
		pos += 2;
		return 2;
	}
	++pos;
	return 1;
}

bool ParseIntegerSuffix(const char* text, uint32_t length, IntegerSuffix& suffix)
{
	uint32_t pos = 0;
	suffix.is_unsigned = TakeUnsignedSuffix(text, length, pos);
	suffix.longs = TakeLongSuffix(text, length, pos);
	if (!suffix.is_unsigned)
		suffix.is_unsigned = TakeUnsignedSuffix(text, length, pos);
	return pos == length;
}

struct TypeList
{
	const FundamentalType* types;
	unsigned count;
};

template<unsigned N>
TypeList MakeTypeList(const FundamentalType (&types)[N])
{
	return TypeList{ types, N };
}

// [lex.icon] 2.14.2 Table 6: the candidate types of an integer literal, by
// suffix and by whether it is decimal.
TypeList CandidateTypes(const IntegerSuffix& suffix, bool decimal)
{
	typedef FundamentalType T;
	static const T kDecimal[] = { T::Int, T::LongInt, T::LongLongInt };
	static const T kOctalOrHex[] = { T::Int, T::UnsignedInt, T::LongInt, T::UnsignedLongInt, T::LongLongInt,
		T::UnsignedLongLongInt };
	static const T kUnsigned[] = { T::UnsignedInt, T::UnsignedLongInt, T::UnsignedLongLongInt };
	static const T kDecimalLong[] = { T::LongInt, T::LongLongInt };
	static const T kOctalOrHexLong[] = { T::LongInt, T::UnsignedLongInt, T::LongLongInt, T::UnsignedLongLongInt };
	static const T kUnsignedLong[] = { T::UnsignedLongInt, T::UnsignedLongLongInt };
	static const T kDecimalLongLong[] = { T::LongLongInt };
	static const T kOctalOrHexLongLong[] = { T::LongLongInt, T::UnsignedLongLongInt };
	static const T kUnsignedLongLong[] = { T::UnsignedLongLongInt };

	switch (suffix.longs)
	{
	case 0:
		if (suffix.is_unsigned)
			return MakeTypeList(kUnsigned);
		return decimal ? MakeTypeList(kDecimal) : MakeTypeList(kOctalOrHex);
	case 1:
		if (suffix.is_unsigned)
			return MakeTypeList(kUnsignedLong);
		return decimal ? MakeTypeList(kDecimalLong) : MakeTypeList(kOctalOrHexLong);
	default:
		if (suffix.is_unsigned)
			return MakeTypeList(kUnsignedLongLong);
		return decimal ? MakeTypeList(kDecimalLongLong) : MakeTypeList(kOctalOrHexLongLong);
	}
}

uint64_t MaxIntegerValue(FundamentalType type)
{
	switch (type)
	{
	case FundamentalType::Int:
		return 0x7FFFFFFFu;
	case FundamentalType::UnsignedInt:
		return 0xFFFFFFFFu;
	case FundamentalType::LongInt:
	case FundamentalType::LongLongInt:
		return 0x7FFFFFFFFFFFFFFFull;
	default:
		return 0xFFFFFFFFFFFFFFFFull;
	}
}

// The value of the digits [begin, end) in `radix`; false if it needs more
// than 64 bits.
bool AccumulateDigits(const char* text, uint32_t begin, uint32_t end, unsigned radix, uint64_t& value)
{
	value = 0;
	for (uint32_t i = begin; i < end; ++i)
	{
		const unsigned digit = static_cast<unsigned>(HexDigitValue(text[i]));
		if (value > (UINT64_MAX - digit) / radix)
			return false;
		value = value * radix + digit;
	}
	return true;
}

// An integer literal whose digits are [digits_begin, digits_end); anything
// after them must be an integer-suffix or a ud-suffix.
NumberLiteral ClassifyInteger(const char* text, uint32_t length, uint32_t digits_begin, uint32_t digits_end,
	unsigned radix, NumberDialect dialect)
{
	NumberLiteral result = { NumberLiteralKind::Invalid, FundamentalType::Int, length, 0 };
	IntegerSuffix suffix;
	if (!ParseIntegerSuffix(text + digits_end, length - digits_end, suffix))
	{
		// Phase 7 does not range-check a user-defined-integer-literal: the
		// literal operator selected later receives its characters or its
		// value (2.14.8p3), so any range check belongs to that call.
		if (IsUserDefinedSuffix(text + digits_end, length - digits_end, dialect == NumberDialect::Gnu))
		{
			result.kind = NumberLiteralKind::UserDefinedInteger;
			result.suffix_begin = digits_end;
		}
		return result;
	}

	uint64_t value;
	if (!AccumulateDigits(text, digits_begin, digits_end, radix, value))
		return result;
	const TypeList candidates = CandidateTypes(suffix, radix == 10);
	for (unsigned i = 0; i < candidates.count; ++i)
	{
		if (value <= MaxIntegerValue(candidates.types[i]))
		{
			result.kind = NumberLiteralKind::Integer;
			result.type = candidates.types[i];
			result.integer = value;
			return result;
		}
	}
	// No extended integer types: a value outside the list is ill-formed
	// (2.14.2p3).
	return result;
}

// A binary literal (C++14's 2.14.2, a GNU extension in C++11): `0b` or `0B`
// and binary digits, typed as octal and hexadecimal literals are.
NumberLiteral ClassifyBinary(const char* text, uint32_t length, NumberDialect dialect)
{
	uint32_t end = 2;
	while (end < length && (text[end] == '0' || text[end] == '1'))
		++end;
	if (end == 2)
		return NumberLiteral{ NumberLiteralKind::Invalid, FundamentalType::Int, length, 0 };
	return ClassifyInteger(text, length, 2, end, 2, dialect);
}

NumberLiteral ClassifyFloating(const char* text, uint32_t length, uint32_t value_end, NumberDialect dialect);

NumberLiteral ClassifyHexadecimal(const char* text, uint32_t length, NumberDialect dialect)
{
	uint32_t end = 2;
	while (end < length && IsHexDigit(text[end]))
		++end;
	const uint32_t integer_digits = end - 2;
	// A hexadecimal floating literal (C++17's 2.13.4, a GNU extension in
	// C++11): hex digits with an optional fraction, then a binary exponent,
	// which is required.
	uint32_t fraction_digits = 0;
	const bool point = end < length && text[end] == '.';
	if (point)
	{
		for (++end; end < length && IsHexDigit(text[end]); ++end)
			++fraction_digits;
	}
	if (dialect != NumberDialect::Cxx11 && end < length && (text[end] == 'p' || text[end] == 'P') &&
		integer_digits + fraction_digits != 0)
	{
		uint32_t exponent = end + 1;
		if (exponent < length && (text[exponent] == '+' || text[exponent] == '-'))
			++exponent;
		const uint32_t digits = exponent;
		while (exponent < length && IsAsciiDigit(text[exponent]))
			++exponent;
		if (exponent != digits)
			return ClassifyFloating(text, length, exponent, dialect);
	}
	if (point || integer_digits == 0)
		return NumberLiteral{ NumberLiteralKind::Invalid, FundamentalType::Int, length, 0 };
	return ClassifyInteger(text, length, 2, end, 16, dialect);
}

// The end of an exponent-part starting at `pos`, or `pos` if there is none.
uint32_t ExponentEnd(const char* text, uint32_t length, uint32_t pos)
{
	if (pos >= length || (text[pos] != 'e' && text[pos] != 'E'))
		return pos;
	uint32_t end = pos + 1;
	if (end < length && (text[end] == '+' || text[end] == '-'))
		++end;
	if (end >= length || !IsAsciiDigit(text[end]))
		return pos;
	while (end < length && IsAsciiDigit(text[end]))
		++end;
	return end;
}

// The extended floating type a suffix names (`f16`/`F16` ... `f64x`).
bool ExtendedFloatingSuffix(const char* suffix, uint32_t length, FundamentalType& type)
{
	if (length < 3 || (suffix[0] != 'f' && suffix[0] != 'F'))
		return false;
	const std::string rest(suffix + 1, length - 1);
	static const struct
	{
		const char* spelling;
		FundamentalType type;
	} kSuffixes[] = { { "16", FundamentalType::Float16 }, { "32", FundamentalType::Float32 },
		{ "64", FundamentalType::Float64 }, { "128", FundamentalType::Float128 }, { "32x", FundamentalType::Float32x },
		{ "64x", FundamentalType::Float64x } };
	for (const auto& entry : kSuffixes)
	{
		if (rest == entry.spelling)
		{
			type = entry.type;
			return true;
		}
	}
	return false;
}

// A floating literal whose value text ends at `value_end`.
NumberLiteral ClassifyFloating(const char* text, uint32_t length, uint32_t value_end, NumberDialect dialect)
{
	NumberLiteral result = { NumberLiteralKind::Invalid, FundamentalType::Double, value_end, 0 };
	const uint32_t suffix_length = length - value_end;
	if (suffix_length == 0)
	{
		result.kind = NumberLiteralKind::Floating;
	}
	else if (suffix_length == 1 && (text[value_end] == 'f' || text[value_end] == 'F'))
	{
		result.kind = NumberLiteralKind::Floating;
		result.type = FundamentalType::Float;
	}
	else if (suffix_length == 1 && (text[value_end] == 'l' || text[value_end] == 'L'))
	{
		result.kind = NumberLiteralKind::Floating;
		result.type = FundamentalType::LongDouble;
	}
	else if (suffix_length == 1 && (text[value_end] == 'q' || text[value_end] == 'Q') &&
		dialect != NumberDialect::Cxx11)
	{
		// GNU's `q` and `Q` name __float128 in both compilation and the
		// typed preprocessor stream; `l` and `L` name long double.
		result.kind = NumberLiteralKind::Floating;
		result.type = FundamentalType::GnuFloat128;
	}
	else if (dialect != NumberDialect::Cxx11 && ExtendedFloatingSuffix(text + value_end, suffix_length, result.type))
	{
		// C++23's (and GCC's) f16, f32, f64, f128, f32x, f64x.
		result.kind = NumberLiteralKind::Floating;
	}
	else if (IsUserDefinedSuffix(text + value_end, suffix_length, dialect == NumberDialect::Gnu))
	{
		result.kind = NumberLiteralKind::UserDefinedFloating;
	}
	return result;
}

} // namespace

NumberLiteral ClassifyNumber(const char* text, uint32_t length, NumberDialect dialect)
{
	if (length >= 2 && text[0] == '0' && (text[1] == 'x' || text[1] == 'X'))
		return ClassifyHexadecimal(text, length, dialect);
	if (dialect != NumberDialect::Cxx11 && length >= 2 && text[0] == '0' && (text[1] == 'b' || text[1] == 'B'))
		return ClassifyBinary(text, length, dialect);

	uint32_t pos = 0;
	while (pos < length && IsAsciiDigit(text[pos]))
		++pos;
	const uint32_t integer_digits = pos;
	uint32_t fraction_digits = 0;
	bool floating = false;
	if (pos < length && text[pos] == '.')
	{
		floating = true;
		for (++pos; pos < length && IsAsciiDigit(text[pos]); ++pos)
			++fraction_digits;
	}
	if (integer_digits + fraction_digits == 0)
		return NumberLiteral{ NumberLiteralKind::Invalid, FundamentalType::Int, length, 0 };

	const uint32_t exponent_end = ExponentEnd(text, length, pos);
	if (floating || exponent_end != pos)
		return ClassifyFloating(text, length, exponent_end, dialect);

	// An integer: a leading 0 makes it octal, and `0` itself is octal.
	if (text[0] != '0')
		return ClassifyInteger(text, length, 0, integer_digits, 10, dialect);
	for (uint32_t i = 1; i < integer_digits; ++i)
	{
		if (!IsOctalDigit(text[i]))
			return NumberLiteral{ NumberLiteralKind::Invalid, FundamentalType::Int, length, 0 };
	}
	return ClassifyInteger(text, length, 0, integer_digits, 8, dialect);
}

bool UserIntegerValue(const char* spelling, uint32_t suffix_begin, uint64_t& value)
{
	if (suffix_begin >= 2 && spelling[0] == '0' && (spelling[1] == 'x' || spelling[1] == 'X'))
		return AccumulateDigits(spelling, 2, suffix_begin, 16, value);
	if (suffix_begin >= 2 && spelling[0] == '0' && (spelling[1] == 'b' || spelling[1] == 'B'))
		return AccumulateDigits(spelling, 2, suffix_begin, 2, value);
	return AccumulateDigits(spelling, 0, suffix_begin, spelling[0] == '0' ? 8 : 10, value);
}

bool ScanFloatingValue(
	const char* text, uint32_t length, FundamentalType type, std::string& scratch, unsigned char* bytes)
{
	// The literal matches the floating-literal grammar, which strtof,
	// strtod and strtold accept in the "C" locale and round correctly. An
	// infinite result can only be an overflow.
	scratch.assign(text, length);
	switch (FloatingRepresentation(type))
	{
	case FundamentalType::Float16:
	case FundamentalType::Float128:
	case FundamentalType::GnuFloat128:
	{
		// Formats no host type has, rounded exactly.
		const bool half = FloatingRepresentation(type) == FundamentalType::Float16;
		uint64_t low = 0, high = 0;
		if (!ScanInterchangeValue(text, length, half ? kBinary16 : kBinary128, low, high))
			return false;
		std::memcpy(bytes, &low, half ? 2 : 8);
		if (!half)
			std::memcpy(bytes + 8, &high, 8);
		return true;
	}
	case FundamentalType::Float:
	{
		const float value = std::strtof(scratch.c_str(), nullptr);
		std::memcpy(bytes, &value, sizeof value);
		return !std::isinf(value);
	}
	case FundamentalType::LongDouble:
	{
		const long double value = std::strtold(scratch.c_str(), nullptr);
		std::memset(bytes, 0, sizeof value);
		std::memcpy(bytes, &value, kLongDoubleValueBytes);
		return !std::isinf(value);
	}
	default:
	{
		const double value = std::strtod(scratch.c_str(), nullptr);
		std::memcpy(bytes, &value, sizeof value);
		return !std::isinf(value);
	}
	}
}
