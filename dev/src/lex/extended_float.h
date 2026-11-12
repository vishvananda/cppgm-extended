#pragma once

#ifdef __wasi__
#include <cstdint>
#include <cmath>
#include <limits>
// x87 target values represented exactly by the WASI host's binary128 type.
// These helpers do not change the native host's floating-point path.
long double LoadExtended80(const unsigned char* bytes);
void StoreExtended80(long double value, unsigned char* bytes);
long double RoundExtended80(long double value);
long double IntegerExtended80(uint64_t low, uint64_t high, bool negative);
long double CalculateExtended80(char operation, long double left, long double right);
// SSE chooses the first NaN operand and the negative indefinite value for
// invalid arithmetic. Wasm permits either operand's payload, so select it
// explicitly when evaluating an x86 target constant on a WASI host.
template<typename T>
long double CalculateIeeeFloat(char operation, T left, T right)
{
	if (std::isnan(left))
		return left + T(0);
	if (std::isnan(right))
		return right + T(0);
	T result = operation == '+' ? left + right
		: operation == '-'      ? left - right
		: operation == '*'      ? left * right
								: left / right;
	return std::isnan(result) ? -std::numeric_limits<T>::quiet_NaN() : result;
}
#endif
