// expect exit 0
// A deduced concrete bound stays known while the element is a unique
// synthesized type during ordering. Fixed dimensions outrank generic
// dimensions, including the two crossing matrix patterns.
template<class T> struct traits;
template<class T,int N> struct traits<T[N]> { enum { value=1 }; };
template<class T,int N> struct traits<T[N][4]> { enum { value=2 }; };
template<class T,int N> struct traits<T[4][N]> { enum { value=3 }; };
template<class T> struct traits<T[4][4]> { enum { value=4 }; };
template<class T,int M,int N> struct traits<T[M][N]> { enum { value=5 }; };
template<class T> struct traits<T[4]> { enum { value=6 }; };
static_assert(traits<int[4]>::value==6,"fixed bound");
static_assert(traits<int const[4]>::value==6,"qualified element");
static_assert(traits<int[4][4]>::value==4,"both fixed dimensions");
static_assert(traits<int[4][4][4]>::value==4,"synthesized array element");
static_assert(traits<int[3][4]>::value==2,"fixed inner dimension");
static_assert(traits<int[4][3]>::value==3,"fixed outer dimension");
static_assert(traits<int[3][5]>::value==5,"both generic dimensions");
template<class T> struct sized;
template<class T,unsigned long N> struct sized<T[N]> { enum { value=1 }; };
template<class T> struct sized<T[8]> { enum { value=2 }; };
static_assert(sized<long[8]>::value==2,"size type bound");
static_assert(sized<long[9]>::value==1,"generic fallback");
int main(){return 0;}
