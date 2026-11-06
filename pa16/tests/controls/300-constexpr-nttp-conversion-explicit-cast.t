// expect exit 0
// N3485 focus: 14.3.2p1/p5, 5.19p3; converted constant expressions do not narrow.
// GCC 15.2.0 g++.dg/cpp0x/Wnarrowing8.C regression and independently authored neighbors.
template<int V> struct Source { constexpr operator int() const { return V; } };
template<signed char V> struct Signed { enum { value = V }; };
template<bool V> struct Boolean { enum { value = V }; };
static_assert(Signed<static_cast<signed char>(Source<1000>{})>::value == static_cast<signed char>(1000), "explicit cast precedes template conversion");
static_assert(Boolean<static_cast<bool>(Source<2>{})>::value == 1, "explicit bool cast");
int main() { return 0; }
