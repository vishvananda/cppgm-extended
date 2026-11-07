// CWG1839/P1787R6: an introduced name does not change its entity target scope.
namespace other { static int target = 100; }
using other::target;
int invoke(int target) { { int target(int); return target(4); } }
int main() { return invoke(7) == 24 ? 0 : 1; }
