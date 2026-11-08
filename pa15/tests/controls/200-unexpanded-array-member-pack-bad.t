// expect reject
// N3485 14.5.3p5; reduced GCC vt-35026.C (GPL-3.0-or-later).
template<class... T> struct A { T* values[1]; }; A<int> a;
