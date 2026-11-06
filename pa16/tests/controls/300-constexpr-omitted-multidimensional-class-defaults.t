// N3485 5.19, 8.5.1: omitted rows and array members retain nonzero class defaults. expect exit 0
struct Leaf { int value; constexpr Leaf(int n = 7) : value(n) {} };
struct Pair { Leaf left, right; };
struct Outer { Pair values[2][2]; };
constexpr Outer all = {};
constexpr Outer partial = {{{{{Leaf(2)}}}}};
constexpr const Pair (&rows)[2][2] = {};
static_assert(all.values[0][1].left.value == 7 && all.values[1][1].right.value == 7, "omitted member rows");
static_assert(partial.values[0][0].left.value == 2 && partial.values[1][1].right.value == 7, "partial rows");
static_assert(rows[0][1].left.value == 7 && rows[1][1].right.value == 7, "temporary rows");
int main() { const volatile Outer* p = &partial; return p->values[0][0].left.value != 2 || p->values[1][1].right.value != 7; }
