#include "lex/interchange_float.h"

#include <cstddef>
#include <vector>

#include "preprocess/lexer/char_classes.h"

const InterchangeFormat kBinary16 = { 11, 15 };
const InterchangeFormat kBinary128 = { 113, 16383 };
const InterchangeFormat kBFloat16 = { 8, 127 };

namespace {

// A nonnegative integer in 32-bit words, least significant first, with no
// leading zero word (zero is empty). A literal's kept digits and scale are
// bounded (ScanInterchangeValue), so its values are too.
typedef std::vector<uint32_t> Big;

void Trim(Big& a)
{
	while (!a.empty() && a.back() == 0)
		a.pop_back();
}

void MultiplyAdd(Big& a, uint32_t factor, uint32_t addend)
{
	uint64_t carry = addend;
	for (uint32_t& word : a)
	{
		const uint64_t product = static_cast<uint64_t>(word) * factor + carry;
		word = static_cast<uint32_t>(product);
		carry = product >> 32;
	}
	if (carry != 0)
		a.push_back(static_cast<uint32_t>(carry));
}

void MultiplyPowerOfTen(Big& a, int64_t count)
{
	for (; count >= 9; count -= 9)
		MultiplyAdd(a, 1000000000u, 0);
	uint32_t factor = 1;
	while (count-- > 0)
		factor *= 10;
	MultiplyAdd(a, factor, 0);
}

void ShiftLeft(Big& a, int64_t bits)
{
	if (a.empty() || bits == 0)
		return;
	const unsigned rest = static_cast<unsigned>(bits % 32);
	if (rest != 0)
	{
		uint32_t carry = 0;
		for (uint32_t& word : a)
		{
			const uint32_t next = word >> (32 - rest);
			word = (word << rest) | carry;
			carry = next;
		}
		if (carry != 0)
			a.push_back(carry);
	}
	a.insert(a.begin(), static_cast<size_t>(bits / 32), 0u);
}

void ShiftRightOne(Big& a)
{
	for (size_t i = 0; i < a.size(); ++i)
		a[i] = (a[i] >> 1) | (i + 1 < a.size() ? a[i + 1] << 31 : 0u);
	Trim(a);
}

int64_t BitLength(const Big& a)
{
	if (a.empty())
		return 0;
	int64_t bits = 32 * static_cast<int64_t>(a.size() - 1);
	for (uint32_t top = a.back(); top != 0; top >>= 1)
		++bits;
	return bits;
}

int Compare(const Big& a, const Big& b)
{
	if (a.size() != b.size())
		return a.size() < b.size() ? -1 : 1;
	for (size_t i = a.size(); i-- > 0;)
	{
		if (a[i] != b[i])
			return a[i] < b[i] ? -1 : 1;
	}
	return 0;
}

// a -= b, with a >= b.
void Subtract(Big& a, const Big& b)
{
	uint32_t borrow = 0;
	for (size_t i = 0; i < a.size(); ++i)
	{
		const uint64_t subtrahend = static_cast<uint64_t>(i < b.size() ? b[i] : 0u) + borrow;
		borrow = a[i] < subtrahend ? 1u : 0u;
		a[i] = static_cast<uint32_t>(
			(static_cast<uint64_t>(a[i]) + (static_cast<uint64_t>(borrow) << 32)) - subtrahend);
	}
	Trim(a);
}

// A 128-bit significand or encoding.
struct Wide
{
	uint64_t low;
	uint64_t high;

