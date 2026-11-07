// N3485 9.2/14.6/14.7.1; GCC cpp0x/nsdmi-template3/4/10 reductions; expect reject
struct B { struct A { int x = missing; }; }; int main() { return 0; }
