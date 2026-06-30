template<class C, bool B>
struct iter;

struct copy
{
  template<class C, bool B>
  int operator()(iter<C, B> first, iter<C, B> last, iter<C, false> result) const;
};

template<class C, bool B>
struct iter
{
  iter(int v) : value(v)
  {
  }

private:
  int value;

  template<class D, bool E>
  friend int copy::operator()(iter<D, E>, iter<D, E>, iter<D, false>) const;
};

template<class C, bool B>
int copy::operator()(iter<C, B> first, iter<C, B>, iter<C, false> result) const
{
  return first.value == result.value ? 0 : 1;
}

int main()
{
  iter<int, false> it(5);
  copy c;
  return c(it, it, it);
}
