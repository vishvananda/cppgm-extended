// expect reject
struct base{base(){}template<class U>base(U&&){}};
struct derived:base{using base::base;derived()=default;derived(const derived&)=delete;derived(derived&&)=delete;};
int main(){derived a;derived b(a);return 0;}
