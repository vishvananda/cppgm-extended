// N3485 8.5.1, 12.1: omitted aggregate-array members cannot bypass a deleted default constructor. expect reject
struct Leaf { int value; Leaf() = delete; constexpr Leaf(int n) : value(n) {} };
struct Pair { Leaf left, right; };
struct Outer { Pair values[2]; };
constexpr Outer invalid = {};
