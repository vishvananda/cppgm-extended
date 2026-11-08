// expect reject
// N3485 14.5.3p5; reduced GCC pr32114.C (GPL-3.0-or-later).
struct Element { typedef int X; };
template<class... T> struct A { typedef typename T::X Y; }; A<Element> a;
