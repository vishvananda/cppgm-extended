// CWG1839/P1787R6: declarations target their namespace despite use-site hiding.
// expect exit 0
static int stored = 3;
int main() { extern int stored; { extern int stored; stored = 8; } return stored == 8 && &stored == &::stored ? 0 : 1; }
