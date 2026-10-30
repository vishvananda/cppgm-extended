// expect exit 0
typedef unsigned long Size;
template<class T,class U>struct same;
template<class T,Size N,Size M>struct same<T[N],T[M]>{enum{value=1};};
template<class T,Size N>struct same<T[N],T[N]>{enum{value=2};};
static_assert(same<int[3],int[3]>::value==2,"same synthetic bound");
static_assert(same<int[3],int[4]>::value==1,"different synthetic bounds");
int main(){return 0;}
