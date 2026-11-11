#pragma once

#include "sema/floating/binary128.h"

namespace sema {
namespace binary128 {

using Wide = unsigned __int128;

struct Decoded
{
	bool negative;
	Wide magnitude;
	int64_t exponent;
};

unsigned length(Wide value);
Decoded decode(Bits value);
bool rounded(const Decoded& value, Bits& result);
Wide shift_jam(Wide value, unsigned count);
Wide rounded_shift(Wide value, unsigned count);
Decoded product(const Decoded& left, const Decoded& right);
Decoded quotient(Decoded left, Decoded right);

} // namespace binary128
} // namespace sema
