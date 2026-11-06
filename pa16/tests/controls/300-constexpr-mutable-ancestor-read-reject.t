// CWG 1405: an ordinary field inside a mutable member inherits that
// mutable access path. expect reject
// CXX11-019.
struct A { int value; };
struct B { mutable A cache; };
constexpr B source = {{8}};
constexpr int value = source.cache.value;
int main() { return 0; }
