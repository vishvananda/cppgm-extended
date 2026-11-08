// expect exit 0
// N3485 14.5.3p5: nested type expansions consume their own packs.
template<class... T> struct Values {};
template<class... T> struct A { typedef Values<T...> X; X value; };
int main() { A<int, long> a; A<> empty; return sizeof(a) && sizeof(empty) ? 0 : 1; }
