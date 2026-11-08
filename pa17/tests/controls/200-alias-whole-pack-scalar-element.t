// CXX11-124: retain the whole pack while substituting a selected element.
// N3485 5.3.3p5,14.5.3p6,14.5.7p2; expect exit 0
template<int...> struct Values {};
struct A { static const int value = 2; };
struct B { static const int value = 5; };
template<class... T> using Counted = Values<(T::value + sizeof...(T))...>;
template<class T> using Scalar = Counted<T, A>;
int check(Values<7, 4>*) { return 0; }
int main() { Scalar<B> b; return check(&b); }
