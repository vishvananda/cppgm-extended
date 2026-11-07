// Adopted CWG1397: defaulted constructor definition/exception-specification cycles; expect exit 0
struct X { int i = (X(),0); X() {} }; int main(){ return 0; }
