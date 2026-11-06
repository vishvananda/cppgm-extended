// expect exit 0
// N3485 focus: 14.3.2p1/p5, 5.19p3; converted constant expressions do not narrow.
// GCC 15.2.0 g++.dg/cpp0x/Wnarrowing8.C regression and independently authored neighbors.
struct Source { constexpr operator unsigned char() const { return 256; } };
template<unsigned char V> struct Tag { enum { value = V }; };
static_assert(Tag<Source{}>::value == 0, "conversion inside the function body precedes the implicit sequence");
int main() { return 0; }
