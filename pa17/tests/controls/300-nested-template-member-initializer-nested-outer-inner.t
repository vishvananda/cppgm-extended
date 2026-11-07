// N3485 9.2/14.6/14.7.1; GCC cpp0x/nsdmi-template3/4/10 reductions; expect exit 0
template<int M> struct B { template<int N> struct A { int x = N; int y = M; }; }; int main() { B<13>::A<17> a; return a.x != 17 || a.y != 13; }
