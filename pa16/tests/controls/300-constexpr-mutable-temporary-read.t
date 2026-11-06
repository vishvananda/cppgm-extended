// Nonmutable members of a class with a mutable sibling remain readable
// in a temporary and by-value parameter. expect exit 0
// CXX11-019: do not reject a whole class just because it has mutable state.
struct A { int fixed; mutable int cache; };
constexpr int read(A value) { return value.fixed; }
constexpr int direct = A{7, 8}.fixed;
static_assert(direct == 7, "temporary nonmutable member");
static_assert(read(A{3, 4}) == 3, "parameter nonmutable member");
int main() { return direct != 7 || read(A{3, 4}) != 3; }
