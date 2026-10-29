constexpr int fixed_number() { return 13; }

template<int N>
struct number_holder
{
  typedef int type;
  static constexpr int value = N;
};

template<class Unused>
int check_fixed_values()
{
  typedef number_holder<fixed_number()> fixed;
  typedef number_holder<fixed_number() + 4> sum;
  const int local = fixed_number() + 2;
  typedef number_holder<local> local_value;
  local_value::type third = local_value::value;
  fixed::type first = fixed::value;
  sum::type second = sum::value;
  return first == 13 && second == 17 && third == 15 ? 0 : 1;
}

template<class T>
int check_dependent_local()
{
  const int size = sizeof(T);
  typedef number_holder<size> dependent;
  typename dependent::type value = dependent::value;
  return value == sizeof(T) ? 0 : 1;
}

int main()
{
  return check_fixed_values<void>() + check_fixed_values<int>() +
         check_dependent_local<char>() + check_dependent_local<long>();
}
