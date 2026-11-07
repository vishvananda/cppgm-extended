// N3485 9.2/14.6/14.7.1; GCC cpp0x/nsdmi-template3/4/10 reductions; expect exit 0
enum E { seven = 7 }; struct B { template<E N> struct A { const int x = N; }; }; int main() { B::A<seven> a; return a.x != 7; }
