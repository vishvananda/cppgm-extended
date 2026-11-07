// Nested alias expansion boundary: N3485 14.5.3/14.5.7 and CWG1558.
// expect exit 0
template<class...> struct Pack {};
// The alias ARGUMENT must substitute, even when the inner expansion is empty.
template<class A, class... B> using Erased = Pack<Pack<A, B>...>;
struct Good { using Member = int; };
struct Bad {};
template<class T> int choose(Erased<typename T::Member>*) { return 1; }
template<class> int choose(...) { return 2; }
int main() {
 Pack<>* p = nullptr;
 return choose<Good>(p) != 1 || choose<Bad>(p) != 2;
}
