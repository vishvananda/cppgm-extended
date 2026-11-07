// N3485 9.2/14.6/14.7.1; GCC cpp0x/nsdmi-template3/4/10 reductions; expect exit 0
struct B { template<int N,int M> struct A; template<int M> struct A<1,M> { int x = M; }; }; int main() { return 0; }
