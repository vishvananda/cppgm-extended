// CXX11-123: dependent expression alias-pack substitution; N3485 14.5.3p4-p6,14.5.7p2.
// expect exit 0
template<int...> struct Values {};
struct A { static const int value = 2; };
struct B { static const int value = 5; };
template<class... T> using All = Values<T::value...>;
template<class... T> using Append = All<T..., A>;
struct Bad {};
template<class T> int choose(Append<T>*) { return 1; }
template<class> int choose(...) { return 2; }
int main() { Values<5, 2>* p = nullptr; return choose<B>(p) != 1 || choose<Bad>(p) != 2; }
