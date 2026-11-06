// A user-defined copy that reads only a nonmutable member remains constexpr.
// expect exit 0. CXX11-019.
struct A {
    int fixed;
    mutable int cache;
    constexpr A(int n, int m) : fixed(n), cache(m) {}
    constexpr A(const A& other) : fixed(other.fixed), cache(0) {}
};
constexpr A source(7, 8);
constexpr A copy = source;
static_assert(copy.fixed == 7, "copy reads only allowed source storage");
int main() { return copy.fixed != 7 || copy.cache != 0; }
