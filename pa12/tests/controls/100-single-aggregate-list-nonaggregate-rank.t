// CWG1467/2137; reduced from GCC cpp0x aggregate list-initialization cases; expect exit 0
struct E { E() {} }; struct W { W(E); }; int f(E) { return 0; } int f(W) { return 1; } int main() { E a; return f({a}); }
