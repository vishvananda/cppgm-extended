// Adopted CWG1397: defaulted constructor definition/exception-specification cycles; expect reject
struct X { X* p = new X; }; X x; int main(){ return 0; }
