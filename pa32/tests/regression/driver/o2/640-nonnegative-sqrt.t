// sqrt of an argument that cannot be negative is the square root instruction
// alone: a sum of squares, or a value a dominating test found at least zero.
// Otherwise the call stays behind a test for a negative argument, which must
// set errno.
#include <cmath>

double length(double x, double y)
{
	return std::sqrt(x * x + y * y);
}

double root(double d)
{
	if (d < 0)
		return 0;
	return std::sqrt(d);
}

double any(double d)
{
	return std::sqrt(d);
}

int main()
{
	volatile double a = 3, b = 4, c = 16, e = -1;
	const double s = length(a, b) + root(c) + root(e) + any(c);
	return static_cast<int>(s) - 13;
}
