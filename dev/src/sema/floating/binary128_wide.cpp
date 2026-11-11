#include "sema/floating/binary128_internal.h"

namespace sema {
namespace binary128 {

namespace {

struct Product
{
	uint64_t words[4];
};

uint64_t shifted_word(const Product& value, unsigned word, unsigned shift)
{
	uint64_t result = word < 4 ? value.words[word] >> shift : 0;
	if (shift != 0 && word + 1 < 4)
		result |= value.words[word + 1] << (64 - shift);
	return result;
}

Wide shortened(const Product& value, unsigned& shift)
{
	unsigned bits = 0;
	for (unsigned i = 0; i < 4; ++i)
		if (value.words[i] != 0)
			bits = 64 * i + 64 - __builtin_clzll(value.words[i]);
	shift = bits > 126 ? bits - 126 : 0;
	const unsigned word = shift / 64, part = shift % 64;
	Wide result = Wide(shifted_word(value, word + 1, part)) << 64 | shifted_word(value, word, part);
	bool sticky = false;
	for (unsigned i = 0; i < word; ++i)
		sticky = sticky || value.words[i] != 0;
	if (part != 0)
		sticky = sticky || (value.words[word] & ((uint64_t(1) << part) - 1)) != 0;
	return result | (sticky ? 1 : 0);
}

void normalize(Decoded& value)
{
	const unsigned shift = 113 - length(value.magnitude);
	value.magnitude <<= shift;
	value.exponent -= shift;
}

} // namespace

Decoded product(const Decoded& left, const Decoded& right)
{
	const uint64_t a[2]{ static_cast<uint64_t>(left.magnitude), static_cast<uint64_t>(left.magnitude >> 64) };
	const uint64_t b[2]{ static_cast<uint64_t>(right.magnitude), static_cast<uint64_t>(right.magnitude >> 64) };
	Product value = {};
	for (unsigned i = 0; i < 2; ++i)
	{
		Wide carry = 0;
		for (unsigned j = 0; j < 2; ++j)
		{
			const Wide next = Wide(a[i]) * b[j] + value.words[i + j] + carry;
			value.words[i + j] = static_cast<uint64_t>(next);
			carry = next >> 64;
		}
		value.words[i + 2] = static_cast<uint64_t>(carry);
	}
	unsigned shift = 0;
	const Wide magnitude = shortened(value, shift);
	return Decoded{ left.negative != right.negative, magnitude, left.exponent + right.exponent + shift };
}

Decoded quotient(Decoded left, Decoded right)
{
	normalize(left);
	normalize(right);
	Wide remainder = left.magnitude;
	Wide value = remainder >= right.magnitude ? 1 : 0;
	if (value != 0)
		remainder -= right.magnitude;
	for (unsigned i = 0; i < 116; ++i)
	{
		value <<= 1;
		remainder <<= 1;
		if (remainder >= right.magnitude)
		{
			remainder -= right.magnitude;
			value |= 1;
		}
	}
	if (remainder != 0)
		value |= 1;
	return Decoded{ left.negative != right.negative, value, left.exponent - right.exponent - 116 };
}

} // namespace binary128
} // namespace sema
