// VALIDATION: run-pass
// Boost.Charconv reduction: a '<' relational operator in a dependent
// enable_if return type must not be parsed as a nested template-id opener.

namespace meta
{
template<bool B, class T = void>
struct enable_if
{
};

template<class T>
struct enable_if<true, T>
{
  typedef T type;
};
}

template<class T>
typename meta::enable_if<sizeof(T) < sizeof(long), int>::type narrow(T value)
{
  return value;
}

int main()
{
  unsigned value = 7;
  return narrow(value) == 7 ? 0 : 1;
}
