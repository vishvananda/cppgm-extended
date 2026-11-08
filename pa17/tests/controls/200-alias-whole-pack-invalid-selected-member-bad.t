// CXX11-124: retain the whole pack while substituting a selected element.
// N3485 5.3.3p5,14.5.3p6,14.5.7p2; expect reject
template<int...> struct Values {};
struct A { static const int value = 2; };
struct B { static const int value = 5; };
template<class... T> using Counted = Values<(T::value + sizeof...(T))...>;
template<class... T> using Append = Counted<T..., A>;
struct Bad {};
Append<Bad> invalid;
