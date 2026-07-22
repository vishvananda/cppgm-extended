template<class...> struct S {};
int sum(int a, int b) { return a + b; }
template<class U, class... T> int get(T...) { return sizeof(U); }
template<class... U, class... T> int transform(S<U...>, T... t) { return sum(get<U>(t...)...); }
int main() { return transform(S<char, short>(), 4) - 3; }
