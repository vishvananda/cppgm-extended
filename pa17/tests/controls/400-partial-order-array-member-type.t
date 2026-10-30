// expect exit 0
typedef unsigned long Size;
template<class T,class U>struct pick;
template<class T,class U,Size N>struct pick<T,U[N]>{enum{value=1};};
template<class T,Size N>struct pick<T,typename T::type[N]>{enum{value=2};};
struct base{typedef int type;};
static_assert(pick<base,int[3]>::value==2, "");
static_assert(pick<base,long[3]>::value==1, "");
int main(){return 0;}
