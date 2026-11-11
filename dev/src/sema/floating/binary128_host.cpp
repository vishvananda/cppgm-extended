#include "sema/floating/binary128_internal.h"

#include <cmath>
#include <cstring>
#include <limits>

namespace sema {
namespace binary128 {

namespace {

Wide host_payload(long double value)
{
	const unsigned precision = std::numeric_limits<long double>::digits;
	uint64_t low = 0, high = 0;
	unsigned char bytes[16] = {};
	std::memcpy(bytes, &value, sizeof value);
	std::memcpy(&low, bytes, 8);
	if (precision == 113)
	{
		std::memcpy(&high, bytes + 8, 8);
		return (Wide(high & 0xffffffffffffull) << 64) | low;
	}
	if (precision == 64)
		return Wide(low & 0x7fffffffffffffffull) << 49;
	if (precision == 53)
		return Wide(low & 0xfffffffffffffull) << 60;
	return Wide(1) << 111;
}

long double host_nan(Bits value, unsigned precision)
{
	const unsigned host = std::numeric_limits<long double>::digits;
	Wide payload = ((Wide(value.high & 0xffffffffffffull) << 64) | value.low) >> (113 - precision);
	payload |= Wide(1) << (precision - 2);
	payload <<= host - precision;
	unsigned char bytes[16] = {};
	uint64_t low = static_cast<uint64_t>(payload), high = 0;
	if (host == 113)
		high = static_cast<uint64_t>(payload >> 64) | (uint64_t(0x7fff) << 48) | (uint64_t(negative(value)) << 63);
	else if (host == 64)
	{
		low |= uint64_t(1) << 63;
		bytes[8] = 0xff;
		bytes[9] = negative(value) ? 0xff : 0x7f;
	}
	else
		low |= (uint64_t(0x7ff) << 52) | (uint64_t(negative(value)) << 63);
	std::memcpy(bytes, &low, 8);
	if (host == 113)
		std::memcpy(bytes + 8, &high, 8);
	long double result = 0;
	std::memcpy(&result, bytes, sizeof result);
	return result;
}

} // namespace

bool from_host(long double value, Bits& result)
{
	const bool sign = std::signbit(value);
	if (std::isnan(value) || std::isinf(value))
	{
		const Wide payload = std::isnan(value) ? host_payload(value) | (Wide(1) << 111) : 0;
		result = Bits{ static_cast<uint64_t>(payload),
			static_cast<uint64_t>(payload >> 64) | (uint64_t(sign) << 63) | (uint64_t(0x7fff) << 48) };
		return true;
	}
	if (value == 0)
	{
		result = Bits{ 0, uint64_t(sign) << 63 };
		return true;
	}
	int exponent = 0;
	const long double fraction = std::frexp(std::fabs(value), &exponent);
	const long double upper = std::ldexp(fraction, 64);
	const uint64_t high = static_cast<uint64_t>(upper);
	const uint64_t low = static_cast<uint64_t>(std::ldexp(upper - static_cast<long double>(high), 64));
	return rounded(Decoded{ sign, (Wide(high) << 64) | low, exponent - 128 }, result);
}

bool to_host(Bits value, unsigned precision, int maximum_exponent, long double& result)
{
	if (precision < 2 || precision > std::numeric_limits<long double>::digits)
		return false;
	if (!finite(value))
	{
		result = nan(value)   ? host_nan(value, precision)
			: negative(value) ? -std::numeric_limits<long double>::infinity()
							  : std::numeric_limits<long double>::infinity();
		return true;
	}
	Decoded decoded = decode(value);
	if (decoded.magnitude == 0)
	{
		result = decoded.negative ? -0.0L : 0.0L;
		return true;
	}
	const int64_t top = decoded.exponent + length(decoded.magnitude) - 1;
	const int64_t normal = top > 1 - maximum_exponent ? top : 1 - maximum_exponent;
	const int64_t quantum = normal - (precision - 1);
	if (quantum > decoded.exponent)
		decoded.magnitude = rounded_shift(decoded.magnitude, static_cast<unsigned>(quantum - decoded.exponent));
	else
		decoded.magnitude <<= decoded.exponent - quantum;
	if (decoded.magnitude != 0 && quantum + length(decoded.magnitude) - 1 > maximum_exponent)
		return false;
	const long double magnitude = std::ldexp(
									  static_cast<long double>(static_cast<uint64_t>(decoded.magnitude >> 64)), 64) +
		static_cast<long double>(static_cast<uint64_t>(decoded.magnitude));
	result = std::ldexp(magnitude, static_cast<int>(quantum));
	if (decoded.negative)
		result = -result;
	return true;
}

} // namespace binary128
} // namespace sema
