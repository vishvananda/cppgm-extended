// GCC's binary16 and binary128 floating values (_Float16; _Float128 and
// __float128) and Clang's bfloat16 ones (__bf16). LowIR's floating types
// are f32, f64 and f80, so a value is an object of its bytes (obj<2x2>,
// obj<16x16>) and its operations are calls of the C runtime's soft-float
// routines, as GCC and Clang call them on x86-64 without AVX512-FP16 or
// AVX512-BF16: binary128 arithmetic, comparisons and conversions by
// libgcc's tf routines; binary16 arithmetic in float, each result rounded
// back (the operands are exact in float, whose 24 bits make that one
// rounding the binary16 operation's, IEEE 754 binary16 semantics rather
// than C's excess precision), and conversions by its hf routines. A
// bfloat16 value is a float's upper half, widened in place (its bits
// shifted up, as Clang widens it) and rounded back by libgcc's bf
// routines; its arithmetic is Clang's: in float, an expression of its
// operators rounded once (FunctionLowering::bfloat_arithmetic). At a host
// object's call boundary each value is SSE class (psABI 3.2.3: binary128's
// SSEUP upper half in the same XMM register).

#include "lower/function_lowering.h"

#include <cmath>
#include <cstring>
#include <limits>
#include <utility>
#include <vector>

#include "lex/interchange_float.h"
#include "lex/number_literal.h"
#include "sema/conversions.h"
#include "sema/integral_arithmetic.h"

