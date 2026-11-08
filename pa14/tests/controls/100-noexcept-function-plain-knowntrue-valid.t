// expect exit 0
// N3485 15.4p3: dependent expression equivalence versus known nonthrowing values.
template<class T>void f()noexcept;template<class U>void f()noexcept(2>1){};void use(){f<int>();}
int main() { use(); }
