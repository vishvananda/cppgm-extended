// N3485 9.2/14.6/14.7.1; GCC cpp0x/nsdmi-template3/4/10 reductions; expect exit 0
struct B { int x = 37; struct A { int x = 41; }; }; int main() { B b; B::A a; return b.x != 37 || a.x != 41; }
