// CWG 1405: a defaulted copy cannot read an external constexpr
// object's mutable member. expect reject
// CXX11-019; reduced from GCC constexpr-mutable2.C.
struct A {
    mutable int cache;
    constexpr A() : cache() {}
};
constexpr A source = {};
constexpr A copy = source;
int main() { return 0; }
