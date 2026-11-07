// CXX11-123: dependent expression alias-pack substitution; N3485 14.5.3p4-p6,14.5.7p2.
// expect exit 0
template<int...> struct Values {};
struct A { static const int value = 2; };
struct B { static const int value = 5; };
template<class... T> using Counted = Values<(T::value + sizeof...(T))...>;
template<class... T> using Append = Counted<T..., A>;
int check(Values<3>*) { return 0; }
int check(Values<7, 4>*) { return 0; }
int main() { Append<> a; Append<B> b; return check(&a) || check(&b); }
