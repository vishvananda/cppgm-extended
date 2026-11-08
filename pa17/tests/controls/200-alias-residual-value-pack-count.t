// CXX11-124: a captured alias pack remains valid before its size is known.
// Neighbor of GCC PR c++/69958 (variadic-sizeof4a.C).
// N3485 5.3.3p5,14.5.7p2; expect exit 0
template<unsigned long N> struct Size {};
template<unsigned... T> using Count = Size<sizeof...(T)>;
template<unsigned... T> using Prefix = Count<0, T...>;
template<unsigned... T> using Suffix = Count<T..., 0>;
int check(Size<1>*) { return 0; }
int check(Size<2>*) { return 0; }
int check(Size<5>*) { return 0; }
int main() {
  Prefix<> a; Suffix<> b;
  Prefix<0> c; Suffix<0> d;
  Prefix<2, 3, 4, 5> e; Suffix<2, 3, 4, 5> f;
  return check(&a) || check(&b) || check(&c) || check(&d) || check(&e) || check(&f);
}
