template<class A>
struct base
{
  static int f(void *)
  {
    return 1;
  }

  template<class T>
  static int f(T *)
  {
    return 2;
  }
};

template<class A>
struct derived : base<A>
{
  using base<A>::f;
};

int main()
{
  int value = 0;
  return derived<int>::f(&value);
}
