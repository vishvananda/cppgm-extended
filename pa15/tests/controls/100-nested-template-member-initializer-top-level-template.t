// N3485 9.2/14.6/14.7.1; GCC cpp0x/nsdmi-template3/4/10 reductions; expect exit 0
template<int N> struct A { int x = N; }; int main() { A<31> a; return a.x != 31; }
