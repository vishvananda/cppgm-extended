// expect exit 0
// 15.4p3 and CWG1330: demanded constants retain declaration lookup and access.
template<class T> struct X { static const bool b=(1==1); void f() noexcept(b); }; template<class T> void X<T>::f() noexcept(true){} int main(){X<int> x;x.f();}
