// N3485 5.19, 12.6.2: constructor subobjects and reference fields retain their initialized storage. expect exit 0
struct Leaf { int value; constexpr Leaf(int n = 7) : value(n) {} };
struct Base { Leaf leaf; constexpr Base(int n) : leaf(n) {} };
struct Value : Base {
  Leaf parts[2];
  const int& alias;
  constexpr Value(int n) : Base(n), parts{Leaf(n + 1), Leaf(n + 2)}, alias(parts[1].value) {}
};
constexpr Value first(9);
constexpr Value second(19);
constexpr Value copied = first;
constexpr int read(Value value) { return value.leaf.value + value.parts[0].value + value.alias; }
static_assert(read(first) == 30 && read(second) == 60 && read(copied) == 30, "constructor writes");
static_assert(&first.alias == &first.parts[1].value && &copied.alias == &first.parts[1].value, "reference copy");
static_assert(&copied.parts[0].value != &first.parts[0].value, "member storage copied");
int main() { const volatile Value* p = &copied; return p->leaf.value != 9 || p->parts[0].value != 10 || p->parts[1].value != 11; }
