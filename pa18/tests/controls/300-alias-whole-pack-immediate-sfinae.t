// CXX11-124: retain the whole pack while substituting a selected element.
// N3485 5.3.3p5,14.5.3p6,14.5.7p2; expect exit 0
template<int...> struct Values {};
struct A { static const int value = 2; };
struct B { static const int value = 5; };
template<class... T> using Counted = Values<(T::value + sizeof...(T))...>;
template<class... T> using Append = Counted<T..., A>;
struct Bad {};
template<class T> int choose(Append<T>*) { return 1; }
template<class> int choose(...) { return 2; }
int main() { Values<7, 4>* p = nullptr; return choose<B>(p) != 1 || choose<Bad>(p) != 2; }
