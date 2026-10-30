// expect exit 0
template<class T> struct detect {
 template<int(*)(int)> struct helper;
 template<class U> static char test(helper<&U::template fun<int>>*) { return U::missing; }
 template<class U> static long test(...) { return U::missing; }
 enum { value=sizeof(test<T>(0))==sizeof(char) };
};
struct present { template<class> static int fun(int); };
struct absent {};
static_assert(detect<present>::value,"unevaluated selected overload body");
static_assert(!detect<absent>::value,"unevaluated fallback body");
int main(){return 0;}
