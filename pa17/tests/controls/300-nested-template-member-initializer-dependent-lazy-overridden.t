// N3485 9.2/14.6/14.7.1; GCC cpp0x/nsdmi-template3/4/10 reductions; expect exit 0
struct B { template<class T> struct A { int x = T::missing; A() : x(23) {} }; }; int main() { B::A<int> a; return a.x != 23; }
