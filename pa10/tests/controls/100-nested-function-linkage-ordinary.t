// N3485 3.5p6: visible redeclarations of the same function preserve its identity.
// expect exit 0
static int target(int value) { return value + 10; }
int invoke(int n) { int target(int first); { int target(int second); return target(n); } }
int main() { return invoke(4) == 14 ? 0 : 1; }
