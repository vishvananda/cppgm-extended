// expect exit 0
void* operator new(unsigned long);
void operator delete(void*) noexcept;
template<class T>T&& declval();
template<class T,class U,class=decltype(::new T(declval<U>()))>char probe(int);
template<class,class>long probe(...);
static_assert(sizeof(probe<int[2],int>(0))==sizeof(long),"array with parenthesized operand");
static_assert(sizeof(probe<int,int>(0))==sizeof(char),"scalar initialization");
int main(){return 0;}
