// expect exit 0
// N3485 9p2, 9.2p14-p15, 14p5: injected class names and member names.
struct A { int A = 0; };
int main() { A a; return a.A; }
