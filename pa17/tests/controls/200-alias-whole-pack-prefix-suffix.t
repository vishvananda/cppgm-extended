// CXX11-124: retain the whole pack while substituting a selected element.
// N3485 5.3.3p5,14.5.3p6,14.5.7p2; expect exit 0
template<int...> struct Values {};
struct A { static const int value = 2; };
struct B { static const int value = 5; };
template<class... T> using Counted = Values<(T::value + sizeof...(T))...>;
template<class... T> using Both = Counted<A, T..., B>;
int check(Values<4, 7>*) { return 0; }
int check(Values<5, 5, 8>*) { return 0; }
int check(Values<6, 6, 9, 9>*) { return 0; }
int main() { Both<> a; Both<A> b; Both<A, B> c; return check(&a) || check(&b) || check(&c); }
