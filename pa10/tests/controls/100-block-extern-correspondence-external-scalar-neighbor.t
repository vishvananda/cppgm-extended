// CWG1839/P1787R6: declarations target their namespace despite use-site hiding.
// expect exit 0
namespace other { int stored = 40; }
int stored = 3;
int main() { int stored = 17; { extern int stored; stored = 9; } return stored == 17 && ::stored == 9 && other::stored == 40 ? 0 : 1; }
