// expect reject
// N3485 focus: 14.3.2p1/p5, 5.19p3; converted constant expressions do not narrow.
// GCC 15.2.0 g++.dg/cpp0x/Wnarrowing8.C regression and independently authored neighbors.
template<int V> struct Source { constexpr operator int() const { return V; } };
template<class T, signed char = T{}> struct Tag {};
Tag<Source<1000>> object;
