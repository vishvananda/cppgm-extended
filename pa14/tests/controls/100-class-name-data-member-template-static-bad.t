// expect reject
// N3485 9p2, 9.2p14-p15, 14p5: injected class names and member names.
template<class T> struct A { static int A; };
A<int> a;
