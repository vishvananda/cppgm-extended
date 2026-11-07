// N3485 focus: 8.3.6p4 [dcl.fct.default], adopted CWG136.
// expect exit 0
template<class T> struct C { friend int f(C*, int n = 7) { return sizeof(T) + n; } };
int main() { C<int> i; C<char> c; return f(&i) == sizeof(int) + 7 && f(&c, 2) == 3 && f(&i, 1) == sizeof(int) + 1 ? 0 : 1; }
