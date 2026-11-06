// A constexpr constructor can initialize mutable storage and read its
// already initialized nonmutable sibling. expect exit 0
// CXX11-019: preserve valid construction of literal types with mutable members.
struct A {
    int fixed;
    mutable int cache;
    constexpr A(int n) : fixed(n), cache(fixed + 1) {}
};
constexpr A source(7);
static_assert(source.fixed == 7, "nonmutable constructor read");
int main() { const volatile A* p = &source; return p->fixed != 7 || p->cache != 8; }
