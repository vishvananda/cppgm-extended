struct Base { int value = 13; };
struct Derived : Base { int other = 7; };
constexpr Derived item = {};
constexpr Derived items[2] = {};
static_assert(item.value == 13 && item.other == 7, "object defaults");
static_assert(items[1].value == 13 && items[1].other == 7, "array defaults");
struct Scalar { int value = 19; };
constexpr Scalar scalar_items[3] = {};
static_assert(scalar_items[2].value == 19, "scalar member default");
