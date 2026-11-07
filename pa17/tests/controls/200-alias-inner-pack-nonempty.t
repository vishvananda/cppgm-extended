// Nested alias expansion boundary: N3485 14.5.3/14.5.7 and CWG1558.
// expect exit 0
template<class...> struct Pack {};
template<class A, class... B> using Erased = Pack<Pack<A, B>...>;
int check(Pack<Pack<int, char>, Pack<int, long>>*) { return 0; }
int main() { Erased<int, char, long>* p = nullptr; return check(p); }
