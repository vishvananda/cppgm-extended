// expect exit 0
template<class T>auto probe(int)->decltype(T{1,2,3},char{});
template<class>long probe(...);
struct pair{int a,b;};struct triple{int a,b,c;};
static_assert(sizeof(probe<pair>(0))==sizeof(long),"return-type substitution");
static_assert(sizeof(probe<triple>(0))==sizeof(char),"return-type success");
int main(){return 0;}
