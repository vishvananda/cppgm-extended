// expect exit 0
// I13: Copy-list initialization of a non-explicit defaulted constructor remains valid.
struct E { E() = default; }; void take(E) {} int main() { E object = {}; take({}); return 0; }
