// CXX11-124: retain the whole pack while substituting a selected element.
// N3485 5.3.3p5,14.5.3p6,14.5.7p2; expect exit 0
template<int...> struct Values {};
struct A { static const int value = 2; };
struct B { static const int value = 5; };
template<class... T> using Counted = Values<(T::value + sizeof...(T))...>;
template<class... T> using Forward = Counted<T...>;
int check(Values<>*) { return 0; }
int check(Values<4, 7>*) { return 0; }
int main() { Counted<A, B> a; Forward<> b; Forward<A, B> c; return check(&a) || check(&b) || check(&c); }
