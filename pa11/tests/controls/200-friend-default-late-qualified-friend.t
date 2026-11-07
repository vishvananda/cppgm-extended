// N3485 focus: 8.3.6p4 [dcl.fct.default], adopted CWG136.
// expect reject
struct C { friend int f(C*, int = 1) { return 0; } };
struct D { friend int ::f(C*, int); };
int main() { return 0; }
