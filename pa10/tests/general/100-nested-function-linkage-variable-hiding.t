// CWG1839/P1787R6: the inner declaration corresponds to the namespace function.
// Outside that inner block, ordinary lookup still names the parameter.
static int target(int value) { return value + 100; }
int invoke(int target) { int result; { int target(int value); result = target(4); } return target + result; }
int main() { return invoke(7) == 111 ? 0 : 1; }
