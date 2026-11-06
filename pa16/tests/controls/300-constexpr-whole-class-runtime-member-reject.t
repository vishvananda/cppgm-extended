// N3485 5.19: an explicit nonconstant member cannot be replaced by an omitted zero value. expect reject
struct Leaf { int value; };
struct Outer { Leaf left, right; };
int runtime;
constexpr Outer invalid = {{runtime}};
