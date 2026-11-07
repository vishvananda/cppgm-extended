// Adopted CWG1397: defaulted constructor definition/exception-specification cycles; expect exit 0
struct X { int i = (X(),0); X() : i(7) {} }; int main(){ X x; return x.i != 7; }
