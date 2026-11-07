// N3485 3.5p6: visible redeclarations of the same function preserve its identity.
static int target(int value) { return value + 100; }
int invoke(int target) { { int target(int value); return target(4); } }
int main() { return invoke(7) == 24 ? 0 : 1; }
