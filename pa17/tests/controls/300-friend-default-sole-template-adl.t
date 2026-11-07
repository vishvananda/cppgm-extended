// N3485 focus: 8.3.6p4 [dcl.fct.default], adopted CWG136.
// expect exit 0
struct C { template<class T> friend int f(C*, T n, int extra = 7) { return n + extra; } };
int main() { C c; return f(&c, 2) == 9 && f(&c, 2, 3) == 5 ? 0 : 1; }
