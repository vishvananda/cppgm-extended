// expect exit 0
typedef unsigned long Size;
template<class T,unsigned I=0>struct extent{static const Size value=0;};
template<class T,Size N>struct extent<T[N],0>{static const Size value=N;};
template<class T,unsigned I,Size N>struct extent<T[N],I>{static const Size value=extent<T,I-1>::value;};
static_assert(extent<int[3]>::value==3,"bounded array zero rank");
static_assert(extent<const int[2][3],1>::value==3,"nested qualified arrays");
int main(){return extent<int[2][3],0>::value!=2;}
