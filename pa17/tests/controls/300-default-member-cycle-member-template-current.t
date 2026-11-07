// Adopted CWG1397: defaulted constructor definition/exception-specification cycles; expect reject
struct B { template<int N> struct X { int i = (X<N>(),0); }; }; B::X<1> x; int main(){ return 0; }
