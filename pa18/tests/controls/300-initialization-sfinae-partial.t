// expect exit 0
template<class...>using void_t=void;
template<class T,class=void>struct accepts{static constexpr bool value=false;};
template<class T>struct accepts<T,void_t<decltype(T{1,2,3})>>{static constexpr bool value=true;};
struct pair{int a,b;};struct triple{int a,b,c;};
static_assert(!accepts<pair>::value,"partial-specialization substitution");
static_assert(accepts<triple>::value,"matching partial specialization");
int main(){return 0;}
