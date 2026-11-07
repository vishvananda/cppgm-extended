// N3485 focus: 3.3.1p4, 3.5p6-p9 [basic.scope.declarative/basic.link].
// expect exit 0
static int Name(int n) { return n; }
int Name(int a, int b) { return a + b; }
int main() { int Name(int); int Name(int, int); return Name(2) + Name(3, 4) - 9; }
