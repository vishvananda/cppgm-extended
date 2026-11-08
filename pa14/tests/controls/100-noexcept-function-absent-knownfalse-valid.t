// expect exit 0
// N3485 15.4p3: dependent expression equivalence versus known nonthrowing values.
template<class T>void f();template<class U>void f()noexcept(false){};void use(){f<int>();}
int main() { use(); }
