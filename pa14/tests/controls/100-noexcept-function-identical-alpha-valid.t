// expect exit 0
// N3485 15.4p3: dependent expression equivalence versus known nonthrowing values.
template<class T>void f()noexcept(sizeof(T)>1);template<class U>void f()noexcept(sizeof(U)>1){};void use(){f<int>();f<char>();}
int main() { use(); }
