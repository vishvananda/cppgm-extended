// expect exit 0
// N3485 9.5p2: static members are not union variants.
int n = 7; union U { static int& alias; int* pointer; }; int& U::alias = n;
struct S { int& reference; };
int main() { U u; u.pointer = &n; S s = {n}; return *u.pointer == 7 && s.reference == U::alias ? 0 : 1; }
