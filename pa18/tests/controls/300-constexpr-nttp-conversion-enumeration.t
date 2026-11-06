// expect exit 0
// N3485 focus: 14.3.2p1/p5, 5.19p3; converted constant expressions do not narrow.
// GCC 15.2.0 g++.dg/cpp0x/Wnarrowing8.C regression and independently authored neighbors.
enum class Choice { valid = 3 };
struct Source { constexpr operator Choice() const { return Choice::valid; } };
template<Choice V> struct Tag { static constexpr Choice value = V; };
static_assert(Tag<Source{}>::value == Choice::valid, "same enumeration conversion");
int main() { return 0; }
