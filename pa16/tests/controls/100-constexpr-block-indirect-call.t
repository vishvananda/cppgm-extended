// N3485 3.5p6, 5.19p2: block function declarations denote their matched namespace entity.
// expect exit 0
constexpr int target(int source) { return source * 2; }
void check() {
    constexpr int target(int renamed);
    constexpr int (*address)(int) = &target;
    static_assert(address(6) == 12, "pointer call reaches namespace definition");
}
int main() { check(); return 0; }
