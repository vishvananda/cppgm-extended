// CXX11-124: a captured alias pack remains valid before its size is known.
// Neighbor of GCC PR c++/69958 (variadic-sizeof4.C).
// N3485 5.3.3p5,14.5.7p2; expect exit 0
template<unsigned long N> struct Size {};
template<class... T> using Count = Size<sizeof...(T)>;
template<class... T> using Prefix = Count<char, T...>;
template<class... T> using Suffix = Count<T..., char>;
int check(Size<1>*) { return 0; }
int check(Size<2>*) { return 0; }
int check(Size<5>*) { return 0; }
int main() {
  Prefix<> a; Suffix<> b;
  Prefix<char> c; Suffix<char> d;
  Prefix<int, double, short, long> e; Suffix<int, double, short, long> f;
  return check(&a) || check(&b) || check(&c) || check(&d) || check(&e) || check(&f);
}
