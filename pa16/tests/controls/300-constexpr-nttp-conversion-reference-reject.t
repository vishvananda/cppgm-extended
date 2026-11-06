// expect reject
// N3485 focus: 14.3.2p1/p5, 5.19p3; converted constant expressions do not narrow.
// GCC 15.2.0 g++.dg/cpp0x/Wnarrowing8.C regression and independently authored neighbors.
constexpr int value = 1000;
struct Source { constexpr operator const int&() const { return value; } };
template<signed char> struct Tag {};
Tag<Source{}> object;
