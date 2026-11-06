// CWG1405: C++11 cannot read a mutable member of a temporary in a
// required constant expression. expect reject. CXX11-019.
struct A { int fixed; mutable int cache; };
constexpr int value = A{7, 8}.cache;
int main() { return 0; }
