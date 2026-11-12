// A branch that tests a call of __builtin_expect(e, c), c a literal, is
// predicted: likely when c is nonzero, unlikely when it is zero.
struct Guard
{
	~Guard();
	operator bool() const;
};

int f(int);

int if_condition(int x)
{
	if (__builtin_expect(x > 0, 1))
		return f(1);
	return f(2);
}

int loops(int n)
{
	int sum = 0;
	while (__builtin_expect(n != 0, 1))
		sum += f(n--);
	for (int i = 0; __builtin_expect(i < sum, true); ++i)
		sum -= f(i);
	do
		sum += f(sum);
	while (__builtin_expect(sum < 0, false));
	return sum;
}

int chain_operand(int x, int y)
{
	if (x != 0 && __builtin_expect(y != 0, 0))
		return f(3);
	return f(4);
}

long value_operands(int x, int y)
{
	return (__builtin_expect(x, 1) && y) + (__builtin_expect(y, 0L) || x);
}

int choice(int x)
{
	return __builtin_expect(x, 0) ? f(5) : f(6);
}

int temporary()
{
	if (__builtin_expect(Guard(), 0))
		return f(7);
	return f(8);
}

// The branch on the call is predicted, the branch inside e is not.
int whole_chain(int x, int y)
{
	if (__builtin_expect(x != 0 && y != 0, 1))
		return f(9);
	return f(10);
}

// None of these branches tests the call with a literal c.
int not_tested(int x, long c)
{
	if (!__builtin_expect(x, 0))
		return f(11);
	if (long v = __builtin_expect(x, 0))
		return f(int(v));
	if (__builtin_expect(x, c))
		return f(12);
	return f(13);
}

long value(int x)
{
	return __builtin_expect(x, 1);
}
