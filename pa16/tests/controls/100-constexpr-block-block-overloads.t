// N3485 3.5p6, 5.19p2: block function declarations denote their matched namespace entity.
// expect exit 0
constexpr int select(int source) { return source + 1; }
constexpr long select(long source) { return source + 7; }
void check() {
    constexpr int select(int local);
    static_assert(select(4) == 5, "the selected overload has the right body");
    { constexpr long select(long different); static_assert(select(4L) == 11, "nested overload"); }
}
int main() { check(); return 0; }
