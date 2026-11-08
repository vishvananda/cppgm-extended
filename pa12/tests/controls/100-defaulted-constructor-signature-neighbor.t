// expect exit 0
// N3485 declaration grammar/category/access regression; GCC cpp0x gap reduction (GPL-3.0-or-later).
struct S { int n; S() = default; S(const S&) = default; }; int main() { S a = {}; a.n = 3; S b(a); return b.n == 3 ? 0 : 1; }
