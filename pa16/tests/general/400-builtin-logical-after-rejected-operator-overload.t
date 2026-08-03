// VALIDATION: compile-pass

namespace library
{
template<class T>
struct parser {};

template<class A, class B>
int operator&&(parser<A> const&, parser<B> const&);

template<class Derived>
struct safe_bool
{
  typedef int Derived::* bool_type;

  operator bool_type() const
  {
    return static_cast<Derived const*>(this)->valid()
        ? &Derived::marker
        : 0;
  }
};

struct match : safe_bool<match>
{
  int marker;
  bool state;

  explicit match(bool value) : marker(0), state(value) {}

  bool valid() const
  {
    return state;
  }
};

bool both(match const& left, match const& right)
{
  return left && right;
}
}

int main()
{
  return library::both(library::match(true), library::match(true)) ? 0 : 1;
}
