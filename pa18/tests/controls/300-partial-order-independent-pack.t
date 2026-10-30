// expect exit 0
template<bool B,class T=void>struct enable_if{};
template<class T>struct enable_if<true,T>{typedef T type;};
template<class T>struct predicate{static const bool value=true;};
struct special{};
template<class...T>struct tags{};
template<class Tags,class F,class=void>struct select;
template<class R,class...A>struct select<tags<A...>,R(special)>{enum{value=1};};
template<class R,class T,class...A>struct select<tags<A...>,R(T),typename enable_if<predicate<T>::value>::type>{enum{value=2};};
static_assert(select<tags<int,long>,void(special)>::value==1,"specific");
int main(){return 0;}
