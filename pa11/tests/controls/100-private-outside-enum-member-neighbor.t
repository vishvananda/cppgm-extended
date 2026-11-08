// expect exit 0
// N3485 declaration grammar/category/access regression; GCC cpp0x gap reduction (GPL-3.0-or-later).
struct S { private: enum E : int; public: static int get(); }; enum S::E : int { value = 3 }; int S::get() { return value; } int main() { return S::get() == 3 ? 0 : 1; }
