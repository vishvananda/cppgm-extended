template<class T>struct Identity{typedef T type;};
template<class T>struct Pack{typedef int type;};
template<class>struct Ret;
template<class T>struct Ret<void(*)(T)>{typedef Identity<T> type;};
struct A{template<class>using Result=typename Ret<void(*)(int)>::type; Result<int>::type operator()(){return 42;}};int main(){A a;return a()!=42;}
