// CXX11-123: dependent expression alias-pack substitution; N3485 14.5.3p4-p6,14.5.7p2.
// expect exit 0
template<int...> struct Values {};
struct A { static const int value = 2; };
struct B { static const int value = 5; };
template<class... T> using All = Values<T::value...>;
template<class... T> using Append = All<T..., A>;
template<class... T> using Prepend = All<A, T...>;
int check(Values<2>*) { return 0; }
int check(Values<5, 2>*) { return 0; }
int check(Values<2, 5>*) { return 0; }
int main() {
 Append<> a; Append<B> b; Prepend<> c; Prepend<B> d;
 return check(&a) || check(&b) || check(&c) || check(&d);
}
