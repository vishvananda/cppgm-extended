// VALIDATION: compile-fail
// N3485 focus: 14.8.2.2 [temp.deduct.funcaddr]
// Expected: an overloaded function argument without enough context must not deduce.

int choose(int)
{
  return 1;
}

double choose(double)
{
  return 2.0;
}

template<typename R, typename A>
R use(R (*fn)(A))
{
  return fn(A());
}

int main()
{
  return use(choose);
}
