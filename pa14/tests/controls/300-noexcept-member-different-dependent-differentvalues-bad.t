// expect reject
// N3485 15.4p3: dependent expression equivalence versus known nonthrowing values.
struct Tag{static const bool a=true,b=false;};template<class T>struct X{void f()noexcept(T::a);};template<class U>void X<U>::f()noexcept(U::b){};void use(){X<Tag>x;x.f();}
