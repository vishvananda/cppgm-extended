#pragma once

#include <cstdint>

// The IEEE 754 binary interchange formats no host type has: binary16
// (_Float16) and binary128 (_Float128, __float128); and bfloat16 (__bf16),
// float's exponent with an 8-bit significand.
struct InterchangeFormat
{
	unsigned precision; // significand bits, the hidden one included
	int max_exponent;   // emax; the bias is emax and emin is 1 - emax
};

extern const InterchangeFormat kBinary16;
extern const InterchangeFormat kBinary128;
extern const InterchangeFormat kBFloat16;

// Scans the value of a floating literal from `text`, its decimal or
// hexadecimal spelling without the floating-suffix, rounded once to the
// nearest value of `format` (ties to even) as that format's bits: the low
// 64 in `low`, the others in `high`. Returns false if the value is beyond
// the format's range ([lex.fcon] 2.14.4p1).
bool ScanInterchangeValue(
	const char* text, uint32_t length, const InterchangeFormat& format, uint64_t& low, uint64_t& high);

// The bits of the value (-1)^negative * magnitude * 2^exponent (the
// magnitude 128 bits, low word first), rounded as ScanInterchangeValue
// rounds; false if it overflows to infinity.
bool RoundInterchangeValue(bool negative, uint64_t magnitude_low, uint64_t magnitude_high, int64_t exponent,
	const InterchangeFormat& format, uint64_t& low, uint64_t& high);
