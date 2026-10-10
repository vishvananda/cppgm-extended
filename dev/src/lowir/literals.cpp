#include "lowir/literals.h"

#include <algorithm>
#include <cfloat>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace lowir {

static_assert(sizeof(long double) == 16 && LDBL_MANT_DIG == 64, "x87 extended long double");

namespace {

const uint64_t kF80QuietBit = 1ull << 62;
const uint64_t kF80IntegerBit = 1ull << 63;

// (high:low) * base + digit, modulo 2^128, for base <= 16.
void multiply_add(Literal& value, unsigned base, unsigned digit)
{
	const uint64_t low_part = (value.low & 0xffffffffu) * base + digit;
	const uint64_t high_part = (value.low >> 32) * base + (low_part >> 32);
	value.low = (high_part << 32) | (low_part & 0xffffffffu);
	value.high = value.high * base + (high_part >> 32);
}

void negate(Literal& value)
{
	value.low = ~value.low + 1;
	value.high = ~value.high + (value.low == 0 ? 1 : 0);
}

// Divides by 10 in place and returns the remainder.
unsigned divide_by_ten(Literal& value)
{
	uint32_t limbs[4] = { static_cast<uint32_t>(value.high >> 32), static_cast<uint32_t>(value.high),
		static_cast<uint32_t>(value.low >> 32), static_cast<uint32_t>(value.low) };
	uint64_t remainder = 0;
	for (int i = 0; i < 4; ++i)
	{
		const uint64_t current = (remainder << 32) | limbs[i];
		limbs[i] = static_cast<uint32_t>(current / 10);
		remainder = current % 10;
	}
	value.high = (static_cast<uint64_t>(limbs[0]) << 32) | limbs[1];
	value.low = (static_cast<uint64_t>(limbs[2]) << 32) | limbs[3];
	return static_cast<unsigned>(remainder);
}

int digit_value(char c)
{
	if (c >= '0' && c <= '9')
		return c - '0';
	if (c >= 'a' && c <= 'f')
		return c - 'a' + 10;
	if (c >= 'A' && c <= 'F')
		return c - 'A' + 10;
	return 99;
}

bool is_hex_prefix(const char* text, uint32_t length)
{
	return length >= 2 && text[0] == '0' && (text[1] == 'x' || text[1] == 'X');
}

TypeKind suffix_kind(char c)
{
	if (c == 'f' || c == 'F')
		return TypeKind::F32;
	if (c == 'l' || c == 'L')
		return TypeKind::F80;
	return TypeKind::F64;
}

void set_sign(TypeKind kind, Literal& bits)
{
	if (kind == TypeKind::F32)
		bits.low |= 1ull << 31;
	else if (kind == TypeKind::F64)
		bits.low |= 1ull << 63;
	else
		bits.high |= 1u << 15;
}

// Classifies from the bits, so a signaling NaN is seen as written.
enum class FloatClass
{
	Finite,
	Infinity,
	QuietNaN,
	SignalingNaN
};

FloatClass classify(TypeKind kind, const Literal& bits, bool& negative)
{
	uint64_t exponent, all_ones, mantissa, quiet;
	if (kind == TypeKind::F32)
	{
		negative = (bits.low >> 31) & 1;
		exponent = (bits.low >> 23) & 0xff;
		all_ones = 0xff;
		mantissa = bits.low & 0x7fffff;
		quiet = 1u << 22;
	}
	else if (kind == TypeKind::F64)
	{
		negative = bits.low >> 63;
		exponent = (bits.low >> 52) & 0x7ff;
		all_ones = 0x7ff;
		mantissa = bits.low & ((1ull << 52) - 1);
		quiet = 1ull << 51;
	}
	else
	{
		negative = (bits.high >> 15) & 1;
		exponent = bits.high & 0x7fff;
		all_ones = 0x7fff;
		mantissa = bits.low & ~kF80IntegerBit;
		quiet = kF80QuietBit;
	}
	if (exponent != all_ones)
		return FloatClass::Finite;
	if (mantissa == 0)
		return FloatClass::Infinity;
	return (mantissa & quiet) != 0 ? FloatClass::QuietNaN : FloatClass::SignalingNaN;
}

uint64_t nan_quiet_bit(TypeKind kind)
{
	return kind == TypeKind::F32 ? 1ull << 22 : kind == TypeKind::F64 ? 1ull << 51 : kF80QuietBit;
}

// Whether decimal text reads back as the value in the type (an f32 also
// through a double, as a reader rounding twice would read it).
bool reads_back(TypeKind kind, const char* text, long double value)
{
	if (kind == TypeKind::F32)
		return std::strtof(text, nullptr) == static_cast<float>(value) &&
			static_cast<float>(std::strtod(text, nullptr)) == static_cast<float>(value);
	if (kind == TypeKind::F64)
		return std::strtod(text, nullptr) == static_cast<double>(value);
	return std::strtold(text, nullptr) == value;
}

} // namespace

