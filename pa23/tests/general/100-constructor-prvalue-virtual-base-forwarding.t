struct A { int a; A(int value = 0) : a(value) {} };
struct B { int b; B(int value = 0) : b(value) {} };
struct C { int c; C(int value = 0) : c(value) {} };
struct D : virtual A, virtual B, virtual C {};

template<class E>
struct W : E {
  W(E const & e, int const & loc) : A(loc), B(e.b), C(e.c), E(e) {}
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
  d.b = 8;
  d.c = 9;
  W<D> w = make_wrap(d, d.a);
  return w.a == 7 && w.b == 8 && w.c == 9 ? 0 : 1;
}
