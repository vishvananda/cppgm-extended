// CXX11-123: dependent expression alias-pack substitution; N3485 14.5.3p4-p6,14.5.7p2.
// expect exit 0
template<int...> struct Values {};
template<class... T> using Sizes = Values<sizeof(T)...>;
template<class... T> using Append = Sizes<T..., char>;
int check(Values<1>*) { return 0; }
int check(Values<sizeof(int), 1>*) { return 0; }
int main() { Append<> a; Append<int> b; return check(&a) || check(&b); }
