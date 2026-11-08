// expect reject
// N3485 declaration grammar/category/access regression; GCC cpp0x gap reduction (GPL-3.0-or-later).
template<class T> struct S { template<class U> template<class V> using Alias = U; };
