// expect exit 0
// 15.4p3 and CWG1330: demanded constants retain declaration lookup and access.
constexpr bool choose(long){return true;} template<class T> struct X {static const bool b=choose(0);void f()noexcept(b);}; constexpr bool choose(int){return false;} template<class T> void X<T>::f()noexcept(true){} int main(){X<int>x;x.f();}
