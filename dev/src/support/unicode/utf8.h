#pragma once

#include <cstddef>
#include <string>

// UTF-8 encoding and validating decoding (RFC 3629).
//
// The decoder accepts exactly the shortest-form encodings of Unicode scalar
// values: overlong forms, surrogate code points and values above U+10FFFF
// are rejected.

enum class Utf8DecodeStatus
{
	Ok,
	InvalidLeadingByte,
	InvalidContinuationByte,
	Truncated,
	InvalidScalarValue
};

struct Utf8Decoded
{
	Utf8DecodeStatus status;
	char32_t code_point;
	unsigned length;
};

// Decode one scalar value from the `available` bytes at `bytes`.
// `available` must be at least 1.
Utf8Decoded DecodeUtf8(const unsigned char* bytes, std::size_t available);

const char* DescribeUtf8DecodeStatus(Utf8DecodeStatus status);

inline unsigned Utf8EncodedLength(char32_t code_point)
{
	if (code_point < 0x80)
		return 1;
	if (code_point < 0x800)
		return 2;
	if (code_point < 0x10000)
		return 3;
	return 4;
}

void AppendUtf8(std::string& out, char32_t code_point);
