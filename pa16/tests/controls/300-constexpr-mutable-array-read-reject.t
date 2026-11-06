// CWG 1405: an element of a mutable array member remains mutable.
// expect reject. CXX11-019.
struct A { mutable int cache[2]; };
constexpr A source = {{7, 8}};
constexpr int value = source.cache[1];
int main() { return 0; }
