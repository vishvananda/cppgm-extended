// CWG1839/P1787R6: declarations target their namespace despite use-site hiding.
// expect reject
static int stored = 3;
int main() { int stored = 17; { extern double stored; } return 0; }
