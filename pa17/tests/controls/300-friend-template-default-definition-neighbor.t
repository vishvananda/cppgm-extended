// expect exit 0
// N3485 declaration grammar/category/access regression; GCC cpp0x gap reduction (GPL-3.0-or-later).
struct S { template<class T = int> friend int f(S) { return 3; } }; int main() { S s; return f(s) == 3 ? 0 : 1; }
