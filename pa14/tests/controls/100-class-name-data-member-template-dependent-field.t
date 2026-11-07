// expect exit 0
// N3485 9p2, 9.2p14-p15, 14p5: injected class names and member names.
template<class T> struct A { T A; };
int main() { A<int> a; a.A = 7; return a.A == 7 ? 0 : 1; }
