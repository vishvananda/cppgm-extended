// N3485 9.2/14.6/14.7.1; GCC cpp0x/nsdmi-template3/4/10 reductions; expect exit 0
struct V { enum { value = 19 }; }; struct B { template<class T> struct A { int x = T::value; }; }; int main() { B::A<V> a; return a.x != 19; }
