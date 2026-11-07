// N3485 focus: 8.3.6p4 [dcl.fct.default], adopted CWG136.
// expect exit 0
struct C { friend int f(C*, int n = 7) { return n; } };
int main() { C c; return f(&c) == 7 && f(&c, 9) == 9 ? 0 : 1; }
