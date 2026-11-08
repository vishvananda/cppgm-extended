// expect reject
// N3485 declaration grammar/category/access regression; GCC cpp0x gap reduction (GPL-3.0-or-later).
struct A { template<class T> void f(); }; struct B { template<class T> friend void A::f() = delete; };
