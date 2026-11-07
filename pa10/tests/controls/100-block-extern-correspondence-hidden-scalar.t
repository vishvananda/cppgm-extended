// CWG1839/P1787R6: declarations target their namespace despite use-site hiding.
// expect exit 0
static int stored = 5;
int main() { int stored = 17; { extern int stored; if (&stored != &::stored) return 1; stored = 7; } return stored == 17 && ::stored == 7 ? 0 : 2; }
