// expect exit 0
// N3485 12.8p15-p16 and 5.19p2: an empty trivial move reads no member values.
struct E {};
E a;
constexpr E b = static_cast<E&&>(a);
int main() { return 0; }
