// expect reject
// 15.4p3 and CWG1330: demanded constants retain declaration lookup and access.
template<class>struct X;struct Key{private:static constexpr bool yes(){return true;}friend struct X<int>;};template<class T>struct X{static const bool b=Key::yes();void f()noexcept(b);};template<class U>void X<U>::f()noexcept(true){}int main(){X<char>x;x.f();}
