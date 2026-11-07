// CWG1467/2137; reduced from GCC cpp0x aggregate list-initialization cases; expect exit 0
int moves; struct M { int x; M(int n) : x(n) {} M(M&& m) : x(m.x) { m.x = 0; ++moves; } }; struct E { M m; }; int main() { E a = {37}; E b{static_cast<E&&>(a)}; return b.m.x != 37 || a.m.x != 0 || moves != 1; }
