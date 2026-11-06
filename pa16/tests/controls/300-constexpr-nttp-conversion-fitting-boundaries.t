// expect exit 0
// N3485 focus: 14.3.2p1/p5, 5.19p3; converted constant expressions do not narrow.
// GCC 15.2.0 g++.dg/cpp0x/Wnarrowing8.C regression and independently authored neighbors.
template<int V> struct Source { constexpr operator int() const { return V; } };
template<signed char V> struct Signed { enum { value = V }; };
template<unsigned char V> struct Byte { enum { value = V }; };
template<bool V> struct Boolean { enum { value = V }; };
static_assert(Signed<Source<-128>{}>::value == -128, "lower signed boundary");
static_assert(Signed<Source<127>{}>::value == 127, "upper signed boundary");
static_assert(Byte<Source<0>{}>::value == 0, "lower unsigned boundary");
static_assert(Byte<Source<255>{}>::value == 255, "upper unsigned boundary");
static_assert(Boolean<Source<0>{}>::value == 0, "false");
static_assert(Boolean<Source<1>{}>::value == 1, "true");
int main() { return 0; }
