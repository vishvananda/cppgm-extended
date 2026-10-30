// expect exit 0
typedef unsigned long Size;
template<class T,unsigned I>struct pick;
template<class T,unsigned I,Size N>struct pick<T(*)[N],I>{enum{value=1};};
template<class T,Size N>struct pick<T(*)[N],0>{enum{value=2};};
template<class T,unsigned I,Size N>struct pick<T(&)[N],I>{enum{value=3};};
template<class T,Size N>struct pick<T(&)[N],0>{enum{value=4};};
static_assert(pick<int(*)[5],0>::value==2,"pointer array bound");
static_assert(pick<const long(&)[6],0>::value==4,"reference array bound");
int main(){return pick<int(*)[5],1>::value!=1;}
