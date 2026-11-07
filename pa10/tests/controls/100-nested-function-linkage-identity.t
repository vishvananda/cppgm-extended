// N3485 3.5p6: visible redeclarations of the same function preserve its identity.
// expect exit 0
namespace chosen { static int target(int value) { return value + 10; }
int invoke(int n) {
    int target(int first); int (*a)(int) = &target;
    { int target(int second); int (*b)(int) = &target;
      { int target(int third); return a == b && b == &target && target(n) == n + 10 ? 0 : 1; } }
}}
int main() { return chosen::invoke(4); }
