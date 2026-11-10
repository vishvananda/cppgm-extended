// Namespace-scope objects whose initializers compute constants become data
// at O1 and up: doubles computed from a readonly double and a double
// initialized after a call; at O2 a struct of a value and an address once
// its constructor is inlined, at O3 a table a loop fills once the loop is
// unrolled. An object written again later, one a call returns and one read
// from a volatile stay dynamic, in order.
#include <cstdio>

volatile int seed = 5;
static const double pi = 3.25;
static double two_pi = pi * 2;

struct Pair
{
	double value;
	const double* at;
	Pair(double v, const double* p) : value(v), at(p) {}
};

static Pair pair(two_pi / 4, &pi);

struct Table
{
	int squares[8];
	Table()
	{
		for (int i = 0; i < 8; ++i)
			squares[i] = i * i;
	}
};

static Table table;
static int calls = 0;

__attribute__((noinline)) int count()
{
	return ++calls;
}

static int first = count();
static double tail = pi * 4;
static double later = pi + 1;
static int dynamic = seed * 3;

__attribute__((noinline)) void reset(double x)
{
	later = x;
}

int main()
{
	std::printf("%g %g %g %d %d\n", two_pi, pair.value, *pair.at, table.squares[7], table.squares[3]);
	std::printf("%d %d %g %g %d\n", calls, first, tail, later, dynamic);
	reset(9);
	std::printf("%g\n", later);
	return 0;
}
