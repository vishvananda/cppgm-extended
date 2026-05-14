struct A { int a; };
struct B { int b; };
struct C { int c; };
struct D : virtual A, virtual B, virtual C {};

template<class E>
struct W : E {
  W(E const & e, int const &) : E(e) {}
};

template<class E>
W<E> make_wrap(E const & e, int const & loc)
{
  return W<E>(e, loc);
}

int main()
{
  D d;
  d.a = 7;
  W<D> w = make_wrap(d, d.a);
  return w.a == 7 ? 0 : 1;
}
