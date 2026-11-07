// Adopted CWG1397: defaulted constructor definition/exception-specification cycles; expect reject
template<int N> struct X { int i = (X<N>(),0); }; X<1> x; int main(){ return 0; }
