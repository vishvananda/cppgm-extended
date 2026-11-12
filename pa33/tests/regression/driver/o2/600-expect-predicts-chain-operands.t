// An expectation of the value of an && or || chain predicts the branch on
// each operand to continue the chain, as LLVM lowers llvm.expect of a phi
// of constants. The lowering predicts only the branch on the expectation
// itself (PA30); the optimizer derives the rest.
__attribute__((noinline)) bool flag(unsigned mask, unsigned bit)
{
	return (mask & (1u << bit)) != 0;
}

int f(int);

int likely_and(unsigned mask)
{
	if (__builtin_expect(!!(flag(mask, 0) && flag(mask, 1) && flag(mask, 2)), 1))
		return f(1);
	return f(2);
}

int unlikely_or(unsigned mask)
{
	if (__builtin_expect(!!(flag(mask, 0) || flag(mask, 1) || flag(mask, 2)), 0))
		return f(3);
	return f(4);
}

int negated_and(unsigned mask)
{
	if (__builtin_expect(!(flag(mask, 0) && flag(mask, 1)), 0))
		return f(5);
	return f(6);
}

int cast_and(unsigned mask)
{
	if (__builtin_expect(static_cast<bool>(flag(mask, 0) && flag(mask, 1)), true))
		return f(7);
	return f(8);
}

// An expectation the chain's value does not decide predicts only the last
// operand: `a && b` expected false, or `a || b` expected true, says nothing
// of `a`.
int fallback_and(unsigned mask)
{
	if (__builtin_expect(!!(flag(mask, 0) && flag(mask, 1)), 0))
		return f(9);
	return f(10);
}

int fallback_or(unsigned mask)
{
	if (__builtin_expect(!!(flag(mask, 0) || flag(mask, 1)), 1))
		return f(11);
	return f(12);
}
