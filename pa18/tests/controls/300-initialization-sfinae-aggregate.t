// expect exit 0
template<class T,class=decltype(T{1,2,3})>char probe(int);
template<class>long probe(...);
struct pair{int a,b;};struct triple{int a,b,c;};
static_assert(sizeof(probe<pair>(0))==sizeof(long),"too many aggregate elements");
static_assert(sizeof(probe<triple>(0))==sizeof(char),"matching elements");
int main(){return 0;}
