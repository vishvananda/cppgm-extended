// CXX11-124: the same selected value with distinct enclosing whole packs.
// N3485 5.3.3p5,14.5.3/14.5.7; expect exit 0
template<int...> struct Values {};
template<class... Outer> struct Holder {
 template<class... Inner> using Counted = Values<(sizeof(Inner)+sizeof...(Inner)+sizeof...(Outer))...>;
 template<class... T> using Append = Counted<T..., char>;
};
int check(Values<sizeof(short)+2, 3>*) { return 0; }
int check(Values<sizeof(short)+4, 5>*) { return 0; }
int main() {
 Holder<>::Append<short> a;
 Holder<int,long>::Append<short> b;
 Holder<int,long>::Append<short> c;
 Holder<>::Append<short> d;
 return check(&a) || check(&b) || check(&c) || check(&d);
}
