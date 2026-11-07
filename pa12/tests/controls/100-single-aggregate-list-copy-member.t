// CWG1467/2137; reduced from GCC cpp0x aggregate list-initialization cases; expect exit 0
int copies; struct M { int x; M(int n) : x(n) {} M(const M& m) : x(m.x) { ++copies; } }; struct E { M m; }; int main() { E a = {31}; E b{a}; return b.m.x != 31 || copies != 1; }
