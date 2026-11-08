// expect exit 0
// A dependent enum operand is valid when both specification patterns agree.
template<class T> struct X { enum { b=1 }; void f() noexcept(b); };
template<class T> void X<T>::f() noexcept(b) {}
int main() { X<int> x; x.f(); }
