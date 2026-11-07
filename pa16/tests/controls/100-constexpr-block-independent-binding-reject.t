// N3485 3.5p6 and 5.19p2: unavailable or unrelated definitions do not suffice.
// expect reject
namespace other { constexpr int target(int source) { return source + 1; } }
void check() { constexpr int target(int renamed); static_assert(target(2) == 3, "another namespace is not the definition"); }
