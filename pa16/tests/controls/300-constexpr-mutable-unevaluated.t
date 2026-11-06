// CWG 1405 concerns evaluated value reads, not unevaluated operands
// or an unselected conditional branch. expect exit 0
// CXX11-019.
struct A { mutable int cache; };
constexpr A source = {8};
constexpr int value = false ? source.cache : 7;
static_assert(value == 7 && sizeof(source.cache) == sizeof(int), "no mutable value read");
int main() { return value != 7; }
