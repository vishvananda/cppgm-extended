// N3485 3.5p6, 5.19p2: a block declaration retains internal namespace identity.
// expect exit 0
namespace chosen { static constexpr int target(int source) { return source + 12; }
void check() {
    constexpr int target(int renamed);
    static_assert(target(3) == 15, "same internal entity");
}}
int main() { chosen::check(); return 0; }
