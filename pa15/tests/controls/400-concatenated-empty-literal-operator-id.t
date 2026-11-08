// expect exit 0
// Adopted CWG 1473: phase 6 precedes literal-operator-id grammar.
unsigned long long operator "" "" _sum(unsigned long long x) { return x + 2; }
unsigned long long operator "" "" "" _sum(unsigned long long);
unsigned long long operator "" ""_next ""(unsigned long long x) { return x + 3; }
int main() { return 5_sum == 7 && 5_next == 8 ? 0 : 1; }
