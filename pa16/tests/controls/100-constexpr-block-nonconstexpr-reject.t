// N3485 3.5p6, 5.19p2: block function declarations denote their matched namespace entity.
// expect reject
int target(int source) { return source + 1; }
void check() { int target(int renamed); static_assert(target(2) == 3, "not constexpr"); }