bool is_floating_spelling(const char* text, uint32_t length)
{
	const bool hex = is_hex_prefix(text, length);
	if (!hex && length > 1 && suffix_kind(text[length - 1]) != TypeKind::F64)
		return true;
	for (uint32_t i = 0; i < length; ++i)
	{
		const char c = text[i];
		if (c == '.' || (hex ? (c == 'p' || c == 'P') : (c == 'e' || c == 'E')))
			return true;
	}
	return false;
}

bool scan_integer_literal(const char* text, uint32_t length, bool negative, bool wide, Literal& bits)
{
	unsigned base = 10;
	uint32_t start = 0;
	if (is_hex_prefix(text, length))
	{
		base = 16;
		start = 2;
	}
	else if (length > 1 && text[0] == '0')
	{
		base = 8;
		start = 1;
	}
	if (start == length)
		return false;
	bits = Literal{ 0, 0 };
	for (uint32_t i = start; i < length; ++i)
	{
		const int digit = digit_value(text[i]);
		if (digit >= static_cast<int>(base))
			return false;
		multiply_add(bits, base, static_cast<unsigned>(digit));
	}
	if (negative)
		negate(bits);
	if (!wide)
		bits.high = (bits.low >> 63) != 0 ? ~static_cast<uint64_t>(0) : 0;
	return true;
}

bool scan_float_literal(const char* text, uint32_t length, bool negative, TypeKind context, TypeKind& kind,
	Literal& bits, std::string& scratch)
{
	// A hexadecimal spelling's trailing 'f' is a digit unless the binary
	// exponent precedes it.
	uint32_t digits = length;
	TypeKind written = TypeKind::F64;
	bool exponent = !is_hex_prefix(text, length);
	for (uint32_t i = 0; !exponent && i < length; ++i)
		exponent = text[i] == 'p' || text[i] == 'P';
	if (exponent && length > 1)
	{
		written = suffix_kind(text[length - 1]);
		if (written != TypeKind::F64)
			--digits;
	}
	kind = float_rank(context) != 0 ? context : written;
	scratch.assign(text, digits);
	char* end = nullptr;
	long double value;
	if (kind == TypeKind::F32)
		value = std::strtof(scratch.c_str(), &end);
	else if (kind == TypeKind::F64)
		value = std::strtod(scratch.c_str(), &end);
	else
		value = std::strtold(scratch.c_str(), &end);
	if (digits == 0 || end != scratch.c_str() + scratch.size() || std::isinf(value))
		return false;
	bits = float_literal(kind, value);
	if (negative)
		set_sign(kind, bits);
	return true;
}

Literal special_float(TypeKind kind, SpecialFloat which, bool negative)
{
	Literal bits = { 0, 0 };
	if (kind == TypeKind::F32)
		bits.low = which == SpecialFloat::Infinity ? 0x7f800000u
			: which == SpecialFloat::QuietNaN      ? 0x7fc00000u
												   : 0x7fa00000u;
	else if (kind == TypeKind::F64)
		bits.low = which == SpecialFloat::Infinity ? 0x7ff0000000000000ull
			: which == SpecialFloat::QuietNaN      ? 0x7ff8000000000000ull
												   : 0x7ff4000000000000ull;
	else
	{
		bits.high = 0x7fff;
		bits.low = which == SpecialFloat::Infinity ? kF80IntegerBit
			: which == SpecialFloat::QuietNaN      ? kF80IntegerBit | kF80QuietBit
												   : kF80IntegerBit | (kF80QuietBit >> 1);
	}
	if (negative)
		set_sign(kind, bits);
	return bits;
}

