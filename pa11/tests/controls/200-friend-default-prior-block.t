// N3485 focus: 8.3.6p4 [dcl.fct.default], adopted CWG136.
// expect reject
void before() { int f(int); }
struct C { friend int f(int = 1) { return 0; } };
int main() { return 0; }
