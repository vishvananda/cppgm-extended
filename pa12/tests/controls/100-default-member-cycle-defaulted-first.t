// Adopted CWG1397: defaulted constructor definition/exception-specification cycles; expect reject
struct X { X() = default; int i = (X(),0); }; X x; int main(){ return 0; }