namespace lower {

namespace {

// Operand and result kinds of a routine: h binary16, b bfloat16, q
// binary128, f float, d double, x long double, i int, u unsigned int, l
// long (and unsigned long; libgcc's comparisons return their word-mode
// CMPtype), t __int128 (and unsigned).
struct SoftFloatSpec
{
	const char* name;
	const char* parameters;
	char result;
};

// Indexed by SoftFloatHelper.
const SoftFloatSpec kSoftFloat[] = {
	{ "__addtf3", "qq", 'q' },
	{ "__subtf3", "qq", 'q' },
	{ "__multf3", "qq", 'q' },
	{ "__divtf3", "qq", 'q' },
	{ "__negtf2", "q", 'q' },
	{ "__eqtf2", "qq", 'l' },
	{ "__netf2", "qq", 'l' },
	{ "__lttf2", "qq", 'l' },
	{ "__letf2", "qq", 'l' },
	{ "__gttf2", "qq", 'l' },
	{ "__getf2", "qq", 'l' },
	{ "__extendsftf2", "f", 'q' },
	{ "__extenddftf2", "d", 'q' },
	{ "__extendxftf2", "x", 'q' },
	{ "__trunctfsf2", "q", 'f' },
	{ "__trunctfdf2", "q", 'd' },
	{ "__trunctfxf2", "q", 'x' },
	{ "__floatsitf", "i", 'q' },
	{ "__floatunsitf", "u", 'q' },
	{ "__floatditf", "l", 'q' },
	{ "__floatunditf", "l", 'q' },
	{ "__floattitf", "t", 'q' },
	{ "__floatuntitf", "t", 'q' },
	{ "__fixtfsi", "q", 'i' },
	{ "__fixunstfsi", "q", 'u' },
	{ "__fixtfdi", "q", 'l' },
	{ "__fixunstfdi", "q", 'l' },
	{ "__fixtfti", "q", 't' },
	{ "__fixunstfti", "q", 't' },
	{ "sqrt", "d", 'd' },
	{ "sqrtf", "f", 'f' },
	{ "rint", "d", 'd' },
	{ "rintf", "f", 'f' },
	{ "__extendhfsf2", "h", 'f' },
	{ "__extendhfdf2", "h", 'd' },
	{ "__extendhfxf2", "h", 'x' },
	{ "__extendhftf2", "h", 'q' },
	{ "__truncsfhf2", "f", 'h' },
	{ "__truncdfhf2", "d", 'h' },
	{ "__truncxfhf2", "x", 'h' },
	{ "__trunctfhf2", "q", 'h' },
	{ "__floattihf", "t", 'h' },
	{ "__floatuntihf", "t", 'h' },
	{ "__fixhfti", "h", 't' },
	{ "__fixunshfti", "h", 't' },
	{ "__truncsfbf2", "f", 'b' },
	{ "__truncdfbf2", "d", 'b' },
	{ "__truncxfbf2", "x", 'b' },
	{ "__trunctfbf2", "q", 'b' },
};

static_assert(sizeof(kSoftFloat) / sizeof(kSoftFloat[0]) == static_cast<size_t>(SoftFloatHelper::Count),
	"one entry per soft-float routine");

lowir::Type SpecType(char code)
{
	switch (code)
	{
	case 'h':
	case 'b':
		return lowir::Type::object(2, 1);
	case 'q':
		return lowir::Type::object(16, 4);
	case 'f':
		return lowir::Type::scalar(lowir::TypeKind::F32);
	case 'd':
		return lowir::Type::scalar(lowir::TypeKind::F64);
	case 'x':
		return lowir::Type::scalar(lowir::TypeKind::F80);
	case 'i':
		return lowir::Type::scalar(lowir::TypeKind::I32);
	case 'u':
		return lowir::Type::scalar(lowir::TypeKind::U32);
	case 't':
		return lowir::Type::scalar(lowir::TypeKind::I128);
	default:
		return lowir::Type::scalar(lowir::TypeKind::I64);
	}
}

SoftFloat SpecKind(char code)
{
	return code == 'h' ? SoftFloat::Half
		: code == 'b'  ? SoftFloat::BFloat
		: code == 'q'  ? SoftFloat::Quad
					   : SoftFloat::None;
}

// The routine converting between a standard floating kind and a soft
// kind (`widen`: to the soft kind).
SoftFloatHelper FloatingHelper(lowir::TypeKind kind, SoftFloat soft, bool widen)
{
	const unsigned index = kind == lowir::TypeKind::F32 ? 0 : kind == lowir::TypeKind::F64 ? 1 : 2;
	static const SoftFloatHelper kToQuad[] = { SoftFloatHelper::ExtendSFTF, SoftFloatHelper::ExtendDFTF,
		SoftFloatHelper::ExtendXFTF };
	static const SoftFloatHelper kFromQuad[] = { SoftFloatHelper::TruncTFSF, SoftFloatHelper::TruncTFDF,
		SoftFloatHelper::TruncTFXF };
	static const SoftFloatHelper kToHalf[] = { SoftFloatHelper::TruncSFHF, SoftFloatHelper::TruncDFHF,
		SoftFloatHelper::TruncXFHF };
	static const SoftFloatHelper kFromHalf[] = { SoftFloatHelper::ExtendHFSF, SoftFloatHelper::ExtendHFDF,
		SoftFloatHelper::ExtendHFXF };
	if (soft == SoftFloat::Quad)
		return widen ? kToQuad[index] : kFromQuad[index];
	return widen ? kToHalf[index] : kFromHalf[index];
}

bool SignedIntegral(const sema::Type* type)
{
	sema::IntegralKind kind;
	return sema::IntegralKindOf(type, kind) && kind.is_signed && kind.bits > 1;
}

} // namespace

namespace {

// A floating value decoded: special (infinity, or NaN with `nan`), or
// (-1)^negative * magnitude * 2^exponent.
struct Decoded
{
	bool negative;
	bool special;
	bool nan;
	uint64_t low;
	uint64_t high;
	int64_t exponent;
};

Decoded DecodeSoft(SoftFloat kind, uint64_t bits, uint64_t high)
{
	Decoded out = Decoded();
	if (kind == SoftFloat::BFloat)
	{
		const uint64_t biased = (bits >> 7) & 0xff;
		out.negative = ((bits >> 15) & 1) != 0;
		out.special = biased == 0xff;
		out.nan = out.special && (bits & 0x7f) != 0;
		out.low = (bits & 0x7f) | (biased != 0 ? 0x80 : 0);
		out.exponent = static_cast<int64_t>(biased != 0 ? biased : 1) - 127 - 7;
		return out;
	}
	if (kind == SoftFloat::Half)
	{
		const uint64_t biased = (bits >> 10) & 0x1f;
		out.negative = ((bits >> 15) & 1) != 0;
		out.special = biased == 0x1f;
		out.nan = out.special && (bits & 0x3ff) != 0;
		out.low = (bits & 0x3ff) | (biased != 0 ? 0x400 : 0);
		out.exponent = static_cast<int64_t>(biased != 0 ? biased : 1) - 15 - 10;
		return out;
	}
	const uint64_t biased = (high >> 48) & 0x7fff;
	out.negative = (high >> 63) != 0;
	out.special = biased == 0x7fff;
	out.nan = out.special && ((high & 0xffffffffffffull) != 0 || bits != 0);
	out.low = bits;
	out.high = (high & 0xffffffffffffull) | (biased != 0 ? 0x1000000000000ull : 0);
	out.exponent = static_cast<int64_t>(biased != 0 ? biased : 1) - 16383 - 112;
	return out;
}

Decoded DecodeExtended(long double value)
{
	// x87's format: an explicit integer bit in a 64-bit significand.
	unsigned char bytes[10];
	std::memcpy(bytes, &value, sizeof bytes);
	uint64_t significand = 0;
	std::memcpy(&significand, bytes, 8);
	const unsigned top = bytes[8] | (static_cast<unsigned>(bytes[9]) << 8);
	const unsigned biased = top & 0x7fff;
	Decoded out = Decoded();
	out.negative = (top >> 15) != 0;
	out.special = biased == 0x7fff;
	out.nan = out.special && (significand << 1) != 0;
	out.low = significand;
	out.exponent = static_cast<int64_t>(biased != 0 ? biased : 1) - 16383 - 63;
	return out;
}

typedef unsigned __int128 Wide;

Wide Magnitude(const Decoded& value)
{
	return (static_cast<Wide>(value.high) << 64) | value.low;
}

int64_t BitLength(Wide value)
{
	int64_t length = 0;
	for (; value != 0; value >>= 1)
		++length;
	return length;
}

// value >> count, a nonzero bit shifted out kept in bit 0 (a sticky bit
// below the rounding position of a result at least 3 bits wider than the
// format's significand rounds as the exact value would).
Wide ShiftSticky(Wide value, int64_t count)
{
	if (count <= 0)
		return value;
	if (count >= 128)
		return value != 0 ? 1 : 0;
	const Wide kept = value >> count;
	return kept | ((kept << count) != value ? 1 : 0);
}

bool SoftInfinity(bool negative, SoftFloat kind, StaticValue& out)
{
	const bool narrow = kind != SoftFloat::Quad;
	out = StaticValue();
	out.kind = StaticValue::Kind::Soft;
	out.bits = narrow ? (kind == SoftFloat::BFloat ? 0x7f80 : 0x7c00) | (negative ? 0x8000 : 0) : 0;
	out.high = narrow ? 0 : 0x7fff000000000000ull | (negative ? 0x8000000000000000ull : 0);
	return true;
}

const InterchangeFormat& FormatOf(SoftFloat kind)
{
	return kind == SoftFloat::Half ? kBinary16 : kind == SoftFloat::BFloat ? kBFloat16 : kBinary128;
}

// The binary16 or binary128 bits of (-1)^negative * magnitude * 2^exponent,
// rounded to nearest, ties to even; beyond the format's range, the
// infinity the run-time routines give (3.6.2p3: the value the dynamic
// initialization would have).
bool SoftRounded(bool negative, Wide magnitude, int64_t exponent, SoftFloat kind, StaticValue& out)
{
	out = StaticValue();
	out.kind = StaticValue::Kind::Soft;
	if (RoundInterchangeValue(negative, static_cast<uint64_t>(magnitude), static_cast<uint64_t>(magnitude >> 64),
			exponent, FormatOf(kind), out.bits, out.high))
		return true;
	return SoftInfinity(negative, kind, out);
}

// A NaN's payload, its fraction's bits from the top (the quiet bit first),
// as libgcc's conversions carry it: left-aligned in 64 bits.
uint64_t NanPayload(SoftFloat kind, uint64_t bits, uint64_t high)
{
	return kind == SoftFloat::Half  ? (bits & 0x3ff) << 54
		: kind == SoftFloat::BFloat ? (bits & 0x7f) << 57
									: ((high & 0xffffffffffffull) << 16) | (bits >> 48);
}

// The magnitude * 2^exponent (nonzero) rounded to nearest, ties to even, to
// `precision` bits of a binary format whose least normal exponent is
// `emin` (below it, its subnormals' quantum): significand * 2^scale.
void RoundBinary(Wide magnitude, int64_t exponent, int64_t precision, int64_t emin, Wide& significand, int64_t& scale)
{
	const int64_t length = BitLength(magnitude);
	const int64_t top = length - 1 + exponent;
	scale = (top > emin ? top : emin) - (precision - 1);
	if (scale <= exponent)
	{
		significand = magnitude << (exponent - scale);
		return;
	}
	const int64_t drop = scale - exponent;
	if (drop > 128)
	{
		significand = 0;
		return;
	}
	const Wide kept = drop == 128 ? 0 : magnitude >> drop;
	const Wide rest = drop == 128 ? magnitude : magnitude & ((Wide(1) << drop) - 1);
	const Wide half = Wide(1) << (drop - 1);
	significand = kept + ((rest > half || (rest == half && (kept & 1) != 0)) ? 1 : 0);
}

// (-1)^negative * magnitude * 2^exponent (magnitude nonzero) as the value
// of a standard floating type of the representation (float, double or
// long double) nearest it, ties to even; beyond its range, the infinity
// the run-time conversions give (3.6.2p3). In long double, which holds
// every such value exactly.
long double StandardRounded(bool negative, Wide magnitude, int64_t exponent, FundamentalType representation)
{
	const int64_t precision = representation == FundamentalType::Float ? 24
		: representation == FundamentalType::Double                    ? 53
																	   : 64;
	const int64_t emax = representation == FundamentalType::Float ? 127
		: representation == FundamentalType::Double               ? 1023
																  : 16383;
	Wide significand = 0;
	int64_t scale = 0;
	RoundBinary(magnitude, exponent, precision, 1 - emax, significand, scale);
	const long double rounded = significand != 0 && BitLength(significand) - 1 + scale > emax
		? std::numeric_limits<long double>::infinity()
		: std::ldexp(static_cast<long double>(significand), static_cast<int>(scale));
	return negative ? -rounded : rounded;
}

} // namespace

bool SoftStaticValue(const StaticValue& value, const sema::Type* from, SoftFloat target, StaticValue& out)
{
	Decoded decoded = Decoded();
	uint64_t payload = 0;
	if (value.kind == StaticValue::Kind::Soft)
	{
		const SoftFloat source = from != nullptr ? SoftFloatOf(from) : SoftFloat::None;
		if (source == SoftFloat::None)
			return false;
		if (source == target)
		{
			out = value;
			return true;
		}
		decoded = DecodeSoft(source, value.bits, value.high);
		payload = NanPayload(source, value.bits, value.high);
	}
	else if (value.kind == StaticValue::Kind::Floating)
	{
		decoded = DecodeExtended(value.floating);
		payload = decoded.low << 1;
	}
	else if (value.kind == StaticValue::Kind::Integer)
	{
		// 128-bit two's complement, negative only for a signed source.
		sema::IntegralKind kind;
		const bool is_signed = from != nullptr && sema::IntegralKindOf(from, kind) && kind.is_signed && kind.bits > 1;
		decoded.negative = is_signed && (value.high >> 63) != 0;
		decoded.low = value.bits;
		decoded.high = value.high;
		if (decoded.negative)
		{
			decoded.low = 0 - value.bits;
			decoded.high = ~value.high + (value.bits == 0 ? 1 : 0);
		}
	}
	else
		return false;
	out = StaticValue();
	out.kind = StaticValue::Kind::Soft;
	const bool half = target != SoftFloat::Quad;
	if (decoded.special)
	{
		// Infinity, or a quiet NaN of the source's payload, its leading
		// fraction bits (libgcc's conversions'; a signaling one quieted).
		if (!decoded.nan)
			payload = 0;
		out.bits = target == SoftFloat::BFloat
			? 0x7f80 | (decoded.nan ? 0x40 | (payload >> 57) : 0) | (decoded.negative ? 0x8000 : 0)
			: half        ? 0x7c00 | (decoded.nan ? 0x200 | (payload >> 54) : 0) | (decoded.negative ? 0x8000 : 0)
			: decoded.nan ? payload << 48
						  : 0;
		out.high = half ? 0
						: 0x7fff000000000000ull | (decoded.nan ? 0x800000000000ull | (payload >> 16) : 0) |
				(decoded.negative ? 0x8000000000000000ull : 0);
		return true;
	}
	return SoftRounded(decoded.negative, Magnitude(decoded), decoded.exponent, target, out);
}

bool SoftStaticConverted(const StaticValue& value, SoftFloat source, const sema::Type* target, StaticValue& out)
{
	const Decoded decoded = DecodeSoft(source, value.bits, value.high);
	const Wide magnitude = Magnitude(decoded);
	target = target->unqualified;
	sema::IntegralKind kind;
	if (sema::IntegralKindOf(target, kind))
	{
		// 4.12: nonzero (a NaN too) is true; 4.9p1: truncated toward zero,
		// undefined (run time) unless the integer type represents it.
		out = StaticValue();
		out.kind = StaticValue::Kind::Integer;
		if (kind.bits == 1)
		{
			out.bits = decoded.nan || magnitude != 0 ? 1 : 0;
			return true;
		}
		if (decoded.special)
			return false;
		Wide whole = magnitude;
		if (decoded.exponent >= 0)
		{
			if (BitLength(magnitude) + decoded.exponent > 128)
				return false;
			whole <<= decoded.exponent;
		}
		else
			whole = decoded.exponent <= -128 ? 0 : magnitude >> -decoded.exponent;
		if (kind.is_signed ? whole > (Wide(1) << (kind.bits - 1)) - (decoded.negative ? 0 : 1)
						   : (decoded.negative && whole != 0) || (kind.bits < 128 && (whole >> kind.bits) != 0))
			return false;
		const Wide bits = decoded.negative ? ~whole + 1 : whole;
		out.bits = static_cast<uint64_t>(bits);
		out.high = static_cast<uint64_t>(bits >> 64);
		return true;
	}
	if (target->kind != sema::TypeKind::Fundamental || !sema::IsFloating(target))
		return false;
	// 4.8p1: a standard floating type's value nearest it (ties to even),
	// beyond its range undefined (run time); in long double, which holds
	// every such value exactly.
	out = StaticValue();
	out.kind = StaticValue::Kind::Floating;
	const FundamentalType representation = FloatingRepresentation(target->fundamental);
	if (decoded.special)
	{
		// Infinity, or a quiet NaN of the payload's leading bits (x87's
		// explicit integer bit, then the fraction).
		long double special = std::numeric_limits<long double>::infinity();
		if (decoded.nan)
		{
			unsigned char bytes[sizeof(long double)] = {};
			const uint64_t significand = 0xc000000000000000ull | (NanPayload(source, value.bits, value.high) >> 1);
			std::memcpy(bytes, &significand, 8);
			bytes[8] = 0xff;
			bytes[9] = 0x7f;
			std::memcpy(&special, bytes, sizeof special);
		}
		out.floating = decoded.negative ? -special : special;
		return true;
	}
	if (magnitude == 0)
	{
		out.floating = decoded.negative ? -0.0L : 0.0L;
		return true;
	}
	out.floating = StandardRounded(decoded.negative, magnitude, decoded.exponent, representation);
	return true;
}

bool IntegralStaticFloating(
	const StaticValue& value, const sema::Type* from, const sema::Type* target, StaticValue& out)
{
	// 4.9p2: the integer's value (a 128-bit one's whole value, negative
	// only for a signed type) rounded once to the standard floating type,
	// as the run-time conversion routines round.
	sema::IntegralKind kind;
	if (from == nullptr || !sema::IntegralKindOf(from, kind) ||
		target->unqualified->kind != sema::TypeKind::Fundamental)
		return false;
	const bool negative = kind.is_signed && kind.bits > 1 && (value.high >> 63) != 0;
	Wide magnitude = (static_cast<Wide>(value.high) << 64) | value.bits;
	if (negative)
		magnitude = ~magnitude + 1;
	out = StaticValue();
	out.kind = StaticValue::Kind::Floating;
	out.floating = magnitude == 0
		? 0.0L
		: StandardRounded(negative, magnitude, 0, FloatingRepresentation(target->unqualified->fundamental));
	return true;
}

bool SoftStaticArithmetic(
	TokenKind op, SoftFloat kind, const StaticValue& left, const StaticValue& right, StaticValue& out)
{
	// IEEE 754's +, -, * and / of finite operands, the exact result rounded
	// once, as libgcc's binary128 routines round; binary16's in float, then
	// rounded back, is that too (24 >= 2 * 11 + 2 bits: double rounding is
	// innocuous), and bfloat16's (each operation rounded, as Clang's
	// constant evaluation rounds it). Infinite and NaN operands and
	// division by zero are left to run time.
	Decoded a = DecodeSoft(kind, left.bits, left.high);
	Decoded b = DecodeSoft(kind, right.bits, right.high);
	if (op == TokenKind::OP_MINUS)
		b.negative = !b.negative;
	Wide x = Magnitude(a), y = Magnitude(b);
	if (a.nan || b.nan)
		return false;
	if (a.special || b.special || (op == TokenKind::OP_DIV && y == 0))
	{
		// IEEE 754's infinite results (an invalid operation's NaN is left
		// to run time): an infinite sum but of opposite infinities, an
		// infinite operand of a product but zero, an infinite dividend but
		// over infinity, a nonzero one over zero; a finite one over an
		// infinite divisor is zero.
		const bool product_sign = a.negative != b.negative;
		bool infinite = false, negative = false;
		if (op == TokenKind::OP_PLUS || op == TokenKind::OP_MINUS)
		{
			if (a.special && b.special && a.negative != b.negative)
				return false;
			infinite = true;
			negative = a.special ? a.negative : b.negative;
		}
		else if (op == TokenKind::OP_STAR)
		{
			if ((a.special && y == 0) || (b.special && x == 0))
				return false;
			infinite = true;
			negative = product_sign;
		}
		else if (op == TokenKind::OP_DIV)
		{
			if ((a.special && b.special) || (!a.special && x == 0 && y == 0 && !b.special))
				return false;
			infinite = !b.special;
			negative = product_sign;
		}
		else
			return false;
		return infinite ? SoftInfinity(negative, kind, out) : SoftRounded(negative, 0, 0, kind, out);
	}
	if (op == TokenKind::OP_PLUS || op == TokenKind::OP_MINUS)
	{
		// Aligned to the greater exponent with 10 guard bits, the smaller
		// operand's bits below them sticky; equal magnitudes of opposite
		// signs (and two zeros of opposite signs) sum to +0.
		if (x == 0 && y == 0)
			return SoftRounded(a.negative && b.negative, 0, 0, kind, out);
		if (x == 0 || y == 0)
			return x == 0 ? SoftRounded(b.negative, y, b.exponent, kind, out)
						  : SoftRounded(a.negative, x, a.exponent, kind, out);
		if (a.exponent < b.exponent)
		{
			std::swap(a, b);
			std::swap(x, y);
		}
		const int64_t exponent = a.exponent - 10;
		x <<= 10;
		y = ShiftSticky(y << 10, a.exponent - b.exponent);
		if (a.negative == b.negative)
			return SoftRounded(a.negative, x + y, exponent, kind, out);
		if (x == y)
			return SoftRounded(false, 0, 0, kind, out);
		return x > y ? SoftRounded(a.negative, x - y, exponent, kind, out)
					 : SoftRounded(b.negative, y - x, exponent, kind, out);
	}
	const bool negative = a.negative != b.negative;
	if (op == TokenKind::OP_STAR)
	{
		if (x == 0 || y == 0)
			return SoftRounded(negative, 0, 0, kind, out);
		// The 226-bit product's top 126 bits, the rest sticky.
		const uint64_t x0 = static_cast<uint64_t>(x), x1 = static_cast<uint64_t>(x >> 64);
		const uint64_t y0 = static_cast<uint64_t>(y), y1 = static_cast<uint64_t>(y >> 64);
		const Wide p00 = Wide(x0) * y0, p01 = Wide(x0) * y1, p10 = Wide(x1) * y0, p11 = Wide(x1) * y1;
		const Wide middle = (p00 >> 64) + static_cast<uint64_t>(p01) + static_cast<uint64_t>(p10);
		const Wide low = (middle << 64) | static_cast<uint64_t>(p00);
		const Wide high = p11 + (p01 >> 64) + (p10 >> 64) + (middle >> 64);
		if (high == 0)
			return SoftRounded(negative, low, a.exponent + b.exponent, kind, out);
		const int64_t shift = BitLength(high) + 2;
		const Wide product = (high << (128 - shift)) | (low >> shift) | ((low & ((Wide(1) << shift) - 1)) != 0 ? 1 : 0);
		return SoftRounded(negative, product, a.exponent + b.exponent + shift, kind, out);
	}
	if (op != TokenKind::OP_DIV)
		return false;
	if (x == 0)
		return SoftRounded(negative, 0, 0, kind, out);
	// Restoring division: x / y brought into [1, 2), then 125 quotient
	// bits, the remainder sticky.
	int64_t exponent = a.exponent - b.exponent;
	while (x < y)
	{
		x <<= 1;
		--exponent;
	}
	while (x >= (y << 1))
	{
		y <<= 1;
		++exponent;
	}
	Wide quotient = 0;
	for (int i = 0; i < 125; ++i)
	{
		quotient <<= 1;
		if (x >= y)
		{
			x -= y;
			quotient |= 1;
		}
		x <<= 1;
	}
	return SoftRounded(negative, quotient | (x != 0 ? 1 : 0), exponent - 124, kind, out);
}

SoftFloat SoftFloatOfValue(lowir::Type type)
{
	// (A bfloat16 operand is widened to float first: see
	// FunctionLowering::bfloat_widened.)
	if (!type.is_object())
		return SoftFloat::None;
	return type.bytes == 2 ? SoftFloat::Half : SoftFloat::Quad;
}

uint8_t ProgramLowering::soft_float_classes(SoftFloat kind) const
{
	if (!object_mode_ || kind == SoftFloat::None)
		return 0;
	return static_cast<uint8_t>(
		lowir::OC_STATED | lowir::EB_SSE | (kind == SoftFloat::Quad ? lowir::EB_SSEUP << 2 : lowir::EB_NONE));
}

lowir::SymbolId ProgramLowering::soft_float_helper(SoftFloatHelper which)
{
	// Declared as the C functions they are, which neither throw nor touch
	// memory the program sees.
	const size_t index = static_cast<size_t>(which);
	if (soft_float_helpers_.empty())
		soft_float_helpers_.assign(static_cast<size_t>(SoftFloatHelper::Count), lowir::kNoId);
	if (soft_float_helpers_[index] != lowir::kNoId)
		return soft_float_helpers_[index];
	const SoftFloatSpec& spec = kSoftFloat[index];
	const lowir::SymbolId symbol = new_symbol(
		std::string("__cppgm_libgcc_") + (spec.name[0] == '_' ? spec.name + 2 : spec.name));
	lowir::Signature signature = lowir::Signature();
	signature.first_parameter = static_cast<uint32_t>(program_.parameters.size());
	const uint32_t count = static_cast<uint32_t>(std::strlen(spec.parameters));
	for (uint32_t i = 0; i < count; ++i)
	{
		lowir::Parameter parameter = lowir::Parameter();
		parameter.name = argument_name(i);
		parameter.type = SpecType(spec.parameters[i]);
		parameter.classes = soft_float_classes(SpecKind(spec.parameters[i]));
		program_.parameters.push_back(parameter);
	}
	signature.parameter_count = count;
	signature.result = SpecType(spec.result);
	signature.boundary.result_classes = soft_float_classes(SpecKind(spec.result));
	signature.boundary.unwind = ir_model::CUM_NO;
	program_.declare_function(symbol, lowir::SymbolKind::FunctionDeclaration, program_.add_signature(signature));
	lowir::SymbolMetadata& metadata = program_.symbols[symbol].metadata;
	metadata.linkage = ir_model::LLM_C;
	metadata.binding = ir_model::SBM_STRONG;
	metadata.object = intern(spec.name, static_cast<uint32_t>(std::strlen(spec.name)));
	soft_float_helpers_[index] = symbol;
	++counters_.declarations;
	return symbol;
}

// ------------------------------------------------------------- values

RValue FunctionLowering::soft_call(SoftFloatHelper which, const RValue& first, const RValue* second)
{
	const size_t base = arguments_.size();
	arguments_.push_back(first.value);
	if (second != nullptr)
		arguments_.push_back(second->value);
	RValue result;
	result.type = SpecType(kSoftFloat[static_cast<size_t>(which)].result);
	result.value = emit_call(
		result.type, lowir::Value::symbol(lowering_.soft_float_helper(which)), base, lowir::kNoId, false);
	return result;
}

RValue FunctionLowering::soft_constant(SoftFloat kind, uint64_t low, uint64_t high)
{
	// The bits stored in a slot of their own, read as one value.
	const lowir::Type type = SpecType(kind == SoftFloat::Half ? 'h' : 'q');
	Place storage = Place();
	storage.storage = lowir::Value::slot(generated_slot(lowir::SlotKind::SoftFloat, type));
	const lowir::Value address = address_of(storage);
	Place word = Place();
	word.storage = address;
	if (kind != SoftFloat::Quad)
		store(lowir::Type::scalar(lowir::TypeKind::U16), integer(lowir::Type::scalar(lowir::TypeKind::U16), low), word);
	else
	{
		const lowir::Type i64 = lowir::Type::scalar(lowir::TypeKind::I64);
		store(i64, integer(i64, low), word);
		word.storage =
			index(lowir::Type::scalar(lowir::TypeKind::I8), ir_model::IPK_FIELD, address, integer(i64, 8)).value;
		store(i64, integer(i64, high), word);
	}
	return load(type, storage);
}

RValue FunctionLowering::soft_literal(SoftFloat kind, const char* text, uint32_t length, uint32_t location)
{
	// 2.14.4p1: the value in the literal's own format, rounded once.
	uint64_t low = 0, high = 0;
	if (!ScanInterchangeValue(text, length, FormatOf(kind), low, high))
		throw LoweringError(location, "a floating literal outside its type's range");
	return soft_constant(kind, low, high);
}

RValue FunctionLowering::soft_zero(SoftFloat kind)
{
	return soft_constant(kind, 0, 0);
}

RValue FunctionLowering::soft_convert(const RValue& value, const sema::Type* from, const sema::Type* to)
{
	// 4.8, 4.9: by the runtime's routine for the two kinds. A narrower
	// integer converts as an int; a binary16 to or from an integer below
	// 128 bits goes through float, which holds every binary16 value and
	// every integer up to binary16's range exactly (an integer beyond it,
	// rounded in float, still converts to infinity).
	const SoftFloat source = SoftFloatOf(from);
	const SoftFloat target = SoftFloatOf(to);
	const lowir::Type f32 = lowir::Type::scalar(lowir::TypeKind::F32);
	const lowir::Type i32 = lowir::Type::scalar(lowir::TypeKind::I32);
	if (source == SoftFloat::BFloat || target == SoftFloat::BFloat)
		return bfloat_convert(value, from, to);
	if (source != SoftFloat::None && target != SoftFloat::None)
	{
		if (source == target)
			return value;
		return soft_call(target == SoftFloat::Quad ? SoftFloatHelper::ExtendHFTF : SoftFloatHelper::TruncTFHF, value);
	}
	if (target != SoftFloat::None)
	{
		if (sema::IsFloating(from))
			return soft_call(FloatingHelper(value.type.kind(), target, true), value);
		const bool is_signed = SignedIntegral(from);
		const unsigned bits = lowir::integer_bits(value.type.kind());
		if (bits == 128)
			return soft_call(target == SoftFloat::Quad
					? (is_signed ? SoftFloatHelper::FloatTITF : SoftFloatHelper::FloatUnsTITF)
					: (is_signed ? SoftFloatHelper::FloatTIHF : SoftFloatHelper::FloatUnsTIHF),
				value);
		if (target == SoftFloat::Half)
			return soft_call(SoftFloatHelper::TruncSFHF,
				convert_op(is_signed ? lowir::ConvertOp::SIToFP : lowir::ConvertOp::UIToFP, f32, value));
		if (bits == 64)
			return soft_call(is_signed ? SoftFloatHelper::FloatDITF : SoftFloatHelper::FloatUnsDITF, value);
		if (bits == 32 && !is_signed)
			return soft_call(SoftFloatHelper::FloatUnsSITF, value);
		return soft_call(
			SoftFloatHelper::FloatSITF, convert_integer(value, from, i32, ConversionContext::Operand, true, false));
	}
	const lowir::Type storage = lowering_.storage_type(to);
	if (storage.is_float())
		return soft_call(FloatingHelper(storage.kind(), source, false), value);
	const bool is_signed = SignedIntegral(to);
	const unsigned bits = lowir::integer_bits(storage.kind());
	if (bits == 128)
		return soft_call(source == SoftFloat::Quad
				? (is_signed ? SoftFloatHelper::FixTFTI : SoftFloatHelper::FixUnsTFTI)
				: (is_signed ? SoftFloatHelper::FixHFTI : SoftFloatHelper::FixUnsHFTI),
			value);
	if (source == SoftFloat::Half)
		return convert_op(is_signed ? lowir::ConvertOp::FPToSI : lowir::ConvertOp::FPToUI, storage,
			soft_call(SoftFloatHelper::ExtendHFSF, value));
	if (bits == 64)
		return soft_call(is_signed ? SoftFloatHelper::FixTFDI : SoftFloatHelper::FixUnsTFDI, value);
	if (bits == 32 && !is_signed)
		return soft_call(SoftFloatHelper::FixUnsTFSI, value);
	const RValue whole = soft_call(SoftFloatHelper::FixTFSI, value);
	if (bits == 32)
		return whole;
	return convert_op(lowir::ConvertOp::Trunc, storage, whole);
}

RValue FunctionLowering::bfloat_convert(const RValue& value, const sema::Type* from, const sema::Type* to)
{
	// Clang's conversions of bfloat16 values: from float, double, long
	// double and binary128 rounded once by libgcc's bf routines, from an
	// integer through float (each conversion rounding, as Clang's x86-64
	// code converts it), from binary16 through float (exact); to any other
	// type from float, which holds every bfloat16 value.
	const lowir::Type f32 = lowir::Type::scalar(lowir::TypeKind::F32);
	const SoftFloat source = SoftFloatOf(from);
	const SoftFloat target = SoftFloatOf(to);
	if (source == target)
		return value;
	if (target == SoftFloat::BFloat)
	{
		if (source == SoftFloat::Quad)
			return soft_call(SoftFloatHelper::TruncTFBF, value);
		if (source == SoftFloat::Half)
			return soft_call(SoftFloatHelper::TruncSFBF, soft_call(SoftFloatHelper::ExtendHFSF, value));
		if (sema::IsFloating(from))
		{
			const lowir::TypeKind kind = value.type.kind();
			return soft_call(kind == lowir::TypeKind::F32 ? SoftFloatHelper::TruncSFBF
					: kind == lowir::TypeKind::F64        ? SoftFloatHelper::TruncDFBF
														  : SoftFloatHelper::TruncXFBF,
				value);
		}
		const bool is_signed = SignedIntegral(from);
		return soft_call(SoftFloatHelper::TruncSFBF,
			convert_op(is_signed ? lowir::ConvertOp::SIToFP : lowir::ConvertOp::UIToFP, f32, value));
	}
	const RValue wide = bfloat_widened(value);
	if (target == SoftFloat::Half)
		return soft_call(SoftFloatHelper::TruncSFHF, wide);
	if (target == SoftFloat::Quad)
		return soft_call(SoftFloatHelper::ExtendSFTF, wide);
	const lowir::Type storage = lowering_.storage_type(to);
	if (storage.is_float())
		return storage.kind() == lowir::TypeKind::F32 ? wide : convert_op(lowir::ConvertOp::FPExt, storage, wide);
	return convert_op(SignedIntegral(to) ? lowir::ConvertOp::FPToSI : lowir::ConvertOp::FPToUI, storage, wide);
}

RValue FunctionLowering::bfloat_widened(const RValue& value)
{
	// A bfloat16 value is a float's upper half: its bits shifted to the top
	// of a float's, read through memory (exact; a NaN keeps its payload).
	const lowir::Type u16 = lowir::Type::scalar(lowir::TypeKind::U16);
	const lowir::Type i32 = lowir::Type::scalar(lowir::TypeKind::I32);
	Place half = Place();
	half.storage = lowir::Value::slot(generated_slot(lowir::SlotKind::SoftFloat, value.type));
	store(value.type, value, half);
	Place bits = Place();
	bits.storage = address_of(half);
	const RValue upper = binary(
		lowir::BinaryOp::Shl, i32, convert_op(lowir::ConvertOp::ZExt, i32, load(u16, bits)), integer(i32, 16));
	Place word = Place();
	word.storage = lowir::Value::slot(generated_slot(lowir::SlotKind::SoftFloat, i32));
	store(i32, upper, word);
	return load(lowir::Type::scalar(lowir::TypeKind::F32), word);
}

namespace {

const syntax::Expr* Unparenthesized(const syntax::Expr* expr)
{
	while (expr->kind == syntax::ExprKind::Paren)
		expr = static_cast<const syntax::ParenExpr*>(expr)->inner;
	return expr;
}

lowir::BinaryOp ArithmeticCode(TokenKind op)
{
	return op == TokenKind::OP_PLUS ? lowir::BinaryOp::Add
		: op == TokenKind::OP_MINUS ? lowir::BinaryOp::Sub
		: op == TokenKind::OP_STAR  ? lowir::BinaryOp::Mul
									: lowir::BinaryOp::Div;
}

bool ArithmeticToken(TokenKind op)
{
	return op == TokenKind::OP_PLUS || op == TokenKind::OP_MINUS || op == TokenKind::OP_STAR || op == TokenKind::OP_DIV;
}

} // namespace

bool FunctionLowering::bfloat_operation(const syntax::Expr& expr) const
{
	// The operators Clang evaluates in float for bfloat16 (its excess
	// precision on x86-64 without AVX10.2): built-in binary +, -, * and /
	// and unary + and - of bfloat16 type.
	const syntax::Expr* e = Unparenthesized(&expr);
	if (e->facts == nullptr || e->facts->type == nullptr || (e->facts->flags & sema::kOperatorCall) != 0 ||
		SoftFloatOf(e->facts->type) != SoftFloat::BFloat)
		return false;
	if (e->kind == syntax::ExprKind::Binary)
		return ArithmeticToken(static_cast<const syntax::BinaryExpr*>(e)->op);
	return e->kind == syntax::ExprKind::Unary &&
		(static_cast<const syntax::UnaryExpr*>(e)->op == TokenKind::OP_PLUS ||
			static_cast<const syntax::UnaryExpr*>(e)->op == TokenKind::OP_MINUS);
}

RValue FunctionLowering::bfloat_arithmetic(const syntax::Expr& expr)
{
	// Such an operator's value: its operation in float, rounded once.
	return soft_call(SoftFloatHelper::TruncSFBF, bfloat_promoted(expr));
}

RValue FunctionLowering::bfloat_promoted(const syntax::Expr& expr)
{
	// A bfloat16 operand in float: such an operator's result kept there,
	// its operands' too; another operand's value (a variable's, a call's, a
	// cast's, a conversion's from another type) rounded to bfloat16, then
	// widened. A left-nested chain of operators is walked iteratively.
	const lowir::Type f32 = lowir::Type::scalar(lowir::TypeKind::F32);
	if (!bfloat_operation(expr))
		return bfloat_widened(apply_conversion(expr, rvalue(expr), ConversionContext::Operand));
	const syntax::Expr* e = Unparenthesized(&expr);
	if (e->kind == syntax::ExprKind::Unary)
	{
		const syntax::UnaryExpr* negation = static_cast<const syntax::UnaryExpr*>(e);
		const RValue value = bfloat_promoted(*negation->operand);
		return negation->op == TokenKind::OP_MINUS ? unary(lowir::UnaryOp::Neg, value) : value;
	}
	std::vector<const syntax::BinaryExpr*> chain;
	while (e->kind == syntax::ExprKind::Binary && bfloat_operation(*e))
	{
		chain.push_back(static_cast<const syntax::BinaryExpr*>(e));
		e = Unparenthesized(chain.back()->left);
	}
	RValue value = bfloat_promoted(*chain.back()->left);
	for (size_t i = chain.size(); i-- > 0;)
		value = binary(ArithmeticCode(chain[i]->op), f32, value, bfloat_promoted(*chain[i]->right));
	return value;
}

RValue FunctionLowering::bfloat_compound(
	const syntax::BinaryExpr& expr, TokenKind op, const sema::Type* left_type, const RValue& old, const RValue& right)
{
	// Clang's E1 op= E2 in bfloat16 (5.17p7): E2 in float (bfloat_promoted),
	// E1's value converted to bfloat16 and widened, the operation in float,
	// and its result converted from float to E1's type (rounded once).
	const sema::Type* operation = expr.facts->target;
	const lowir::Type f32 = lowir::Type::scalar(lowir::TypeKind::F32);
	const RValue left = bfloat_widened(convert(old, left_type, operation, ConversionContext::CompoundAssignment));
	const sema::Type* single = lowering_.graph().types.fundamental(FundamentalType::Float);
	return fit(
		convert(binary(ArithmeticCode(op), f32, left, right), single, left_type, ConversionContext::CompoundAssignment),
		left_type);
}

RValue FunctionLowering::soft_arithmetic(lowir::BinaryOp op, SoftFloat kind, const RValue& left, const RValue& right)
{
	if (kind == SoftFloat::BFloat)
	{
		// One operation, rounded once (an expression of several:
		// bfloat_arithmetic).
		const RValue b = bfloat_widened(right);
		return soft_call(
			SoftFloatHelper::TruncSFBF, binary(op, lowir::Type::scalar(lowir::TypeKind::F32), bfloat_widened(left), b));
	}
	if (kind == SoftFloat::Quad)
	{
		static const SoftFloatHelper kHelpers[] = { SoftFloatHelper::AddTF, SoftFloatHelper::SubTF,
			SoftFloatHelper::MulTF, SoftFloatHelper::DivTF };
		return soft_call(kHelpers[static_cast<uint8_t>(op)], left, &right);
	}
	const lowir::Type f32 = lowir::Type::scalar(lowir::TypeKind::F32);
	const RValue a = soft_call(SoftFloatHelper::ExtendHFSF, left);
	const RValue b = soft_call(SoftFloatHelper::ExtendHFSF, right);
	return soft_call(SoftFloatHelper::TruncSFHF, binary(op, f32, a, b));
}

RValue FunctionLowering::soft_negate(SoftFloat kind, const RValue& value)
{
	if (kind == SoftFloat::Quad)
		return soft_call(SoftFloatHelper::NegTF, value);
	if (kind == SoftFloat::BFloat)
		return soft_call(SoftFloatHelper::TruncSFBF, unary(lowir::UnaryOp::Neg, bfloat_widened(value)));
	return soft_call(
		SoftFloatHelper::TruncSFHF, unary(lowir::UnaryOp::Neg, soft_call(SoftFloatHelper::ExtendHFSF, value)));
}

RValue FunctionLowering::soft_magnitude_at_least(
	SoftFloat kind, const RValue& x, uint64_t half_bits, uint64_t quad_high)
{
	// |x| >= v for the positive value v of those bits: x >= v or x <= -v
	// (an i32 truth; false for a NaN).
	const bool half = kind == SoftFloat::Half;
	const RValue positive = soft_constant(kind, half ? half_bits : 0, half ? 0 : quad_high);
	const RValue negative = soft_constant(
		kind, half ? half_bits | 0x8000 : 0, half ? 0 : quad_high | (uint64_t(1) << 63));
	// (Operands emitted last first: see FunctionLowering::binary.)
	const RValue below = truth_i32(soft_compare(lowir::ComparePredicate::Le, kind, x, negative));
	return binary(lowir::BinaryOp::Or, lowir::Type::scalar(lowir::TypeKind::I32),
		truth_i32(soft_compare(lowir::ComparePredicate::Ge, kind, x, positive)), below);
}

RValue FunctionLowering::soft_compare(
	lowir::ComparePredicate predicate, SoftFloat kind, const RValue& left, const RValue& right)
{
	// A binary16 comparison is exact in float. A binary128 one is its
	// routine's result against zero: each routine answers its own
	// predicate false for unordered operands.
	if (kind == SoftFloat::Half)
	{
		const lowir::Type f32 = lowir::Type::scalar(lowir::TypeKind::F32);
		const RValue wide_right = soft_call(SoftFloatHelper::ExtendHFSF, right);
		return compare(predicate, f32, soft_call(SoftFloatHelper::ExtendHFSF, left), wide_right);
	}
	if (kind == SoftFloat::BFloat)
	{
		const RValue wide_right = bfloat_widened(right);
		return compare(predicate, lowir::Type::scalar(lowir::TypeKind::F32), bfloat_widened(left), wide_right);
	}
	static const SoftFloatHelper kHelpers[] = { SoftFloatHelper::EqTF, SoftFloatHelper::NeTF, SoftFloatHelper::LtTF,
		SoftFloatHelper::LeTF, SoftFloatHelper::GtTF, SoftFloatHelper::GeTF };
	const uint8_t index = static_cast<uint8_t>(predicate);
	if (index >= sizeof(kHelpers) / sizeof(kHelpers[0]))
		throw LoweringError(0, "an unsigned comparison of binary128 values");
	const lowir::Type i64 = lowir::Type::scalar(lowir::TypeKind::I64);
	return compare(predicate, i64, soft_call(kHelpers[index], left, &right), integer(i64, 0));
}

} // namespace lower
