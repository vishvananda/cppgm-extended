// expect exit 0
template<bool B,class T=void>struct enable_if{};
template<class T>struct enable_if<true,T>{typedef T type;};
template<class T>struct predicate{static const bool value=true;};
template<class F,class=void>struct select{enum{value=0};};
struct special{static const bool value=true;};
struct disabled{static const bool value=false;};
struct missing{};
struct hidden{private:static const bool value=true;};
template<class R>struct select<R(special)>{enum{value=1};};
template<class R,class T>struct select<R(T),typename enable_if<T::value>::type>{enum{value=2};};
static_assert(select<void(special)>::value==1,"specific");
static_assert(select<void(disabled)>::value==0,"false predicate");
static_assert(select<void(missing)>::value==0,"invalid immediate operand");
static_assert(select<void(hidden)>::value==0,"private member remains inaccessible");
int main(){return 0;}
