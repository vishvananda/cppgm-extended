// expect exit 0
struct base{int value;template<class U>base(U&&,int):value(99){}};
struct derived:base{using base::base;derived(const derived&)=delete;};
int main(){derived a(7,0);derived b(a,0);return b.value==99?0:1;}
