// CXX11-123: dependent expression alias-pack substitution; N3485 14.5.3p4-p6,14.5.7p2.
// expect exit 0
template<int...> struct Values {};
struct A { static const int value = 2; };
struct B { static const int value = 5; };
template<class... T> using All = Values<T::value...>;
template<class... T> using Forward = All<T...>;
int check(Values<>*) { return 0; }
int check(Values<2, 5>*) { return 0; }
int main() { All<A, B> a; Forward<> b; Forward<A, B> c; return check(&a) || check(&b) || check(&c); }
