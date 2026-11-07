// Adopted CWG1397: defaulted constructor definition/exception-specification cycles; expect exit 0
struct X { X* p = new X; X() : p(nullptr) {} }; int main(){ X x; return x.p != nullptr; }