	void set(unsigned bit)
	{
		if (bit < 64)
			low |= uint64_t(1) << bit;
		else
			high |= uint64_t(1) << (bit - 64);
	}
	bool test(unsigned bit) const { return bit < 64 ? ((low >> bit) & 1) != 0 : ((high >> (bit - 64)) & 1) != 0; }
};

// The format's bits of the value num / den * 2^shift (positive), rounded
// to nearest, ties to even; false if it overflows to infinity.
bool RoundToFormat(
	const Big& num, const Big& den, int64_t shift, const InterchangeFormat& format, uint64_t& low, uint64_t& high)
{
	const int64_t p = format.precision;
	const int64_t emax = format.max_exponent;
	const int64_t emin = 1 - emax;

	// e = floor(log2(value)): 2^(t-1) < num/den < 2^(t+1).
	const int64_t t = BitLength(num) - BitLength(den);
	bool at_least = false;
	if (t >= 0)
	{
		Big scaled = den;
		ShiftLeft(scaled, t);
		at_least = Compare(num, scaled) >= 0;
	}
	else
	{
		Big scaled = num;
		ShiftLeft(scaled, -t);
		at_least = Compare(scaled, den) >= 0;
	}
	const int64_t e = (at_least ? t : t - 1) + shift;

	// The significand M = value / 2^q with q the quantum of the value's
	// binade (of the subnormal range below emin), M < 2^p, rounded to
	// nearest, ties to even, by the remainder of the division.
	int64_t q = (e > emin ? e : emin) - (p - 1);
	Big n = num;
	Big d = den;
	if (shift - q >= 0)
		ShiftLeft(n, shift - q);
	else
		ShiftLeft(d, q - shift);
	const Big divisor = d;
	ShiftLeft(d, p);
	Wide m = { 0, 0 };
	for (int64_t bit = p; bit >= 0; --bit)
	{
		if (Compare(n, d) >= 0)
		{
			Subtract(n, d);
			m.set(static_cast<unsigned>(bit));
		}
		ShiftRightOne(d);
	}
	ShiftLeft(n, 1);
	const int half = Compare(n, divisor);
	if (half > 0 || (half == 0 && (m.low & 1) != 0))
	{
		if (++m.low == 0)
			++m.high;
	}
	if (m.test(static_cast<unsigned>(p)))
	{
		m.low = (m.low >> 1) | (m.high << 63);
		m.high >>= 1;
		++q;
	}
	const bool normal = m.test(static_cast<unsigned>(p - 1));
	const int64_t biased = normal ? q + (p - 1) + emax : 0;
	if (biased >= 2 * emax + 1)
		return false;

	// sign 0, the biased exponent, then the fraction (M without its
	// hidden bit).
	const unsigned fraction = static_cast<unsigned>(p - 1);
	if (fraction < 64)
	{
		low = (m.low & ((uint64_t(1) << fraction) - 1)) | (static_cast<uint64_t>(biased) << fraction);
		high = 0;
	}
	else
	{
		low = m.low;
		high = (m.high & ((uint64_t(1) << (fraction - 64)) - 1)) | (static_cast<uint64_t>(biased) << (fraction - 64));
	}
	return true;
}

// Sets the format's sign bit.
void Negate(const InterchangeFormat& format, uint64_t& low, uint64_t& high)
{
	const unsigned sign = format.precision == 113 ? 127 : 15;
	if (sign < 64)
		low |= uint64_t(1) << sign;
	else
		high |= uint64_t(1) << (sign - 64);
}

} // namespace

bool RoundInterchangeValue(bool negative, uint64_t magnitude_low, uint64_t magnitude_high, int64_t exponent,
	const InterchangeFormat& format, uint64_t& low, uint64_t& high)
{
	low = high = 0;
	Big num;
	num.push_back(static_cast<uint32_t>(magnitude_low));
	num.push_back(static_cast<uint32_t>(magnitude_low >> 32));
	num.push_back(static_cast<uint32_t>(magnitude_high));
	num.push_back(static_cast<uint32_t>(magnitude_high >> 32));
	Trim(num);
	if (!num.empty())
	{
		// Beyond the largest finite value, or below half the least
		// subnormal (zero), without dividing.
		const int64_t top = BitLength(num) + exponent;
		if (top - 1 >= format.max_exponent + 1)
			return false;
		if (top >= (1 - format.max_exponent) - static_cast<int64_t>(format.precision) &&
			!RoundToFormat(num, Big(1, 1u), exponent, format, low, high))
			return false;
	}
	if (negative)
		Negate(format, low, high);
	return true;
}

