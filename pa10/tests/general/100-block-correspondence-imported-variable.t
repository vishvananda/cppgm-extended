// CWG1839/P1787R6: an introduced name does not change its entity target scope.
namespace other { static int target = 100; }
using other::target;
int invoke(int target) { { extern int target; return target; } }
int main() { return invoke(7) == 20 ? 0 : 1; }
