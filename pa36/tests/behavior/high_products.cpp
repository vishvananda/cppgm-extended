// The high half of a 64-bit product, `(u64)(((u128)a * b) >> 64)` and its
// signed form, is one mul or imul: unsigned and signed operands, a square,
// globals, literal factors, an extension two products share, both halves of
// one product, and a product in a loop.
#include <cstdio>

typedef unsigned long u64;
typedef long i64;
typedef unsigned __int128 u128;
typedef __int128 i128;

u64 g1 = 0xfedcba9876543210ull, g2 = 0x123456789abcdef1ull;

__attribute__((noinline)) u64 unsigned_high(u64 a, u64 b) { return (u64)(((u128)a * b) >> 64); }
__attribute__((noinline)) i64 signed_high(i64 a, i64 b) { return (i64)(((i128)a * b) >> 64); }
__attribute__((noinline)) u64 square_high(u64 a) { return (u64)(((u128)a * a) >> 64); }
__attribute__((noinline)) u64 global_high() { return (u64)(((u128)g1 * g2) >> 64); }
__attribute__((noinline)) u64 literal_high(u64 a) { return (u64)(((u128)a * 12379400392853802749ull) >> 64); }
__attribute__((noinline)) i64 negative_literal_high(i64 a) { return (i64)(((i128)a * -7) >> 64); }

__attribute__((noinline)) u64 chained(u64 a, u64 b, u64 c)
{
	const u64 h = (u64)(((u128)a * b) >> 64);
	const u64 k = (u64)(((u128)h * c) >> 64);
	return h ^ k ^ (a * b);
}

__attribute__((noinline)) u64 both_halves(u64 a, u64 b)
{
	const u128 p = (u128)a * b;
	return (u64)(p >> 64) + (u64)p + (u64)(p >> 32);
}

__attribute__((noinline)) u64 shared(u64 a, u64 b, u64 c, int f)
{
	const u128 x = a;
	u64 h = (u64)((x * b) >> 64);
	if (f)
		h += (u64)((x * c) >> 64);
	return h + (u64)x;
}

__attribute__((noinline)) u64 looped(const u64* p, int n, u64 m)
{
	u64 s = 0;
	for (int i = 0; i < n; ++i)
		s += (u64)(((u128)p[i] * m) >> 64) + i;
	return s;
}

int main()
{
	const u64 v[] = { 0, 1, 2, 3, ~0ull, 0x8000000000000000ull, 0x7fffffffffffffffull, 12345678901234567ull,
		0xdeadbeefcafebabeull };
	u64 h = 0;
	for (u64 a : v)
	{
		h = h * 31 + square_high(a);
		h = h * 31 + literal_high(a);
		h = h * 31 + (u64)negative_literal_high((i64)a);
		for (u64 b : v)
		{
			h = h * 31 + unsigned_high(a, b);
			h = h * 31 + (u64)signed_high((i64)a, (i64)b);
			h = h * 31 + chained(a, b, a ^ b);
			h = h * 31 + both_halves(a, b);
			h = h * 31 + shared(a, b, ~b, (int)(b & 1));
		}
	}
	h = h * 31 + global_high();
	h = h * 31 + looped(v, 9, 0x9e3779b97f4a7c15ull);
	std::printf("%016lx\n", h);
	return 0;
}
