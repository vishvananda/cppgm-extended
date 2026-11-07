// N3485 focus: 8.3.6p4 [dcl.fct.default], adopted CWG136.
// expect reject
template<class T> int f(T, int);
struct C { template<class T> friend int f(T, int = 1) { return 0; } };
int main() { return 0; }
