// expect exit 0
// N3485 15.4p3: dependent expression equivalence versus known nonthrowing values.
template<class T>struct X{void f()noexcept;};template<class U>void X<U>::f()noexcept(2>1){};void use(){X<int>x;x.f();}
int main() { use(); }
