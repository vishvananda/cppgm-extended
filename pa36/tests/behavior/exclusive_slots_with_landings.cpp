// Frame storage shared between objects that no execution uses together
// (O1+), in a function with landings: each arm of pick() holds one object
// with a destructor, of its own type and size, and a call in the arm may
// throw. The arm's cleanup then reads its object, and the handler reads the
// object every arm reaches, which keeps storage of its own.
#include <cstdio>
#include <stdexcept>
struct Small
{
	unsigned long a, b;
	~Small();
};
struct Wide
{
	unsigned long w[8];
	~Wide();
};
struct Word
{
	unsigned long v;
	~Word();
};
static unsigned long sink = 0;
__attribute__((noinline)) Small::~Small() { sink = sink * 31 + a + b; }
__attribute__((noinline)) Wide::~Wide() { sink = sink * 31 + w[0] + w[7]; }
__attribute__((noinline)) Word::~Word() { sink = sink * 31 + v; }
__attribute__((noinline)) unsigned long use(const unsigned long* p, unsigned n, bool fail)
{
	unsigned long s = 0;
	for (unsigned k = 0; k < n; ++k)
		s += p[k];
	if (fail)
		throw std::runtime_error("use");
	return s;
}
__attribute__((noinline)) unsigned long pick(unsigned which, bool fail)
{
	Small kept = { 100, which };
	unsigned long r = 0;
	try
	{
		switch (which)
		{
		case 0:
		{
			Small s = { 3, 4 };
			r = use(&s.a, 2, fail);
			break;
		}
		case 1:
		{
			Wide w = { { 10, 0, 0, 0, 0, 0, 0, 20 } };
			r = use(w.w, 8, fail);
			break;
		}
		default:
		{
			Word d = { which };
			r = use(&d.v, 1, fail);
			break;
		}
		}
	}
	catch (const std::exception&)
	{
		r = 1000 + kept.b;
	}
	return r + kept.a;
}
int main()
{
	for (unsigned which = 0; which < 4; ++which)
		for (unsigned fail = 0; fail < 2; ++fail)
		{
			const unsigned long r = pick(which, fail != 0);
			std::printf("%u %u %lu %lu\n", which, fail, r, sink);
		}
	return 0;
}
