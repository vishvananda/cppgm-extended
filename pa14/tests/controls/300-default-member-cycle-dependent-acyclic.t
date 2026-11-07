// Adopted CWG1397: defaulted constructor definition/exception-specification cycles; expect exit 0
template<class T> struct X { X* p = new typename T::type; X()=default; X(int) : p(nullptr) {} }; struct Good; struct D : X<Good> { D() : X<Good>(0) {} }; struct Good { using type=D; }; X<Good> x; int main(){ return x.p == nullptr || x.p->p != nullptr; }
