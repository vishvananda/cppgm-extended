// CWG 1405: nested member accesses preserve mutable-subobject identity.
// expect reject. CXX11-019; reduced from GCC constexpr-mutable4.C.
struct A { mutable int cache = 0; };
struct B { A a; };
struct C { B b; };
constexpr C source;
constexpr int value = source.b.a.cache;
int main() { return 0; }
