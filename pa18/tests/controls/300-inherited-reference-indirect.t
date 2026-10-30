// expect exit 0
void* operator new(unsigned long);
void operator delete(void*) noexcept;
template<class T>T&& declval();
template<class T,class U,class=decltype(::new T(declval<U>()))>char test(int);
template<class,class>long test(...);
struct base{base(){}template<class U>base(U&&){}};
struct middle:base{using base::base;middle(const middle&)=delete;};
struct derived:middle{using middle::middle;};
static_assert(sizeof(test<derived,middle&>(0))==sizeof(long),"base copy is not inherited");
static_assert(sizeof(test<derived,derived&>(0))==sizeof(long),"derived copy");
static_assert(sizeof(test<derived,derived&&>(0))==sizeof(long),"deleted implicit move stays unavailable");
int main(){return 0;}
