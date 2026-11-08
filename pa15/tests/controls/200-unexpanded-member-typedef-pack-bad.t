// expect reject
// N3485 14.5.3p5; reduced GCC pr34058.C (GPL-3.0-or-later).
template<class... T> struct A { typedef T X; }; A<int> a;
