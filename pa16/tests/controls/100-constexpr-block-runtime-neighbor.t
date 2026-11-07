// N3485 3.5p6, 5.19p2: block function declarations denote their matched namespace entity.
// expect exit 0
constexpr int target(int source) { return source + 2; }
int call(int x) { constexpr int target(int renamed); return target(x); }
int main(int argc, char**) { return call(argc) == argc + 2 ? 0 : 1; }
