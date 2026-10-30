// expect exit 0
void* operator new(unsigned long);
void operator delete(void*) noexcept;
template<class T>T&& declval();
template<class T,class U,class=decltype(::new T(declval<U>()))>char probe(int);
template<class,class>long probe(...);
struct element{element(){}element(const element&){};};
static_assert(sizeof(probe<element[2],element>(0))==sizeof(long),"array with parenthesized operand");
static_assert(sizeof(probe<element,element>(0))==sizeof(char),"class copy initialization");
int main(){return 0;}
