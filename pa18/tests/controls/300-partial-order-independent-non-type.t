// expect exit 0
template<bool B,class T=void>struct enable_if{};
template<class T>struct enable_if<true,T>{typedef T type;};
template<class T>struct predicate{static const bool value=true;};
struct special{};
template<int N,class F,class=void>struct select;
template<int N,class R>struct select<N,R(special)>{enum{value=1};};
template<int N,class R,class T>struct select<N,R(T),typename enable_if<predicate<T>::value>::type>{enum{value=2};};
static_assert(select<7,void(special)>::value==1,"specific");
int main(){return 0;}
