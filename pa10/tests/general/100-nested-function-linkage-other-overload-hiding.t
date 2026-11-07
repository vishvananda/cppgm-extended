// CWG1839/P1787R6: correspondence does not depend on ordinary lookup hiding.
// After the inner block, ordinary lookup still selects the outer long overload.
static int target(int value) { return value + 100; }
long target(long value) { return value + 200; }
int invoke() { long target(long value); int result; { int target(int other); result = target(4); } return result + target(4L); }
int main() { return invoke() == 308 ? 0 : 1; }
