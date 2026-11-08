// expect exit 0
// N3485 14.8.2.5: a deduced template-id includes that template's omitted defaults.
template<class T> struct X {};
template<class T, class U = void> struct Set {};
template<class V, template<class...> class C> int select(const X<C<V>>&) { return 1; }
int select(...) { return 2; }
int main() { X<Set<int, short>> value; return select(value) == 2 ? 0 : 1; }
