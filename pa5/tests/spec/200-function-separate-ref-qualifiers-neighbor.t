// expect exit 0
// N3485 declaration grammar/category/access regression; GCC cpp0x gap reduction (GPL-3.0-or-later).
struct S { int f() &; int f() &&; int g() const &; }; int main() { return 0; }
