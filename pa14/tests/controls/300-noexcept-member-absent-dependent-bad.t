// expect reject
// N3485 15.4p3: dependent expression equivalence versus known nonthrowing values.
template<class T>struct X{void f();};template<class U>void X<U>::f()noexcept(sizeof(U)>0){};void use(){X<int>x;x.f();}
