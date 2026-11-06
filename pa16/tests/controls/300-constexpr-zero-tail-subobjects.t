// N3485 8.5p5, 5.19: zero tails retain typed subobjects and identity. expect exit 0
struct Empty {};
struct Derived : Empty {};
struct Leaf { unsigned bits : 3; int value; };
union U { Leaf first; long second; };
union B { unsigned first : 3; int second; };
struct A { U u; Empty e; int a[3]; };
constexpr const A (&values)[3] = {};
constexpr const B (&bits)[3] = {};
constexpr const Derived (&bases)[3] = {};
static_assert(values[2].u.first.bits == 0, "nested bitfield");
static_assert(values[2].u.first.value == 0, "active union field");
static_assert(values[2].a[2] == 0, "nested array");
static_assert(bits[2].first == 0, "union bitfield");
constexpr Empty base = bases[2];
constexpr Empty empty = values[2].e;
constexpr Leaf leaf = values[2].u.first;
constexpr U whole = values[2].u;
static_assert(leaf.bits == 0 && whole.first.value == 0, "whole small values");
static_assert(&values[0].a[0] != &values[2].a[0], "identity");
int main() { const volatile int* p=&values[2].u.first.value; return *p; }
