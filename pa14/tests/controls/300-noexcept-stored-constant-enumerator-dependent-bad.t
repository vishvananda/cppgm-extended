// expect reject
// A recorded enumerator is fixed only when its value is independent.
template<class T> struct X { enum { b=sizeof(T)>1 }; void f() noexcept(b); };
template<class T> void X<T>::f() noexcept(b && true) {}
int main() { X<int> x; x.f(); }
