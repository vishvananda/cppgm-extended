// CXX11-124: retain the whole pack while substituting a selected element.
// N3485 5.3.3p5,14.5.3p6,14.5.7p2; expect exit 0
template<int...> struct Values {};
template<int... I> using Counted = Values<(I + sizeof...(I))...>;
template<int... I> using Append = Counted<I..., 7>;
int check(Values<8>*) { return 0; }
int check(Values<5, 8, 10>*) { return 0; }
int main() { Append<> a; Append<2, 5> b; return check(&a) || check(&b); }
