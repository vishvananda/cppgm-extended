// An explicit copy constructor reads only the active nonmutable member;
// the inactive mutable alternative does not forbid this copy. expect exit 0
// CXX11-019: this is not the defaulted union copy covered by CWG2004.
union A {
    int fixed;
    mutable int cache;
    constexpr A(int n) : fixed(n) {}
    constexpr A(const A& other) : fixed(other.fixed) {}
};
constexpr A source = {7};
constexpr A copy = source;
static_assert(copy.fixed == 7, "selective union copy");
int main() { return copy.fixed != 7; }
