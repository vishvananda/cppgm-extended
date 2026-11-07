// CWG1839/P1787R6: an introduced name does not change its entity target scope.
namespace chosen { inline namespace inside { static int target(int n) { return n + 100; } }
int invoke(int target) { { int target(int); return target(4); } } }
int main() { return chosen::invoke(7) == 24 ? 0 : 1; }
