// N3485 focus: 8.3.6p4 [dcl.fct.default], adopted CWG136.
// expect exit 0
int f(int n = 7);
struct C { friend int f(int); };
int f(int n) { return n; }
int main() { return f() == 7 ? 0 : 1; }
