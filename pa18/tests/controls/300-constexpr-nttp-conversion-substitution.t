// expect exit 0
// N3485 focus: 14.3.2p1/p5, 5.19p3; converted constant expressions do not narrow.
// GCC 15.2.0 g++.dg/cpp0x/Wnarrowing8.C regression and independently authored neighbors.
template<int V> struct Source { constexpr operator int() const { return V; } };
template<class T, signed char = T{}> char select(int);
template<class T> long select(...);
static_assert(sizeof(select<Source<1000>>(0)) == sizeof(long), "narrowing removes the candidate");
static_assert(sizeof(select<Source<127>>(0)) == sizeof(char), "fitting conversion stays viable");
int main() { return 0; }
