// N3485 5.19, 8.5.1, 12.8: copied class values retain independent subobject places. expect exit 0
struct Leaf { int value; };
struct Pair { Leaf left, right; };
struct Outer { Pair values[2]; };
constexpr Outer first = {{{{7}, {8}}, {{9}, {10}}}};
constexpr Outer second = first;
constexpr Outer zero = {};
constexpr int sum(Outer value) { return value.values[0].left.value + value.values[1].right.value; }
static_assert(sum(first) == 17 && sum(second) == 17 && sum(zero) == 0, "copied values");
static_assert(&first.values[0].left.value != &second.values[0].left.value, "independent objects");
static_assert(&second.values[0].left.value != &second.values[1].left.value, "independent elements");
int main() { const volatile Outer* p = &second; return p->values[0].right.value != 8 || p->values[1].left.value != 9; }
