// CWG 1405 does not forbid reading a nonmutable member or constructing
// an object whose other member is mutable. expect exit 0
// CXX11-019.
struct A { int fixed; mutable int cache; };
constexpr A source = {7, 8};
constexpr int value = source.fixed;
static_assert(value == 7, "nonmutable member remains constant");
int main() { return value != 7; }
