// A mutable read in an unevaluated conditional arm does not invalidate
// construction. expect exit 0. CXX11-019.
struct A {
    mutable int cache;
    int fixed;
    constexpr A(int n) : cache(n), fixed(n ? cache : 0) {}
};
constexpr A source(0);
static_assert(source.fixed == 0, "unselected mutable read");
int main() { const volatile A* p = &source; return p->cache != 0 || p->fixed != 0; }
