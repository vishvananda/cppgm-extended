// expect exit 0
void* operator new(unsigned long);
void operator delete(void*) noexcept;
template<class T>T&& declval();
template<class T,class U,class=decltype(::new T(declval<U>()))>char test(int);
template<class,class>long test(...);
struct base{base(){}template<class U>base(U&&){}};
struct derived:base{using base::base;derived()=default;derived(const derived&)=delete;derived(derived&&)=delete;};
static_assert(sizeof(test<derived,derived&>(0))==sizeof(long),"mutable copy");
static_assert(sizeof(test<derived,derived&&>(0))==sizeof(long),"move");
static_assert(sizeof(test<derived,const derived&>(0))==sizeof(long),"const copy");
int main(){return 0;}
