// N3485 3.5p6, 5.19p2: block function declarations denote their matched namespace entity.
// expect exit 0
constexpr int value(int original = 2) { return original + 1; }
void check() {
    constexpr int value(int local = 8);
    static_assert(value() == 9, "block default is local");
    static_assert(::value() == 3, "namespace default remains independent");
    static_assert(value(4) == 5, "bind canonical body parameter");
}
int main() { check(); return 0; }