bool ScanInterchangeValue(
	const char* text, uint32_t length, const InterchangeFormat& format, uint64_t& low, uint64_t& high)
{
	// The value is digits * base^exponent (base 10, or 2 for a hexadecimal
	// spelling, each of whose fraction digits scales by 2^-4).
	low = high = 0;
	const bool hex = length >= 2 && text[0] == '0' && (text[1] == 'x' || text[1] == 'X');
	const int64_t p = format.precision;
	const int64_t emax = format.max_exponent;
	const int64_t emin = 1 - emax;
	// Significant digits past the most a value halfway between two of the
	// format's values has (the significand's decimal digits and the least
	// subnormal's fraction digits: 11566 for binary128, 23 for binary16;
	// p + 1 bits for a hexadecimal spelling) decide the rounding only by
	// being nonzero: they are dropped, a nonzero one leaving a digit 1 below
	// the kept ones, so the scan stays linear in the spelling's length.
	const int64_t kept = hex ? (p + 3) / 4 + 2 : (p * 30103 + (emax - 1 + p) * 69897) / 100000 + 3;
	const uint32_t base = hex ? 16 : 10;
	const int64_t step = hex ? 4 : 1;
	// Digits are accumulated a word at a time (9 decimal, 7 hexadecimal).
	const uint32_t chunk_limit = hex ? 7 : 9;
	uint32_t pos = hex ? 2 : 0;
	Big digits;
	int64_t exponent = 0;
	int64_t significant = 0;
	bool point = false;
	bool sticky = false;
	uint32_t chunk = 0;
	uint32_t chunk_digits = 0;
	uint32_t chunk_scale = 1;
	for (; pos < length; ++pos)
	{
		const char c = text[pos];
		if (c == '.')
		{
			point = true;
			continue;
		}
		uint32_t value = 0;
		if (IsAsciiDigit(c))
			value = static_cast<uint32_t>(c - '0');
		else if (hex && IsHexDigit(c))
			value = static_cast<uint32_t>((c | 0x20) - 'a' + 10);
		else
			break;
		if (significant >= kept)
		{
			// Dropped: an integer digit still scales the kept ones.
			sticky = sticky || value != 0;
			if (!point)
				exponent += step;
			continue;
		}
		if (point)
			exponent -= step;
		if (significant == 0 && value == 0)
			continue;
		chunk = chunk * base + value;
		chunk_scale *= base;
		++significant;
		if (++chunk_digits == chunk_limit)
		{
			MultiplyAdd(digits, chunk_scale, chunk);
			chunk = chunk_digits = 0;
			chunk_scale = 1;
		}
	}
	if (chunk_digits != 0)
		MultiplyAdd(digits, chunk_scale, chunk);
	if (sticky)
	{
		MultiplyAdd(digits, base, 1);
		exponent -= step;
		++significant;
	}
	if (pos < length && (text[pos] == 'e' || text[pos] == 'E' || text[pos] == 'p' || text[pos] == 'P'))
	{
		++pos;
		bool negative = false;
		if (pos < length && (text[pos] == '+' || text[pos] == '-'))
			negative = text[pos++] == '-';
		int64_t written = 0;
		for (; pos < length && IsAsciiDigit(text[pos]); ++pos)
		{
			if (written < 1000000000)
				written = written * 10 + (text[pos] - '0');
		}
		exponent += negative ? -written : written;
	}
	if (digits.empty())
		return true;

	// Values certainly beyond the largest finite one overflow; values below
	// half the least subnormal round to zero. Bounds use log2(10) between
	// 3.3219 and 3.3220; a decimal value lies in [10^(m-1), 10^m).
	if (hex)
	{
		const int64_t top = BitLength(digits) + exponent;
		if (top - 1 >= emax + 1)
			return false;
		if (top < emin - p)
			return true;
	}
	else
	{
		const int64_t m = significant + exponent;
		if ((m - 1) * 33219 >= (emax + 1) * 10000)
			return false;
		if (m * 33220 < (emin - p) * 10000)
			return true;
	}

	// value = num / den * 2^shift.
	Big num = digits;
	Big den(1, 1u);
	int64_t shift = 0;
	if (hex)
		shift = exponent;
	else if (exponent >= 0)
		MultiplyPowerOfTen(num, exponent);
	else
		MultiplyPowerOfTen(den, -exponent);

	return RoundToFormat(num, den, shift, format, low, high);
}
