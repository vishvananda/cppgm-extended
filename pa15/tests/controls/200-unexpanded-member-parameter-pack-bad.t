// expect reject
// N3485 14.5.3p5; reduced GCC pr31437.C (GPL-3.0-or-later).
template<class... T> struct A { A(T* p) {} }; A<int> a(0);
