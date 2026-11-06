// CWG 1405: an implicit copy reads the mutable member even when only
// a different member of the result is later inspected. expect reject
// CXX11-019.
struct A { int fixed; mutable int cache; };
constexpr A source = {7, 8};
constexpr A copy = source;
int main() { return copy.fixed != 7; }
