// N3485 3.5p6, 5.19p2: block function declarations denote their matched namespace entity.
// expect exit 0
constexpr int transform(int source) { return source * 3 + 1; }
void check() {
    constexpr int transform(int renamed);
    static_assert(transform(7) == 22, "use the namespace definition's parameter");
}
int main() { check(); return 0; }
