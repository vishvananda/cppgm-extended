// expect reject
// Its value is 1, but the class-member enum TYPE is still dependent.
template<class T> struct X { enum { b=1 }; void f() noexcept(b); };
template<class T> void X<T>::f() noexcept(true) {}
int main() { X<int> x; x.f(); }
