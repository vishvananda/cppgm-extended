// Without a frame pointer (host O1+) a function pushes its callee-saved
// registers first, right below the return address, and reserves its frame
// below them: deep recursion with frame objects and many saved registers,
// an exception unwinding through such frames, and leaf functions with an
// odd and an even number of saves and an aligned frame object.
#include <cstdio>
#include <stdexcept>

__attribute__((noinline)) int ack(int m, int n)
{
	return m ? ack(m - 1, n ? ack(m, n - 1) : 1) : n + 1;
}

__attribute__((noinline)) void touch(volatile long* p, int n)
{
	for (int i = 0; i < n; ++i)
		p[i] += i;
}

// A frame object live across a recursive call, and values that need saved
// registers on both sides of it.
__attribute__((noinline)) long walk(long depth, long a, long b, long c)
{
	volatile long local[5] = { depth, a, b, c, depth ^ a };
	touch(local, 5);
	if (depth == 0)
		return local[0] + local[1] + local[2] + local[3] + local[4];
	const long x = a * 3 + depth;
	const long y = b ^ (c << 1);
	const long z = c + depth * 7;
	const long below = walk(depth - 1, x, y, z);
	return below + local[1] - local[4] + x + y + z;
}

__attribute__((noinline)) long thrower(long depth, long a, long b)
{
	volatile long local[3] = { depth, a, b };
	touch(local, 3);
	if (depth == 0)
		throw std::runtime_error("bottom");
	const long x = a + b * depth;
	const long y = b - a;
	const long r = thrower(depth - 1, x, y);
	return r + x * y + local[2];
}

struct alignas(16) Pair
{
	double a, b;
};

// Leaves: an aligned frame object whose address escapes into a volatile
// read, with three and with four saved registers.
__attribute__((noinline)) double leaf_odd(const double* v, long n, long s, long t)
{
	Pair p = { 0, 0 };
	Pair* volatile q = &p;
	long i = 0, j = n, k = s, l = t;
	for (; i < n; ++i, --j, k += 3, l ^= k)
	{
		q->a += v[i] * static_cast<double>(j);
		q->b += static_cast<double>(k ^ l);
	}
	return q->a + q->b + static_cast<double>(reinterpret_cast<unsigned long>(q) % 16);
}

__attribute__((noinline)) double leaf_even(const double* v, long n, long s, long t, long u)
{
	Pair p = { 1, 2 };
	Pair* volatile q = &p;
	long i = 0, j = n, k = s, l = t, m = u;
	for (; i < n; ++i, --j, k += 3, l ^= k, m += l)
	{
		q->a += v[i] * static_cast<double>(j) + static_cast<double>(m & 7);
		q->b += static_cast<double>(k ^ l);
	}
	return q->a + q->b + static_cast<double>(reinterpret_cast<unsigned long>(q) % 16);
}

int main()
{
	std::printf("ack %d %d\n", ack(2, 9), ack(3, 6));
	std::printf("walk %ld %ld\n", walk(40, 1, 2, 3), walk(3000, 5, 6, 7));
	try
	{
		std::printf("thrower %ld\n", thrower(500, 1, 2));
	}
	catch (const std::exception& e)
	{
		std::printf("caught %s\n", e.what());
	}
	double v[64];
	for (int i = 0; i < 64; ++i)
		v[i] = i * 0.5;
	std::printf("leaves %.1f %.1f\n", leaf_odd(v, 64, 3, 5), leaf_even(v, 64, 3, 5, 7));
	return 0;
}
