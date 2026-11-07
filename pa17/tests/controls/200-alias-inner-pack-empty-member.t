// Nested alias expansion boundary: N3485 14.5.3/14.5.7 and CWG1558.
template<class...> struct Pack {};
// Lookup INSIDE a zero-element expansion is not performed.
// expect exit 0
template<class A, class... B> using Maybe = Pack<Pack<typename A::Member, B>...>;
struct Bad {};
Maybe<Bad>* object = nullptr;
int main() { return object != nullptr; }
