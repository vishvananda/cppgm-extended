// N3485 3.5p6 and 5.19p2: unavailable or unrelated definitions do not suffice.
// expect reject
constexpr int target(int source);
void check() { constexpr int target(int renamed); static_assert(target(2) == 3, "definition is later"); }
constexpr int target(int source) { return source + 1; }
