// N3485 focus: 8.3.6p4 [dcl.fct.default], 8.4.3; adopted CWG136.
// expect exit 0
struct C { friend void f(int = 1) = delete; };
int main() { return 0; }
