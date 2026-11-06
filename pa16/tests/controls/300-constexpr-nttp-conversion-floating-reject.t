// expect reject
// N3485 focus: 14.3.2p1/p5, 5.19p3; converted constant expressions do not narrow.
// GCC 15.2.0 g++.dg/cpp0x/Wnarrowing8.C regression and independently authored neighbors.
struct Source { constexpr operator double() const { return 1.0; } };
template<int> struct Tag {};
Tag<Source{}> object;
