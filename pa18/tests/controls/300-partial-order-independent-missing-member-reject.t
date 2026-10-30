// expect reject
template<bool B,class T=void>struct enable_if{};
template<class T>struct enable_if<true,T>{typedef T type;};
template<class T>struct predicate{static const bool value=true;};
template<class F,class=void>struct select;
struct special{static const bool value=true;};
struct disabled{static const bool value=false;};
struct missing{};
template<class R>struct select<R(special)>{enum{value=1};};
template<class R,class T>struct select<R(T),typename enable_if<T::value>::type>{enum{value=2};};
select<void(missing)> incomplete;
int main(){return 0;}