bool nan_with_payload(TypeKind kind, SpecialFloat which, bool negative, uint64_t payload, Literal& bits)
{
	const uint64_t quiet = nan_quiet_bit(kind);
	if (which == SpecialFloat::Infinity || payload >= quiet || (which == SpecialFloat::SignalingNaN && payload == 0))
		return false;
	bits = special_float(kind, which, negative);
	bits.low = (bits.low & ~(quiet - 1)) | payload;
	return true;
}

Literal float_literal(TypeKind kind, long double value)
{
	Literal bits = { 0, 0 };
	if (value != value && (kind == TypeKind::F32 || kind == TypeKind::F64))
	{
		// A NaN keeps its sign, quiet bit and payload: the top fraction bits
		// of its x87 significand (a hardware conversion would quiet a
		// signaling one); one left with no fraction bit is quiet.
		unsigned char bytes[sizeof(long double)];
		std::memcpy(bytes, &value, sizeof value);
		uint64_t significand;
		uint16_t sign_exponent;
		std::memcpy(&significand, bytes, 8);
		std::memcpy(&sign_exponent, bytes + 8, 2);
		const uint64_t sign = sign_exponent >> 15;
		const unsigned width = kind == TypeKind::F32 ? 23 : 52;
		uint64_t fraction = (significand >> (63 - width)) & ((uint64_t(1) << width) - 1);
		if (fraction == 0)
			fraction = uint64_t(1) << (width - 1);
		bits.low = kind == TypeKind::F32 ? (sign << 31) | 0x7f800000ull | fraction
										 : (sign << 63) | 0x7ff0000000000000ull | fraction;
		return bits;
	}
	if (kind == TypeKind::F32)
	{
		const float narrow = static_cast<float>(value);
		uint32_t word;
		std::memcpy(&word, &narrow, sizeof word);
		bits.low = word;
	}
	else if (kind == TypeKind::F64)
	{
		const double narrow = static_cast<double>(value);
		std::memcpy(&bits.low, &narrow, sizeof bits.low);
	}
	else
	{
		unsigned char bytes[sizeof(long double)];
		std::memcpy(bytes, &value, sizeof value);
		std::memcpy(&bits.low, bytes, 8);
		uint16_t sign_exponent;
		std::memcpy(&sign_exponent, bytes + 8, 2);
		bits.high = sign_exponent;
	}
	return bits;
}

long double float_value(TypeKind kind, const Literal& bits)
{
	if (kind == TypeKind::F32)
	{
		const uint32_t word = static_cast<uint32_t>(bits.low);
		float narrow;
		std::memcpy(&narrow, &word, sizeof narrow);
		return narrow;
	}
	if (kind == TypeKind::F64)
	{
		double wide;
		std::memcpy(&wide, &bits.low, sizeof wide);
		return wide;
	}
	unsigned char bytes[sizeof(long double)] = {};
	std::memcpy(bytes, &bits.low, 8);
	const uint16_t sign_exponent = static_cast<uint16_t>(bits.high);
	std::memcpy(bytes + 8, &sign_exponent, 2);
	long double value;
	std::memcpy(&value, bytes, sizeof value);
	return value;
}

Literal integer_as_float(TypeKind kind, const Literal& bits, bool negative_zero)
{
	// The 128-bit two's complement value, rounded once to the kind.
	Literal magnitude = bits;
	const bool negative = (bits.high >> 63) != 0;
	if (negative)
		negate(magnitude);
	long double value = static_cast<long double>(magnitude.high) * 18446744073709551616.0L +
		static_cast<long double>(magnitude.low);
	if (negative || (negative_zero && value == 0))
		value = -value;
	return float_literal(kind, value);
}

