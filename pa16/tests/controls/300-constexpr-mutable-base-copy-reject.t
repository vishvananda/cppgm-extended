// CWG 1405: an implicit derived copy reads mutable base subobjects.
// expect reject. CXX11-019.
struct A { mutable int cache; constexpr A(int n) : cache(n) {} };
struct B : A { int fixed; constexpr B() : A(8), fixed(7) {} };
constexpr B source;
constexpr B copy = source;
int main() { return copy.fixed != 7; }
