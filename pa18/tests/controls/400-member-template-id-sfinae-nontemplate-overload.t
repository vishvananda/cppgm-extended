// expect exit 0
template<class T> struct detect {
 template<int(*)(int)> struct helper;
 template<class U> static char test(helper<&U::template fun<int>>*);
 template<class U> static long test(...);
 enum { value=sizeof(test<T>(0))==sizeof(char) };
};
struct ordinary { static int fun(int); };
struct wrong_argument { template<int> static int fun(int); };
struct wrong_return { template<class> static long fun(int); };
static_assert(!detect<ordinary>::value,"not a template");
static_assert(!detect<wrong_argument>::value,"wrong template argument");
static_assert(!detect<wrong_return>::value,"wrong function type");
int main(){return 0;}
