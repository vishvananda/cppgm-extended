// expect reject
// N3485 declaration grammar/category/access regression; GCC cpp0x gap reduction (GPL-3.0-or-later).
struct S { private: enum E : int; }; enum S::E : int { value }; int main() { return S::value; }
