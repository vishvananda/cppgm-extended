// CXX11-123: dependent expression alias-pack substitution; N3485 14.5.3p4-p6,14.5.7p2.
// expect reject
template<int...> struct Values {};
struct A { static const int value = 2; };
struct B { static const int value = 5; };
template<class... T> using All = Values<T::value...>;
template<class... T> using Append = All<T..., A>;
struct Bad {};
Append<Bad> invalid;
