// expect exit 0
// 15.4p3 and CWG1330: demanded constants retain declaration lookup and access.
template<class T> struct X {static const bool a=(2>1);static const bool b=a;void f()noexcept(b);}; template<class U>void X<U>::f()noexcept(true){} int main(){X<int>x;x.f();}
