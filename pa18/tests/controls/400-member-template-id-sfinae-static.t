// expect exit 0
template<class T> struct detect {
 template<int(*)(int)> struct helper;
 template<class U> static char test(helper<&U::template fun<int>>*);
 template<class U> static long test(...);
 enum { value=sizeof(test<T>(0))==sizeof(char) };
};
struct present { template<class T> static int fun(int); };
struct absent {};
static_assert(detect<present>::value,"present");
static_assert(!detect<absent>::value,"absent");
int main(){return 0;}
