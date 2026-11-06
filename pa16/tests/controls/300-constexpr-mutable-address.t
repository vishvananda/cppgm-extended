// A mutable member has a constant address even when its stored value
// cannot be used in a constant expression. expect exit 0
// CXX11-019.
struct A { mutable int cache; };
constexpr A source = {8};
constexpr int* address = &source.cache;
constexpr int& alias = source.cache;
static_assert(address == &alias, "aliases retain storage identity");
int main() { *address = 9; return alias != 9; }
