// CWG 1405: copying an enclosing aggregate reads nested mutable members.
// expect reject. CXX11-019.
struct A { mutable int cache; };
struct B { A value; int fixed; };
constexpr B source = {{8}, 7};
constexpr B copy = source;
int main() { return copy.fixed != 7; }
