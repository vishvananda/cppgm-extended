// expect exit 0
typedef unsigned long Size;
template<class T,unsigned I=0>struct extent{static const Size value=0;};
template<class T,unsigned I,Size N>struct extent<T[N],I>{static const Size value=extent<T,I-1>::value;};
template<class T,Size N>struct extent<T[N],0>{static const Size value=N;};
static_assert(extent<long[7]>::value==7,"reverse declarations");
int main(){return extent<long[4][7],1>::value!=7;}
