// expect exit 0
// 15.4p3: a recorded integral constant can make a specification known.
template<class T> struct X { static const bool b=true; void f() noexcept(b); };
template<class U> void X<U>::f() noexcept(true) {}
int main() { X<int> x; x.f(); }
