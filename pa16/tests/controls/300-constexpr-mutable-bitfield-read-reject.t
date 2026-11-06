// CWG 1405: a mutable bit-field is not a readable constant subobject.
// expect reject. CXX11-019.
struct A { mutable unsigned cache : 3; };
constexpr A source = {7};
constexpr unsigned value = source.cache;
int main() { return 0; }
