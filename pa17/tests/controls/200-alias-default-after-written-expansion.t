// N3485 14.1p9/14.5.7; defaults belong to the alias and wait for expansion arity; expect exit 0
template<class A, class B> struct Pair {};
template<class A, class B = char> using WithDefault = Pair<A, B>;
int identify(Pair<int, char>) { return 1; }
int identify(Pair<int, int>) { return 2; }
template<class... T> int inspect(WithDefault<T...> value) { return identify(value); }
int main() { return inspect<int>({}) + inspect<int, int>({}) - 3; }
