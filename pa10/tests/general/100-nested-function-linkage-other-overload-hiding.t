// N3485 3.5p6: visible redeclarations of the same function preserve its identity.
static int target(int value) { return value + 100; }
long target(long value) { return value + 200; }
int invoke() { long target(long value); { int target(int other); return target(4); } }
int main() { return invoke() == 24 ? 0 : 1; }
