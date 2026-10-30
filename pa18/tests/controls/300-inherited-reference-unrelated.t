// expect exit 0
void* operator new(unsigned long);
void operator delete(void*) noexcept;
template<class T>T&& declval();
template<class T,class U,class=decltype(::new T(declval<U>()))>char test(int);
template<class,class>long test(...);
struct base{int value;base():value(0){}template<class U>base(U&&):value(77){}};
struct derived:base{using base::base;};struct sibling:base{};
static_assert(sizeof(test<derived,sibling&>(0))==sizeof(char),"unrelated to constructed class");
int main(){sibling s;derived d(s);return d.value==77?0:1;}
