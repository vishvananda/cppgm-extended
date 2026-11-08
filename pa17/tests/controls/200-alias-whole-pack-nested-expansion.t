// CXX11-124: retain the whole pack while substituting a selected element.
// N3485 5.3.3p5,14.5.3p6,14.5.7p2; expect exit 0
template<int...> struct Values {};
template<int...> struct Sum;
template<> struct Sum<> { static const int value = 0; };
template<int I, int... J> struct Sum<I, J...> { static const int value = I + Sum<J...>::value; };
template<int... I> using Counted = Values<(I + Sum<I...>::value)...>;
template<int... I> using Append = Counted<I..., 7>;
int check(Values<14>*) { return 0; }
int check(Values<16, 19, 21>*) { return 0; }
int main() { Append<> a; Append<2, 5> b; return check(&a) || check(&b); }
