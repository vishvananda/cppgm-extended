// CWG 1405: a reference alias must not hide a mutable member's origin.
// expect reject. CXX11-019.
struct A { mutable int cache; };
constexpr A source = {8};
constexpr int& alias = source.cache;
constexpr int value = alias;
int main() { return 0; }
