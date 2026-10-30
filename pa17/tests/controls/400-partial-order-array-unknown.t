// expect exit 0
template<class T,unsigned I=0>struct extent{enum{value=9};};
template<class T>struct extent<T[],0>{enum{value=0};};
template<class T,unsigned I>struct extent<T[],I>{enum{value=1};};
static_assert(extent<int[]>::value==0,"omitted bound remains omitted");
static_assert(extent<int[],1>::value==1,"omitted bound generic rank");
int main(){return 0;}
