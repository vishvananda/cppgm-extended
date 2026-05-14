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
