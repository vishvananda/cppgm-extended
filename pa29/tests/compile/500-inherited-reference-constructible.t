struct base{base(){}template<class U>base(U&&){}};
struct derived:base{using base::base;derived()=default;derived(const derived&)=delete;derived(derived&&)=delete;};
static_assert(!__is_constructible(derived,derived&),"mutable");
static_assert(!__is_constructible(derived,derived&&),"move");
static_assert(!__is_constructible(derived,const derived&),"const copy");
static_assert(__is_constructible(derived,int),"ordinary argument");
int main(){return 0;}
