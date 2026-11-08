// expect reject
// N3485 15.4p3: dependent expression equivalence versus known nonthrowing values.
template<class T>struct X{void f()noexcept(sizeof(T)>1);};template<class U>void X<U>::f()noexcept((sizeof(U)>1)&&true){};void use(){X<int>x;x.f();X<char>y;y.f();}
