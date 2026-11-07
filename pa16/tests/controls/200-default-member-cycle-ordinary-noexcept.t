// Adopted CWG1397: defaulted constructor definition/exception-specification cycles; expect reject
struct X { int i = noexcept(X()); }; X x; int main(){ return 0; }
