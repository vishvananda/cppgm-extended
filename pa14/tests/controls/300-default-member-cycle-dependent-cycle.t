// Adopted CWG1397: defaulted constructor definition/exception-specification cycles; expect reject
template<class T> struct X { X* p = new typename T::type; X()=default; X(int) : p(nullptr) {} }; struct Bad { using type=X<Bad>; }; X<Bad> x; int main(){ return 0; }
