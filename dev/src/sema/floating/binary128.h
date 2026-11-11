#pragma once

#include <cstdint>

namespace sema {
namespace binary128 {

struct Bits
{
	std::uint64_t low;
	std::uint64_t high;
};

enum class Operation : std::uint8_t
{
	Add,
	Subtract,
	Multiply,
	Divide
};

bool zero(Bits value);
bool finite(Bits value);
bool nan(Bits value);
bool normal(Bits value);
bool negative(Bits value);
Bits negate(Bits value);
Bits infinity();
Bits quiet_nan(std::uint64_t payload);
Bits signaling_nan(std::uint64_t payload);
// -1, 0, 1, or 2 for unordered.
int compare(Bits left, Bits right);
// Constant operations reject nonfinite results and zero divisors.
bool calculate(Operation operation, Bits left, Bits right, Bits& result);
bool from_integer(bool negative, Bits magnitude, Bits& result);
bool to_integer(Bits value, unsigned width, bool is_signed, Bits& result);
bool from_host(long double value, Bits& result);
bool to_host(Bits value, unsigned precision, int maximum_exponent, long double& result);

} // namespace binary128
} // namespace sema
