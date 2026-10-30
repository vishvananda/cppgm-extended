// expect exit 0
template<class T> struct detect {
 template<int(*)(int)> struct helper;
 template<class U> static char test(helper<&U::template fun<int>>*);
 template<class U> static long test(...);
 enum { value=sizeof(test<T>(0))==sizeof(char) };
};
struct variable { static int fun; };
struct alias { typedef int fun; };
struct nested { struct fun {}; };
static_assert(!detect<variable>::value,"variable");
static_assert(!detect<alias>::value,"alias");
static_assert(!detect<nested>::value,"class");
int main(){return 0;}
