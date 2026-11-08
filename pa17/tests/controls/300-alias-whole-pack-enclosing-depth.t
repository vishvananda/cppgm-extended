// CXX11-124: retain the whole pack while substituting a selected element.
// N3485 5.3.3p5,14.5.3p6,14.5.7p2; expect exit 0
template<int...> struct Values {};
template<class... Outer> struct Holder {
 template<class... Inner> using Counted = Values<(sizeof(Inner) + sizeof...(Inner) + sizeof...(Outer))...>;
 template<class... T> using Append = Counted<T..., char>;
};
int check(Values<4>*) { return 0; }
int check(Values<sizeof(short)+4, 5>*) { return 0; }
int main() { Holder<int, long>::Append<> a; Holder<int, long>::Append<short> b; return check(&a) || check(&b); }
