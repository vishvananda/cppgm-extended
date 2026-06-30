template<class T>
struct base
{
};

struct mid : base<int>
{
};

struct derived : mid
{
};

template<class T>
int helper(base<T> &)
{
  return 7;
}

int main()
{
  derived value;
  return helper(value) == 7 ? 0 : 1;
}
