// expect reject
// N3485 declaration grammar/category/access regression; GCC cpp0x gap reduction (GPL-3.0-or-later).
template<class T> template<class U> using Alias = T;
