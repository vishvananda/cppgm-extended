// expect reject
// N3485 declaration grammar/category/access regression; GCC cpp0x gap reduction (GPL-3.0-or-later).
struct S { public: enum class E : int; private: enum class E : int {}; };
