// expect exit 0
template<class T,class=decltype(T())>char probe(int);
template<class>long probe(...);
using function=int();
static_assert(sizeof(probe<function>(0))==sizeof(long),"function cannot be value-initialized");
static_assert(sizeof(probe<int>(0))==sizeof(char),"scalar");
static_assert(sizeof(probe<void>(0))==sizeof(char),"void expression");
int main(){return 0;}
