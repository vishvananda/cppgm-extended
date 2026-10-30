// expect exit 0
template<class T>T&& declval();
template<class T,class=decltype(T{declval<int&>(),2,3})>char probe(int);
template<class>long probe(...);
struct pair{int& a;int b;};struct triple{int& a;int b,c;};
static_assert(sizeof(probe<pair>(0))==sizeof(long),"excess after reference member");
static_assert(sizeof(probe<triple>(0))==sizeof(char),"valid reference member");
int main(){return 0;}
