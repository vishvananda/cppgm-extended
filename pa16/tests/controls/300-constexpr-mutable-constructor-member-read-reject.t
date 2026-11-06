// Reading mutable storage is not a C++11 constant expression, even
// during construction. expect reject. CXX11-019.
// Argument zero is valid, so this constructor is not an IFNDR definition.
struct A {
    mutable int cache;
    int fixed;
    constexpr A(int n) : cache(n), fixed(n ? cache : 0) {}
};
constexpr A source(7);
int main() { return 0; }
