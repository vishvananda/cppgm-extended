template<class,class>struct same {static const bool value=false;};template<class T>struct same<T,T>{static const bool value=true;};
struct A {int x; void f(){auto l=[*this] {static_assert(same<decltype(this),const A*>::value,"this pointer");static_assert(same<decltype((x)),const int&>::value,"member");};l();}};
int main(){A a{1};a.f();}
