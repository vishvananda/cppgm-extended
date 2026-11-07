// N3485 3.5p6, 5.19p2: block function declarations denote their matched namespace entity.
// expect exit 0
namespace chosen { static constexpr int target(int source) { return source + 12; }
void check() {
    constexpr int target(int renamed);
    { constexpr int target(int again); static_assert(target(3) == 15, "same internal entity"); }
}}
int main() { chosen::check(); return 0; }
