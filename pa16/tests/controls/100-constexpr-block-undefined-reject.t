// N3485 3.5p6, 5.19p2: block function declarations denote their matched namespace entity.
// expect reject
constexpr int target(int source);
void check() {
    constexpr int target(int renamed);
    static_assert(target(2) == 3, "no definition is available");
}
