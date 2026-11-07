// N3485 3.5p6, 5.19p2: block function declarations denote their matched namespace entity.
// expect exit 0
constexpr int target(int source) { return source; }
void check() {
    constexpr int target(int renamed);
    static_assert(&target == &::target, "one function identity");
}
int main() { check(); return 0; }
