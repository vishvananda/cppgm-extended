// CXX11-123: a scalar argument replaces one element of an expression pack.
// N3485 14.5.3p6/14.5.7p2; expect exit 0
template<int...> struct Values {};
struct A { static const int value = 2; };
struct B { static const int value = 5; };
template<class... T> using All = Values<T::value...>;
template<class T> using Scalar = All<T, A>;
int check(Values<5, 2>*) { return 0; }
int main() { Scalar<B> b; return check(&b); }
