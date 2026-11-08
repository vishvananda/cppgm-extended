// expect exit 0
// N3485 declaration grammar/category/access regression; GCC cpp0x gap reduction (GPL-3.0-or-later).
struct S { int n; S(int v) : n(v) {} }; int main() { int a[2] = {1,2}; int sum = 0; for (S value : a) sum += value.n; return sum == 3 ? 0 : 1; }
