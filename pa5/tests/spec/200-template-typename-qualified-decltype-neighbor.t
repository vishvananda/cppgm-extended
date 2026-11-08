// expect exit 0
// N3485 declaration grammar/category/access regression; GCC cpp0x gap reduction (GPL-3.0-or-later).
struct A { typedef int type; }; A value; template<typename decltype(value)::type N> struct S {}; int main() { return 0; }
