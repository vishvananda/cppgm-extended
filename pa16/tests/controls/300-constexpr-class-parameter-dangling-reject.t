// N3485 5.19, 12.2: a pointer into a by-value parameter cannot escape its lifetime. expect reject
struct Leaf { int value; };
struct Outer { Leaf left, right; };
constexpr const int* identity(const int* value) { return value; }
constexpr const int* escape(Outer value) { return identity(&value.right.value); }
constexpr const int* invalid = escape({});
