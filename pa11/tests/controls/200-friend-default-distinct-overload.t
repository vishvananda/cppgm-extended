// N3485 focus: 8.3.6p4 [dcl.fct.default], adopted CWG136.
// expect exit 0
struct C { friend int f(C*, int n = 7) { return n; } friend int f(C*, int, int); };
int f(C*, int a, int b) { return a + b; }
int main() { C c; return f(&c) == 7 && f(&c, 2, 3) == 5 ? 0 : 1; }
