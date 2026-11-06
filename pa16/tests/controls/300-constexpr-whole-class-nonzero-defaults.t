// N3485 5.19, 8.5.1: omitted aggregate class members invoke their nonzero defaults. expect exit 0
struct Leaf { int value; constexpr Leaf(int n = 7) : value(n) {} };
struct Pair { Leaf left, right; };
struct Outer { Pair values[2]; };
constexpr Outer all = {};
constexpr Outer partial = {{{{Leaf(2)}}}};
constexpr int read(Outer value) { return value.values[0].left.value + value.values[1].right.value; }
static_assert(read(all) == 14 && read(partial) == 9, "nonzero omitted members");
static_assert(partial.values[0].right.value == 7 && partial.values[1].left.value == 7, "all omitted defaults");
int main() { const volatile Outer* p = &partial; return p->values[0].left.value != 2 || p->values[1].right.value != 7; }