void append_integer_literal(std::string& out, const Literal& bits, TypeKind context)
{
	// The value as an operand of the context's width: signed for iN,
	// unsigned for uN, i1 and ptr; the 128 bits written for none.
	Literal value = bits;
	const unsigned width = context == TypeKind::Ptr ? 64 : integer_bits(context);
	const bool is_signed = width > 1 && context != TypeKind::Ptr && context != TypeKind::U8 &&
		context != TypeKind::U16 && context != TypeKind::U32;
	if (width != 0 && width < 128)
	{
		if (width <= 64)
		{
			value.high = 0;
			if (width < 64)
				value.low &= (uint64_t(1) << width) - 1;
			if (is_signed && ((value.low >> (width - 1)) & 1) != 0)
			{
				if (width < 64)
					value.low |= ~((uint64_t(1) << width) - 1);
				value.high = ~uint64_t(0);
			}
		}
	}
	Literal magnitude = value;
	if ((width == 0 || is_signed) && (value.high >> 63) != 0)
	{
		out.push_back('-');
		negate(magnitude);
	}
	char digits[40];
	int count = 0;
	do
		digits[count++] = static_cast<char>('0' + divide_by_ten(magnitude));
	while (magnitude.low != 0 || magnitude.high != 0);
	while (count > 0)
		out.push_back(digits[--count]);
}

void append_float_literal(std::string& out, TypeKind kind, const Literal& bits, bool exact, bool marked)
{
	bool negative = false;
	const FloatClass category = classify(kind, bits, negative);
	const char suffix = kind == TypeKind::F32 ? 'f' : kind == TypeKind::F80 ? 'L' : '\0';
	if (category != FloatClass::Finite)
	{
		if (negative)
			out.push_back('-');
		// The course spells an f32 or f80 infinity with its literal suffix
		// (`INFINITYL`), and a NaN too (`nanf`, `snanL`): an untyped
		// operand, a call argument, would otherwise read as an f64 and be
		// converted (quieted, if signaling).
		if (category == FloatClass::Infinity)
		{
			out += kind == TypeKind::F32 ? "INFINITYf" : kind == TypeKind::F80 ? "INFINITYL" : "inf";
			return;
		}
		out += category == FloatClass::QuietNaN ? "nan" : "snan";
		if (suffix != '\0')
			out.push_back(suffix);
		const uint64_t quiet = nan_quiet_bit(kind);
		const uint64_t payload = bits.low & (quiet - 1);
		if (exact && payload != (category == FloatClass::QuietNaN ? 0 : quiet >> 1))
		{
			char buffer[24];
			const int length = std::snprintf(
				buffer, sizeof buffer, "(0x%llx)", static_cast<unsigned long long>(payload));
			out.append(buffer, static_cast<size_t>(length));
		}
		return;
	}
	const long double value = float_value(kind, bits);
	char buffer[64];
	int length;
	if (value != 0 && std::fabs(value) < (kind == TypeKind::F32 ? FLT_MIN : kind == TypeKind::F64 ? DBL_MIN : LDBL_MIN))
	{
		// A subnormal value as exact hexadecimal floating text: its decimal
		// reads back only with an underflow the reference reader rejects.
		length = kind == TypeKind::F80 ? std::snprintf(buffer, sizeof buffer, "%La", value)
									   : std::snprintf(buffer, sizeof buffer, "%a", static_cast<double>(value));
	}
	else
	{
		// The course's spelling: 20 significant digits of the value, trailing
		// zeros dropped (`8`, `1.5`, `0.10000000000000000555`); an f80 that
		// needs it, 21.
		length = std::snprintf(buffer, sizeof buffer, "%.20Lg", value);
		if (kind == TypeKind::F80 && !reads_back(kind, buffer, value))
			length = std::snprintf(buffer, sizeof buffer, "%.21Lg", value);
	}
	out.append(buffer, static_cast<size_t>(length));
	if (suffix != '\0')
		out.push_back(suffix);
	else if (marked && std::strpbrk(buffer, ".ep") == nullptr)
		out += ".0";
}

} // namespace lowir
