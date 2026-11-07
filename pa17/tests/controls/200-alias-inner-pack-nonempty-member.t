// Nested alias expansion boundary: N3485 14.5.3/14.5.7 and CWG1558.
template<class...> struct Pack {};
// expect exit 0
template<class A, class... B> using Maybe = Pack<Pack<typename A::Member, B>...>;
struct Good { using Member = int; };
int check(Pack<Pack<int, char>>*) { return 0; }
int main() { Maybe<Good, char>* p = nullptr; return check(p); }
