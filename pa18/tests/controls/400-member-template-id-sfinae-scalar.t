// expect exit 0
template<class T> struct detect {
 template<int(*)(int)> struct helper;
 template<class U> static char test(helper<&U::template fun<int>>*);
 template<class U> static long test(...);
 enum { value=sizeof(test<T>(0))==sizeof(char) };
};
static_assert(!detect<int>::value,"not a class");
int main(){return 0;}
