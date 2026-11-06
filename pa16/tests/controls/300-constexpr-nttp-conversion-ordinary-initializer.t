// expect exit 0
// N3485 focus: 14.3.2p1/p5, 5.19p3; converted constant expressions do not narrow.
// GCC 15.2.0 g++.dg/cpp0x/Wnarrowing8.C regression and independently authored neighbors.
template<int V> struct Source { constexpr operator int() const { return V; } };
constexpr signed char value = Source<1000>{};
constexpr bool enabled = Source<2>{};
static_assert(value == static_cast<signed char>(1000), "ordinary initialization may convert");
static_assert(enabled, "ordinary bool initialization");
int main() { signed char local = Source<1000>{}; return local != value; }
