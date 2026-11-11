#include "sema/floating/binary128_internal.h"

#include "lex/interchange_float.h"

namespace sema {
namespace binary128 {

namespace {

const uint64_t kSign = uint64_t(1) << 63;
const uint64_t kExponent = uint64_t(0x7fff) << 48;
const uint64_t kFraction = (uint64_t(1) << 48) - 1;

Decoded sum(Decoded left, Decoded right)
{
	if (left.exponent < right.exponent)
	{
		const Decoded temporary = left;
		left = right;
		right = temporary;
	}
	left.magnitude <<= 3;
	right.magnitude = shift_jam(right.magnitude << 3, static_cast<unsigned>(left.exponent - right.exponent));
	left.exponent -= 3;
	if (left.negative == right.negative)
		left.magnitude += right.magnitude;
	else if (left.magnitude >= right.magnitude)
		left.magnitude -= right.magnitude;
	else
	{
		left.magnitude = right.magnitude - left.magnitude;
		left.negative = right.negative;
	}
	if (left.magnitude == 0)
		left.negative = left.negative && right.negative;
	return left;
}

} // namespace

bool zero(Bits value)
{
	return value.low == 0 && (value.high & ~kSign) == 0;
}
bool finite(Bits value)
{
	return (value.high & kExponent) != kExponent;
}
bool nan(Bits value)
{
	return !finite(value) && ((value.high & kFraction) != 0 || value.low != 0);
}
bool normal(Bits value)
{
	return finite(value) && (value.high & kExponent) != 0;
}
bool negative(Bits value)
{
	return (value.high & kSign) != 0;
}
Bits negate(Bits value)
{
	value.high ^= kSign;
	return value;
}

Bits infinity()
{
	return Bits{ 0, uint64_t(0x7fff) << 48 };
}
Bits quiet_nan(uint64_t payload)
{
	return Bits{ payload, (uint64_t(0x7fff) << 48) | (uint64_t(1) << 47) };
}
Bits signaling_nan(uint64_t payload)
{
	return Bits{ payload, (uint64_t(0x7fff) << 48) | (payload == 0 ? uint64_t(1) << 46 : 0) };
}

unsigned length(Wide value)
{
	const uint64_t high = static_cast<uint64_t>(value >> 64);
	return high != 0 ? 128 - __builtin_clzll(high)
		: value != 0 ? 64 - __builtin_clzll(static_cast<uint64_t>(value))
					 : 0;
}

Decoded decode(Bits value)
{
	const uint32_t exponent = static_cast<uint32_t>((value.high >> 48) & 0x7fff);
	Wide magnitude = (Wide(value.high & kFraction) << 64) | value.low;
	if (exponent != 0)
		magnitude |= Wide(1) << 112;
	return Decoded{ negative(value), magnitude, (exponent == 0 ? 1 : int64_t(exponent)) - 16383 - 112 };
}

bool rounded(const Decoded& value, Bits& result)
{
	return RoundInterchangeValue(value.negative, static_cast<uint64_t>(value.magnitude),
		static_cast<uint64_t>(value.magnitude >> 64), value.exponent, kBinary128, result.low, result.high);
}

Wide shift_jam(Wide value, unsigned count)
{
	if (count == 0)
		return value;
	if (count >= 128)
		return value != 0 ? 1 : 0;
	return (value >> count) | ((value & ((Wide(1) << count) - 1)) != 0 ? 1 : 0);
}

Wide rounded_shift(Wide value, unsigned count)
{
	if (count == 0)
		return value;
	if (count > 128)
		return 0;
	const Wide kept = count == 128 ? 0 : value >> count;
	const Wide remainder = count == 128 ? value : value & ((Wide(1) << count) - 1);
	const Wide half = Wide(1) << (count - 1);
	return kept + (remainder > half || (remainder == half && (kept & 1) != 0) ? 1 : 0);
}

int compare(Bits left, Bits right)
{
	if (nan(left) || nan(right))
		return 2;
	if (zero(left) && zero(right))
		return 0;
	if (negative(left) != negative(right))
		return negative(left) ? -1 : 1;
	const int magnitude = left.high != right.high ? (left.high < right.high ? -1 : 1)
		: left.low == right.low                   ? 0
		: left.low < right.low                    ? -1
												  : 1;
	return negative(left) ? -magnitude : magnitude;
}

bool calculate(Operation operation, Bits left, Bits right, Bits& result)
{
	if (!finite(left) || !finite(right))
		return false;
	if (operation == Operation::Subtract)
		right = negate(right);
	const Decoded a = decode(left), b = decode(right);
	if (operation == Operation::Add || operation == Operation::Subtract)
		return rounded(sum(a, b), result);
	if (operation == Operation::Multiply)
		return rounded(product(a, b), result);
	if (zero(right))
		return false;
	if (zero(left))
		return rounded(Decoded{ a.negative != b.negative, 0, 0 }, result);
	return rounded(quotient(a, b), result);
}

bool from_integer(bool sign, Bits magnitude, Bits& result)
{
	return rounded(Decoded{ sign, (Wide(magnitude.high) << 64) | magnitude.low, 0 }, result);
}

bool to_integer(Bits value, unsigned width, bool is_signed, Bits& result)
{
	if (!finite(value) || width == 0 || width > 128)
		return false;
	const Decoded decoded = decode(value);
	Wide magnitude = decoded.magnitude;
	if (decoded.exponent < 0)
		magnitude = decoded.exponent <= -128 ? 0 : magnitude >> -decoded.exponent;
	else
	{
		if (decoded.exponent + length(magnitude) > 128)
			return false;
		magnitude <<= decoded.exponent;
	}
	const Wide maximum = is_signed ? (Wide(1) << (width - 1)) - (decoded.negative ? 0 : 1)
		: width == 128             ? ~Wide(0)
								   : (Wide(1) << width) - 1;
	if (magnitude > maximum || (!is_signed && decoded.negative && magnitude != 0))
		return false;
	if (decoded.negative)
		magnitude = ~magnitude + 1;
	result = Bits{ static_cast<uint64_t>(magnitude), static_cast<uint64_t>(magnitude >> 64) };
	return true;
}

} // namespace binary128
} // namespace sema
