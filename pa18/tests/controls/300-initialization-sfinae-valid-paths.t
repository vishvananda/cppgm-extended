// expect exit 0
void* operator new(unsigned long);
void operator delete(void*) noexcept;
template<class T>T&& declval();
template<class T,class U,class=decltype(::new T(declval<U>()))>char probe(int);
template<class,class>long probe(...);
template<class T,class=decltype(T())>char value_probe(int);
template<class>long value_probe(...);
template<class T,class=decltype(T{1,2})>char list_probe(int);
template<class>long list_probe(...);
struct pair{int a,b;};
static_assert(sizeof(value_probe<pair>(0))==sizeof(char),"aggregate value-init");
static_assert(sizeof(list_probe<pair>(0))==sizeof(char),"aggregate list-init");
static_assert(sizeof(probe<int,int>(0))==sizeof(char),"scalar new");
using array=int[2];
using allocated=decltype(::new array());
static_assert(sizeof(allocated)==sizeof(int*),"array value-init stays valid");
int main(){return 0;}
