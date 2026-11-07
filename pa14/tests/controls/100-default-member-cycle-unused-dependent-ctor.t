// Adopted CWG1397: defaulted constructor definition/exception-specification cycles; expect exit 0
template<class T> struct X { int i = (T(),0); }; int main(){ return sizeof(X<int>) != sizeof(int); }
